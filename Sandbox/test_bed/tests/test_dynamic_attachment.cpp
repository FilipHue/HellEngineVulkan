#include "test_dynamic_attachment.h"

#if _TEST_BED_ENABLED
TestDynamicAttachment::TestDynamicAttachment(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestDynamicAttachment::~TestDynamicAttachment()
{
	NO_OP;
}

void TestDynamicAttachment::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_texture_index = 0;

	m_camera.SetPosition({ 0.0f, 0.5f, 7.0f });

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_model = AssetManager::LoadModel(FileManager::ReadFile("assets/models/gltf/Treasure/treasure_smooth.gltf"));

	m_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_SRGB, m_window->GetWidth(), m_window->GetHeight());
	m_depth_texture = m_backend->CreateTexture2D(VK_FORMAT_D32_SFLOAT, m_window->GetWidth(), m_window->GetHeight());

	m_color_attachment = {
		m_color_texture->GetHandle(),
		m_color_texture->GetImageView(),
		m_color_texture->GetFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		{ { m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a } }
	};

	m_depth_attachment = {
		m_depth_texture->GetHandle(),
		m_depth_texture->GetImageView(),
		m_depth_texture->GetFormat(),
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		{ { 1.0f, 0 } }
	};

	m_rendering_info = {
		{ m_color_attachment },
		m_depth_attachment,
		0
	};

	CreatePipelines();
	CreateDescriptorSets();

	m_model->UploadToGPU<VertexFormatBase>();
	m_model->GenerateDescriptorSets(m_color_pipeline, 2);
}

void TestDynamicAttachment::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestDynamicAttachment::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_data_buffer, &m_data, sizeof(Data));
}

void TestDynamicAttachment::RenderUpdate()
{
	m_backend->BeginDynamicRenderingWithAttachments(m_rendering_info);

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->BindPipeline(m_color_pipeline);
	m_backend->BindDescriptorSet(m_color_pipeline, m_camera_descriptor);
	m_backend->BindDescriptorSet(m_color_pipeline, m_model_descriptor);

	m_model->Draw(m_color_pipeline, 0);

	m_backend->EndDynamicRenderingWithAttachments(m_rendering_info);
}

void TestDynamicAttachment::RenderEnd()
{
	m_backend->BeginDynamicRendering();

	m_backend->BindPipeline(m_screen_pipeline);
	m_backend->BindDescriptorSet(m_screen_pipeline, m_attachment_descriptor);
	m_backend->BindDescriptorSet(m_screen_pipeline, m_data_descriptor);
	m_backend->BindPushConstants(m_screen_pipeline, ShaderStage_Fragment, 0, sizeof(i32), &m_texture_index);
	m_backend->Draw(3, 1, 0, 0);

	m_backend->EndDynamicRendering();
}

void TestDynamicAttachment::UIRender()
{
}

void TestDynamicAttachment::Cleanup()
{
	m_backend->DestroyTexture(m_color_texture);
	m_backend->DestroyTexture(m_depth_texture);

	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_model_buffer);
	m_backend->DestroyBuffer(m_data_buffer);

	m_backend->DestroyPipeline(m_color_pipeline);
	m_backend->DestroyPipeline(m_screen_pipeline);
}

b8 TestDynamicAttachment::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	m_backend->DestroyTexture(m_color_texture);
	m_backend->DestroyTexture(m_depth_texture);

	m_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_SRGB, m_window->GetWidth(), m_window->GetHeight());
	m_depth_texture = m_backend->CreateTexture2D(VK_FORMAT_D32_SFLOAT, m_window->GetWidth(), m_window->GetHeight());

	m_color_attachment.image = m_color_texture->GetHandle();
	m_color_attachment.image_view = m_color_texture->GetImageView();

	m_depth_attachment.image = m_depth_texture->GetHandle();
	m_depth_attachment.image_view = m_depth_texture->GetImageView();

	m_rendering_info.color_attachments.clear();
	m_rendering_info.color_attachments.push_back(m_color_attachment);
	m_rendering_info.depth_attachment = m_depth_attachment;

	DescriptorSetWriteData descriptor_data1;
	descriptor_data1.type = DescriptorType_CombinedImageSampler;
	descriptor_data1.binding = 0;
	descriptor_data1.data.image.image_views = m_color_texture->GetImageView();
	descriptor_data1.data.image.samplers = m_color_texture->GetSampler();

	DescriptorSetWriteData descriptor_data2;
	descriptor_data2.type = DescriptorType_CombinedImageSampler;
	descriptor_data2.binding = 1;
	descriptor_data2.data.image.image_views = m_depth_texture->GetImageView();
	descriptor_data2.data.image.samplers = m_depth_texture->GetSampler();

	std::vector<DescriptorSetWriteData> descriptor_data = { descriptor_data1, descriptor_data2 };
	m_backend->WriteDescriptor(&m_attachment_descriptor, descriptor_data);

	return false;
}

b8 TestDynamicAttachment::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_SPACE)
	{
		m_texture_index = (m_texture_index + 1) % 2;
	}

	if (event.data.key_event.mods == MOD_KEY_SHIFT)
	{
		if (event.data.key_event.key == KEY_RIGHT)
		{
			m_data.brightness += 0.1f;
			if (m_data.brightness > 1.0f)
			{
				m_data.brightness = 1.0f;
			}
		}
		else if (event.data.key_event.key == KEY_LEFT)
		{
			m_data.brightness -= 0.1f;
			if (m_data.brightness < 0.0f)
			{
				m_data.brightness = 0.0f;
			}
		}

		if (event.data.key_event.key == KEY_UP)
		{
			m_data.contrast += 0.1f;
			if (m_data.contrast > 2.0f)
			{
				m_data.contrast = 2.0f;
			}
		}
		else if (event.data.key_event.key == KEY_DOWN)
		{
			m_data.contrast -= 0.1f;
			if (m_data.contrast < 0.0f)
			{
				m_data.contrast = 0.0f;
			}
		}
	}
	else
	{
		if (event.data.key_event.key == KEY_UP)
		{
			m_data.range.y += 0.1f;
			if (m_data.range.y > 1.0f)
			{
				m_data.range.y = 1.0f;
			}
		}
		else if (event.data.key_event.key == KEY_DOWN)
		{
			m_data.range.y -= 0.1f;
			if (m_data.range.y < 0.0f)
			{
				m_data.range.y = 0.0f;
			}
		}

		if (event.data.key_event.key == KEY_RIGHT)
		{
			m_data.range.x += 0.1f;
			if (m_data.range.x > 1.0f)
			{
				m_data.range.x = 1.0f;
			}
		}
		else if (event.data.key_event.key == KEY_LEFT)
		{
			m_data.range.x -= 0.1f;
			if (m_data.range.x < 0.0f)
			{
				m_data.range.x = 0.0f;
			}
		}
	}


	return false;
}

b8 TestDynamicAttachment::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestDynamicAttachment::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestDynamicAttachment::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_Back;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;

	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT
	};

	ShaderStageInfo shader_info = {};

	// Attachment pipeline
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment }
		}
	};
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "dynamic_attachment_write.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "dynamic_attachment_write.frag");

	m_color_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Screen pipeline
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.vertex_binding_description = {};
	pipeline_info.vertex_attribute_descriptions = {};
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;

	pipeline_info.line_width = 1.0f;

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment }
		}
	};

	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_SRGB },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.push_constant_ranges = {
		{ ShaderStage_Fragment, 0, sizeof(i32) }
	};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "dynamic_attachment_read.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "dynamic_attachment_read.frag");

	m_screen_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_screen_pipeline->GetRenderingCreateInfo());
}

void TestDynamicAttachment::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_color_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Objects descriptors
	{
		m_model_data.model = glm::mat4(1.0f);
		m_model_data.model = glm::scale(m_model_data.model, glm::vec3(1.0f));

		m_model_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_model_descriptor = m_backend->CreateDescriptorSet(m_color_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_model_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_model_descriptor, descriptor_data_objects);
	}

	// Attachment descriptor
	{
		m_attachment_descriptor = m_backend->CreateDescriptorSet(m_screen_pipeline, 0);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_views = m_color_texture->GetImageView();
		descriptor_data1.data.image.samplers = m_color_texture->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_views = m_depth_texture->GetImageView();
		descriptor_data2.data.image.samplers = m_depth_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_screen = { descriptor_data1, descriptor_data2 };
		m_backend->WriteDescriptor(&m_attachment_descriptor, descriptor_data_screen);
	}

	// Data descriptor
	{
		m_data_descriptor = m_backend->CreateDescriptorSet(m_screen_pipeline, 1);
		m_data_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(Data));

		m_data.brightness = 0.5f;
		m_data.contrast = 1.8f;
		m_data.range = { 0.6f, 1.0f };

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_data_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(Data);

		std::vector<DescriptorSetWriteData> descriptor_data_screen = { descriptor_data };
		m_backend->WriteDescriptor(&m_data_descriptor, descriptor_data_screen);
	}
}
#endif // _TEST_BED_ENABLED