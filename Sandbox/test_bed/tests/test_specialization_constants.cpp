#include "test_specialization_constants.h"

TestSpecializationConstants::TestSpecializationConstants(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestSpecializationConstants::~TestSpecializationConstants()
{
	NO_OP;
}

void TestSpecializationConstants::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_specialization_data.lighting_model = 0;
	m_specialization_data.toon_desaturation_factor = 0.5f;

	m_light_data.position = { 0.0f, 0.0f, 5.0f, 1.0f };
	m_light_data.view_position = { 0.0f, 0.0f, 5.0f, 1.0f };

	m_camera.SetPosition({ 0.0f, 0.0f, 5.0f });
	m_camera.SetAspect((f32)m_window->GetWidth() / 3, (f32)m_window->GetHeight());

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	CreatePipelines();
	CreateDescriptorSets();

	LoadResources();
	m_model->UploadToGPU<VertexFormatBase>();
}

void TestSpecializationConstants::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestSpecializationConstants::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_light_buffer, &m_light_data, sizeof(LightData));
}

void TestSpecializationConstants::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	// Phong pipeline
	{
		m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth() / 3, (f32)m_window->GetHeight(), 0.0f, 1.0f } });
		m_backend->BindPipeline(m_phog_pipeline);

		m_backend->BindDescriptorSet(m_phog_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_phog_pipeline, m_model_descriptor);
		m_backend->BindDescriptorSet(m_phog_pipeline, m_light_descriptor);
		m_backend->BindDescriptorSet(m_textured_pipeline, m_texture_descriptor);

		m_model->Draw(m_phog_pipeline, 0);
	}

	// Toon pipeline
	{
		m_backend->SetViewport({ { (f32)m_window->GetWidth() / 3, 0.0f, (f32)m_window->GetWidth() / 3, (f32)m_window->GetHeight(), 0.0f, 1.0f } });
		m_backend->BindPipeline(m_toon_pipeline);

		m_backend->BindDescriptorSet(m_toon_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_toon_pipeline, m_model_descriptor);
		m_backend->BindDescriptorSet(m_toon_pipeline, m_light_descriptor);
		m_backend->BindDescriptorSet(m_textured_pipeline, m_texture_descriptor);

		m_model->Draw(m_toon_pipeline, 0);
	}

	// Textured pipeline
	{
		m_backend->SetViewport({ { 2 * (f32)m_window->GetWidth() / 3, 0.0f, (f32)m_window->GetWidth() / 3, (f32)m_window->GetHeight(), 0.0f, 1.0f } });
		m_backend->BindPipeline(m_textured_pipeline);

		m_backend->BindDescriptorSet(m_textured_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_textured_pipeline, m_model_descriptor);
		m_backend->BindDescriptorSet(m_textured_pipeline, m_light_descriptor);
		m_backend->BindDescriptorSet(m_textured_pipeline, m_texture_descriptor);

		m_model->Draw(m_textured_pipeline, 0);
	}

	m_backend->EndDynamicRendering();
}

void TestSpecializationConstants::RenderEnd()
{
}

void TestSpecializationConstants::UIRender()
{
}

void TestSpecializationConstants::Cleanup()
{
	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_model_buffer);
	m_backend->DestroyBuffer(m_light_buffer);

	m_backend->DestroyPipeline(m_phog_pipeline);
	m_backend->DestroyPipeline(m_toon_pipeline);
	m_backend->DestroyPipeline(m_textured_pipeline);
}

b8 TestSpecializationConstants::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnWindowResize(EventContext& event)
{
	m_camera.SetAspect((f32)event.data.window_resize.width / 3, (f32)event.data.window_resize.height);

	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestSpecializationConstants::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestSpecializationConstants::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestSpecializationConstants::LoadResources()
{
	m_model = AssetManager::LoadModel(FileManager::ReadFile("assets/models/gltf/TeapotSpheres/color_teapot_spheres.gltf"));
	m_model->GenerateDescriptorSets(m_phog_pipeline, 3);	
}

void TestSpecializationConstants::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_Back;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_SRGB },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment },
		},
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	ShaderStageInfo shader_info = {};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "specialization_constants.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "specialization_constants.frag");

	shader_info.specialization_infos.push_back({
		ShaderStage_Fragment,
		sizeof(SpecializationData),
		&m_specialization_data,
		{
			{ 0, sizeof(u32), offsetof(SpecializationData, lighting_model) },
			{ 1, sizeof(f32), offsetof(SpecializationData, toon_desaturation_factor) }
		}
	});

	m_specialization_data.lighting_model = 0;
	m_phog_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_specialization_data.lighting_model = 1;
	m_toon_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_specialization_data.lighting_model = 2;
	m_textured_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_phog_pipeline->GetRenderingCreateInfo());
}

void TestSpecializationConstants::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPool({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_phog_pipeline, 0);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Model descriptor
	{
		m_model_data.model = glm::mat4(1.0f);
		m_model_data.model = glm::translate(m_model_data.model, glm::vec3(0.0f, 0.0f, 0.0f));
		m_model_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_model_descriptor = m_backend->CreateDescriptorSet(m_phog_pipeline, 1);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_model_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_model = { descriptor_data };
		m_backend->WriteDescriptor(&m_model_descriptor, descriptor_data_model);
	}

	// Light descriptor
	{
		m_light_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightData));
		m_light_descriptor = m_backend->CreateDescriptorSet(m_phog_pipeline, 2);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_light_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(LightData);

		std::vector<DescriptorSetWriteData> descriptor_data_light = { descriptor_data };
		m_backend->WriteDescriptor(&m_light_descriptor, descriptor_data_light);
	}

	// Texture descriptor
	{
		m_texture = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "metalplate_nomips_rgba.png")));
		m_texture_descriptor = m_backend->CreateDescriptorSet(m_textured_pipeline, 4);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_view = m_texture->GetImageView();
		descriptor_data.data.image.sampler = m_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_texture = { descriptor_data };
		m_backend->WriteDescriptor(&m_texture_descriptor, descriptor_data_texture);
	}
}
