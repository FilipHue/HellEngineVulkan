#include "sandbox.h"

SandboxApplication::SandboxApplication(ApplicationConfiguration* configuration) : Application(configuration)
{
	m_cursor_mode = GLFW_CURSOR_NORMAL;
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

	SetCursorMode(m_cursor_mode);

	m_camera = PerspectiveCamera();
	m_camera.Create(45.0f, (f32)m_window->GetWidth() / (f32)m_window->GetHeight(), 0.001f, 1000.0f);

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_controller = PerspectiveController();
	m_controller.Init();
	m_controller.SetCamera(&m_camera);

	m_texture1 = m_backend->CreateTexture2D("assets/textures/statue.jpg");
	m_texture2 = m_backend->CreateTexture2D("assets/textures/wall.jpg");

	AssetManager::LoadAsset(FileManager::ReadFile("assets/objects/box.gltf"));

	CreatePipeline();
	CreateDescriptorSet();
	CreateCubeMesh();
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
	m_backend->UpdateUniformBuffer(m_object_buffer, &m_object_data, sizeof(ObjectData));

	m_backend->SetClearColor({ 0.2f, 0.5f, 1.0f, 1.0f });
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight()});
}

void SandboxApplication::OnRenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->BindPipeline(m_pipeline);
	m_backend->BindDescriptorSet(m_pipeline, m_camera_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_texture_descriptor);

	m_backend->BindVertexBuffer(m_vertex_buffer, 0);
	m_backend->BindIndexBuffer(m_index_buffer, 0);
	m_backend->BindDescriptorSet(m_pipeline, m_object_descriptor);
	m_backend->DrawIndexed(m_index_buffer->GetCount(), 1, 0, 0, 0);

	m_backend->EndDynamicRendering();
}

void SandboxApplication::OnRenderEnd()
{
}

void SandboxApplication::Shutdown()
{
	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_object_buffer);
	m_backend->DestroyBuffer(m_vertex_buffer);
	m_backend->DestroyBuffer(m_index_buffer);

	m_backend->DestroyTexture(m_texture1);
	m_backend->DestroyTexture(m_texture2);

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

	return false;
}

void SandboxApplication::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layouts = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 1, DescriptorType_UniformBuffer, ShaderStage_Vertex}
		},
		{
			{ 2, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 3, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};
	pipeline_info.vertex_binding_description = Vertex::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = Vertex::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.line_width = 1.0f;

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = "assets/shaders/shader.vert";
	shader_info.sources[ShaderType_Fragment] = "assets/shaders/shader.frag";

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
}

void SandboxApplication::CreateDescriptorSet()
{
	// Init pool
	m_backend->InitDescriptorPool({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 3);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBuffer(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Objects descriptors
	{
		m_object_data.model = glm::mat4(1.0f);
		m_object_data.model = glm::translate(m_object_data.model, glm::vec3(0.0f, 0.0f, -5.0f));

		m_object_buffer = m_backend->CreateUniformBuffer(sizeof(ObjectData));
		m_object_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 1;
		descriptor_data.data.buffer.buffer = m_object_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_object_descriptor, descriptor_data_objects);
	}

	// Textures descriptor
	{
		m_texture_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 2);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 2;
		descriptor_data1.data.image.image_view = m_texture1->GetImageView();
		descriptor_data1.data.image.sampler = m_texture1->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 3;
		descriptor_data2.data.image.image_view = m_texture2->GetImageView();
		descriptor_data2.data.image.sampler = m_texture2->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_texture = { descriptor_data1, descriptor_data2 };
		m_backend->WriteDescriptor(&m_texture_descriptor, descriptor_data_texture);
	}
}

void SandboxApplication::CreateCubeMesh()
{
	std::vector<Vertex> vertices = {
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

	std::vector<u32> indices = {
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

	m_vertex_buffer = m_backend->CreateVertexBuffer(vertices.data(), vertices.size());
	m_index_buffer = m_backend->CreateIndexBuffer(indices.data(), indices.size());
}
