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

	CreateEditorUI();

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

	UpdateLights();
	m_backend->UpdateUniformBuffer(m_pbr_lights_ubo, &m_lights_ubo_data, sizeof(LightsUBOData));

	m_backend->UpdateUniformBuffer(m_gi_settings_ubo, &m_gi_data, sizeof(GlobalIlluminationData));

	m_backend->UpdateUniformBuffer(m_shadow_settings_ubo, &m_shadow_settings, sizeof(ShadowSettings));

	RenderShadowMaps();

	m_viewport_panel->RenderBegin();
}

void Editor::OnRenderUpdate()
{
	// Render scene with active debug mode
	RenderDebugMode();

	// Render light gizmos on top
	if (m_show_light_gizmos)
	{
		UpdateLightGizmos();
		RenderLightGizmos();
	}
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

		// Debug Visualization Toolbar
		{
			ImGuiWindow* vp = ImGui::FindWindowByName("Viewport");
			if (vp)
			{
				ImGui::SetCursorScreenPos(ImVec2(vp->Pos.x + 10.0f, vp->Pos.y + 30.0f));

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.8f));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

				ImGui::BeginChild("##DebugToolbar", ImVec2(220.0f, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);

				// Light Gizmos Toggle
				ImGui::Checkbox("Light Gizmos", reinterpret_cast<bool*>(&m_show_light_gizmos));

				ImGui::Separator();

				// Shadow Settings
				if (ImGui::CollapsingHeader("Shadow Settings", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Checkbox("Enable Shadows", reinterpret_cast<bool*>(&m_shadow_settings.enabled));

					if (m_shadow_settings.enabled)
					{
						i32 shadowMapSize = static_cast<i32>(m_shadow_settings.shadow_map_size);
						if (ImGui::DragInt("Shadow Map Size", &shadowMapSize, 1.0f, 512, 4096))
						{
							m_shadow_settings.shadow_map_size = static_cast<u32>(shadowMapSize);
						}

						ImGui::DragFloat("Min Bias", &m_shadow_settings.min_bias, 0.0001f, 0.0001f, 0.01f, "%.4f");
						ImGui::DragFloat("Max Bias", &m_shadow_settings.max_bias, 0.001f, 0.001f, 0.1f, "%.4f");
						ImGui::DragFloat("Normal Offset", &m_shadow_settings.normal_offset, 0.01f, 0.0f, 1.0f, "%.2f");

						i32 pcfSamples = static_cast<i32>(m_shadow_settings.pcf_samples);
						if (ImGui::DragInt("PCF Samples", &pcfSamples, 0.1f, 0, 4))
						{
							m_shadow_settings.pcf_samples = static_cast<u32>(pcfSamples);
						}

						ImGui::DragFloat("Softness", &m_shadow_settings.softness, 0.1f, 0.0f, 5.0f, "%.2f");
					}
				}

				ImGui::Separator();
				ImGui::Text("World Light");
				ImGui::Separator();

				// Ambient Color
				ImGui::ColorEdit3("Ambient##Color", &m_global_shader_data.ambient_color_intensity.x);

				// Ambient Intensity
				ImGui::SliderFloat("Intensity##Ambient", &m_global_shader_data.ambient_color_intensity.w, 0.0f, 1.0f);

				ImGui::Separator();
				ImGui::Text("Global Illumination");
				ImGui::Separator();

				// GI Enable Toggle
				b8 gi_enabled = m_gi_data.enabled != 0;
				if (ImGui::Checkbox("Enable GI", reinterpret_cast<bool*>(&gi_enabled)))
				{
					m_gi_data.enabled = gi_enabled ? 1 : 0;
				}

				// Only show settings if GI is enabled
				if (m_gi_data.enabled)
				{
					ImGui::SliderInt("Samples", reinterpret_cast<i32*>(&m_gi_data.sample_count), 4, 32);
					ImGui::SliderFloat("Intensity##GI", &m_gi_data.intensity, 0.0f, 2.0f);
					ImGui::SliderFloat("Ray Distance", &m_gi_data.ray_distance, 1.0f, 20.0f);
					ImGui::SliderFloat("Thickness", &m_gi_data.thickness, 0.1f, 2.0f);
					ImGui::SliderFloat("Falloff", &m_gi_data.falloff, 1.0f, 4.0f);
				}

				ImGui::EndChild();

				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor();
			}
		}

		m_viewport_panel->End();
	}

	m_menu_bar->Draw();

	m_ui->EndDocking();
}

void Editor::Shutdown()
{
	// Cleanup shadow maps
	for (size_t i = 0; i < m_shadow_maps.size(); ++i)
	{
		std::string name = "ShadowMap_" + std::to_string(i);
		TextureManager::GetInstance()->DestroyTexture2D(name);
	}
	m_shadow_maps.clear();

	m_backend->DestroyBuffer(m_pbr_global_ubo);
	m_backend->DestroyBuffer(m_pbr_lights_ubo);
	m_backend->DestroyBuffer(m_gi_settings_ubo);
	m_backend->DestroyBuffer(m_shadow_settings_ubo);

	if (m_light_gizmo_vb)
	{
		m_backend->DestroyBuffer(m_light_gizmo_vb);
	}

	if (m_light_gizmo_ib)
	{
		m_backend->DestroyBuffer(m_light_gizmo_ib);
	}

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
		// Force viewport focus so other editor windows don't keep keyboard focus while freelook is active
		ImGui::SetWindowFocus("Viewport");

		// Hard-disable ImGui interactions while freelook camera is active
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

		m_editor_camera_controller.SetActive(true);
		m_window->SetCursorMode(GLFW_CURSOR_DISABLED);
		m_viewport_panel->CanPick(false);
		return true;
	}

	if (event.data.mouse_button_event.button == MOUSE_BUTTON_LEFT && m_viewport_panel->IsHovered() && SceneManager::GetInstance()->GetActiveScene())
	{
		// Don't pick while freelook camera is active or while manipulating gizmo
		if (!m_editor_camera_controller.IsActive() && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
		{
			m_viewport_panel->OnMouseButtonPressed();
			return true;
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
		m_viewport_panel->CanPick(true);

		// Restore ImGui interactions after freelook ends
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;

		return true;
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
		}, 110000);

	// Camera
	m_editor_camera = MultiProjectionCamera();
	m_editor_camera.CreatePerspective(60.0f, (f32)m_window->GetWidth() / (f32)m_window->GetHeight(), 0.1f, 1000.0f);

	m_editor_camera.SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

	m_editor_camera_controller = MultiProjectionController();
	m_editor_camera_controller.Init();
	m_editor_camera_controller.SetCamera(&m_editor_camera);
	m_editor_camera_controller.SetActive(false);

	m_show_light_gizmos = true;
	m_light_gizmo_vb = nullptr;
	m_light_gizmo_ib = nullptr;

	m_editor_data.show_debug_info = true;
}

void Editor::CreatePipelines()
{
	// PBR Pipeline
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
				{
					{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },	// Global data
					{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },						// Lights data
					{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },						// GI settings
					{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }						// Shadow settings
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }	// Per-draw data
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },	// Per-object data
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },											// Shadow maps
					{ 1, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }	// Material textures
				},
				DescriptorSetFlags_UpdateAfterBindPool
			}
		};

		pipeline_info.vertex_binding_description = VertexFormatTangent::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexFormatTangent::GetAttributeDescriptions();

		pipeline_info.push_constant_ranges = {
			{ ShaderStage_Fragment, 0, sizeof(u32) }
		};
		pipeline_info.depth_stencil_info = { true, true };

		pipeline_info.dynamic_rendering_info = { 
			false, 
			{ VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32_UINT },
			VK_FORMAT_D32_SFLOAT
		};

		ShaderStageInfo shader_info = {};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "pbr.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "pbr.frag");

		shader_info.specialization_infos.push_back({
			ShaderStage_Fragment,
			sizeof(m_editor_data),
			&m_editor_data,
			{
				{ 0, sizeof(m_editor_data.show_debug_info), offsetof(EditorSpecificData, show_debug_info) }
			}
		});

		m_pbr_pipeline = PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_PBR, pipeline_info, shader_info);
		MeshManager::GetInstance()->CreateDescriptors(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR));
	}

	// Editor Pipeline
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
				{ { 0, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_None } },	// Full-screen texture
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
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "full_screen.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "full_screen.frag");

		PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_EDITOR, pipeline_info, shader_info);
		m_backend->InitImGuiForDynamicRendering(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_EDITOR)->GetRenderingCreateInfo());
	}

	// Debug Wireframe Pipeline
	{
		PipelineCreateInfo pipeline_info = {};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
		pipeline_info.polygon_mode = PipelinePolygonMode_Line;
		pipeline_info.cull_mode = PipelineCullMode_None;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.line_width = 1.0f;

		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
		pipeline_info.layout = {
			{
				{
					{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },	// Global data
					{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },						// Lights data
					{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },						// GI settings
					{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }						// Shadow settings
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }	// Per-draw data
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },	// Per-object data
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },											// Shadow maps
					{ 1, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }	// Material textures
				},
				DescriptorSetFlags_UpdateAfterBindPool
			}
		};

		pipeline_info.vertex_binding_description = VertexFormatTangent::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexFormatTangent::GetAttributeDescriptions();

		pipeline_info.push_constant_ranges = {
			{ ShaderStage_Fragment, 0, sizeof(u32) }
		};
		pipeline_info.depth_stencil_info = { true, true };

		pipeline_info.dynamic_rendering_info = {
			false,
			{ VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32_UINT },
			VK_FORMAT_D32_SFLOAT
		};

		ShaderStageInfo shader_info = {};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "pbr.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "pbr.frag");

		shader_info.specialization_infos.push_back({
			ShaderStage_Fragment,
			sizeof(m_editor_data),
			&m_editor_data,
			{
				{ 0, sizeof(m_editor_data.show_debug_info), offsetof(EditorSpecificData, show_debug_info) }
			}
		});

		PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_WIREFRAME, pipeline_info, shader_info);
	}

	// Gizmo Pipeline
	{
		PipelineCreateInfo pipeline_info = {};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_LineList;
		pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
		pipeline_info.cull_mode = PipelineCullMode_None;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.line_width = 2.0f;

		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
		pipeline_info.layout = {};

		pipeline_info.vertex_binding_description = VertexGuizmo::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexGuizmo::GetAttributeDescriptions();

		pipeline_info.push_constant_ranges = {
			{ ShaderStage_Vertex, 0, sizeof(glm::mat4) }
		};
		pipeline_info.depth_stencil_info = { true, false };

		pipeline_info.dynamic_rendering_info = {
			false,
			{ VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32_UINT },
			VK_FORMAT_D32_SFLOAT
		};

		ShaderStageInfo shader_info = {};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "gizmo.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "gizmo.frag");

		PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_LIGHT_GIZMO, pipeline_info, shader_info);
	}

	// ShadowMap Pipeline
	{
		PipelineCreateInfo pipeline_info = {};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
		pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
		pipeline_info.cull_mode = PipelineCullMode_Front;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.line_width = 1.0f;

		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
		pipeline_info.layout = {
			{
				{
					{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },	// Global data
					{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },						// Lights data
					{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },						// GI settings
					{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }						// Shadow settings
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }	// Per-draw data
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },	// Per-object data
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },											// Shadow maps
					{ 1, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }	// Material textures
				},
				DescriptorSetFlags_UpdateAfterBindPool
			}
		};

		pipeline_info.vertex_binding_description = VertexFormatPosition::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexFormatPosition::GetAttributeDescriptions();

		pipeline_info.push_constant_ranges = { { ShaderStage_Vertex, 0, sizeof(glm::mat4) } };
		pipeline_info.depth_stencil_info = { true, true, false, PipelineDethStencilCompareOp_Less };

		pipeline_info.dynamic_rendering_info = { false, {}, VK_FORMAT_D32_SFLOAT };

		ShaderStageInfo shader_info = {};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "shadow_depth.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "shadow_depth.frag");

		m_shadow_pipeline = PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_SHADOW_DEPTH, pipeline_info, shader_info);
	}
}

void Editor::CreateDescriptors()
{
	// PBR Global Descriptor Set
	{
		m_pbr_global_descriptor = m_backend->CreateDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR), 0);

		// Initialize global shader data (camera + ambient)
		m_global_shader_data.proj = m_editor_camera.GetProjection();
		m_global_shader_data.view = m_editor_camera.GetView();
		m_global_shader_data.camera_position = m_editor_camera.GetPosition();
		m_global_shader_data.ambient_color_intensity = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);  // Default ambient light

		m_pbr_global_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(GlobalShaderData), 1);

		// Initialize lights UBO with default data
		m_lights_ubo_data = {};
		m_lights_ubo_data.light_count = 0;
		m_pbr_lights_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightsUBOData), 1);

		// Initialize Global Illumination settings
		m_gi_data = {};
		m_gi_data.enabled = 0;              // Disabled by default
		m_gi_data.sample_count = 16;        // 16 samples per pixel
		m_gi_data.ray_distance = 5.0f;      // 5 units maximum ray distance
		m_gi_data.intensity = 1.0f;         // 100% intensity
		m_gi_data.thickness = 0.5f;         // Surface thickness
		m_gi_data.falloff = 2.0f;           // Quadratic falloff
		m_gi_data.bias = 0.05f;             // Small bias to avoid self-occlusion
		m_gi_data.temporal_weight = 0.95f;  // 95% temporal accumulation
		m_gi_data.debug_visualization = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);  // No debug
		m_gi_settings_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(GlobalIlluminationData), 1);

		// Initialize Shadow settings
		m_shadow_settings = {};
		m_shadow_settings.enabled = 1;                  // Enabled by default
		m_shadow_settings.shadow_map_size = 2048;       // 2048x2048 shadow maps
		m_shadow_settings.cascade_split_lambda = 0.75f; // CSM split
		m_shadow_settings.min_bias = 0.001f;            // Min bias
		m_shadow_settings.max_bias = 0.01f;             // Max bias
		m_shadow_settings.normal_offset = 0.1f;         // Normal offset
		m_shadow_settings.pcf_samples = 1;              // 3x3 PCF
		m_shadow_settings.softness = 1.0f;              // Shadow softness
		m_shadow_settings.cascade_distances = glm::vec4(10.0f, 30.0f, 80.0f, 200.0f);  // Cascade distances
		m_shadow_settings_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(ShadowSettings), 1);

		DescriptorSetWriteData data1{};
		data1.type = DescriptorType_UniformBuffer;
		data1.binding = 0;
		data1.data.buffer.buffers = new VkBuffer(m_pbr_global_ubo->GetHandle());
		data1.data.buffer.offsets = new VkDeviceSize(0);
		data1.data.buffer.ranges = new VkDeviceSize(sizeof(GlobalShaderData));

		DescriptorSetWriteData data2{};
		data2.type = DescriptorType_UniformBuffer;
		data2.binding = 1;
		data2.data.buffer.buffers = new VkBuffer(m_pbr_lights_ubo->GetHandle());
		data2.data.buffer.offsets = new VkDeviceSize(0);
		data2.data.buffer.ranges = new VkDeviceSize(sizeof(LightsUBOData));

		DescriptorSetWriteData data3{};
		data3.type = DescriptorType_UniformBuffer;
		data3.binding = 2;
		data3.data.buffer.buffers = new VkBuffer(m_gi_settings_ubo->GetHandle());
		data3.data.buffer.offsets = new VkDeviceSize(0);
		data3.data.buffer.ranges = new VkDeviceSize(sizeof(GlobalIlluminationData));

		DescriptorSetWriteData data4{};
		data4.type = DescriptorType_UniformBuffer;
		data4.binding = 3;
		data4.data.buffer.buffers = new VkBuffer(m_shadow_settings_ubo->GetHandle());
		data4.data.buffer.offsets = new VkDeviceSize(0);
		data4.data.buffer.ranges = new VkDeviceSize(sizeof(ShadowSettings));

		std::vector<DescriptorSetWriteData> write_data = { data1, data2, data3, data4 };
		m_backend->WriteDescriptor(&m_pbr_global_descriptor, write_data);
	}

	{
		m_shadow_maps.clear();
		u32 shadowMapSize = m_shadow_settings.shadow_map_size;

		for (u32 i = 0; i < MAX_LIGHTS; ++i)
		{
			std::string shadowMapName = "ShadowMap_" + std::to_string(i);
			VulkanTexture2D* shadowMap = TextureManager::GetInstance()->CreateTexture2D(
				shadowMapName,
				VK_FORMAT_D32_SFLOAT,  // 32-bit depth format
				shadowMapSize,
				shadowMapSize
			);
			m_shadow_maps.push_back(shadowMap);
		}

		m_textures_descriptor = MeshManager::GetInstance()->GetTexturesDescriptor();

		std::vector<VkImageView> shadowMapViewValues;
		std::vector<VkSampler> shadowMapSamplerValues;
		for (auto* shadowMap : m_shadow_maps)
		{
			shadowMapViewValues.push_back(shadowMap->GetImageView());
			shadowMapSamplerValues.push_back(shadowMap->GetSampler());
		}

		DescriptorSetWriteData shadowMapWrite{};
		shadowMapWrite.type = DescriptorType_CombinedImageSampler;
		shadowMapWrite.binding = 0;
		shadowMapWrite.data.image.image_views = shadowMapViewValues.data();
		shadowMapWrite.data.image.samplers = shadowMapSamplerValues.data();

		std::vector<DescriptorSetWriteData> shadowMapWrites = { shadowMapWrite };
		if (m_textures_descriptor)
		{
			m_backend->WriteDescriptorVariable(&m_textures_descriptor, shadowMapWrites, static_cast<u32>(m_shadow_maps.size()), 0);
		}
	}
}

void Editor::CreateEditorUI()
{
	m_inspector_panel = new EditorInspector();
	m_inspector_panel->Init();

	m_hierarchy_panel = new EditorHierarchy();
	m_hierarchy_panel->Init(m_inspector_panel);

	m_viewport_panel = new EditorViewport();
	m_viewport_panel->Init(m_backend, m_frontend, m_hierarchy_panel);
	m_viewport_panel->SetSize(m_window->GetWidth(), m_window->GetHeight());
	m_viewport_panel->SetViewportEditorReferences(&m_editor_camera);
	m_viewport_panel->CreateViewportResources();

	m_menu_bar = new EditorMenuBar();
	m_menu_bar->Init(m_hierarchy_panel);
	m_editor_settings = m_menu_bar->GetSettings();
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

void Editor::UpdateLights()
{
	// Get active scene
	Scene* activeScene = SceneManager::GetInstance()->GetActiveScene();
	if (!activeScene)
	{
		m_lights_ubo_data.light_count = 0;
		return;
	}

	// Gather all entities with LightComponent
	auto view = activeScene->GetRegistry().view<LightComponent, TransformComponent>();

	u32 lightIndex = 0;
	for (auto entity : view)
	{
		if (lightIndex >= MAX_LIGHTS) break;  // Max lights reached

		auto& lightComp = view.get<LightComponent>(entity);
		auto& transformComp = view.get<TransformComponent>(entity);

		// Skip disabled lights
		if (!lightComp.enabled) continue;

		LightGPUData& gpuLight = m_lights_ubo_data.lights[lightIndex];

		// Get world position from transform
		glm::vec3 worldPos = glm::vec3(transformComp.world_transform[3]);

		// Derive world scale from transform basis vectors (handles parent scaling)
		f32 scaleX = glm::length(glm::vec3(transformComp.world_transform[0]));
		f32 scaleY = glm::length(glm::vec3(transformComp.world_transform[1]));
		f32 scaleZ = glm::length(glm::vec3(transformComp.world_transform[2]));
		f32 maxWorldScale = glm::max(scaleX, glm::max(scaleY, scaleZ));
		f32 effectiveRange = lightComp.range * glm::max(maxWorldScale, 0.0001f);

		// Set light type
		f32 lightType = static_cast<f32>(lightComp.type);

		if (lightComp.type == LightType_Point)
		{
			// Point light: position is world position
			gpuLight.position_type = glm::vec4(worldPos, lightType);
			gpuLight.direction_range = glm::vec4(0.0f, 0.0f, 0.0f, effectiveRange);
		}
		else if (lightComp.type == LightType_Directional)
		{
			// Directional light: position stores direction (derived from forward axis)
			glm::vec3 forward = -glm::normalize(glm::vec3(transformComp.world_transform[2]));
			gpuLight.position_type = glm::vec4(forward, lightType);
			gpuLight.direction_range = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (lightComp.type == LightType_Spot)
		{
			// Spot light: position and direction
			glm::vec3 forward = -glm::normalize(glm::vec3(transformComp.world_transform[2]));
			gpuLight.position_type = glm::vec4(worldPos, lightType);
			gpuLight.direction_range = glm::vec4(forward, effectiveRange);
			gpuLight.cone_attenuation.x = glm::cos(lightComp.inner_cone_angle);
			gpuLight.cone_attenuation.y = glm::cos(lightComp.outer_cone_angle);
		}

		// Common properties
		gpuLight.color_intensity = glm::vec4(lightComp.color, lightComp.intensity);
		gpuLight.cone_attenuation.z = lightComp.attenuation;
		gpuLight.cone_attenuation.w = 1.0f;  // Enabled

		// Shadow properties
		gpuLight.shadow_params.x = static_cast<f32>(lightIndex);  // Shadow map index (placeholder)
		gpuLight.shadow_params.y = m_shadow_settings.min_bias;     // Shadow bias
		gpuLight.shadow_params.z = 1.0f;                           // Shadow strength
		gpuLight.shadow_params.w = lightComp.cast_shadows ? 1.0f : 0.0f;  // Cast shadows flag

		// Compute shadow matrix (simplified - for directional/spot lights)
		if (lightComp.cast_shadows && m_shadow_settings.enabled)
		{
			// Vulkan depth range correction matrix
			// GLM uses OpenGL NDC [-1,1] for Z, Vulkan uses [0,1]
			// Also flip Y because Vulkan's viewport Y is inverted
			const glm::mat4 vulkanClipCorrection = glm::mat4(
				1.0f,  0.0f,  0.0f, 0.0f,
				0.0f, -1.0f,  0.0f, 0.0f,  // Flip Y (Vulkan Y-down)
				0.0f,  0.0f,  0.5f, 0.5f,  // Z: scale to [0,1] range
				0.0f,  0.0f,  0.0f, 1.0f
			);

			if (lightComp.type == LightType_Directional)
			{
				// Simple orthographic shadow matrix for directional light
				glm::vec3 lightDir = glm::normalize(glm::vec3(gpuLight.position_type));
				glm::vec3 lightPos = -lightDir * 50.0f;  // Position light far back
				glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 200.0f);
				gpuLight.shadow_matrix = vulkanClipCorrection * lightProj * lightView;
			}
			else if (lightComp.type == LightType_Spot)
			{
				// Perspective shadow matrix for spot light
				glm::vec3 lightDir = glm::normalize(glm::vec3(gpuLight.direction_range));
				glm::mat4 lightView = glm::lookAt(worldPos, worldPos + lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
				f32 fov = lightComp.outer_cone_angle * 2.0f;  // FOV from outer cone
				f32 nearPlane = 0.1f;
				f32 farPlane = gpuLight.direction_range.w;

				glm::mat4 lightProj = glm::perspective(fov, 1.0f, nearPlane, farPlane);
				gpuLight.shadow_matrix = lightProj * lightView;
			}
			else
			{
				// Point lights use cubemap shadows (not implemented yet)
				gpuLight.shadow_matrix = glm::mat4(1.0f);
			}
		}
		else
		{
			gpuLight.shadow_matrix = glm::mat4(1.0f);
		}

		lightIndex++;
	}

	m_lights_ubo_data.light_count = lightIndex;
}

void Editor::UpdateLightGizmos()
{
	m_light_gizmo_vertices.clear();
	m_light_gizmo_indices.clear();

	Scene* activeScene = SceneManager::GetInstance()->GetActiveScene();
	if (!activeScene) return;

	auto view = activeScene->GetRegistry().view<LightComponent, TransformComponent>();

	for (auto entity : view)
	{
		auto& lightComp = view.get<LightComponent>(entity);
		auto& transformComp = view.get<TransformComponent>(entity);

		glm::vec3 worldPos = glm::vec3(transformComp.world_transform[3]);
		glm::vec4 lightColor = glm::vec4(lightComp.color, 1.0f);

		u32 baseIdx = static_cast<u32>(m_light_gizmo_vertices.size());

		if (lightComp.type == LightType_Point)
		{
			// Draw a wireframe sphere
			f32 radius = lightComp.range * 0.1f;  // Visual size
			const u32 segments = 16;
			const u32 rings = 8;

			// Generate sphere vertices
			for (u32 ring = 0; ring <= rings; ++ring)
			{
				f32 phi = glm::pi<f32>() * ring / rings;
				for (u32 seg = 0; seg <= segments; ++seg)
				{
					f32 theta = 2.0f * glm::pi<f32>() * seg / segments;
					glm::vec3 p = worldPos + radius * glm::vec3(
						sin(phi) * cos(theta),
						cos(phi),
						sin(phi) * sin(theta)
					);
					m_light_gizmo_vertices.push_back({ p, lightColor });
				}
			}

			// Generate sphere line indices
			for (u32 ring = 0; ring < rings; ++ring)
			{
				for (u32 seg = 0; seg < segments; ++seg)
				{
					u32 curr = baseIdx + ring * (segments + 1) + seg;
					u32 next = curr + segments + 1;

					// Horizontal ring
					m_light_gizmo_indices.push_back(curr);
					m_light_gizmo_indices.push_back(curr + 1);

					// Vertical line
					m_light_gizmo_indices.push_back(curr);
					m_light_gizmo_indices.push_back(next);
				}
			}
		}
		else if (lightComp.type == LightType_Directional)
		{
			// Draw direction arrow
			glm::vec3 forward = glm::normalize(glm::vec3(transformComp.world_transform * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
			f32 arrowLength = 2.0f;
			f32 arrowHeadSize = 0.5f;

			glm::vec3 arrowEnd = worldPos + forward * arrowLength;

			// Arrow shaft
			m_light_gizmo_vertices.push_back({ worldPos, lightColor });
			m_light_gizmo_vertices.push_back({ arrowEnd, lightColor });
			m_light_gizmo_indices.push_back(baseIdx);
			m_light_gizmo_indices.push_back(baseIdx + 1);

			// Arrow head (4 lines forming a cone)
			glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
			glm::vec3 up = glm::cross(right, forward);

			m_light_gizmo_vertices.push_back({ arrowEnd, lightColor });
			m_light_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize + right * arrowHeadSize * 0.5f, lightColor });
			m_light_gizmo_indices.push_back(baseIdx + 1);
			m_light_gizmo_indices.push_back(baseIdx + 2);

			m_light_gizmo_vertices.push_back({ arrowEnd, lightColor });
			m_light_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize - right * arrowHeadSize * 0.5f, lightColor });
			m_light_gizmo_indices.push_back(baseIdx + 1);
			m_light_gizmo_indices.push_back(baseIdx + 3);

			m_light_gizmo_vertices.push_back({ arrowEnd, lightColor });
			m_light_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize + up * arrowHeadSize * 0.5f, lightColor });
			m_light_gizmo_indices.push_back(baseIdx + 1);
			m_light_gizmo_indices.push_back(baseIdx + 4);

			m_light_gizmo_vertices.push_back({ arrowEnd, lightColor });
			m_light_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize - up * arrowHeadSize * 0.5f, lightColor });
			m_light_gizmo_indices.push_back(baseIdx + 1);
			m_light_gizmo_indices.push_back(baseIdx + 5);
		}
		else if (lightComp.type == LightType_Spot)
		{
			// Draw cone
			glm::vec3 forward = glm::normalize(glm::vec3(transformComp.world_transform * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
			glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
			glm::vec3 up = glm::cross(right, forward);

			f32 coneLength = lightComp.range * 0.5f;
			f32 coneRadius = tan(lightComp.outer_cone_angle) * coneLength;

			const u32 segments = 16;
			glm::vec3 coneBase = worldPos + forward * coneLength;

			// Cone base circle
			for (u32 i = 0; i < segments; ++i)
			{
				f32 angle1 = 2.0f * glm::pi<f32>() * i / segments;
				f32 angle2 = 2.0f * glm::pi<f32>() * ((i + 1) % segments) / segments;

				glm::vec3 p1 = coneBase + coneRadius * (cos(angle1) * right + sin(angle1) * up);
				glm::vec3 p2 = coneBase + coneRadius * (cos(angle2) * right + sin(angle2) * up);

				m_light_gizmo_vertices.push_back({ p1, lightColor });
				m_light_gizmo_vertices.push_back({ p2, lightColor });
				m_light_gizmo_indices.push_back(baseIdx + i * 2);
				m_light_gizmo_indices.push_back(baseIdx + i * 2 + 1);

				// Lines from apex to base
				if (i % 4 == 0)
				{
					m_light_gizmo_vertices.push_back({ worldPos, lightColor });
					m_light_gizmo_vertices.push_back({ p1, lightColor });
					u32 apexIdx = static_cast<u32>(m_light_gizmo_vertices.size()) - 2;
					m_light_gizmo_indices.push_back(apexIdx);
					m_light_gizmo_indices.push_back(apexIdx + 1);
				}
			}
		}
	}

	// Upload to GPU
	if (!m_light_gizmo_vertices.empty())
	{
		u32 vb_size = static_cast<u32>(m_light_gizmo_vertices.size() * sizeof(VertexGuizmo));
		u32 ib_size = static_cast<u32>(m_light_gizmo_indices.size() * sizeof(u32));

		// Recreate buffers if size changed or doesn't exist
		if (!m_light_gizmo_vb)
		{
			m_light_gizmo_vb = m_backend->CreateVertexBufferEmpty(10000);
		}
		if (!m_light_gizmo_ib)
		{
			m_light_gizmo_ib = m_backend->CreateIndexBufferEmpty(10000);
		}

		// Update buffer contents
		m_backend->UpdateVertexBuffer(m_light_gizmo_vb, 0, m_light_gizmo_vertices.data(), vb_size);
		m_backend->UpdateIndexBuffer(m_light_gizmo_ib, 0, m_light_gizmo_indices.data(), ib_size);
	}
}

void Editor::RenderLightGizmos()
{
	if (!m_show_light_gizmos || !m_light_gizmo_vb || !m_light_gizmo_ib)
		return;

	Pipeline* gizmoPipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_LIGHT_GIZMO);
	if (!gizmoPipeline) return;

	m_backend->BindPipeline(gizmoPipeline);

	// MVP matrix (no model transform needed, vertices are in world space)
	glm::mat4 vp = m_editor_camera.GetProjection() * m_editor_camera.GetView();
	m_backend->BindPushConstants(gizmoPipeline, ShaderStage_Vertex, 0, sizeof(glm::mat4), &vp);

	m_backend->BindVertexBuffer(m_light_gizmo_vb, 0);
	m_backend->BindIndexBuffer(m_light_gizmo_ib, 0);
	m_backend->DrawIndexed(static_cast<u32>(m_light_gizmo_indices.size()), 1, 0, 0, 0);
}

void Editor::RenderShadowMaps()
{
	if (!m_shadow_settings.enabled) return;

	Scene* scene = SceneManager::GetInstance()->GetActiveScene();
	if (!scene) return;

	Pipeline* shadowPipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_SHADOW_DEPTH);
	if (!shadowPipeline) return;

	u32 shadowMapSize = m_shadow_settings.shadow_map_size;

	// Iterate through all lights in the scene using registry view
	auto view = scene->GetRegistry().view<LightComponent, TransformComponent>();

	u32 lightIndex = 0;  // Index in the lights UBO (matches UpdateLights logic)
	for (auto entity : view)
	{
		auto& lightComp = view.get<LightComponent>(entity);
		auto& transformComp = view.get<TransformComponent>(entity);

		// Skip disabled lights (must match UpdateLights() logic)
		if (!lightComp.enabled)
		{
			continue;  // Don't increment lightIndex
		}

		if (lightIndex >= MAX_LIGHTS) break;

		// Skip if light doesn't cast shadows
		if (!lightComp.cast_shadows)
		{
			lightIndex++;
			continue;
		}

		// Get the shadow matrix for this light (already computed in UpdateLights)
		glm::mat4 lightViewProj = m_lights_ubo_data.lights[lightIndex].shadow_matrix;

		// Get shadow map texture for this light
		VulkanTexture2D* shadowMap = m_shadow_maps[lightIndex];

		// Begin rendering to shadow map (depth-only pass)
		DynamicRenderingAttachmentInfo depthAttachment{};
		depthAttachment.image = shadowMap->GetHandle();
		depthAttachment.image_view = shadowMap->GetImageView();
		depthAttachment.format = VK_FORMAT_D32_SFLOAT;
		depthAttachment.image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clear_value.depthStencil = { 1.0f, 0 };  // Clear depth to far plane
		depthAttachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		DynamicRenderingInfo shadowDRI{};
		shadowDRI.extent = { shadowMapSize, shadowMapSize };
		shadowDRI.depth_attachment = depthAttachment;
		shadowDRI.flags = 0;

		m_backend->BeginDynamicRenderingWithAttachments(shadowDRI);

		// Set viewport and scissor for shadow map resolution
		m_backend->SetViewport({ { 0.0f, 0.0f, (f32)shadowMapSize, (f32)shadowMapSize, 0.0f, 1.0f } });
		m_backend->SetScissor({ { { 0, 0 }, { shadowMapSize, shadowMapSize } } });

		// Bind shadow pipeline
		m_backend->BindPipeline(shadowPipeline);

		// Push light view-projection matrix
		m_backend->BindPushConstants(shadowPipeline, ShaderStage_Vertex, 0, sizeof(glm::mat4), &lightViewProj);

		// Render all meshes from light's perspective (depth-only, no material/texture descriptors)
		MeshManager::GetInstance()->DrawMeshes(shadowPipeline);

		m_backend->EndDynamicRenderingWithAttachments(shadowDRI);

		lightIndex++;
	}
}

void Editor::RenderDebugMode()
{
	Pipeline* activePipeline = m_pbr_pipeline;
	u32 debugViewMode = static_cast<u32>(m_editor_settings->render_mode);

	if (m_editor_settings->render_mode == EditorRenderMode_Wireframe)
	{
		activePipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_WIREFRAME);
	}

	if (!activePipeline) return;

	m_backend->BindPipeline(activePipeline);

	m_backend->BindPushConstants(activePipeline, ShaderStage_Fragment, 0, sizeof(u32), &debugViewMode);

	m_backend->BindDescriptorSet(activePipeline, m_pbr_global_descriptor);

	MeshManager::GetInstance()->DrawMeshes(activePipeline);
}
