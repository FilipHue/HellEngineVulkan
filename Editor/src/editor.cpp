#include "editor.h"

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
	CreateDescriptors();

	CreateEditorPanels();

	LoadResourcesForTest();

	m_viewport_last_size = m_viewport_panel->GetSize();
}

void Editor::OnProcessUpdate(f32 delta_time)
{
	if (m_editor_camera_controller.IsActive())
	{
		m_editor_camera_controller.OnProcessUpdate(delta_time);
		m_global_shader_data.view = m_editor_camera.GetView();
		m_global_shader_data.camera_position = m_editor_camera.GetPosition();
		m_viewport_panel->UpdateGridCameraData();
	}

	if (SceneManager::GetInstance()->GetActiveScene())
	{
		SceneManager::GetInstance()->GetActiveScene()->UpdateTransforms();
	}

	MeshManager::GetInstance()->UpdatePerDrawData();
}

void Editor::OnRenderBegin()
{
	glm::uvec2& viewport_current_size = m_viewport_panel->GetSize();
	if (viewport_current_size != m_viewport_last_size && viewport_current_size.x && viewport_current_size.y)
	{
		m_viewport_last_size = m_viewport_panel->GetSize();
		m_editor_camera.SetAspect((f32)m_viewport_last_size.x, (f32)m_viewport_last_size.y);
		m_global_shader_data.proj = m_editor_camera.GetProjection();

		m_viewport_panel->UpdateGridCameraData();
		m_viewport_panel->OnViewportResize();
	}

	m_backend->UpdateUniformBuffer(m_pbr_global_ubo, &m_global_shader_data, sizeof(GlobalShaderData));
	m_viewport_panel->RenderBegin();
}

void Editor::OnRenderUpdate()
{
	m_backend->BindPipeline(m_pbr_pipeline);
	m_backend->BindDescriptorSet(m_pbr_pipeline, m_pbr_global_descriptor);
	
	MeshManager::GetInstance()->DrawMeshes(m_pbr_pipeline);
}

void Editor::OnRenderEnd()
{
	m_viewport_panel->RenderEnd();
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

	MenuBar();

	m_ui->EndDocking();
}

void Editor::Shutdown()
{
	m_backend->DestroyPipeline(m_editor_pipeline);

	m_backend->DestroyBuffer(m_pbr_global_ubo);
	m_backend->DestroyPipeline(m_pbr_pipeline);

	delete m_hierarchy_panel;
	delete m_inspector_panel;
	delete m_viewport_panel;
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

	if (m_inspector_panel->GetSelectedEntity() != NULL_ENTITY && m_viewport_panel->IsHovered() && !m_editor_camera_controller.IsActive())
	{
		if (event.data.key_event.key == KEY_W)
		{
			m_guizmo_operation = ImGuizmo::TRANSLATE;
			m_guizmo_mode = ImGuizmo::WORLD;
		}
		else if (event.data.key_event.key == KEY_E)
		{
			m_guizmo_operation = ImGuizmo::ROTATE;
			m_guizmo_mode = ImGuizmo::LOCAL;
		}
		else if (event.data.key_event.key == KEY_R)
		{
			m_guizmo_operation = ImGuizmo::SCALE;
			m_guizmo_mode = ImGuizmo::LOCAL;
		}
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
		m_viewport_panel->OnMouseButtonPressed();
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
		}, 100000);

	// Camera
	m_editor_camera = MultiProjectionCamera();
	m_editor_camera.CreatePerspective(60.0f, (f32)m_window->GetWidth() / (f32)m_window->GetHeight(), 0.1f, 1000.0f);

	m_editor_camera.SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

	m_editor_camera_controller = MultiProjectionController();
	m_editor_camera_controller.Init();
	m_editor_camera_controller.SetCamera(&m_editor_camera);
	m_editor_camera_controller.SetActive(false);
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
}

void Editor::CreateDescriptors()
{
}

void Editor::CreateEditorPanels()
{
	m_inspector_panel = new EditorInspector();
	m_inspector_panel->Init();

	m_hierarchy_panel = new EditorHierarchy();
	m_hierarchy_panel->Init(m_inspector_panel);

	m_viewport_panel = new EditorViewport();
	m_viewport_panel->Init(m_backend, m_frontend, m_hierarchy_panel);
	m_viewport_panel->SetSize(m_window->GetWidth(), m_window->GetHeight());
	m_viewport_panel->SetViewportEditorReferences(m_editor_pipeline, &m_editor_camera);
	m_viewport_panel->CreateViewportResources();
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
				MeshManager::GetInstance()->UploadToGpu(m_pbr_pipeline, 1, TextureType_Diffuse | TextureType_Ambient | TextureType_Specular);
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

		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		if (ImGui::BeginMenu("Component"))
		{
			if (ImGui::MenuItem("	Add", "Ctrl + Shift + A", nullptr, m_hierarchy_panel->GetSelectedGameObject() != NULL_ENTITY))
			{
			}

			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));

			if (ImGui::BeginMenu("Mesh"))
			{
				Entity selected = m_hierarchy_panel->GetSelectedGameObject();
				const b8 hasSelection = (selected != NULL_ENTITY);

				b8 enabled = hasSelection && !selected.HasComponent<MeshFilterComponent>();
				if (ImGui::MenuItem("	Mesh Filter", nullptr, false, enabled))
				{
					selected.AddComponent<MeshFilterComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndMenu();
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

	glm::mat4 parentWorld(1.0f);
	Scene* scene = SceneManager::GetInstance()->GetActiveScene();
	if (scene)
	{
		UUID selectedId = selected.GetComponent<IDComponent>().id;
		UUID parentId = scene->GetHierarchy().GetParent(selectedId);

		if ((u64)parentId != (u64)INVALID_ID)
		{
			Entity parent = scene->GetEntity(parentId);
			if (parent)
			{
				parentWorld = parent.GetComponent<TransformComponent>().world_transform;
			}
		}
	}

	glm::mat4 local =
		glm::translate(glm::mat4(1.0f), tc.local_position) *
		glm::mat4_cast(glm::quat(tc.local_rotation)) *
		glm::scale(glm::mat4(1.0f), tc.local_scale);

	glm::mat4 model = parentWorld * local;

	bool  snapOn = Input::IsKeyPressed(KEY_LEFT_CONTROL);
	float snap[3]{ 0.5f, 0.5f, 0.5f };

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::Manipulate(glm::value_ptr(view),
		glm::value_ptr(proj),
		m_guizmo_operation, m_guizmo_mode,
		glm::value_ptr(model),
		nullptr,
		snapOn ? snap : nullptr);

	if (ImGuizmo::IsUsing())
	{
		m_viewport_panel->CanPick(false);

		glm::mat4 newLocal = glm::inverse(parentWorld) * model;

		glm::vec3 t, s, skew;
		glm::vec4 persp;
		glm::quat r;
		glm::decompose(newLocal, s, r, t, skew, persp);

		// Stabilize quaternion sign to reduce flip jitter
		static glm::quat lastQ = glm::quat(1, 0, 0, 0);
		if (glm::dot(lastQ, r) < 0.0f) r = -r;
		lastQ = r;

		tc.local_position = t;
		tc.local_rotation = glm::eulerAngles(r); // radians
		tc.local_scale = s;

		tc.is_dirty = true;
	}
	else
	{
		m_viewport_panel->CanPick(true);
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
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
			},
			DescriptorSetFlags_UpdateAfterBindPool
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

	pipeline_info.dynamic_rendering_info = {
		false,
		{ VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R32_UINT },
		VK_FORMAT_D32_SFLOAT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "test.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "test.frag");

	m_pbr_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_pbr_global_descriptor = m_backend->CreateDescriptorSet(m_pbr_pipeline, 0);

	m_global_shader_data = {};
	m_global_shader_data.proj = m_editor_camera.GetProjection();
	m_global_shader_data.view = m_editor_camera.GetView();
	m_global_shader_data.camera_position = m_editor_camera.GetPosition();

	m_pbr_global_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(GlobalShaderData), 1);

	DescriptorSetWriteData data1{};
	data1.type = DescriptorType_UniformBuffer;
	data1.binding = 0;
	data1.data.buffer.buffers = new VkBuffer(m_pbr_global_ubo->GetHandle());
	data1.data.buffer.offsets = new VkDeviceSize(0);
	data1.data.buffer.ranges = new VkDeviceSize(sizeof(GlobalShaderData));

	std::vector<DescriptorSetWriteData> write_data = { data1 };
	m_backend->WriteDescriptor(&m_pbr_global_descriptor, write_data);

	MeshManager::GetInstance()->CreateDescriptors(m_pbr_pipeline, 1);
}
