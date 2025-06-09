#include "test_texture.h"

TestTexture::TestTexture(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestTexture::~TestTexture()
{
	NO_OP;
}

void TestTexture::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_load_bias = 0.0f;

	m_camera.SetPosition({ 0.0f, 0.0f, -2.5f });
	m_camera.Rotate(0.0f, 0.0f, 0.0f);
	m_camera.SetAspect((f32)m_window->GetWidth(), (f32)m_window->GetHeight());

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_light_data.view_position = glm::vec4(m_camera.GetPosition(), 1.0f);

	SetMeshData();
	LoadResources();

	CreatePipelines();
	CreateDescriptorSets();
}

void TestTexture::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
		m_light_data.view_position = glm::vec4(m_camera.GetPosition(), 1.0f);
	}
}

void TestTexture::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_light_buffer, &m_light_data, sizeof(LightData));
	m_backend->UpdateUniformBuffer(m_image_buffer, &m_load_bias, sizeof(f32));
}

void TestTexture::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });
	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });

	m_backend->BindPipeline(m_pipeline);

	m_backend->BindDescriptorSet(m_pipeline, m_camera_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_model_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_light_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_image_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_texture_descriptor);

	m_frontend->DrawMesh("Quad");

	m_backend->EndDynamicRendering();
}

void TestTexture::RenderEnd()
{
}

void TestTexture::UIRender()
{
}

void TestTexture::Cleanup()
{
	m_backend->DestroyTexture(m_texture);

	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_model_buffer);
	m_backend->DestroyBuffer(m_light_buffer);
	m_backend->DestroyBuffer(m_image_buffer);

	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestTexture::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestTexture::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestTexture::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestTexture::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestTexture::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestTexture::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_UP)
	{
		m_load_bias += 1.0f;

		if (m_load_bias > m_texture->GetMipLevels())
		{
			m_load_bias = (f32)m_texture->GetMipLevels();
		}
	}
	else if (event.data.key_event.key == KEY_DOWN)
	{
		m_load_bias -= 1.0f;

		if (m_load_bias < 0.0f)
		{
			m_load_bias = 0.0f;
		}
	}

	return false;
}

b8 TestTexture::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestTexture::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestTexture::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestTexture::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestTexture::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestTexture::LoadResources()
{
	m_texture = m_backend->CreateTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "metalplate01_rgba.ktx")));
	m_frontend->CreateMesh("Quad", m_vertices, m_indices);
}

void TestTexture::SetMeshData()
{
	m_vertices = {
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }},
		{ { -1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }},
		{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }},
		{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }}
	};

	m_indices = {
		0, 1, 2,
		2, 3, 0
	};
}

void TestTexture::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.layout = {
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
		},
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
		},
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
		},
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
		},
		{
			{ { 0, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	ShaderStageInfo shader_info = {};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "texture.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "texture.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestTexture::CreateDescriptorSets()
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
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Model descriptor
	{
		m_model_data.model = glm::mat4(1.0f);
		m_model_data.model = glm::translate(m_model_data.model, glm::vec3(0.0f, 0.0f, -5.0f));
		m_model_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_model_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 1);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_model_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_model = { descriptor_data };
		m_backend->WriteDescriptor(&m_model_descriptor, descriptor_data_model);
	}

	// Light descriptor
	{
		m_light_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightData));
		m_light_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 2);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_light_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(LightData);

		std::vector<DescriptorSetWriteData> descriptor_data_light = { descriptor_data };
		m_backend->WriteDescriptor(&m_light_descriptor, descriptor_data_light);
	}

	// Image LOD bias
	{
		m_image_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(f32));
		m_image_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 3);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_image_buffer->GetHandle();
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(f32);

		std::vector<DescriptorSetWriteData> descriptor_data_image = { descriptor_data };
		m_backend->WriteDescriptor(&m_image_descriptor, descriptor_data_image);
	}

	// Texture descriptor
	{
		m_texture_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 4);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_views = new VkImageView[1];
		descriptor_data.data.image.image_views[0] = m_texture->GetImageView();
		descriptor_data.data.image.samplers = new VkSampler[1];
		descriptor_data.data.image.samplers[0] = m_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_texture = { descriptor_data };
		m_backend->WriteDescriptor(&m_texture_descriptor, descriptor_data_texture);
	}
}
