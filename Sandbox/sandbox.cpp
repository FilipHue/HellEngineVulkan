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
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void SandboxApplication::OnRenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

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
	m_backend->DestroyBuffer(m_camera_buffer);

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

	return false;
}

void SandboxApplication::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None }
			},
			DescriptorSetFlags_None
		},
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
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
		false,
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "triangle.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "triangle.frag");

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
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData), 1);
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}
}