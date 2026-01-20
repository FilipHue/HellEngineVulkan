#include "test_texture_3d.h"

#if _TEST_BED_ENABLED
TestTexture3D::TestTexture3D(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestTexture3D::~TestTexture3D()
{
	NO_OP;
}

void TestTexture3D::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_depth = 0.0f;

	srand((u32)time(NULL));

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

void TestTexture3D::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
		m_light_data.view_position = glm::vec4(m_camera.GetPosition(), 1.0f);
	}

	m_depth += dt * 0.15f;
	if (m_depth > 1.0f)
	{
		m_depth = 1.0f - m_depth;
	}
}

void TestTexture3D::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_light_buffer, &m_light_data, sizeof(LightData));
	m_backend->UpdateUniformBuffer(m_image_buffer, &m_depth, sizeof(f32));
}

void TestTexture3D::RenderUpdate()
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

void TestTexture3D::RenderEnd()
{
}

void TestTexture3D::UIRender()
{
}

void TestTexture3D::Cleanup()
{
	m_backend->DestroyTexture(m_texture);

	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_model_buffer);
	m_backend->DestroyBuffer(m_light_buffer);
	m_backend->DestroyBuffer(m_image_buffer);

	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestTexture3D::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestTexture3D::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_SPACE)
	{
		CreateNoiseTexture();
		m_backend->UpdateTexture(m_texture, m_texture_data);
		m_depth = 0.0f;
	}

	return false;
}

b8 TestTexture3D::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestTexture3D::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestTexture3D::CreateNoiseTexture()
{
	m_texture_width = 128;
	m_texture_height = 128;
	m_texture_depth = 128;

	m_texture_data = new u8[m_texture_width * m_texture_height * m_texture_depth];
	memset(m_texture_data, 0, m_texture_width * m_texture_height * m_texture_depth);

	PerlinNoise<f32> noise(true);
	FractalNoise<f32> fractal_noise(noise);

	const f32 noise_scale = static_cast<f32>(rand() % 10) + 4.0f;

#pragma omp parallel for
	for (i32 z = 0; z < static_cast<i32>(m_texture_depth); z++)
	{
		for (i32 y = 0; y < static_cast<i32>(m_texture_height); y++)
		{
			for (i32 x = 0; x < static_cast<i32>(m_texture_width); x++)
			{
				float nx = (f32)x / (f32)m_texture_width;
				float ny = (f32)y / (f32)m_texture_height;
				float nz = (f32)z / (f32)m_texture_depth;
				float n = fractal_noise.noise(nx * noise_scale, ny * noise_scale, nz * noise_scale);
				n = n - floor(n);
				m_texture_data[x + y * m_texture_width + z * m_texture_width * m_texture_height] = static_cast<u8>(floor(n * 255));
			}
		}
	}
}

void TestTexture3D::LoadResources()
{
	CreateNoiseTexture();
	m_texture = m_backend->CreateTexture3D(VK_FORMAT_R8_UNORM, m_texture_data, m_texture_width, m_texture_height, m_texture_depth);
	m_frontend->CreateMesh("Quad", m_vertices, m_indices);
}

void TestTexture3D::SetMeshData()
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

void TestTexture3D::CreatePipelines()
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

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "texture_3d.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "texture_3d.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestTexture3D::CreateDescriptorSets()
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
#endif // _TEST_BED_ENABLED