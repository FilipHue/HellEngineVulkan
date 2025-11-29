#include "editor.h"

// External
#include <imguizmo/ImGuizmo.h>

Editor::Editor(ApplicationConfiguration* configuration) : Application(configuration)
{
	NO_OP;
}

void Editor::Init()
{
	EventDispatcher::AddListener(EventType_WindowClose, HE_BIND_EVENTCALLBACK(OnEventWindowClose));
	EventDispatcher::AddListener(EventType_WindowResize, HE_BIND_EVENTCALLBACK(OnEventWindowResize));
	EventDispatcher::AddListener(EventType_WindowFocus, HE_BIND_EVENTCALLBACK(OnEventWindowFocus));
	EventDispatcher::AddListener(EventType_WindowIconified, HE_BIND_EVENTCALLBACK(OnEventWindowIconified));
	EventDispatcher::AddListener(EventType_WindowMoved, HE_BIND_EVENTCALLBACK(OnEventWindowMoved));

	EventDispatcher::AddListener(EventType_KeyPressed, HE_BIND_EVENTCALLBACK(OnEventKeyPressed));
	EventDispatcher::AddListener(EventType_KeyReleased, HE_BIND_EVENTCALLBACK(OnEventKeyReleased));

	EventDispatcher::AddListener(EventType_MouseButtonPressed, HE_BIND_EVENTCALLBACK(OnEventMouseButtonPressed));
	EventDispatcher::AddListener(EventType_MouseButtonReleased, HE_BIND_EVENTCALLBACK(OnEventMouseButtonReleased));
	EventDispatcher::AddListener(EventType_MouseMoved, HE_BIND_EVENTCALLBACK(OnEventMouseMoved));
	EventDispatcher::AddListener(EventType_MouseScrolled, HE_BIND_EVENTCALLBACK(OnEventMouseScrolled));

	CreateResources();
	CreatePipelines();
	CreateAttachments();
	CreateDescriptors();

	CreateEditorPanels();

	LoadResourcesForTest();
}

void Editor::OnProcessUpdate(f32 delta_time)
{
	if (m_editor_camera_controller.IsActive())
	{
		m_editor_camera_controller.OnProcessUpdate(delta_time);
		m_global_shader_data.view = m_editor_camera.GetView();
		m_global_shader_data.camera_position = m_editor_camera.GetPosition();

		m_grid_data.view = m_editor_camera.GetView();
		m_grid_data.pos = m_editor_camera.GetPosition();
	}

	if (SceneManager::GetInstance()->GetActiveScene())
	{
		SceneManager::GetInstance()->GetActiveScene()->UpdateTransforms();
	}

	MeshManager::GetInstance()->CleanUp();
	MeshManager::GetInstance()->UpdatePerDrawData();
}

void Editor::OnRenderBegin()
{
	glm::uvec2& viewport_panel_size = m_viewport_panel->GetSize();
	if (viewport_panel_size != m_viewport_size && viewport_panel_size.x && viewport_panel_size.y)
	{
		m_viewport_size = m_viewport_panel->GetSize();
		m_editor_camera.SetAspect((f32)m_viewport_size.x, (f32)m_viewport_size.y);
		m_global_shader_data.proj = m_editor_camera.GetProjection();
		m_grid_data.proj = m_editor_camera.GetProjection();

		OnViewportResize();
	}

	m_backend->UpdateUniformBuffer(m_grid_buffer, &m_grid_data, sizeof(GridCameraData));
	m_backend->UpdateUniformBuffer(m_test_ubo, &m_global_shader_data, sizeof(GlobalShaderData));
}

void Editor::OnRenderUpdate()
{
	m_backend->BeginDynamicRenderingWithAttachments(m_viewport_dri);

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_viewport_size.x, (f32)m_viewport_size.y, 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_viewport_size.x, m_viewport_size.y } } });

	m_backend->BindPipeline(m_test_pipeline);
	m_backend->BindDescriptorSet(m_test_pipeline, m_test_descriptor);
	
	MeshManager::GetInstance()->DrawMeshes(m_test_pipeline);

	m_backend->EndDynamicRenderingWithAttachments(m_viewport_dri);
}

void Editor::OnRenderEnd()
{
	DrawGrid();
	DrawToSwapchain();
}

void Editor::OnUIRender()
{
	m_ui->BeginDocking();

	if (m_hierarchy_panel->Begin())
	{
		m_hierarchy_panel->Draw();
		m_hierarchy_panel->End();
	}

	if (m_inspector_panel->Begin())
	{
		m_inspector_panel->Draw();
		m_inspector_panel->End();
	}

	if (m_viewport_panel->Begin())
	{
		m_viewport_panel->Draw();
		ShowGuizmo();
		m_viewport_panel->End();
	}

	m_viewport_panel_bounds = m_viewport_panel->GetBounds();
	m_viewport_panel_size = m_viewport_panel->GetSize();

	MenuBar();

	m_ui->EndDocking();
}

void Editor::Shutdown()
{
	m_backend->DestroyPipeline(m_editor_pipeline);

	m_backend->DestroyPipeline(m_grid_pipeline);
	m_backend->DestroyBuffer(m_grid_buffer);

	m_backend->DestroyBuffer(m_test_ubo);
	m_backend->DestroyPipeline(m_test_pipeline);

	delete m_hierarchy_panel;
}

b8 Editor::OnEventWindowClose(EventContext& event)
{
	m_running = false;

	return true;
}

b8 Editor::OnEventWindowResize(EventContext& event)
{
	if (event.data.window_resize.width == 0 || event.data.window_resize.height == 0)
	{
		return false;
	}
	m_window->SetSize(event.data.window_resize.width, event.data.window_resize.height);
	m_backend->OnFramebufferResize();

	return false;
}

b8 Editor::OnEventWindowFocus(EventContext& event)
{
	return false;
}

b8 Editor::OnEventWindowIconified(EventContext& event)
{
	m_suspended = event.data.window_iconified.is_iconified;

	return false;
}

b8 Editor::OnEventWindowMoved(EventContext& event)
{
	return false;
}

b8 Editor::OnEventKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_ESCAPE)
	{
		m_running = false;
	}

	return false;
}

b8 Editor::OnEventKeyReleased(EventContext& event)
{
	return false;
}

b8 Editor::OnEventMouseButtonPressed(EventContext& event)
{
	if (event.data.mouse_button_event.button == MOUSE_BUTTON_RIGHT && m_viewport_panel->IsHovered())
	{
		m_editor_camera_controller.SetActive(true);
		m_window->SetCursorMode(GLFW_CURSOR_DISABLED);
	}

	if (event.data.mouse_button_event.button == MOUSE_BUTTON_RIGHT && m_hierarchy_panel->IsHovered())
	{
		m_editor_camera_controller.SetActive(false);
		m_window->SetCursorMode(GLFW_CURSOR_NORMAL);
	}

	if (event.data.mouse_button_event.button == MOUSE_BUTTON_LEFT && m_viewport_panel->IsFocused() && m_viewport_panel->IsHovered() && SceneManager::GetInstance()->GetActiveScene())
	{
		auto mouse_pos = Input::GetMousePosition();
		i32 mx = (i32)mouse_pos.GetFirst();
		i32 my = (i32)mouse_pos.GetSecond();

		mx -= (i32)m_viewport_panel_bounds.min.x;
		my -= (i32)m_viewport_panel_bounds.min.y;

		auto viewport_width = m_viewport_panel_bounds.max.x - m_viewport_panel_bounds.min.x;
		auto viewport_height = m_viewport_panel_bounds.max.y - m_viewport_panel_bounds.min.y;

		i32 mouse_x = mx;
		i32 mouse_y = (i32)viewport_height - my;

		if (mouse_x >= 0 && mouse_y >= 0 && mouse_x < (int)viewport_width && mouse_y < (int)viewport_height)
		{
			i32 id = m_backend->ReadPixel<i32>(m_viewport_pick_texture, (u32)mouse_x, (u32)mouse_y, 0, 0);

			HE_CLIENT_DEBUG("Picked ID: {}", id);
			m_inspector_panel->SetSelectedEntity({ (EntityId)(id), SceneManager::GetInstance()->GetActiveScene() });
		}
	}

	return false;
}

b8 Editor::OnEventMouseButtonReleased(EventContext& event)
{
	if (event.data.mouse_button_event.button == MOUSE_BUTTON_RIGHT && m_editor_camera_controller.IsActive())
	{
		m_editor_camera_controller.SetActive(false);
		m_window->SetCursorMode(GLFW_CURSOR_NORMAL);
	}

	return false;
}

b8 Editor::OnEventMouseMoved(EventContext& event)
{
	return false;
}

b8 Editor::OnEventMouseScrolled(EventContext& event)
{
	return false;
}

void Editor::CreateResources()
{
	// Init descriptor pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 100 },
		{ DescriptorType_StorageBuffer, 100 },
		{ DescriptorType_CombinedImageSampler, 100 }
		}, 100);

	// Camera
	m_editor_camera = MultiProjectionCamera();
	m_editor_camera.CreatePerspective(60.0f, (f32)m_window->GetWidth() / (f32)m_window->GetHeight(), 0.1f, 1000.0f);

	m_editor_camera.SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

	m_editor_camera_controller = MultiProjectionController();
	m_editor_camera_controller.Init();
	m_editor_camera_controller.SetCamera(&m_editor_camera);
	m_editor_camera_controller.SetActive(false);

	// Viewport
	m_viewport_clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_viewport_clear_depth = { 1.0f, 0.0f };

	m_viewport_size = { m_window->GetWidth(), m_window->GetHeight() };
}

void Editor::CreatePipelines()
{
	// Editor
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
				{ { 0, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			}
		};
		pipeline_info.vertex_binding_description = {};
		pipeline_info.vertex_attribute_descriptions = {};

		pipeline_info.push_constant_ranges = {};
		pipeline_info.depth_stencil_info = { true, true, false };

		pipeline_info.dynamic_rendering_info = {
			false,
			{ VK_FORMAT_B8G8R8A8_UNORM },
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		ShaderStageInfo shader_info = {};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "screen.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "screen.frag");

		m_editor_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
		m_backend->InitImGuiForDynamicRendering(m_editor_pipeline->GetRenderingCreateInfo());
	}

	// Viewport grid
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
			{ VK_FORMAT_B8G8R8A8_SRGB },
			VK_FORMAT_D32_SFLOAT,
		};

		ShaderStageInfo shader_info = {};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "grid.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "grid.frag");

		pipeline_info.depth_stencil_info = { true, true };

		m_grid_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
	}
}

void Editor::CreateAttachments()
{
	// Viewport
	{
		m_viewport_color_texture = m_frontend->CreateTexture2D(VIEWPORT_COLOR, VK_FORMAT_B8G8R8A8_SRGB, m_viewport_size.x, m_viewport_size.y);
		m_viewport_pick_texture = m_frontend->CreateTexture2D(VIEWPORT_PICK, VK_FORMAT_R32_UINT, m_viewport_size.x, m_viewport_size.y);
		m_viewport_depth_texture = m_frontend->CreateTexture2D(VIEWPORT_DEPTH, VK_FORMAT_D32_SFLOAT, m_viewport_size.x, m_viewport_size.y);

		m_viewport_color_attachment = {
			m_viewport_color_texture->GetHandle(),
			m_viewport_color_texture->GetImageView(),
			m_viewport_color_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { m_viewport_clear_color.r, m_viewport_clear_color.g, m_viewport_clear_color.b, m_viewport_clear_color.a } }
		};

		m_viewport_pick_attachment = {
			m_viewport_pick_texture->GetHandle(),
			m_viewport_pick_texture->GetImageView(),
			m_viewport_pick_texture->GetFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { 0.0f, 0.0f, 0.0f, 0.0f } }
		};

		m_viewport_depth_attachment = {
			m_viewport_depth_texture->GetHandle(),
			m_viewport_depth_texture->GetImageView(),
			m_viewport_depth_texture->GetFormat(),
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			{ { m_viewport_clear_depth.r, m_viewport_clear_depth.g } }
		};

		m_viewport_dri = {
			{ m_viewport_color_attachment, m_viewport_pick_attachment },
			m_viewport_depth_attachment,
			0,
			{ m_viewport_size.x, m_viewport_size.y }
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
			{ { m_viewport_clear_color.r, m_viewport_clear_color.g, m_viewport_clear_color.b, m_viewport_clear_color.a } }
		};

		m_grid_depth_attachment = {
			m_viewport_depth_texture->GetHandle(),
			m_viewport_depth_texture->GetImageView(),
			m_viewport_depth_texture->GetFormat(),
			VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			{ { m_viewport_clear_depth.r, m_viewport_clear_depth.g } }
		};

		m_grid_dri = {
			{ m_grid_color_attachment },
			m_grid_depth_attachment,
			0,
			{ m_viewport_size.x, m_viewport_size.y }
		};
	}
}

void Editor::CreateDescriptors()
{
	// Viewport
	{
		m_viewport_descriptor = m_backend->CreateDescriptorSet(m_editor_pipeline, 0);

		DescriptorSetWriteData editor_write_data{};
		editor_write_data.type = DescriptorType_CombinedImageSampler;
		editor_write_data.binding = 0;

		editor_write_data.data.image.image_views = new VkImageView[1];
		editor_write_data.data.image.samplers = new VkSampler[1];
		editor_write_data.data.image.image_views[0] = m_viewport_color_texture->GetImageView();
		editor_write_data.data.image.samplers[0] = m_viewport_color_texture->GetSampler();

		std::vector<DescriptorSetWriteData> write_data = { editor_write_data };
		m_backend->WriteDescriptor(&m_viewport_descriptor, write_data);
	}

	// Viewport grid
	{
		m_grid_data.proj = m_editor_camera.GetProjection();
		m_grid_data.view = m_editor_camera.GetView();
		m_grid_data.pos = m_editor_camera.GetPosition();

		m_grid_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(GridCameraData), 1);
		m_grid_descriptor = m_backend->CreateDescriptorSet(m_grid_pipeline, 0);

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

void Editor::CreateEditorPanels()
{
	m_viewport_panel = new Viewport("Viewport");
	m_viewport_panel->SetHandle(m_viewport_descriptor->GetHandle());

	m_inspector_panel = new EditorInspector();
	m_inspector_panel->Init();

	m_hierarchy_panel = new EditorHierarchy();
	m_hierarchy_panel->Init(m_inspector_panel);
}

void Editor::OnViewportResize()
{
	m_frontend->DestroyTexture2D(VIEWPORT_COLOR);
	m_frontend->DestroyTexture2D(VIEWPORT_PICK);
	m_frontend->DestroyTexture2D(VIEWPORT_DEPTH);

	m_viewport_color_texture = m_frontend->CreateTexture2D(VIEWPORT_COLOR, VK_FORMAT_B8G8R8A8_SRGB, m_viewport_size.x, m_viewport_size.y);
	m_viewport_pick_texture = m_frontend->CreateTexture2D(VIEWPORT_PICK, VK_FORMAT_R32_UINT, m_viewport_size.x, m_viewport_size.y);
	m_viewport_depth_texture = m_frontend->CreateTexture2D(VIEWPORT_DEPTH, VK_FORMAT_D32_SFLOAT, m_viewport_size.x, m_viewport_size.y);

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
		{ m_viewport_size.x, m_viewport_size.y }
	};

	m_grid_dri = {
		{ m_grid_color_attachment },
		m_grid_depth_attachment,
		0,
		{ m_viewport_size.x, m_viewport_size.y }
	};

	DescriptorSetWriteData editor_write_data{};
	editor_write_data.type = DescriptorType_CombinedImageSampler;
	editor_write_data.binding = 0;

	editor_write_data.data.image.image_views = new VkImageView(m_viewport_color_texture->GetImageView());
	editor_write_data.data.image.samplers = new VkSampler(m_viewport_color_texture->GetSampler());

	std::vector<DescriptorSetWriteData> data = { editor_write_data };
	m_backend->WriteDescriptor(&m_viewport_descriptor, data);
}

void Editor::DrawGrid()
{
	m_backend->BeginDynamicRenderingWithAttachments(m_grid_dri);

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_viewport_size.x, (f32)m_viewport_size.y, 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_viewport_size.x, m_viewport_size.y } } });

	m_backend->BindPipeline(m_grid_pipeline);
	m_backend->BindDescriptorSet(m_grid_pipeline, m_grid_descriptor);

	m_backend->Draw(6, 1, 0, 0);

	m_backend->EndDynamicRenderingWithAttachments(m_grid_dri);
}

void Editor::DrawToSwapchain()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->EndDynamicRendering();
}

void Editor::MenuBar()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { style.ItemSpacing.x * 2.0f, style.ItemSpacing.y });

	if (ImGui::BeginMenuBar())
	{
		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		if (ImGui::BeginMenu("File"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));
			if (ImGui::MenuItem("	New Scene", "Ctrl + N"))
			{
				SceneManager::GetInstance()->CreateScene("Untitled");
			}

			if (ImGui::MenuItem("	Open Scene", "Ctrl + O"))
			{
			}

			ImGui::Separator();

			if (ImGui::MenuItem("	Save", "Ctrl + S"))
			{
			}

			if (ImGui::MenuItem("	Save As", "Ctrl + Shift + S"))
			{
			}

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		if (ImGui::BeginMenu("Assets"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));

			ImGui::Separator();

			if (ImGui::MenuItem("	Import New Asset"))
			{
				if (SceneManager::GetInstance()->GetActiveScene() == nullptr)
				{
					SceneManager::GetInstance()->CreateScene("Untitled");
				}

				File file = FileManager::OpenFile("Model Files (*.gltf)\0*.gltf\0Model Files (*.glb)\0*.glb\0All Files (*.*)\0*.*\0");
				AssetManager::LoadModel(file);
				MeshManager::GetInstance()->UploadToGpu(m_test_pipeline, 1, TextureType_Diffuse | TextureType_Ambient | TextureType_Specular);
			}

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		if (ImGui::BeginMenu("GameObject"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));
			if (ImGui::MenuItem("	Create Empty", "Ctrl + Shift + N"))
			{
				if (SceneManager::GetInstance()->GetActiveScene() == nullptr)
				{
					SceneManager::GetInstance()->CreateScene("Untitled");
				}

				SceneManager::GetInstance()->GetActiveScene()->CreateGameObject("GameObject", m_hierarchy_panel->GetSelectedGameObject());
			}

			ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::PopStyleVar();
}

void Editor::ShowGuizmo()
{
	Entity selected = m_hierarchy_panel->GetSelectedGameObject();
	if (!selected) return;

	ImGuiWindow* vp = ImGui::FindWindowByName("Viewport");
	if (!vp) return;

	ImGuizmo::BeginFrame();
	ImGuizmo::SetDrawlist(vp->DrawList);

	ImVec2 min = ImVec2(
		vp->Pos.x + vp->WindowPadding.x,
		vp->Pos.y + vp->WindowPadding.y);

	ImVec2 size = ImVec2(
		vp->Size.x - vp->WindowPadding.x * 2.0f,
		vp->Size.y - vp->WindowPadding.y * 2.0f);

	ImGuizmo::SetRect(min.x, min.y, size.x, size.y);

	glm::mat4 view = m_editor_camera.GetView();
	glm::mat4 proj = m_editor_camera.GetProjection();
	proj[1][1] *= -1.0f; // Invert Y axis for ImGuizmo

	auto& tc = selected.GetComponent<TransformComponent>();
	glm::mat4 model = tc.world_transform;

	bool  snapOn = Input::IsKeyPressed(KEY_LEFT_CONTROL);
	float snap[3]{ 0.5f, 0.5f, 0.5f };

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::Manipulate(glm::value_ptr(view),
		glm::value_ptr(proj),
		ImGuizmo::TRANSLATE, ImGuizmo::LOCAL,
		glm::value_ptr(model),
		nullptr,
		snapOn ? snap : nullptr);

	if (ImGuizmo::IsUsing())
	{
		glm::mat4 parentWorld(1.0f);
		auto& rel = selected.GetComponent<RelationshipComponent>();
		if (rel.parent != entt::null)
			parentWorld = SceneManager::GetInstance()->GetActiveScene()->GetRegistry().get<TransformComponent>(rel.parent).world_transform;

		glm::mat4 newLocal = glm::inverse(parentWorld) * model;

		glm::vec3 t, s, skew;  glm::quat r;  glm::vec4 persp;
		glm::decompose(newLocal, s, r, t, skew, persp);

		tc.local_position = t;
		tc.local_rotation = glm::eulerAngles(r);
		tc.local_scale = s;
		tc.is_dirty = true;
	}
}

void Editor::LoadResourcesForTest()
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
			{ 
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_None }
			},
			DescriptorSetFlags_None
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_None },
			},
			DescriptorSetFlags_None
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_None },
			},
			DescriptorSetFlags_None
		},
		{
			{
				{ 0, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		}
	};
	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	pipeline_info.depth_stencil_info = { true, true };

	//pipeline_info.push_constant_ranges = {
	//	{ ShaderStage_Vertex | ShaderStage_Fragment, 0, sizeof(glm::mat4) + 2 * sizeof(u32) }
	//};

	pipeline_info.dynamic_rendering_info = {
		false,
		{ VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R32_UINT },
		VK_FORMAT_D32_SFLOAT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "test.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "test.frag");

	m_test_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_test_descriptor = m_backend->CreateDescriptorSet(m_test_pipeline, 0);

	m_global_shader_data = {};
	m_global_shader_data.proj = m_editor_camera.GetProjection();
	m_global_shader_data.view = m_editor_camera.GetView();
	m_global_shader_data.camera_position = m_editor_camera.GetPosition();

	m_test_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(GlobalShaderData), 1);

	DescriptorSetWriteData data1{};
	data1.type = DescriptorType_UniformBuffer;
	data1.binding = 0;
	data1.data.buffer.buffers = new VkBuffer(m_test_ubo->GetHandle());
	data1.data.buffer.offsets = new VkDeviceSize(0);
	data1.data.buffer.ranges = new VkDeviceSize(sizeof(GlobalShaderData));

	std::vector<DescriptorSetWriteData> write_data = { data1 };
	m_backend->WriteDescriptor(&m_test_descriptor, write_data);

	MeshManager::GetInstance()->CreateMeshResource(m_test_pipeline, 1);
}
