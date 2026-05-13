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
	// Update grid camera uniform buffer before rendering
	m_backend->UpdateUniformBuffer(m_grid_buffer, &m_grid_data, sizeof(GridCameraData));
}

void EditorViewport::RenderUpdate()
{
	// Execute the entire render graph - viewport pass then grid pass
	if (m_render_graph)
	{
		m_render_graph->Execute(m_backend);
	}
}

void EditorViewport::RenderEnd()
{
	// Render graph handles everything now
}

void EditorViewport::SetViewportClearColor(const glm::vec4& color)
{
	m_clear_color = color;

	// Update the resource clear value in the render graph
	if (m_render_graph)
	{
		auto* color_resource = m_render_graph->GetResource("viewport_color");
		if (color_resource)
		{
			color_resource->clear_value.color = { m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a };
		}
	}
}

void EditorViewport::SetViewportClearDepth(const glm::vec2& depth)
{
	m_depth_color = depth;

	// Update the resource clear value in the render graph
	if (m_render_graph)
	{
		auto* depth_resource = m_render_graph->GetResource("viewport_depth");
		if (depth_resource)
		{
			depth_resource->clear_value.depthStencil = { m_depth_color.r, (u32)m_depth_color.g };
		}
	}
}

void EditorViewport::SetViewportEditorReferences(MultiProjectionCamera* camera)
{
	m_editor_camera = camera;
}

void EditorViewport::CreateViewportResources()
{
	CreatePipelines();
	SetupRenderGraph();
	CreateDescriptors();
}

void EditorViewport::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};

	pipeline_info.type = PipelineType_Graphics;
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

	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_GRID, pipeline_info, shader_info);
}

void EditorViewport::SetupRenderGraph()
{
	m_render_graph = std::make_unique<RenderGraph>();

	// ==========================================
	// STEP 1: CREATE PHYSICAL TEXTURES
	// ==========================================

	m_viewport_color_texture = m_frontend->CreateTexture2D(VIEWPORT_COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, m_size.x, m_size.y);
	m_viewport_pick_texture = m_frontend->CreateTexture2D(VIEWPORT_PICK, VK_FORMAT_R32_UINT, m_size.x, m_size.y);
	m_viewport_depth_texture = m_frontend->CreateTexture2D(VIEWPORT_DEPTH, VK_FORMAT_D32_SFLOAT, m_size.x, m_size.y);

	// ==========================================
	// STEP 2: IMPORT RESOURCES INTO RENDER GRAPH
	// ==========================================

	ImportPhysicalResources();

	// ==========================================
	// STEP 3: DEFINE VIEWPORT MAIN PASS
	// ==========================================

	RenderPassDescriptor viewport_pass{};
	viewport_pass.name = "viewport_main";
	viewport_pass.execution_order = 0;

	// Bind color attachments (viewport color + pick)
	// Using UNDEFINED as initial_layout with LOAD_OP_CLEAR is valid and tells Vulkan
	// we don't care about previous contents - it will transition from any layout
	viewport_pass.color_attachments.push_back({
		"viewport_color",
		ResourceUsage::ColorAttachment,
		VK_ATTACHMENT_LOAD_OP_CLEAR,  // CLEAR allows UNDEFINED initial_layout
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_UNDEFINED,  // Don't care about previous contents, will clear anyway
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Transition to COLOR_ATTACHMENT for rendering
	});

	viewport_pass.color_attachments.push_back({
		"viewport_pick",
		ResourceUsage::ColorAttachment,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // For potential reading later
	});

	// Bind depth attachment  
	viewport_pass.depth_attachment = PassResourceBinding{
		"viewport_depth",
		ResourceUsage::DepthAttachment,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_UNDEFINED,  // CLEAR allows UNDEFINED initial_layout
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
	};

	// Execution callback - render the scene
	viewport_pass.execute = [this](VulkanBackend* backend, const RenderPassDescriptor& pass) {
		// Render scene meshes with PBR pipeline
		if (m_scene_descriptor)
		{
			backend->BindPipeline(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR));
			backend->BindDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR), m_scene_descriptor);
			MeshManager::GetInstance()->DrawMeshes(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR));
		}
	};

	m_render_graph->AddPass(viewport_pass);

	// ==========================================
	// STEP 4: DEFINE GRID OVERLAY PASS
	// ==========================================

	RenderPassDescriptor grid_pass{};
	grid_pass.name = "viewport_grid";
	grid_pass.execution_order = 1;

	// Grid renders on top of viewport color (LOAD previous content)
	grid_pass.color_attachments.push_back({
		"viewport_color",
		ResourceUsage::ColorAttachment,
		VK_ATTACHMENT_LOAD_OP_LOAD,      // IMPORTANT: Load existing content from viewport pass
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // Previous pass left it in this layout
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL   // Final transition for ImGui
	});

	// Use depth from previous pass
	grid_pass.depth_attachment = PassResourceBinding{
		"viewport_depth",
		ResourceUsage::DepthAttachment,
		VK_ATTACHMENT_LOAD_OP_LOAD,      // Load depth from viewport pass
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,  // Previous pass left it in this layout
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	// Grid depends on viewport pass
	grid_pass.dependencies.push_back({
		"viewport_main",
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	});

	// Grid execution callback
	grid_pass.execute = [this](VulkanBackend* backend, const RenderPassDescriptor& pass) {
		// Bind pipeline and descriptor set
		backend->BindPipeline(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GRID));
		backend->BindDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GRID), m_grid_descriptor);

		// Draw grid (fullscreen quad)
		backend->Draw(6, 1, 0, 0);
	};

	m_render_graph->AddPass(grid_pass);

	// ==========================================
	// STEP 5: BUILD THE GRAPH
	// ==========================================

	auto result = m_render_graph->Build();
	if (result != GraphBuildResult::Success)
	{
		HE_CORE_ERROR("Failed to build render graph for EditorViewport: {}", (u32)result);
	}
}

void EditorViewport::ImportPhysicalResources()
{
	// Import viewport color texture
	m_render_graph->ImportResource(
		"viewport_color",
		m_viewport_color_texture->GetHandle(),
		m_viewport_color_texture->GetImageView(),
		m_viewport_color_texture->GetFormat(),
		{ m_size.x, m_size.y },
		ResourceUsage::ColorAttachment
	);

	HE_CORE_INFO("Imported viewport_color: image=0x{:x}, view=0x{:x}", 
		(u64)m_viewport_color_texture->GetHandle(), 
		(u64)m_viewport_color_texture->GetImageView());

	// Set clear value for color
	auto* color_resource = m_render_graph->GetResource("viewport_color");
	if (color_resource)
	{
		color_resource->clear_value.color = { m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a };
	}

	// Import viewport pick texture
	m_render_graph->ImportResource(
		"viewport_pick",
		m_viewport_pick_texture->GetHandle(),
		m_viewport_pick_texture->GetImageView(),
		m_viewport_pick_texture->GetFormat(),
		{ m_size.x, m_size.y },
		ResourceUsage::ColorAttachment
	);

	// Set clear value for pick (black)
	auto* pick_resource = m_render_graph->GetResource("viewport_pick");
	if (pick_resource)
	{
		pick_resource->clear_value.color = { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	// Import viewport depth texture
	m_render_graph->ImportResource(
		"viewport_depth",
		m_viewport_depth_texture->GetHandle(),
		m_viewport_depth_texture->GetImageView(),
		m_viewport_depth_texture->GetFormat(),
		{ m_size.x, m_size.y },
		ResourceUsage::DepthAttachment
	);

	// Set clear value for depth
	auto* depth_resource = m_render_graph->GetResource("viewport_depth");
	if (depth_resource)
	{
		depth_resource->clear_value.depthStencil = { m_depth_color.r, (u32)m_depth_color.g };
	}

	HE_CORE_INFO("EditorViewport: Resources imported into render graph");
}

void EditorViewport::CreateDescriptors()
{
	// Viewport descriptor for ImGui display
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

	// Grid uniform buffer and descriptor
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

void EditorViewport::UpdateGridCameraData()
{
	m_grid_data.view = m_editor_camera->GetView();
	m_grid_data.proj = m_editor_camera->GetProjection();
	m_grid_data.pos = m_editor_camera->GetPosition();
}

void EditorViewport::OnViewportResize()
{
	// Destroy old textures
	m_frontend->DestroyTexture2D(VIEWPORT_COLOR);
	m_frontend->DestroyTexture2D(VIEWPORT_PICK);
	m_frontend->DestroyTexture2D(VIEWPORT_DEPTH);

	// Recreate textures with new size
	m_viewport_color_texture = m_frontend->CreateTexture2D(VIEWPORT_COLOR, VK_FORMAT_R16G16B16A16_SFLOAT, m_size.x, m_size.y);
	m_viewport_pick_texture = m_frontend->CreateTexture2D(VIEWPORT_PICK, VK_FORMAT_R32_UINT, m_size.x, m_size.y);
	m_viewport_depth_texture = m_frontend->CreateTexture2D(VIEWPORT_DEPTH, VK_FORMAT_D32_SFLOAT, m_size.x, m_size.y);

	// Re-import resources into render graph
	ImportPhysicalResources();

	// Rebuild the render graph with new resources
	if (m_render_graph)
	{
		m_render_graph->Rebuild();
	}

	// Update ImGui descriptor
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
