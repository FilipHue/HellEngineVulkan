#include "test_texture_cubemap_array.h"

TestTextureCubemapArray::TestTextureCubemapArray(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestTextureCubemapArray::~TestTextureCubemapArray()
{
	NO_OP;
}

void TestTextureCubemapArray::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_model_index = 0;

	m_camera.SetPosition({ 0.0f, 0.0f, 4.0f });
	m_camera.SetAspect((f32)m_window->GetWidth(), (f32)m_window->GetHeight());

	LoadResources();

	CreatePipelines();
	CreateDescriptorSets();
}

void TestTextureCubemapArray::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_ubo_data.projection = m_camera.GetProjection();
		m_ubo_data.modelView = m_camera.GetView();
		m_ubo_data.inverseModelview = glm::inverse(m_camera.GetView());
	}
}

void TestTextureCubemapArray::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_ubo, &m_ubo_data, sizeof(UBOData));
}

void TestTextureCubemapArray::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });
	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });

	m_backend->BindPipeline(m_pipeline_skybox);
	m_backend->BindDescriptorSet(m_pipeline_skybox, m_ubo_descriptor);
	m_cube_model->Draw(m_pipeline_skybox, 0);

	m_backend->BindPipeline(m_pipeline_reflect);
	m_backend->BindDescriptorSet(m_pipeline_reflect, m_ubo_descriptor);
	m_models[m_model_index]->Draw(m_pipeline_reflect, 0);

	m_backend->EndDynamicRendering();
}

void TestTextureCubemapArray::RenderEnd()
{
}

void TestTextureCubemapArray::UIRender()
{
}

void TestTextureCubemapArray::Cleanup()
{
	m_backend->DestroyUniformBuffer(m_ubo);

	m_backend->DestroyPipeline(m_pipeline_skybox);
	m_backend->DestroyPipeline(m_pipeline_reflect);
}

b8 TestTextureCubemapArray::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnWindowResize(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_UP)
	{
		m_ubo_data.lodBias += 1.0f;

		if (m_ubo_data.lodBias > m_cubemap_texture->GetMipLevels())
		{
			m_ubo_data.lodBias = (f32)m_cubemap_texture->GetMipLevels();
		}
	}
	else if (event.data.key_event.key == KEY_DOWN)
	{
		m_ubo_data.lodBias -= 1.0f;

		if (m_ubo_data.lodBias < 0.0f)
		{
			m_ubo_data.lodBias = 0.0f;
		}
	}

	if (event.data.key_event.key == KEY_RIGHT)
	{
		m_model_index++;
		if (m_model_index >= m_models.size())
		{
			m_model_index = 0;
		}
	}
	else if (event.data.key_event.key == KEY_LEFT)
	{
		if (m_model_index == 0)
		{
			m_model_index = (u32)m_models.size() - 1;
		}
		else
		{
			m_model_index--;
		}
	}

	if (event.data.key_event.key == KEY_SPACE)
	{
		m_ubo_data.cubemapIndex++;
		if (m_ubo_data.cubemapIndex >= 3)
		{
			m_ubo_data.cubemapIndex = 0;
		}
	}

	return false;
}

b8 TestTextureCubemapArray::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestTextureCubemapArray::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestTextureCubemapArray::LoadResources()
{
	m_cubemap_texture = m_frontend->CreateTextureCubemapArray("CubemapArray", FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "cubemap_array.ktx")));

	m_cube_model = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "cube.gltf")));
	m_cube_model->UploadToGPU<VertexFormatBase>();

	m_model_names = { "sphere.gltf", "teapot.gltf", "torusknot.gltf", "venus.gltf" };
	for (u32 i = 0; i < m_model_names.size(); i++)
	{
		m_models.push_back(AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, m_model_names[i]))));
		m_models[i]->UploadToGPU<VertexFormatBase>();
	}
}

void TestTextureCubemapArray::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_Front;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.layout = {
		{
			{ 
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_None },
				{ 1, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_None }
			},
			DescriptorSetFlags_None
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	pipeline_info.depth_stencil_info = {
		false,
		false,
		false,
		PipelineDethStencilCompareOp_LessOrEqual
	};

	ShaderStageInfo shader_info = {};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "texture_cubemap_array_skybox.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "texture_cubemap_array_skybox.frag");

	m_pipeline_skybox = m_backend->CreatePipeline(pipeline_info, shader_info);

	pipeline_info.cull_mode = PipelineCullMode_Back;

	pipeline_info.depth_stencil_info = {
		true,
		true,
		false,
		PipelineDethStencilCompareOp_Less
	};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "texture_cubemap_array_reflect.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "texture_cubemap_array_reflect.frag");

	m_pipeline_reflect = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline_reflect->GetRenderingCreateInfo());
}

void TestTextureCubemapArray::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// UBO descriptor
	m_ubo_data.projection = m_camera.GetProjection();
	m_ubo_data.modelView = m_camera.GetView();
	m_ubo_data.inverseModelview = glm::inverse(m_camera.GetView());
	m_ubo_data.lodBias = 0.0f;
	m_ubo_data.cubemapIndex = 1;

	m_ubo_descriptor = m_backend->CreateDescriptorSet(m_pipeline_skybox, 0);
	m_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(UBOData));

	DescriptorSetWriteData ubo_write1 = {};
	ubo_write1.type = DescriptorType_UniformBuffer;
	ubo_write1.binding = 0;
	ubo_write1.data.buffer.buffers = new VkBuffer[1];
	ubo_write1.data.buffer.buffers[0] = m_ubo->GetHandle();
	ubo_write1.data.buffer.offsets = new VkDeviceSize[1];
	ubo_write1.data.buffer.offsets[0] = 0;
	ubo_write1.data.buffer.ranges = new VkDeviceSize[1];
	ubo_write1.data.buffer.ranges[0] = sizeof(UBOData);

	DescriptorSetWriteData ubo_write2 = {};
	ubo_write2.type = DescriptorType_CombinedImageSampler;
	ubo_write2.binding = 1;
	ubo_write2.data.image.image_views = new VkImageView[1];
	ubo_write2.data.image.image_views[0] = m_cubemap_texture->GetImageView();
	ubo_write2.data.image.samplers = new VkSampler[1];
	ubo_write2.data.image.samplers[0] = m_cubemap_texture->GetSampler();

	std::vector<DescriptorSetWriteData> writes = { ubo_write1, ubo_write2 };
	m_backend->WriteDescriptor(&m_ubo_descriptor, writes);
}
