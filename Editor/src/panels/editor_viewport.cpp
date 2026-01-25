#include "editor_viewport.h"

EditorViewport::EditorViewport() : Viewport("Viewport")
{
	NO_OP;
}

EditorViewport::~EditorViewport()
{
	m_backend->DestroyBuffer(m_grid_buffer);
}

void EditorViewport::Init(VulkanBackend* backend, VulkanFrontend* frontend, EditorHierarchy* hierarchy_panel)
{
	m_backend = backend;
	m_frontend = frontend;
	m_hierarchy_panel = hierarchy_panel;

	m_clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_depth_color = { 1.0f, 0.0f };

	m_can_pick = true;
}

void EditorViewport::RenderBegin()
{
	m_backend->UpdateUniformBuffer(m_grid_buffer, &m_grid_data, sizeof(GridCameraData));

	m_backend->BeginDynamicRenderingWithAttachments(m_viewport_dri);

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_size.x, (f32)m_size.y, 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_size.x, m_size.y } } });
}

void EditorViewport::RenderUpdate()
{
}

void EditorViewport::RenderEnd()
{
	m_backend->EndDynamicRenderingWithAttachments(m_viewport_dri);

	DrawGrid();
}

void EditorViewport::SetViewportClearColor(const glm::vec4& color)
{
	m_clear_color = color;
}

void EditorViewport::SetViewportClearDepth(const glm::vec2& depth)
{
	m_depth_color = depth;
}

void EditorViewport::SetViewportEditorReferences(MultiProjectionCamera* camera)
{
	m_editor_camera = camera;
}

void EditorViewport::CreateViewportResources()
{
	CreatePipelines();
	CreateAttachments();
	CreateDescriptors();
}

void EditorViewport::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};

	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
		}
	};
	pipeline_info.vertex_binding_description = {};
	pipeline_info.vertex_attribute_descriptions = {};

	pipeline_info.push_constant_ranges = {};
	pipeline_info.depth_stencil_info = { true, true, false };

	pipeline_info.dynamic_rendering_info = {
		true,
		{ VK_FORMAT_R16G16B16A16_SFLOAT },
		VK_FORMAT_D32_SFLOAT,
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "grid.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "grid.frag");

	pipeline_info.depth_stencil_info = { true, true };

	//m_grid_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_GRID, pipeline_info, shader_info);
}

void EditorViewport::CreateAttachments()
{
	// Viewport
	{
		m_viewport_color_texture = m_frontend->CreateTexture2D(VIEWPORT_COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, m_size.x, m_size.y);
		m_viewport_pick_texture = m_frontend->CreateTexture2D(VIEWPORT_PICK, VK_FORMAT_R32_UINT, m_size.x, m_size.y);
		m_viewport_depth_texture = m_frontend->CreateTexture2D(VIEWPORT_DEPTH, VK_FORMAT_D32_SFLOAT, m_size.x, m_size.y);

		m_viewport_color_attachment = {
			m_viewport_color_texture->GetHandle(),
			m_viewport_color_texture->GetImageView(),
			m_viewport_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a } },
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		m_viewport_pick_attachment = {
			m_viewport_pick_texture->GetHandle(),
			m_viewport_pick_texture->GetImageView(),
			m_viewport_pick_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { 0.0f, 0.0f, 0.0f, 0.0f } },
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		m_viewport_depth_attachment = {
			m_viewport_depth_texture->GetHandle(),
			m_viewport_depth_texture->GetImageView(),
			m_viewport_depth_texture->GetFormat(),
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { m_depth_color.r, m_depth_color.g } },
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		m_viewport_dri = {
			{ m_viewport_color_attachment, m_viewport_pick_attachment },
			m_viewport_depth_attachment,
			0,
			{ m_size.x, m_size.y }
		};
	}

	// Viewport grid
	{
		m_grid_color_attachment = {
			m_viewport_color_texture->GetHandle(),
			m_viewport_color_texture->GetImageView(),
			m_viewport_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a } },
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		m_grid_depth_attachment = {
			m_viewport_depth_texture->GetHandle(),
			m_viewport_depth_texture->GetImageView(),
			m_viewport_depth_texture->GetFormat(),
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			{ { m_depth_color.r, m_depth_color.g } },
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		m_grid_dri = {
			{ m_grid_color_attachment },
			m_grid_depth_attachment,
			0,
			{ m_size.x, m_size.y }
		};
	}
}

void EditorViewport::CreateDescriptors()
{
	// Viewport
	{
		m_viewport_descriptor = m_backend->CreateDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_EDITOR), 0);

		DescriptorSetWriteData editor_write_data{};
		editor_write_data.type = DescriptorType_CombinedImageSampler;
		editor_write_data.binding = 0;

		editor_write_data.data.image.image_views = new VkImageView[1];
		editor_write_data.data.image.samplers = new VkSampler[1];
		editor_write_data.data.image.image_views[0] = m_viewport_color_texture->GetImageView();
		editor_write_data.data.image.samplers[0] = m_viewport_color_texture->GetSampler();

		std::vector<DescriptorSetWriteData> write_data = { editor_write_data };
		m_backend->WriteDescriptor(&m_viewport_descriptor, write_data);

		m_handle = (void*)m_viewport_descriptor->GetHandle();
	}

	// Viewport grid
	{
		m_grid_data.proj = m_editor_camera->GetProjection();
		m_grid_data.view = m_editor_camera->GetView();
		m_grid_data.pos = m_editor_camera->GetPosition();

		m_grid_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(GridCameraData), 1);
		m_grid_descriptor = m_backend->CreateDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GRID), 0);

		DescriptorSetWriteData data{};

		data.type = DescriptorType_UniformBuffer;
		data.binding = 0;
		data.data.buffer.buffers = new VkBuffer(m_grid_buffer->GetHandle());
		data.data.buffer.offsets = new VkDeviceSize(0);
		data.data.buffer.ranges = new VkDeviceSize(sizeof(GridCameraData));

		std::vector<DescriptorSetWriteData> write_data = { data };
		m_backend->WriteDescriptor(&m_grid_descriptor, write_data);
	}
}

void EditorViewport::DrawGrid()
{
	m_backend->BeginDynamicRenderingWithAttachments(m_grid_dri);

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_size.x, (f32)m_size.y, 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_size.x, m_size.y } } });

	m_backend->BindPipeline(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GRID));
	m_backend->BindDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GRID), m_grid_descriptor);

	m_backend->Draw(6, 1, 0, 0);

	m_backend->EndDynamicRenderingWithAttachments(m_grid_dri);
}

void EditorViewport::UpdateGridCameraData()
{
	m_grid_data.view = m_editor_camera->GetView();
	m_grid_data.proj = m_editor_camera->GetProjection();
	m_grid_data.pos = m_editor_camera->GetPosition();
}

void EditorViewport::OnViewportResize()
{
	m_frontend->DestroyTexture2D(VIEWPORT_COLOR);
	m_frontend->DestroyTexture2D(VIEWPORT_PICK);
	m_frontend->DestroyTexture2D(VIEWPORT_DEPTH);

	m_viewport_color_texture = m_frontend->CreateTexture2D(VIEWPORT_COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, m_size.x, m_size.y);
	m_viewport_pick_texture = m_frontend->CreateTexture2D(VIEWPORT_PICK, VK_FORMAT_R32_UINT, m_size.x, m_size.y);
	m_viewport_depth_texture = m_frontend->CreateTexture2D(VIEWPORT_DEPTH, VK_FORMAT_D32_SFLOAT, m_size.x, m_size.y);

	m_viewport_color_attachment.image = m_viewport_color_texture->GetHandle();
	m_viewport_color_attachment.image_view = m_viewport_color_texture->GetImageView();
	m_viewport_color_attachment.format = m_viewport_color_texture->GetFormat();

	m_viewport_pick_attachment.image = m_viewport_pick_texture->GetHandle();
	m_viewport_pick_attachment.image_view = m_viewport_pick_texture->GetImageView();
	m_viewport_pick_attachment.format = m_viewport_pick_texture->GetFormat();

	m_viewport_depth_attachment.image = m_viewport_depth_texture->GetHandle();
	m_viewport_depth_attachment.image_view = m_viewport_depth_texture->GetImageView();
	m_viewport_depth_attachment.format = m_viewport_depth_texture->GetFormat();

	m_grid_color_attachment.image = m_viewport_color_texture->GetHandle();
	m_grid_color_attachment.image_view = m_viewport_color_texture->GetImageView();
	m_grid_color_attachment.format = m_viewport_color_texture->GetFormat();

	m_grid_depth_attachment.image = m_viewport_depth_texture->GetHandle();
	m_grid_depth_attachment.image_view = m_viewport_depth_texture->GetImageView();
	m_grid_depth_attachment.format = m_viewport_depth_texture->GetFormat();

	m_viewport_dri = {
		{ m_viewport_color_attachment, m_viewport_pick_attachment },
		m_viewport_depth_attachment,
		0,
		{ m_size.x, m_size.y }
	};

	m_grid_dri = {
		{ m_grid_color_attachment },
		m_grid_depth_attachment,
		0,
		{ m_size.x, m_size.y }
	};

	DescriptorSetWriteData editor_write_data{};
	editor_write_data.type = DescriptorType_CombinedImageSampler;
	editor_write_data.binding = 0;

	editor_write_data.data.image.image_views = new VkImageView(m_viewport_color_texture->GetImageView());
	editor_write_data.data.image.samplers = new VkSampler(m_viewport_color_texture->GetSampler());

	std::vector<DescriptorSetWriteData> data = { editor_write_data };
	m_backend->WriteDescriptor(&m_viewport_descriptor, data);
}

void EditorViewport::OnMouseButtonPressed()
{
	if (!m_can_pick)
	{
		return;
	}

	auto mouse_pos = Input::GetMousePosition();
	i32 mx = (i32)mouse_pos.GetFirst();
	i32 my = (i32)mouse_pos.GetSecond();

	mx -= (i32)m_bounds.min.x;
	my -= (i32)m_bounds.min.y;

	auto viewport_width = m_bounds.max.x - m_bounds.min.x;
	auto viewport_height = m_bounds.max.y - m_bounds.min.y;

	i32 mouse_x = mx;
	i32 mouse_y = my;

	if (mouse_x >= 0 && mouse_y >= 0 && mouse_x < (int)viewport_width && mouse_y < (int)viewport_height)
	{
		i32 id = m_backend->ReadPixel<i32>(m_viewport_pick_texture, (u32)mouse_x, (u32)mouse_y, 0, 0) - 1;

		m_hierarchy_panel->SetSelectedGameObject({ (EntityId)(id), SceneManager::GetInstance()->GetActiveScene() });
	}
}
