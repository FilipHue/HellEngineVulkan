#include "test_offscreen.h"

#if _TEST_BED_ENABLED
TestOffscreen::TestOffscreen(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}	

TestOffscreen::~TestOffscreen()
{
	NO_OP;
}

void TestOffscreen::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_resized = false;
	m_show_render_target = false;
	m_rotation = 0.0f;

	m_camera.SetPosition({ 0.0f, 4.0f, 6.0f });
	m_camera.RotateOX(glm::radians(-35.0f));

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	LoadResources();

	CreateRenderPasses();
	CreatePipeline();
	CreateDescriptorSets();

	m_chinese_dragon_model->UploadToGPU<VertexFormatBase>();
	m_plane_model->UploadToGPU<VertexFormatBase>();
	m_chinese_dragon_model->GenerateDescriptorSets(m_shaded_pipeline, 3);
	m_chinese_dragon_model->GenerateDescriptorSets(m_offscreen_pipeline, 3);
	m_plane_model->GenerateDescriptorSets(m_mirror_pipeline, 3);
}

void TestOffscreen::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}

	m_rotation += dt * 10.0f;

	m_offscreen_data.model = glm::mat4(1.0f);
	m_offscreen_data.model = glm::scale(m_offscreen_data.model, { 1.0f, -1.0f, 1.0f });
	m_offscreen_data.model = glm::rotate(m_offscreen_data.model, glm::radians(m_rotation), { 0.0f, 1.0f, 0.0f });
	m_offscreen_data.model = glm::translate(m_offscreen_data.model, { 0.0f, 1.0f, 0.0f });

	m_chinese_dragon_data.model = glm::mat4(1.0f);
	m_chinese_dragon_data.model = glm::rotate(m_chinese_dragon_data.model, glm::radians(m_rotation), {0.0f, 1.0f, 0.0f});
	m_chinese_dragon_data.model = glm::translate(m_chinese_dragon_data.model, { 0.0f, 1.0f, 0.0f });

	m_plane_data.model = glm::mat4(1.0f);
}

void TestOffscreen::RenderBegin()
{
	if (m_resized)
	{
		m_backend->RecreateRenderPassFramebuffers(m_swapchain_render_pass, m_swapchain_render_pass_info);

		m_resized = false;
	}

	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_offscreen_buffer, &m_offscreen_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_chinese_dragon_buffer, &m_chinese_dragon_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_plane_buffer, &m_plane_data, sizeof(ObjectData));
	m_backend->UpdateUniformBuffer(m_light_buffer, &m_light_data, sizeof(LightData));
}

void TestOffscreen::RenderUpdate()
{
	// Offscreen renderpass
	{
		m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_offscreen_render_pass_info.width, (f32)m_offscreen_render_pass_info.height, 0.0f, 1.0f } });
		m_backend->SetScissor({ { { 0, 0 }, { m_offscreen_render_pass_info.width, m_offscreen_render_pass_info.height } } });

		m_backend->BeginRenderPass(m_offscreen_render_pass, { 0, 0, m_offscreen_render_pass_info.width, m_offscreen_render_pass_info.height }, { { m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a }, {1.0f, 0.0f } });

		m_backend->BindPipeline(m_offscreen_pipeline);

		m_backend->BindDescriptorSet(m_offscreen_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_offscreen_pipeline, m_offscreen_descriptor);
		m_backend->BindDescriptorSet(m_offscreen_pipeline, m_light_descriptor);

		m_chinese_dragon_model->Draw(m_offscreen_pipeline, 1);

		m_backend->EndRenderPass();
	}

	// Swapchain renderpass
	{
		m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
		m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

		m_backend->BeginRenderPassWithSwapchain(m_swapchain_render_pass, { 0, 0, m_window->GetWidth(), m_window->GetHeight() }, { { 0.005f, 0.005f, 0.005f, 1.0f }, { 1.0f, 0.0f } });


		if (m_show_render_target)
		{
			m_backend->BindPipeline(m_debug_pipeline);

			m_backend->BindDescriptorSet(m_debug_pipeline, m_debug_descriptor);

			m_backend->Draw(3, 1, 0, 0);
		}
		else
		{
			m_backend->BindPipeline(m_shaded_pipeline);

			m_backend->BindDescriptorSet(m_shaded_pipeline, m_camera_descriptor);
			m_backend->BindDescriptorSet(m_shaded_pipeline, m_chinese_dragon_descriptor);
			m_backend->BindDescriptorSet(m_shaded_pipeline, m_light_descriptor);

			m_chinese_dragon_model->Draw(m_shaded_pipeline, 0);

			m_backend->BindPipeline(m_mirror_pipeline);

			m_backend->BindDescriptorSet(m_shaded_pipeline, m_camera_descriptor);
			m_backend->BindDescriptorSet(m_mirror_pipeline, m_mirror_descriptor);
			m_backend->BindDescriptorSet(m_mirror_pipeline, m_plane_descriptor);

			m_plane_model->Draw(m_mirror_pipeline, 0);
		}

		m_backend->EndRenderPass();
	}
}

void TestOffscreen::RenderEnd()
{
}

void TestOffscreen::UIRender()
{
}

void TestOffscreen::Cleanup()
{
	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_chinese_dragon_buffer);
	m_backend->DestroyBuffer(m_plane_buffer);
	m_backend->DestroyBuffer(m_light_buffer);
	m_backend->DestroyBuffer(m_offscreen_buffer);

	m_backend->DestroyTexture(m_offscreen_color_texture);
	m_backend->DestroyTexture(m_offscreen_depth_texture);

	m_backend->DestroyRenderPass(m_offscreen_render_pass);
	m_backend->DestroyRenderPass(m_swapchain_render_pass);

	m_backend->DestroyPipeline(m_offscreen_pipeline);
	m_backend->DestroyPipeline(m_mirror_pipeline);
	m_backend->DestroyPipeline(m_shaded_pipeline);
	m_backend->DestroyPipeline(m_debug_pipeline);
}

b8 TestOffscreen::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	m_resized = true;

	return false;
}

b8 TestOffscreen::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_ENTER)
	{
		m_show_render_target = !m_show_render_target;
	}

	return false;
}

b8 TestOffscreen::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestOffscreen::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestOffscreen::LoadResources()
{
	m_plane_model = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "plane.gltf")));
	m_chinese_dragon_model = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "chinesedragon.gltf")));
}

void TestOffscreen::CreateRenderPasses()
{
	// Offscreen renderpass
	{
		m_offscreen_color_texture = m_backend->CreateTexture2D(VK_FORMAT_B8G8R8A8_SRGB, FM_DIM, FM_DIM);
		m_offscreen_depth_texture = m_backend->CreateTexture2D(VK_FORMAT_D32_SFLOAT, FM_DIM, FM_DIM);

		RenderPassAttachmentInfo color_attachment = {};
		color_attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
		color_attachment.load_op = AttachmentLoadOp_Clear;
		color_attachment.store_op = AttachmentStoreOp_Store;
		color_attachment.stencil_load_op = AttachmentLoadOp_DontCare;
		color_attachment.stencil_store_op = AttachmentStoreOp_DontCare;
		color_attachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		RenderPassAttachmentInfo depth_attachment = {};
		depth_attachment.format = VK_FORMAT_D32_SFLOAT;
		depth_attachment.load_op = AttachmentLoadOp_Clear;
		depth_attachment.store_op = AttachmentStoreOp_DontCare;
		depth_attachment.stencil_load_op = AttachmentLoadOp_DontCare;
		depth_attachment.stencil_store_op = AttachmentStoreOp_DontCare;
		depth_attachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		m_offscreen_render_pass_info.attachments.push_back(color_attachment);
		m_offscreen_render_pass_info.attachments.push_back(depth_attachment);

		RenderSubpassInfo subpass = {};
		subpass.pipeline_bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.color_attachments.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		subpass.depth_attachment = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		m_offscreen_render_pass_info.subpasses.push_back(subpass);

		RenderPassDepencyInfo dependency_color = {};
		dependency_color.src_subpass = VK_SUBPASS_EXTERNAL;
		dependency_color.dst_subpass = 0;
		dependency_color.src_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency_color.dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency_color.src_access_mask = VK_ACCESS_NONE_KHR;
		dependency_color.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency_color.dependency_flags = VK_DEPENDENCY_BY_REGION_BIT;

		RenderPassDepencyInfo dependency_depth = {};
		dependency_depth.src_subpass = 0;
		dependency_depth.dst_subpass = VK_SUBPASS_EXTERNAL;
		dependency_depth.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency_depth.dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency_depth.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency_depth.dst_access_mask = VK_ACCESS_MEMORY_READ_BIT;
		dependency_depth.dependency_flags = VK_DEPENDENCY_BY_REGION_BIT;

		m_offscreen_render_pass_info.dependencies.push_back(dependency_color);
		m_offscreen_render_pass_info.dependencies.push_back(dependency_depth);

		m_offscreen_render_pass_info.type = RenderPassType_Offscreen;

		m_offscreen_render_pass_info.custom_attachment_views.push_back(m_offscreen_color_texture->GetImageView());
		m_offscreen_render_pass_info.custom_attachment_views.push_back(m_offscreen_depth_texture->GetImageView());

		m_offscreen_render_pass_info.width = FM_DIM;
		m_offscreen_render_pass_info.height = FM_DIM;

		m_offscreen_render_pass = m_backend->CreateRenderPass(m_offscreen_render_pass_info);
	}

	// Swapchain renderpass
	{
		RenderPassAttachmentInfo color_attachment = {};
		color_attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
		color_attachment.load_op = AttachmentLoadOp_Clear;
		color_attachment.store_op = AttachmentStoreOp_Store;
		color_attachment.stencil_load_op = AttachmentLoadOp_DontCare;
		color_attachment.stencil_store_op = AttachmentStoreOp_DontCare;
		color_attachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_attachment.is_swapchain_attachment = true;

		RenderPassAttachmentInfo depth_attachment = {};
		depth_attachment.format = VK_FORMAT_D32_SFLOAT;
		depth_attachment.load_op = AttachmentLoadOp_Clear;
		depth_attachment.store_op = AttachmentStoreOp_DontCare;
		depth_attachment.stencil_load_op = AttachmentLoadOp_DontCare;
		depth_attachment.stencil_store_op = AttachmentStoreOp_DontCare;
		depth_attachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.is_depth_attachment = true;

		m_swapchain_render_pass_info.attachments.push_back(color_attachment);
		m_swapchain_render_pass_info.attachments.push_back(depth_attachment);

		RenderSubpassInfo subpass = {};
		subpass.pipeline_bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.color_attachments.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		subpass.depth_attachment = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		m_swapchain_render_pass_info.subpasses.push_back(subpass);

		RenderPassDepencyInfo dependency_color = {};
		dependency_color.src_subpass = VK_SUBPASS_EXTERNAL;
		dependency_color.dst_subpass = 0;
		dependency_color.src_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency_color.dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency_color.src_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency_color.dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		dependency_color.dependency_flags = 0;

		RenderPassDepencyInfo dependency_depth = {};
		dependency_depth.src_subpass = VK_SUBPASS_EXTERNAL;
		dependency_depth.dst_subpass = 0;
		dependency_depth.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency_depth.dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency_depth.src_access_mask = 0;
		dependency_depth.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependency_depth.dependency_flags = 0;

		m_swapchain_render_pass_info.dependencies.push_back(dependency_color);
		m_swapchain_render_pass_info.dependencies.push_back(dependency_depth);

		m_swapchain_render_pass_info.type = RenderPassType_Swapchain;

		m_swapchain_render_pass = m_backend->CreateRenderPass(m_swapchain_render_pass_info);
	}

	m_backend->InitImGuiForRenderpass(m_swapchain_render_pass);
}

void TestOffscreen::CreatePipeline()
{
	// Shaded
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
		},
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

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_Back;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.line_width = 1.0f;

	pipeline_info.renderpass_rendering_info = {
		m_swapchain_render_pass->GetHandle(),
		0
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "offscreen_phong.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "offscreen_phong.frag");

	m_shaded_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Offscreen
	pipeline_info.cull_mode = PipelineCullMode_Front;

	pipeline_info.renderpass_rendering_info = {
		m_offscreen_render_pass->GetHandle(),
		0
	};

	m_offscreen_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Mirror
	pipeline_info.cull_mode = PipelineCullMode_None;

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
		},
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment }
		}
	};

	pipeline_info.renderpass_rendering_info = {
		m_swapchain_render_pass->GetHandle(),
		0
	};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "offscreen_mirror.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "offscreen_mirror.frag");

	m_mirror_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Debug
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};

	pipeline_info.vertex_attribute_descriptions = {};
	pipeline_info.vertex_binding_description = {};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "offscreen_quad.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "offscreen_quad.frag");

	m_debug_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
}

void TestOffscreen::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
	}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_shaded_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Chinese dragon descriptor
	{
		auto data = ObjectData();
		data.model = glm::mat4(1.0f);
		m_chinese_dragon_data = data;

		m_chinese_dragon_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_chinese_dragon_descriptor = m_backend->CreateDescriptorSet(m_shaded_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_chinese_dragon_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_chinese_dragon_descriptor, descriptor_data_objects);
	}

	// Plane descriptor
	{
		auto data = ObjectData();
		data.model = glm::mat4(1.0f);
		m_plane_data = data;

		m_plane_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_plane_descriptor = m_backend->CreateDescriptorSet(m_mirror_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_plane_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_plane = { descriptor_data };
		m_backend->WriteDescriptor(&m_plane_descriptor, descriptor_data_plane);
	}

	// Light descriptor
	{
		auto data = LightData();
		data.position = glm::vec4(0.0f, 5.0f, 0.0f, 1.0f);
		m_light_data = data;

		m_light_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightData));
		m_light_descriptor = m_backend->CreateDescriptorSet(m_shaded_pipeline, 2);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_light_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(LightData);

		std::vector<DescriptorSetWriteData> descriptor_data_light = { descriptor_data };
		m_backend->WriteDescriptor(&m_light_descriptor, descriptor_data_light);
	}

	// Offscreen descriptor
	{
		auto data = ObjectData();
		data.model = glm::mat4(1.0f);
		m_chinese_dragon_data = data;

		m_offscreen_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_offscreen_descriptor = m_backend->CreateDescriptorSet(m_offscreen_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_offscreen_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_offscreen_descriptor, descriptor_data_objects);
	}

	// Mirror descriptor
	{
		m_mirror_descriptor = m_backend->CreateDescriptorSet(m_mirror_pipeline, 2);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_views = m_offscreen_color_texture->GetImageView();
		descriptor_data.data.image.samplers = m_offscreen_color_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_mirror = { descriptor_data };
		m_backend->WriteDescriptor(&m_mirror_descriptor, descriptor_data_mirror);
	}

	// Debug descriptor
	{
		m_debug_descriptor = m_backend->CreateDescriptorSet(m_debug_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_views = m_offscreen_color_texture->GetImageView();
		descriptor_data.data.image.samplers = m_offscreen_color_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_debug = { descriptor_data };
		m_backend->WriteDescriptor(&m_debug_descriptor, descriptor_data_debug);
	}
}
#endif // _TEST_BED_ENABLED