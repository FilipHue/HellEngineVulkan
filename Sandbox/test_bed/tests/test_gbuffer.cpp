#include "test_gbuffer.h"

#if _TEST_BED_ENABLED
TestGBuffer::TestGBuffer(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestGBuffer::~TestGBuffer()
{
	NO_OP;
}

void TestGBuffer::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera.SetPosition({ -3.2f, 1.0f, 5.9f });
	m_camera.RotateOX(glm::radians(0.5f));
	m_camera.RotateOY(glm::radians(330.0f));
	m_camera.SetFov(60.0f);

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	CreateLightsData();
	LoadResources();
	CreateAttachments();

	CreatePipeline();
	CreateDescriptorSets();

	m_scene_model->UploadToGPU<VertexFormatBase>();
	m_glass_model->UploadToGPU<VertexFormatBase>();
	m_scene_model->GenerateDescriptorSets(m_gbuffer_pipeline, 2);
	m_scene_model->GenerateDescriptorSets(m_glass_pipeline, 3);
	m_glass_model->GenerateDescriptorSets(m_glass_pipeline, 3);
}

void TestGBuffer::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestGBuffer::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_object_buffer, &m_object_data, sizeof(ObjectData));
}

void TestGBuffer::RenderUpdate()
{
	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	// Gbuffer
	{
		m_backend->BeginDynamicRenderingWithAttachments(m_gbuffer_rendering_info);

		m_backend->BindPipeline(m_gbuffer_pipeline);
		m_backend->BindDescriptorSet(m_gbuffer_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_gbuffer_pipeline, m_object_descriptor);

		m_scene_model->Draw(m_gbuffer_pipeline, 0);

		m_backend->EndDynamicRenderingWithAttachments(m_gbuffer_rendering_info);
	}

	// Composition
	{
		m_backend->BeginDynamicRenderingWithAttachments(m_composition_rendering_info);

		m_backend->BindPipeline(m_composition_pipeline);
		m_backend->BindDescriptorSet(m_composition_pipeline, m_composition_descriptor);

		m_backend->Draw(3, 1, 0, 0);

		m_backend->EndDynamicRenderingWithAttachments(m_composition_rendering_info);
	}

	// Glass
	{
		m_backend->BeginDynamicRenderingWithAttachments(m_glass_rendering_info);

		m_backend->BindPipeline(m_glass_pipeline);
		m_backend->BindDescriptorSet(m_glass_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_glass_pipeline, m_object_descriptor);

		m_backend->BindDescriptorSet(m_glass_pipeline, m_glass_descriptor);
		m_glass_model->Draw(m_glass_pipeline, 0);

		m_backend->EndDynamicRenderingWithAttachments(m_glass_rendering_info);
	}

	// Screen
	{
		m_backend->BeginDynamicRendering();

		m_backend->BindPipeline(m_final_pipeline);
		m_backend->BindDescriptorSet(m_final_pipeline, m_final_descriptor);

		m_backend->Draw(3, 1, 0, 0);

		m_backend->EndDynamicRendering();
	}
}

void TestGBuffer::RenderEnd()
{
}

void TestGBuffer::UIRender()
{
}

void TestGBuffer::Cleanup()
{
	m_backend->DestroyUniformBuffer(m_camera_buffer);
	m_backend->DestroyUniformBuffer(m_object_buffer);
	m_backend->DestroyStorageBuffer(m_lights_buffer);

	m_backend->DestroyPipeline(m_gbuffer_pipeline);
	m_backend->DestroyPipeline(m_composition_pipeline);
	m_backend->DestroyPipeline(m_glass_pipeline);
	m_backend->DestroyPipeline(m_final_pipeline);

	m_backend->DestroyTexture(m_gbuffer_color_texture);
	m_backend->DestroyTexture(m_gbuffer_normal_texture);
	m_backend->DestroyTexture(m_gbuffer_position_texture);
	m_backend->DestroyTexture(m_gbuffer_albedo_texture);
	m_backend->DestroyTexture(m_gbuffer_depth_texture);
	m_backend->DestroyTexture(m_composition_color_texture);
	m_backend->DestroyTexture(m_glass_color_texture);
	m_backend->DestroyTexture(m_glass_depth_texture);
	m_backend->DestroyTexture(m_final_color_texture);
}

b8 TestGBuffer::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	UpdateAttachments();

	return false;
}

b8 TestGBuffer::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_ENTER)
	{
		CreateLightsData();

		m_backend->UpdateStorageBuffer(m_lights_buffer, &m_lights[0], sizeof(Light) * (u32)m_lights.size());
	}

	return false;
}

b8 TestGBuffer::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestGBuffer::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestGBuffer::LoadResources()
{
	m_scene_model = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "samplebuilding.gltf")));
	m_glass_model = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "samplebuilding_glass.gltf")));
	m_glass_texture = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "colored_glass_rgba.ktx")));
}

void TestGBuffer::CreateAttachments()
{
	// GBuffer attachments
	{
		m_gbuffer_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_UNORM, m_window->GetWidth(), m_window->GetHeight());
		m_gbuffer_normal_texture = m_backend->CreateTexture2D(VK_FORMAT_R16G16B16A16_SFLOAT, m_window->GetWidth(), m_window->GetHeight());
		m_gbuffer_position_texture = m_backend->CreateTexture2D(VK_FORMAT_R16G16B16A16_SFLOAT, m_window->GetWidth(), m_window->GetHeight());
		m_gbuffer_albedo_texture = m_backend->CreateTexture2D(VK_FORMAT_R8G8B8A8_SRGB, m_window->GetWidth(), m_window->GetHeight());
		m_gbuffer_depth_texture = m_backend->CreateTexture2D(VK_FORMAT_D32_SFLOAT, m_window->GetWidth(), m_window->GetHeight());

		m_gbuffer_color_attachment = {
			m_gbuffer_color_texture->GetHandle(),
			m_gbuffer_color_texture->GetImageView(),
			m_gbuffer_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_gbuffer_normal_attachment = {
			m_gbuffer_normal_texture->GetHandle(),
			m_gbuffer_normal_texture->GetImageView(),
			m_gbuffer_normal_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_gbuffer_position_attachment = {
			m_gbuffer_position_texture->GetHandle(),
			m_gbuffer_position_texture->GetImageView(),
			m_gbuffer_position_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_gbuffer_albedo_attachment = {
			m_gbuffer_albedo_texture->GetHandle(),
			m_gbuffer_albedo_texture->GetImageView(),
			m_gbuffer_albedo_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_gbuffer_depth_attachment = {
			m_gbuffer_depth_texture->GetHandle(),
			m_gbuffer_depth_texture->GetImageView(),
			m_gbuffer_depth_texture->GetFormat(),
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			{ { 1.0f, 0 } }
		};

		m_gbuffer_rendering_info = {
			{ m_gbuffer_color_attachment, m_gbuffer_normal_attachment, m_gbuffer_position_attachment, m_gbuffer_albedo_attachment },
			m_gbuffer_depth_attachment,
			0
		};
	}

	// Composition attachments
	{
		m_composition_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_UNORM, m_window->GetWidth(), m_window->GetHeight());

		m_composition_color_attachment = {
			m_composition_color_texture->GetHandle(),
			m_composition_color_texture->GetImageView(),
			m_composition_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_composition_rendering_info = {
			{ m_composition_color_attachment },
			std::nullopt,
			0
		};
	}

	// Glass attachments
	{
		m_glass_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_UNORM, m_window->GetWidth(), m_window->GetHeight());
		m_glass_depth_texture = m_backend->CreateTexture2D(VK_FORMAT_D32_SFLOAT, m_window->GetWidth(), m_window->GetHeight());

		m_glass_color_attachment = {
			m_glass_color_texture->GetHandle(),
			m_glass_color_texture->GetImageView(),
			m_glass_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_glass_depth_attachment = {
			m_glass_depth_texture->GetHandle(),
			m_glass_depth_texture->GetImageView(),
			m_glass_depth_texture->GetFormat(),
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			{ { 1.0f, 0 } }
		};

		m_glass_rendering_info = {
			{ m_glass_color_attachment },
			m_glass_depth_attachment,
			0
		};
	}

	// Final attachments
	{
		m_final_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_UNORM, m_window->GetWidth(), m_window->GetHeight());

		m_final_color_attachment = {
			m_final_color_texture->GetHandle(),
			m_final_color_texture->GetImageView(),
			m_final_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }
		};

		m_final_rendering_info = {
			{ m_final_color_attachment },
			std::nullopt,
			0
		};
	}
}

void TestGBuffer::CreateLightsData()
{
	std::vector<glm::vec3> colors =
	{
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
	};

	std::random_device rndDevice;
	std::default_random_engine rndGen(rndDevice());
	std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);
	std::uniform_real_distribution<float> rndCol(0.0f, 0.5f);

	for (auto& light : m_lights)
	{
		light.position = glm::vec4(rndDist(rndGen) * 8.0f, 0.25f + std::abs(rndDist(rndGen)) * 4.0f, rndDist(rndGen) * 8.0f, 1.0f);
		light.color = glm::vec3(rndCol(rndGen), rndCol(rndGen), rndCol(rndGen)) * 2.0f;
		light.radius = 1.0f + std::abs(rndDist(rndGen));
	}
}

void TestGBuffer::CreatePipeline()
{
	// GBuffer pipeline
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex}
		},
		{
			{0, DescriptorType_UniformBuffer, ShaderStage_Fragment}
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
		{ m_gbuffer_color_attachment.format, m_gbuffer_normal_attachment.format, m_gbuffer_position_attachment.format, m_gbuffer_albedo_attachment.format },
		m_gbuffer_depth_attachment.format
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "gbuffer_gbuffer.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "gbuffer_gbuffer.frag");

	m_gbuffer_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Composition pipeline
	pipeline_info.vertex_binding_description = {};
	pipeline_info.vertex_attribute_descriptions = {};

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 2, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 3, DescriptorType_StorageBuffer, ShaderStage_Fragment }
		}
	};

	pipeline_info.dynamic_rendering_info = {
		{ m_composition_color_attachment.format },
		std::nullopt
	};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "gbuffer_composition.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "gbuffer_composition.frag");

	m_composition_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Glass pipeline
	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment }
		}
	};

	pipeline_info.dynamic_rendering_info = {
		{ m_glass_color_attachment.format },
		m_glass_depth_attachment.format
	};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "gbuffer_glass.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "gbuffer_glass.frag");

	m_glass_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Final pipeline
	pipeline_info.vertex_binding_description = {};
	pipeline_info.vertex_attribute_descriptions = {};

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 2, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 3, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};

	pipeline_info.dynamic_rendering_info = {
		{ m_final_color_attachment.format },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "gbuffer_final.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "gbuffer_final.frag");

	m_final_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_final_pipeline->GetRenderingCreateInfo());
}

void TestGBuffer::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_gbuffer_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Object descriptor
	{
		m_object_data.model = glm::mat4(1.0f);

		m_object_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_object_descriptor = m_backend->CreateDescriptorSet(m_gbuffer_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_object_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_object = { descriptor_data };
		m_backend->WriteDescriptor(&m_object_descriptor, descriptor_data_object);
	}

	// Composition pipeline descriptor
	{
		void* data = nullptr;
		data = static_cast<void*>(m_lights.data());
		m_lights_buffer = m_backend->CreateStorageBufferMappedOnce(data, sizeof(Light) * (u32)m_lights.size());
		m_composition_descriptor = m_backend->CreateDescriptorSet(m_composition_pipeline, 0);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_views = m_gbuffer_position_texture->GetImageView();
		descriptor_data1.data.image.samplers = m_gbuffer_position_texture->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_views = m_gbuffer_normal_texture->GetImageView();
		descriptor_data2.data.image.samplers = m_gbuffer_normal_texture->GetSampler();

		DescriptorSetWriteData descriptor_data3;
		descriptor_data3.type = DescriptorType_CombinedImageSampler;
		descriptor_data3.binding = 2;
		descriptor_data3.data.image.image_views = m_gbuffer_albedo_texture->GetImageView();
		descriptor_data3.data.image.samplers = m_gbuffer_albedo_texture->GetSampler();

		DescriptorSetWriteData descriptor_data4;
		descriptor_data4.type = DescriptorType_StorageBuffer;
		descriptor_data4.binding = 3;
		descriptor_data4.data.buffer.buffers = m_lights_buffer->GetHandle();
		descriptor_data4.data.buffer.offsets = 0;
		descriptor_data4.data.buffer.ranges = sizeof(Light) * m_lights.size();

		std::vector<DescriptorSetWriteData> descriptor_data_final = { descriptor_data1, descriptor_data2, descriptor_data3, descriptor_data4 };
		m_backend->WriteDescriptor(&m_composition_descriptor, descriptor_data_final);
	}

	// Glass pipeline descriptor
	{
		m_glass_descriptor = m_backend->CreateDescriptorSet(m_glass_pipeline, 2);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_views = m_glass_texture->GetImageView();
		descriptor_data.data.image.samplers = m_glass_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_final = { descriptor_data };
		m_backend->WriteDescriptor(&m_glass_descriptor, descriptor_data_final);
	}

	// Final pipeline descriptor
	{
		m_final_descriptor = m_backend->CreateDescriptorSet(m_final_pipeline, 0);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_views = m_glass_color_texture->GetImageView();
		descriptor_data1.data.image.samplers = m_glass_color_texture->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_views = m_glass_depth_texture->GetImageView();
		descriptor_data2.data.image.samplers = m_glass_depth_texture->GetSampler();

		DescriptorSetWriteData descriptor_data3;
		descriptor_data3.type = DescriptorType_CombinedImageSampler;
		descriptor_data3.binding = 2;
		descriptor_data3.data.image.image_views = m_composition_color_texture->GetImageView();
		descriptor_data3.data.image.samplers = m_composition_color_texture->GetSampler();

		DescriptorSetWriteData descriptor_data4;
		descriptor_data4.type = DescriptorType_CombinedImageSampler;
		descriptor_data4.binding = 3;
		descriptor_data4.data.image.image_views = m_gbuffer_depth_texture->GetImageView();
		descriptor_data4.data.image.samplers = m_gbuffer_depth_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_final = { descriptor_data1, descriptor_data2, descriptor_data3, descriptor_data4 };
		m_backend->WriteDescriptor(&m_final_descriptor, descriptor_data_final);
	}
}

void TestGBuffer::UpdateAttachments()
{
	m_backend->DestroyTexture(m_gbuffer_color_texture);
	m_backend->DestroyTexture(m_gbuffer_normal_texture);
	m_backend->DestroyTexture(m_gbuffer_position_texture);
	m_backend->DestroyTexture(m_gbuffer_albedo_texture);
	m_backend->DestroyTexture(m_gbuffer_depth_texture);
	m_backend->DestroyTexture(m_composition_color_texture);
	m_backend->DestroyTexture(m_glass_color_texture);
	m_backend->DestroyTexture(m_glass_depth_texture);
	m_backend->DestroyTexture(m_final_color_texture);

	CreateAttachments();

	// Composite
	{
		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_views = m_gbuffer_position_texture->GetImageView();
		descriptor_data1.data.image.samplers = m_gbuffer_position_texture->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_views = m_gbuffer_normal_texture->GetImageView();
		descriptor_data2.data.image.samplers = m_gbuffer_normal_texture->GetSampler();

		DescriptorSetWriteData descriptor_data3;
		descriptor_data3.type = DescriptorType_CombinedImageSampler;
		descriptor_data3.binding = 2;
		descriptor_data3.data.image.image_views = m_gbuffer_albedo_texture->GetImageView();
		descriptor_data3.data.image.samplers = m_gbuffer_albedo_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_composition = { descriptor_data1, descriptor_data2, descriptor_data3 };
		m_backend->WriteDescriptor(&m_composition_descriptor, descriptor_data_composition);
	}

	// Final
	{
		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_views = m_glass_color_texture->GetImageView();
		descriptor_data1.data.image.samplers = m_glass_color_texture->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_views = m_glass_depth_texture->GetImageView();
		descriptor_data2.data.image.samplers = m_glass_depth_texture->GetSampler();

		DescriptorSetWriteData descriptor_data3;
		descriptor_data3.type = DescriptorType_CombinedImageSampler;
		descriptor_data3.binding = 2;
		descriptor_data3.data.image.image_views = m_composition_color_texture->GetImageView();
		descriptor_data3.data.image.samplers = m_composition_color_texture->GetSampler();

		DescriptorSetWriteData descriptor_data4;
		descriptor_data4.type = DescriptorType_CombinedImageSampler;
		descriptor_data4.binding = 3;
		descriptor_data4.data.image.image_views = m_gbuffer_depth_texture->GetImageView();
		descriptor_data4.data.image.samplers = m_gbuffer_depth_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_final = { descriptor_data1, descriptor_data2, descriptor_data3, descriptor_data4 };
		m_backend->WriteDescriptor(&m_final_descriptor, descriptor_data_final);
	}
}
#endif // _TEST_BED_ENABLED