#include "sandbox.h"

// TEMP
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

SandboxApplication::SandboxApplication(ApplicationConfiguration* configuration) : Application(configuration)
{
	m_cursor_mode = GLFW_CURSOR_NORMAL;

	m_meshes = 0;
}

SandboxApplication::~SandboxApplication()
{
}

void SandboxApplication::Init()
{
	EventDispatcher::AddListener(EventType_KeyPressed, HE_BIND_EVENTCALLBACK(OnKeyPressed));
	EventDispatcher::AddListener(EventType_WindowClose, HE_BIND_EVENTCALLBACK(OnWindowClose));
	EventDispatcher::AddListener(EventType_WindowResize, HE_BIND_EVENTCALLBACK(OnWindowResize));
	EventDispatcher::AddListener(EventType_WindowIconified, HE_BIND_EVENTCALLBACK(OnWindowMinimised));

	m_texture = m_frontend->GetTexture2D(DEFAULT_WHITE_TEXTURE);
	m_test_texture = m_backend->CreateTexture2D(FileManager::ReadFile("assets/textures/wall.jpg"));

	SetCursorMode(m_cursor_mode);

	m_camera = PerspectiveCamera();
	m_camera.Create(45.0f, (f32)m_window->GetWidth() / (f32)m_window->GetHeight(), 0.001f, 1000.0f);

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_controller = PerspectiveController();
	m_controller.Init();
	m_controller.SetCamera(&m_camera);

	SetMeshData();
	CreateCubeMesh("cube0");

	CreatePipeline();
	CreateDescriptorSet();
}

void SandboxApplication::OnProcessUpdate(f32 delta_time)
{
	if (m_cursor_mode == GLFW_CURSOR_DISABLED)
	{
		m_controller.OnProcessUpdate(delta_time);
		m_camera_data.view = m_camera.GetView();
	}
}

void SandboxApplication::OnRenderBegin()
{
	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));

	for (u32 i = 0; i < m_meshes; i++)
	{
		m_backend->UpdateUniformBuffer(m_mesh_buffers[i], &m_mesh_data[i], sizeof(ObjectData));
	}

	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void SandboxApplication::OnRenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->BindPipeline(m_pipeline);

	m_backend->BindDescriptorSet(m_pipeline, m_camera_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_material_descriptor);

	for (u32 i = 0; i < m_mesh_descriptors.size(); i++)
	{
		m_backend->BindDescriptorSet(m_pipeline, m_mesh_descriptors[i]);
		m_frontend->DrawMesh("cube" + std::to_string(i));
	}

	m_backend->EndDynamicRendering();
}

void SandboxApplication::OnRenderEnd()
{
}

void SandboxApplication::OnUIRender()
{
	m_ui->ShowDemoWindow();
}

void SandboxApplication::Shutdown()
{
	m_backend->DestroyUniformBuffer(m_camera_buffer);

	for (u32 i = 0; i < m_mesh_buffers.size(); i++)
	{
		m_backend->DestroyUniformBuffer(m_mesh_buffers[i]);
	}

	m_backend->DestroyTexture(m_test_texture);

	m_backend->DestroyPipeline(m_pipeline);
}

b8 SandboxApplication::OnWindowClose(EventContext& event)
{
	m_running = false;

	return true;
}

b8 SandboxApplication::OnWindowResize(EventContext& event)
{
	if (event.data.window_resize.width == 0 || event.data.window_resize.height == 0)
	{
		return false;
	}
	m_window->SetSize(event.data.window_resize.width, event.data.window_resize.height);

	m_camera.SetAspect((f32)event.data.window_resize.width, (f32)event.data.window_resize.height);
	m_camera_data.projection = m_camera.GetProjection();

	m_backend->OnFramebufferResize();

	return false;
}

b8 SandboxApplication::OnWindowMinimised(EventContext& event)
{
	m_suspended = event.data.window_iconified.is_iconified;

	return false;
}

b8 SandboxApplication::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_F1)
	{
		if (m_cursor_mode == GLFW_CURSOR_DISABLED)
		{
			m_cursor_mode = GLFW_CURSOR_NORMAL;
		}
		else
		{
			m_cursor_mode = GLFW_CURSOR_DISABLED;
		}

		SetCursorMode(m_cursor_mode);
	}

	if (event.data.key_event.key == KEY_ESCAPE)
	{
		m_running = false;
	}

	if (event.data.key_event.key == KEY_SPACE)
	{
		CreateCubeDescriptorSet("cube" + std::to_string(m_meshes));
	}

	return false;
}

void SandboxApplication::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};
	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = "assets/shaders/vertex.vert";
	shader_info.sources[ShaderType_Fragment] = "assets/shaders/vertex.frag";

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void SandboxApplication::CreateDescriptorSet()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Model descriptor
	{
		for (u32 i = 0; i < m_meshes; i++)
		{
			auto data = ObjectData();
			data.model = glm::mat4(1.0f);
			data.model = glm::translate(data.model, glm::vec3(0.0f, 0.0f, i * -2.0f));
			m_mesh_data.push_back(data);

			m_mesh_descriptors.push_back(m_backend->CreateDescriptorSet(m_pipeline, 1));
			m_mesh_buffers.push_back(m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData)));

			DescriptorSetWriteData descriptor_data;
			descriptor_data.type = DescriptorType_UniformBuffer;
			descriptor_data.binding = 0;
			descriptor_data.data.buffer.buffers = m_mesh_buffers[i]->GetHandle();
			descriptor_data.data.buffer.offsets = 0;
			descriptor_data.data.buffer.ranges = sizeof(ObjectData);

			std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
			m_backend->WriteDescriptor(&m_mesh_descriptors[i], descriptor_data_objects);
		}
	}

	// Material descriptor
	{
		m_material_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 2);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_views = m_test_texture->GetImageView();
		descriptor_data.data.image.samplers = m_test_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_material = { descriptor_data };
		m_backend->WriteDescriptor(&m_material_descriptor, descriptor_data_material);
	}
}

void SandboxApplication::SetMeshData()
{
	m_vertices = {
		// Front face
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 0
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 1
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 2
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 3

		// Back face
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 4
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 5
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 6
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 7

		// Top face
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 8
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 9
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 10
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 11

		// Bottom face
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 12
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 13
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 14
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 15
	};

	m_indices = {
		// Front face
		0, 1, 2, 0, 2, 3,

		// Back face
		4, 5, 6, 4, 6, 7,

		// Left face
		4, 0, 3, 4, 3, 7,

		// Right face
		1, 5, 6, 1, 6, 2,

		// Top face
		8, 10, 9, 8, 10, 11,

		// Bottom face
		12, 13, 14, 12, 14, 15
	};
}

void SandboxApplication::CreateCubeMesh(std::string name)
{
	if (m_frontend->CreateMesh(name, m_vertices, m_indices))
	{
		m_meshes++;
	}
}

void SandboxApplication::CreateCubeDescriptorSet(std::string name)
{
	if (m_frontend->CreateMesh(name, m_vertices, m_indices))
	{
		m_meshes++;
	}
	else
	{
		return;
	}

	auto data = ObjectData();
	data.model = glm::mat4(1.0f);
	data.model = glm::translate(data.model, glm::vec3(0.0f, 0.0f, (m_meshes - 1) * -2.0f));
	m_mesh_data.push_back(data);

	m_mesh_descriptors.push_back(m_backend->CreateDescriptorSet(m_pipeline, 1));
	m_mesh_buffers.push_back(m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData)));

	DescriptorSetWriteData descriptor_data;
	descriptor_data.type = DescriptorType_UniformBuffer;
	descriptor_data.binding = 0;
	descriptor_data.data.buffer.buffers = m_mesh_buffers[m_meshes - 1]->GetHandle();
	descriptor_data.data.buffer.offsets = 0;
	descriptor_data.data.buffer.ranges = sizeof(ObjectData);

	std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
	m_backend->WriteDescriptor(&m_mesh_descriptors[m_meshes - 1], descriptor_data_objects);
}
