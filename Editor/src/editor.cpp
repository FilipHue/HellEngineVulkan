#include "editor.h"

Editor::Editor(ApplicationConfiguration* configuration) : Application(configuration)
{
	/* Other */
	m_global_data = {};
	m_lights_data = {};
	m_viewport_last_size = { 0, 0 };

	/* Editor UI */
	m_hierarchy_panel = nullptr;
	m_inspector_panel = nullptr;
	m_viewport_panel = nullptr;
	m_menu_bar = nullptr;

	/* Graphics resources */
	// PBR
	m_pbr_descriptor = nullptr;
	m_pbr_gi_descriptor = nullptr;
	m_pbr_global_data_ubo = nullptr;
	m_pbr_lights_data_ubo = nullptr;
	m_pbr_gi_data_ubo = nullptr;
	m_pbr_shadow_data_ubo = nullptr;

	// GBuffer
	m_gbuffer_position = nullptr;
	m_gbuffer_normal = nullptr;
	m_gbuffer_albedo_ao = nullptr;
	m_gbuffer_depth = nullptr;
	m_gbuffer_lighting = nullptr;

	// Global Illumination
	m_gi_descriptor = nullptr;
	m_gi_texture = nullptr;

	// Shadow Maps
	m_shadow_maps = {};
	m_textures_descriptor = nullptr;

	// Debug Visualization
	m_gizmo_vb = nullptr;
	m_gizmo_ib = nullptr;
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

	m_editor_settings.Deserialize();

	CreateResources();
	CreatePipelines();
	CreateDescriptors();

	CreateEditorUI();

	CreatePBRResources();
	CreateGBufferResources();
	CreateGIResources();

	CreatePBRDescriptors();
	UpdatePBRDescriptors();
	CreateGBufferDescriptors();
	CreateGIDescriptors();
	UpdateGIDescriptors();

	m_viewport_last_size = m_viewport_panel->GetSize();
}

void Editor::OnProcessUpdate(f32 delta_time)
{
	if (m_editor_camera_controller.IsActive())
	{
		m_editor_camera_controller.OnProcessUpdate(delta_time);
		m_global_data.camera.view = m_editor_camera.GetView();
		m_global_data.camera.position = glm::vec4(m_editor_camera.GetPosition(), 0.0f);
		m_viewport_panel->UpdateGridCameraData();
	}

	m_global_data.world.time.x += delta_time;
	m_global_data.world.time.y = delta_time;

	//if (SceneManager::GetInstance()->GetActiveScene())
	//{
	//	SceneManager::GetInstance()->GetActiveScene()->UpdateAllTransforms();
	//}

	MeshManager::GetInstance()->UpdatePerDrawData();
}

void Editor::OnRenderBegin()
{
	glm::uvec2& viewport_current_size = m_viewport_panel->GetSize();

	if ((viewport_current_size != m_viewport_last_size) && viewport_current_size.x && viewport_current_size.y)
	{
		m_viewport_last_size = viewport_current_size;

		m_editor_camera.SetAspect((f32)m_viewport_last_size.x, (f32)m_viewport_last_size.y);
		m_global_data.camera.projection = m_editor_camera.GetProjection();

		m_viewport_panel->UpdateGridCameraData();
		m_viewport_panel->OnViewportResize();

		DestroyGBufferResources();
		DestroyGIResources();

		CreateGBufferResources();
		CreateGIResources();

		UpdateGIDescriptors();
		UpdatePBRDescriptors();
	}

	LoadLightData();

	m_backend->UpdateUniformBuffer(m_pbr_global_data_ubo, &m_global_data, sizeof(GlobalData));
	m_backend->UpdateUniformBuffer(m_pbr_lights_data_ubo, &m_lights_data, sizeof(LightsUBOData));
	m_backend->UpdateUniformBuffer(m_pbr_gi_data_ubo, &m_editor_settings.gi_settings, sizeof(GlobalIlluminationSettings));
	m_backend->UpdateUniformBuffer(m_pbr_shadow_data_ubo, &m_editor_settings.shadow_settings, sizeof(ShadowSettings));

	RenderShadowMaps();
	RenderGBuffer();
	RenderGI();

	m_viewport_panel->RenderBegin();
}

void Editor::OnRenderUpdate()
{
	RenderPBR();

	if (m_editor_settings.show_gizmos)
	{
		UpdateDebug();
		RenderDebug();
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

	if (m_editor_settings.window_settings.show_hierarchy && m_hierarchy_panel->Begin())
	{
		m_hierarchy_panel->Draw();
		m_hierarchy_panel->End();
	}

	if (m_editor_settings.window_settings.show_inspector && m_inspector_panel->Begin())
	{
		m_inspector_panel->Draw();
		m_inspector_panel->End();
	}

	if (m_viewport_panel->Begin())
	{
		m_viewport_panel->DrawToolbar();
		m_viewport_panel->Draw();
		ShowTransformGizmo();

		m_viewport_panel->End();
	}

	m_menu_bar->Draw();

	m_ui->EndDocking();
}

void Editor::Shutdown()
{
	m_editor_settings.Serialize();

	DestroyResources();

	delete m_hierarchy_panel;
	delete m_inspector_panel;
	delete m_viewport_panel;
	delete m_menu_bar;
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
			m_guizmo_mode = ImGuizmo::LOCAL;
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

	m_menu_bar->OnKeyPressed(event.data.key_event.key, event.data.key_event.scancode, event.data.key_event.mods);

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

void Editor::ImportAsset()
{
	if (!SceneManager::GetInstance()->GetActiveScene())
	{
		SceneManager::GetInstance()->CreateScene("Untitled Scene");
	}

	TextureType material_texture_types =
		TextureType_Diffuse |
		TextureType_Specular |
		TextureType_Ambient |
		TextureType_Emissive |
		TextureType_Height |
		TextureType_Normals |
		TextureType_Shininess |
		TextureType_Opacity |
		TextureType_Displacement |
		TextureType_Lightmap |
		TextureType_Reflection;
	File file = FileManager::OpenFile("All Supported\0*.fbx;*.obj;*.gltf;*.glb\0FBX Files\0*.fbx\0OBJ Files\0*.obj\0GLTF Files\0*.gltf;*.glb\0");
	if (FileManager::Exists(file.GetAbsolutePath()))
	{
		AssetManager::LoadModel(file);
		SceneManager::GetInstance()->CreateEntitiesFromMeshes();
		MeshManager::GetInstance()->UploadToGpu(material_texture_types);
	}
}

void Editor::CreateEmptyGameObject()
{
	if (!SceneManager::GetInstance()->GetActiveScene())
	{
		SceneManager::GetInstance()->CreateScene("Untitled Scene");
	}
	Entity entity = SceneManager::GetInstance()->GetActiveScene()->CreateEntity("Empty GameObject");
	m_hierarchy_panel->SetSelectedGameObject(entity);
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

	CreateEditorResources();

	CreateShadowResources();

	CreateDebugResources();
}

void Editor::CreatePipelines()
{
	CreateEditorPipeline();

	CreatePBRPipeline();
	CreateGBufferPipeline();
	CreateGIPipeline();
	CreateShadowPipeline();

	CreateDebugPipeline();
}

void Editor::CreateDescriptors()
{
	MeshManager::GetInstance()->CreateDescriptors(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR));

	CreateEditorDescriptors();

	CreateShadowDescriptors();

	CreateDebugDescriptors();
}

void Editor::DestroyResources()
{
	DestroyEditorResources();

	DestroyPBRResources();
	DestroyGBufferResources();
	DestroyGIResources();
	DestroyShadowResources();

	DestroyDebugResources();
}

void Editor::CreateEditorPipeline()
{
	PipelineCreateInfo pipeline_info = {};

	pipeline_info.type = PipelineType_Graphics;
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;
	pipeline_info.depth_clamp_enable = false;

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
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "full_screen.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "full_screen.frag");

	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_EDITOR, pipeline_info, shader_info);
	m_backend->InitImGuiForDynamicRendering(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_EDITOR)->GetRenderingCreateInfo());
}

void Editor::CreateEditorResources()
{
}

void Editor::DestroyEditorResources()
{
}

void Editor::CreateEditorDescriptors()
{
}

void Editor::UpdateEditorDescriptors()
{
}

void Editor::RenderEditor()
{
}

void Editor::CreateEditorUI()
{
	m_inspector_panel = new EditorInspector();
	m_inspector_panel->Init();

	m_hierarchy_panel = new EditorHierarchy();
	m_hierarchy_panel->Init(m_inspector_panel);

	m_menu_bar = new EditorMenuBar();

	m_viewport_panel = new EditorViewport();
	m_viewport_panel->Init(m_backend, m_frontend, m_hierarchy_panel);
	m_viewport_panel->SetSize(m_window->GetWidth(), m_window->GetHeight());
	m_viewport_panel->SetViewportEditorReferences(&m_editor_camera, &m_editor_settings);
	m_viewport_panel->CreateViewportResources();

	m_menu_bar->Init(m_hierarchy_panel, &m_editor_settings);
	m_menu_bar->SetImportAssetCallback([this]() { ImportAsset(); });
	m_menu_bar->SetCreateEmptyGameObjectCallback([this]() { CreateEmptyGameObject(); });
}

void Editor::CreatePBRPipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.type = PipelineType_Graphics;
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;
	pipeline_info.depth_clamp_enable = false;

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 4, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
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
				{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS * MAX_SHADOW_CASCADES, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }
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

	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_PBR, pipeline_info, shader_info);
}

void Editor::CreatePBRResources()
{
	m_global_data = {};
	m_global_data.camera.projection = m_editor_camera.GetProjection();
	m_global_data.camera.view = m_editor_camera.GetView();
	m_global_data.camera.position = glm::vec4(m_editor_camera.GetPosition(), 0.0f);

	m_global_data.world.time = glm::vec4(0.0f);

	m_pbr_global_data_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(GlobalData), 1);

	m_lights_data = {};
	m_lights_data.light_count = 0;

	m_pbr_lights_data_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightsUBOData), 1);

	m_pbr_gi_data_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(GlobalIlluminationSettings), 1);

	m_pbr_shadow_data_ubo = m_backend->CreateUniformBufferMappedPersistent(sizeof(ShadowSettings), 1);
}

void Editor::DestroyPBRResources()
{
	m_backend->DestroyBuffer(m_pbr_global_data_ubo);
	m_backend->DestroyBuffer(m_pbr_lights_data_ubo);
	m_backend->DestroyBuffer(m_pbr_gi_data_ubo);
	m_backend->DestroyBuffer(m_pbr_shadow_data_ubo);
}

void Editor::CreatePBRDescriptors()
{
	m_pbr_descriptor = m_backend->CreateDescriptorSet(PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR), 0);
}

void Editor::UpdatePBRDescriptors()
{
	VkBuffer     buf1 = m_pbr_global_data_ubo->GetHandle();
	VkDeviceSize off1 = 0;
	VkDeviceSize range1 = sizeof(GlobalData);

	DescriptorSetWriteData data1{};
	data1.type = DescriptorType_UniformBuffer;
	data1.binding = 0;
	data1.data.buffer.buffers = &buf1;
	data1.data.buffer.offsets = &off1;
	data1.data.buffer.ranges = &range1;

	VkBuffer     buf2 = m_pbr_lights_data_ubo->GetHandle();
	VkDeviceSize off2 = 0;
	VkDeviceSize range2 = sizeof(LightsUBOData);

	DescriptorSetWriteData data2{};
	data2.type = DescriptorType_UniformBuffer;
	data2.binding = 1;
	data2.data.buffer.buffers = &buf2;
	data2.data.buffer.offsets = &off2;
	data2.data.buffer.ranges = &range2;

	VkBuffer     buf3 = m_pbr_gi_data_ubo->GetHandle();
	VkDeviceSize off3 = 0;
	VkDeviceSize range3 = sizeof(GlobalIlluminationSettings);

	DescriptorSetWriteData data3{};
	data3.type = DescriptorType_UniformBuffer;
	data3.binding = 2;
	data3.data.buffer.buffers = &buf3;
	data3.data.buffer.offsets = &off3;
	data3.data.buffer.ranges = &range3;

	VkBuffer     buf4 = m_pbr_shadow_data_ubo->GetHandle();
	VkDeviceSize off4 = 0;
	VkDeviceSize range4 = sizeof(ShadowSettings);

	DescriptorSetWriteData data4{};
	data4.type = DescriptorType_UniformBuffer;
	data4.binding = 3;
	data4.data.buffer.buffers = &buf4;
	data4.data.buffer.offsets = &off4;
	data4.data.buffer.ranges = &range4;

	VkImageView  view5 = m_gi_texture->GetImageView();
	VkSampler    samp5 = m_gi_texture->GetSampler();

	DescriptorSetWriteData data5{};
	data5.type = DescriptorType_CombinedImageSampler;
	data5.binding = 4;
	data5.data.image.image_views = &view5;
	data5.data.image.samplers = &samp5;

	std::vector<DescriptorSetWriteData> write_data = {
		data1, data2, data3, data4, data5
	};

	m_backend->WriteDescriptor(&m_pbr_descriptor, write_data);
}

void Editor::RenderPBR()
{
	Pipeline* activePipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_PBR);
	u32 debugViewMode = static_cast<u32>(m_editor_settings.render_mode);

	if (m_editor_settings.render_mode == EditorRenderMode_Wireframe)
	{
		activePipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_WIREFRAME);
	}

	m_backend->BindPipeline(activePipeline);
	m_backend->BindPushConstants(activePipeline, ShaderStage_Fragment, 0, sizeof(u32), &debugViewMode);

	m_backend->BindDescriptorSet(activePipeline, m_pbr_descriptor);

	MeshManager::GetInstance()->DrawMeshes(activePipeline);
}

void Editor::CreateGBufferPipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.type = PipelineType_Graphics;
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;
	pipeline_info.depth_clamp_enable = false;

	pipeline_info.dynamic_states = {
		PipelineDynamicState_Viewport,
		PipelineDynamicState_Scissor
	};

	pipeline_info.layout = {
		{
			{
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 4, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS * MAX_SHADOW_CASCADES, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatTangent::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatTangent::GetAttributeDescriptions();
	pipeline_info.depth_stencil_info = { true, true };

	pipeline_info.dynamic_rendering_info = {
		false,
		{
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_FORMAT_R16G16B16A16_SFLOAT
		},
		VK_FORMAT_D32_SFLOAT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "pbr.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "gbuffer.frag");

	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_GBUFFER, pipeline_info, shader_info);
}

void Editor::CreateGBufferResources()
{
	u32 width = m_viewport_panel->GetSize().x;
	u32 height = m_viewport_panel->GetSize().y;

	if (width == 0 || height == 0)
	{
		return;
	}

	m_gbuffer_position = TextureManager::GetInstance()->CreateTexture2D(
		"GBuffer_Position",
		VK_FORMAT_R16G16B16A16_SFLOAT,
		width, height
	);

	m_gbuffer_normal = TextureManager::GetInstance()->CreateTexture2D(
		"GBuffer_Normal",
		VK_FORMAT_R16G16B16A16_SFLOAT,
		width, height
	);

	m_gbuffer_albedo_ao = TextureManager::GetInstance()->CreateTexture2D(
		"GBuffer_AlbedoAO",
		VK_FORMAT_R16G16B16A16_SFLOAT,
		width, height
	);

	m_gbuffer_depth = TextureManager::GetInstance()->CreateTexture2D(
		"GBuffer_Depth",
		VK_FORMAT_D32_SFLOAT,
		width, height
	);

	m_gbuffer_lighting = TextureManager::GetInstance()->CreateTexture2D(
		"GBuffer_Lighting",
		VK_FORMAT_R16G16B16A16_SFLOAT,
		width, height
	);
}

void Editor::DestroyGBufferResources()
{
	TextureManager::GetInstance()->DestroyTexture2D("GBuffer_Position");
	TextureManager::GetInstance()->DestroyTexture2D("GBuffer_Normal");
	TextureManager::GetInstance()->DestroyTexture2D("GBuffer_AlbedoAO");
	TextureManager::GetInstance()->DestroyTexture2D("GBuffer_Depth");
	TextureManager::GetInstance()->DestroyTexture2D("GBuffer_Lighting");

	m_gbuffer_position = nullptr;
	m_gbuffer_normal = nullptr;
	m_gbuffer_albedo_ao = nullptr;
	m_gbuffer_depth = nullptr;
	m_gbuffer_lighting = nullptr;
}

void Editor::CreateGBufferDescriptors()
{
}

void Editor::UpdateGBufferDescriptors()
{
}

void Editor::RenderGBuffer()
{
	Pipeline* gbufferPipeline =
		PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GBUFFER);

	if (!gbufferPipeline ||
		!m_gbuffer_position ||
		!m_gbuffer_normal ||
		!m_gbuffer_albedo_ao ||
		!m_gbuffer_depth ||
		!m_gbuffer_lighting)
	{
		return;
	}

	DynamicRenderingAttachmentInfo positionAttachment{};
	positionAttachment.image = m_gbuffer_position->GetHandle();
	positionAttachment.image_view = m_gbuffer_position->GetImageView();
	positionAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	positionAttachment.image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	positionAttachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
	positionAttachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
	positionAttachment.clear_value.color = { 0.0f, 0.0f, 0.0f, 0.0f };
	positionAttachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	positionAttachment.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	DynamicRenderingAttachmentInfo normalAttachment = positionAttachment;
	normalAttachment.image = m_gbuffer_normal->GetHandle();
	normalAttachment.image_view = m_gbuffer_normal->GetImageView();
	normalAttachment.clear_value.color = { 0.5f, 0.5f, 1.0f, 1.0f };

	DynamicRenderingAttachmentInfo albedoAOAttachment = positionAttachment;
	albedoAOAttachment.image = m_gbuffer_albedo_ao->GetHandle();
	albedoAOAttachment.image_view = m_gbuffer_albedo_ao->GetImageView();
	albedoAOAttachment.clear_value.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	DynamicRenderingAttachmentInfo lightingAttachment = positionAttachment;
	lightingAttachment.image = m_gbuffer_lighting->GetHandle();
	lightingAttachment.image_view = m_gbuffer_lighting->GetImageView();
	lightingAttachment.clear_value.color = { 0.0f, 0.0f, 0.0f, 0.0f };

	DynamicRenderingAttachmentInfo depthAttachment{};
	depthAttachment.image = m_gbuffer_depth->GetHandle();
	depthAttachment.image_view = m_gbuffer_depth->GetImageView();
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.clear_value.depthStencil = { 1.0f, 0 };
	depthAttachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	DynamicRenderingInfo dri{};
	dri.extent = { m_viewport_last_size.x, m_viewport_last_size.y };
	dri.color_attachments = {
		positionAttachment,
		normalAttachment,
		albedoAOAttachment,
		lightingAttachment
	};
	dri.depth_attachment = depthAttachment;

	m_backend->BeginDynamicRenderingWithAttachments(dri);

	m_backend->SetViewport({
		{ 0.0f, 0.0f, (f32)m_viewport_last_size.x, (f32)m_viewport_last_size.y, 0.0f, 1.0f }
		});
	m_backend->SetScissor({
		{ { 0, 0 }, { m_viewport_last_size.x, m_viewport_last_size.y } }
		});

	m_backend->BindPipeline(gbufferPipeline);
	m_backend->BindDescriptorSet(gbufferPipeline, m_pbr_descriptor);

	MeshManager::GetInstance()->DrawMeshes(gbufferPipeline);

	m_backend->EndDynamicRenderingWithAttachments(dri);
}

void Editor::CreateGIPipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.type = PipelineType_Graphics;
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;
	pipeline_info.depth_clamp_enable = false;

	pipeline_info.dynamic_states = {
		PipelineDynamicState_Viewport,
		PipelineDynamicState_Scissor
	};

	pipeline_info.layout = {
		{
			{
				{ 0, DescriptorType_UniformBuffer,        1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_UniformBuffer,        1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 3, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 4, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 5, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 6, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		}
	};

	pipeline_info.vertex_binding_description = {};
	pipeline_info.vertex_attribute_descriptions = {};
	pipeline_info.depth_stencil_info = { false, false };

	pipeline_info.dynamic_rendering_info = {
		false,
		{ VK_FORMAT_R16G16B16A16_SFLOAT },
		VK_FORMAT_UNDEFINED
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "full_screen.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "global_illumination.frag");

	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_GLOBAL_ILLUMINATION, pipeline_info, shader_info);
}

void Editor::CreateGIResources()
{
	u32 width = m_viewport_panel->GetSize().x;
	u32 height = m_viewport_panel->GetSize().y;

	if (width == 0 || height == 0)
	{
		return;
	}

	m_gi_texture = TextureManager::GetInstance()->CreateTexture2D(
		"GlobalIllumination",
		VK_FORMAT_R16G16B16A16_SFLOAT,
		width, height
	);
}

void Editor::DestroyGIResources()
{
	TextureManager::GetInstance()->DestroyTexture2D("GlobalIllumination");

	m_gi_texture = nullptr;
}

void Editor::CreateGIDescriptors()
{
	Pipeline* giPipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GLOBAL_ILLUMINATION);
	m_gi_descriptor = m_backend->CreateDescriptorSet(giPipeline, 0);
}

void Editor::UpdateGIDescriptors()
{
	if (!m_gi_descriptor ||
		!m_gi_texture ||
		!m_gbuffer_position ||
		!m_gbuffer_normal ||
		!m_gbuffer_albedo_ao ||
		!m_gbuffer_depth ||
		!m_gbuffer_lighting)
	{
		return;
	}

	VkBuffer     globalBuf = m_pbr_global_data_ubo->GetHandle();
	VkDeviceSize globalOff = 0;
	VkDeviceSize globalRange = sizeof(GlobalData);

	DescriptorSetWriteData globalWrite{};
	globalWrite.type = DescriptorType_UniformBuffer;
	globalWrite.binding = 0;
	globalWrite.data.buffer.buffers = &globalBuf;
	globalWrite.data.buffer.offsets = &globalOff;
	globalWrite.data.buffer.ranges = &globalRange;

	VkBuffer     giBuf = m_pbr_gi_data_ubo->GetHandle();
	VkDeviceSize giOff = 0;
	VkDeviceSize giRange = sizeof(GlobalIlluminationSettings);

	DescriptorSetWriteData giSettingsWrite{};
	giSettingsWrite.type = DescriptorType_UniformBuffer;
	giSettingsWrite.binding = 1;
	giSettingsWrite.data.buffer.buffers = &giBuf;
	giSettingsWrite.data.buffer.offsets = &giOff;
	giSettingsWrite.data.buffer.ranges = &giRange;

	VkImageView posView = m_gbuffer_position->GetImageView();
	VkSampler   posSamp = m_gbuffer_position->GetSampler();

	DescriptorSetWriteData positionWrite{};
	positionWrite.type = DescriptorType_CombinedImageSampler;
	positionWrite.binding = 2;
	positionWrite.data.image.image_views = &posView;
	positionWrite.data.image.samplers = &posSamp;

	VkImageView normView = m_gbuffer_normal->GetImageView();
	VkSampler   normSamp = m_gbuffer_normal->GetSampler();

	DescriptorSetWriteData normalWrite{};
	normalWrite.type = DescriptorType_CombinedImageSampler;
	normalWrite.binding = 3;
	normalWrite.data.image.image_views = &normView;
	normalWrite.data.image.samplers = &normSamp;

	VkImageView albView = m_gbuffer_albedo_ao->GetImageView();
	VkSampler   albSamp = m_gbuffer_albedo_ao->GetSampler();

	DescriptorSetWriteData albedoAOWrite{};
	albedoAOWrite.type = DescriptorType_CombinedImageSampler;
	albedoAOWrite.binding = 4;
	albedoAOWrite.data.image.image_views = &albView;
	albedoAOWrite.data.image.samplers = &albSamp;

	VkImageView depthView = m_gbuffer_depth->GetImageView();
	VkSampler   depthSamp = m_gbuffer_depth->GetSampler();

	DescriptorSetWriteData depthWrite{};
	depthWrite.type = DescriptorType_CombinedImageSampler;
	depthWrite.binding = 5;
	depthWrite.data.image.image_views = &depthView;
	depthWrite.data.image.samplers = &depthSamp;

	VkImageView lightView = m_gbuffer_lighting->GetImageView();
	VkSampler   lightSamp = m_gbuffer_lighting->GetSampler();

	DescriptorSetWriteData lightingWrite{};
	lightingWrite.type = DescriptorType_CombinedImageSampler;
	lightingWrite.binding = 6;
	lightingWrite.data.image.image_views = &lightView;
	lightingWrite.data.image.samplers = &lightSamp;

	std::vector<DescriptorSetWriteData> writes = {
		globalWrite, giSettingsWrite, positionWrite,
		normalWrite, albedoAOWrite,   depthWrite,
		lightingWrite
	};

	m_backend->WriteDescriptor(&m_gi_descriptor, writes);
}

void Editor::RenderGI()
{
	if (!m_gi_texture)
	{
		return;
	}

	Pipeline* giPipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GLOBAL_ILLUMINATION);

	DynamicRenderingAttachmentInfo giAttachment{};
	giAttachment.image = m_gi_texture->GetHandle();
	giAttachment.image_view = m_gi_texture->GetImageView();
	giAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	giAttachment.image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	giAttachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
	giAttachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
	giAttachment.clear_value.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	giAttachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	giAttachment.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	DynamicRenderingInfo dri{};
	dri.extent = { m_viewport_last_size.x, m_viewport_last_size.y };
	dri.color_attachments = { giAttachment };

	m_backend->BeginDynamicRenderingWithAttachments(dri);

	m_backend->SetViewport({
		{ 0.0f, 0.0f, (f32)m_viewport_last_size.x, (f32)m_viewport_last_size.y, 0.0f, 1.0f }
		});
	m_backend->SetScissor({
		{ { 0, 0 }, { m_viewport_last_size.x, m_viewport_last_size.y } }
		});

	m_backend->BindPipeline(giPipeline);
	m_backend->BindDescriptorSet(giPipeline, m_gi_descriptor);

	m_backend->Draw(3, 1, 0, 0);

	m_backend->EndDynamicRenderingWithAttachments(dri);
}

void Editor::CreateShadowPipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.type = PipelineType_Graphics;
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;
	pipeline_info.depth_clamp_enable = true;

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
				{ 4, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		},
		{
			{
				{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
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
				{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS * MAX_SHADOW_CASCADES, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
				{ 1, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
				{ 2, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }
			},
			DescriptorSetFlags_UpdateAfterBindPool
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatTangent::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatTangent::GetAttributeDescriptions();

	pipeline_info.push_constant_ranges = { { ShaderStage_Vertex | ShaderStage_Fragment, 0, sizeof(glm::mat4) + sizeof(glm::vec4) } };
	pipeline_info.depth_stencil_info = { true, true, false, PipelineDethStencilCompareOp_Less };

	pipeline_info.dynamic_rendering_info = { false, {}, VK_FORMAT_D32_SFLOAT };

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(EDITOR_SHADER_PATH, "shadow_depth.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(EDITOR_SHADER_PATH, "shadow_depth.frag");

	PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_SHADOW_DEPTH, pipeline_info, shader_info);
}

void Editor::CreateShadowResources()
{
	for (size_t i = 0; i < m_shadow_maps.size(); ++i)
	{
		std::string name = "ShadowMap_" + std::to_string(i);
		TextureManager::GetInstance()->DestroyTexture2D(name);
	}
	m_shadow_maps.clear();

	for (size_t i = 0; i < m_point_shadow_maps.size(); ++i)
	{
		std::string name = "PointShadowMap_" + std::to_string(i);
		TextureManager::GetInstance()->DestroyTextureCubemap(name);
	}
	m_point_shadow_maps.clear();

	u32 shadowMapSize = m_editor_settings.shadow_settings.shadow_map_size;
	u32 shadowMapCount = MAX_LIGHTS * MAX_SHADOW_CASCADES;

	// 2D shadow maps for directional (cascades) and spot lights
	for (u32 i = 0; i < shadowMapCount; ++i)
	{
		std::string name = "ShadowMap_" + std::to_string(i);
		VulkanTexture2D* shadowMap = TextureManager::GetInstance()->CreateTexture2D(
			name, VK_FORMAT_D32_SFLOAT, shadowMapSize, shadowMapSize
		);
		m_shadow_maps.push_back(shadowMap);
	}

	// Cube shadow maps for point lights — one per light slot
	for (u32 i = 0; i < MAX_LIGHTS; ++i)
	{
		std::string name = "PointShadowMap_" + std::to_string(i);
		VulkanTextureCubemap* cubemap = TextureManager::GetInstance()->CreateTextureCubemap(
			name, VK_FORMAT_D32_SFLOAT, shadowMapSize
		);
		m_point_shadow_maps.push_back(cubemap);
	}
}

void Editor::DestroyShadowResources()
{
	for (size_t i = 0; i < m_shadow_maps.size(); ++i)
	{
		std::string name = "ShadowMap_" + std::to_string(i);
		TextureManager::GetInstance()->DestroyTexture2D(name);
	}
	m_shadow_maps.clear();

	for (size_t i = 0; i < m_point_shadow_maps.size(); ++i)
	{
		std::string name = "PointShadowMap_" + std::to_string(i);
		TextureManager::GetInstance()->DestroyTextureCubemap(name);
	}
	m_point_shadow_maps.clear();
}

void Editor::CreateShadowDescriptors()
{
	m_textures_descriptor = MeshManager::GetInstance()->GetTexturesDescriptor();

	if (!m_textures_descriptor || m_shadow_maps.empty() || m_point_shadow_maps.empty())
		return;

	// -- binding 0: 2D shadow maps 
	{
		std::vector<VkImageView> views;
		std::vector<VkSampler>   samplers;
		views.reserve(m_shadow_maps.size());
		samplers.reserve(m_shadow_maps.size());

		for (auto* sm : m_shadow_maps)
		{
			views.push_back(sm->GetImageView());
			samplers.push_back(sm->GetSampler());
		}

		DescriptorSetWriteData write{};
		write.type = DescriptorType_CombinedImageSampler;
		write.binding = 0;
		write.data.image.image_views = views.data();
		write.data.image.samplers = samplers.data();

		std::vector<DescriptorSetWriteData> writes = { write };
		m_backend->WriteDescriptorVariable(&m_textures_descriptor, writes, static_cast<u32>(m_shadow_maps.size()), 0);
	}

	// -- binding 1: point light cube shadow maps
	{
		std::vector<VkImageView> views;
		std::vector<VkSampler>   samplers;
		views.reserve(m_point_shadow_maps.size());
		samplers.reserve(m_point_shadow_maps.size());

		for (auto* cm : m_point_shadow_maps)
		{
			views.push_back(cm->GetImageView());
			samplers.push_back(cm->GetSampler());
		}

		DescriptorSetWriteData write{};
		write.type = DescriptorType_CombinedImageSampler;
		write.binding = 1;
		write.data.image.image_views = views.data();
		write.data.image.samplers = samplers.data();

		std::vector<DescriptorSetWriteData> writes = { write };
		m_backend->WriteDescriptorVariable(&m_textures_descriptor, writes, static_cast<u32>(m_point_shadow_maps.size()), 0);
	}
}

void Editor::UpdateShadowDescriptors()
{
}

void Editor::RenderShadowMaps()
{
	if (!m_editor_settings.shadow_settings.enabled) return;

	struct ShadowPushConstants
	{
		glm::mat4 lightViewProj;
		glm::vec4 lightPosAndFar;
	};

	Scene* scene = SceneManager::GetInstance()->GetActiveScene();
	if (!scene) return;

	Pipeline* shadowPipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_SHADOW_DEPTH);
	if (!shadowPipeline) return;

	u32  shadowMapSize = m_editor_settings.shadow_settings.shadow_map_size;
	auto view = scene->GetRegistry().view<LightComponent, TransformComponent>();

	u32 lightIndex = 0;
	for (auto entity : view)
	{
		auto& light_component = view.get<LightComponent>(entity);

		if (!light_component.enabled)
			continue;

		if (lightIndex >= MAX_LIGHTS)
			break;

		if (!light_component.cast_shadows)
		{
			lightIndex++;
			continue;
		}

		LightGPUData& gpuLight = m_lights_data.lights[lightIndex];

		if (light_component.type == LightType_Point)
		{
			VulkanTextureCubemap* cubemap = m_point_shadow_maps[lightIndex];

			// Transition the entire cubemap to attachment layout once before rendering any face.
			// BeginDynamicRenderingWithAttachments barriers on the whole VkImage handle which
			// would conflict with per-face transitions if done inside the loop.
			m_backend->TransitionTexture(
				cubemap,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			);

			for (u32 face = 0; face < 6; ++face)
			{
				DynamicRenderingAttachmentInfo depthAttachment{};
				depthAttachment.image = cubemap->GetHandle();
				depthAttachment.image_view = cubemap->GetFaceImageView(face);
				depthAttachment.format = VK_FORMAT_D32_SFLOAT;
				depthAttachment.image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
				depthAttachment.clear_value.depthStencil = { 1.0f, 0 };
				// Already in attachment layout — no transition needed inside the loop
				depthAttachment.initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				DynamicRenderingInfo shadowDRI{};
				shadowDRI.extent = { shadowMapSize, shadowMapSize };
				shadowDRI.depth_attachment = depthAttachment;
				shadowDRI.flags = 0;

				m_backend->BeginDynamicRenderingWithAttachments(shadowDRI);

				m_backend->SetViewport({ { 0.0f, 0.0f, static_cast<f32>(shadowMapSize), static_cast<f32>(shadowMapSize), 0.0f, 1.0f } });
				m_backend->SetScissor({ { { 0, 0 }, { shadowMapSize, shadowMapSize } } });

				m_backend->BindPipeline(shadowPipeline);
				m_backend->BindDescriptorSet(shadowPipeline, m_pbr_descriptor);
				ShadowPushConstants pc;
				pc.lightViewProj = gpuLight.point_matrices[face];
				pc.lightPosAndFar = glm::vec4(gpuLight.position_type.x, gpuLight.position_type.y, gpuLight.position_type.z, gpuLight.direction_range.w);

				m_backend->BindPushConstants(shadowPipeline, ShaderStage_Vertex | ShaderStage_Fragment, 0, sizeof(ShadowPushConstants), &pc);

				MeshManager::GetInstance()->DrawMeshes(shadowPipeline);

				m_backend->EndDynamicRenderingWithAttachments(shadowDRI);
			}

			// Transition entire cubemap to shader read after all 6 faces are done
			m_backend->TransitionTexture(
				cubemap,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
			);
		}
		else
		{
			const u32 cascadeCount = light_component.type == LightType_Directional
				? MAX_SHADOW_CASCADES : 1;

			for (u32 cascade = 0; cascade < cascadeCount; ++cascade)
			{
				const u32 shadowMapIndex = light_component.type == LightType_Directional
					? lightIndex * MAX_SHADOW_CASCADES + cascade
					: lightIndex * MAX_SHADOW_CASCADES;

				if (shadowMapIndex >= m_shadow_maps.size())
					continue;

				glm::mat4 lightViewProj = light_component.type == LightType_Directional
					? gpuLight.cascade_matrices[cascade]
					: gpuLight.shadow_matrix;

				VulkanTexture2D* shadowMap = m_shadow_maps[shadowMapIndex];

				DynamicRenderingAttachmentInfo depthAttachment{};
				depthAttachment.image = shadowMap->GetHandle();
				depthAttachment.image_view = shadowMap->GetImageView();
				depthAttachment.format = VK_FORMAT_D32_SFLOAT;
				depthAttachment.image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
				depthAttachment.clear_value.depthStencil = { 1.0f, 0 };
				depthAttachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthAttachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

				DynamicRenderingInfo shadowDRI{};
				shadowDRI.extent = { shadowMapSize, shadowMapSize };
				shadowDRI.depth_attachment = depthAttachment;
				shadowDRI.flags = 0;

				m_backend->BeginDynamicRenderingWithAttachments(shadowDRI);

				m_backend->SetViewport({ { 0.0f, 0.0f, static_cast<f32>(shadowMapSize), static_cast<f32>(shadowMapSize), 0.0f, 1.0f } });
				m_backend->SetScissor({ { { 0, 0 }, { shadowMapSize, shadowMapSize } } });

				m_backend->BindPipeline(shadowPipeline);
				m_backend->BindDescriptorSet(shadowPipeline, m_pbr_descriptor);
				ShadowPushConstants pc;
				pc.lightViewProj = lightViewProj;
				pc.lightPosAndFar = glm::vec4(0.0f);

				m_backend->BindPushConstants(shadowPipeline, ShaderStage_Vertex | ShaderStage_Fragment, 0, sizeof(ShadowPushConstants), &pc);

				MeshManager::GetInstance()->DrawMeshes(shadowPipeline);

				m_backend->EndDynamicRenderingWithAttachments(shadowDRI);
			}
		}

		lightIndex++;
	}
}

void Editor::CreateDebugPipeline()
{
	// Debug Wireframe Pipeline
	{
		PipelineCreateInfo pipeline_info = {};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
		pipeline_info.polygon_mode = PipelinePolygonMode_Line;
		pipeline_info.cull_mode = PipelineCullMode_None;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.line_width = 1.0f;
		pipeline_info.depth_clamp_enable = false;

		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
		pipeline_info.layout = {
			{
				{
					{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex | ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
					{ 1, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
					{ 2, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
					{ 3, DescriptorType_UniformBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind },
					{ 4, DescriptorType_CombinedImageSampler, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
				},
				DescriptorSetFlags_UpdateAfterBindPool
			},
			{
				{
					{ 0, DescriptorType_StorageBuffer, 1, ShaderStage_Fragment, DescriptorBindingFlags_UpdateAfterBind }
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
					{ 0, DescriptorType_CombinedImageSampler, MAX_LIGHTS * MAX_SHADOW_CASCADES, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
					{ 1, DescriptorType_CombinedImageSampler, MAX_LIGHTS, ShaderStage_Fragment, DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind },
					{ 2, DescriptorType_CombinedImageSampler, MAX_TEXTURES, ShaderStage_Fragment, DescriptorBindingFlags_VariableCount | DescriptorBindingFlags_PartiallyBound | DescriptorBindingFlags_UpdateAfterBind }
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

		PipelineManager::GetInstance()->CreatePipeline(C_PIPELINE_GIZMO, pipeline_info, shader_info);
	}
}

void Editor::CreateDebugResources()
{
	m_gizmo_vb = m_backend->CreateVertexBufferEmpty(10000);
	m_gizmo_ib = m_backend->CreateIndexBufferEmpty(10000);
}

void Editor::DestroyDebugResources()
{
	if ((m_gizmo_vb != nullptr) && (m_gizmo_ib != nullptr))
	{
		m_backend->DestroyBuffer(m_gizmo_vb);
		m_backend->DestroyBuffer(m_gizmo_ib);
	}
}

void Editor::CreateDebugDescriptors()
{
}

void Editor::UpdateDebugDescriptors()
{
}

void Editor::UpdateDebug()
{
	m_gizmo_vertices.clear();
	m_gizmo_indices.clear();

	Scene* active_scene = SceneManager::GetInstance()->GetActiveScene();
	if (active_scene == nullptr)
	{
		return;
	}

	UpdateLightGizmos();

	if (!m_gizmo_vertices.empty())
	{
		u32 vb_size = static_cast<u32>(m_gizmo_vertices.size() * sizeof(VertexGuizmo));
		u32 ib_size = static_cast<u32>(m_gizmo_indices.size() * sizeof(u32));

		m_backend->UpdateVertexBuffer(m_gizmo_vb, 0, m_gizmo_vertices.data(), vb_size);
		m_backend->UpdateIndexBuffer(m_gizmo_ib, 0, m_gizmo_indices.data(), ib_size);
	}
}

void Editor::RenderDebug()
{
	Pipeline* gizmoPipeline = PipelineManager::GetInstance()->GetPipeline(C_PIPELINE_GIZMO);

	m_backend->BindPipeline(gizmoPipeline);

	glm::mat4 vp = m_editor_camera.GetProjection() * m_editor_camera.GetView();
	m_backend->BindPushConstants(gizmoPipeline, ShaderStage_Vertex, 0, sizeof(glm::mat4), &vp);

	m_backend->BindVertexBuffer(m_gizmo_vb, 0);
	m_backend->BindIndexBuffer(m_gizmo_ib, 0);

	m_backend->DrawIndexed(static_cast<u32>(m_gizmo_indices.size()), 1, 0, 0, 0);
}

void Editor::UpdateLightGizmos()
{
	const auto& view = SceneManager::GetInstance()->GetActiveScene()->GetRegistry().view<LightComponent, TransformComponent>();
	for (auto& entity : view)
	{
		Entity e = { entity, SceneManager::GetInstance()->GetActiveScene() };

		if (e.HasComponent<LightComponent>())
		{
			auto& light_component = e.GetComponent<LightComponent>();
			auto& transform_component = e.GetComponent<TransformComponent>();

			u32 base_index = static_cast<u32>(m_gizmo_vertices.size());

			glm::vec3 world_position = glm::vec3(transform_component.world_transform[3]);
			glm::vec4 light_color = glm::vec4(light_component.color, 1.0f);

			if (light_component.type == LightType_Point)
			{
				f32       radius = light_component.range * 0.1f;
				const u32 segments = 16;
				const u32 rings = 8;

				for (u32 ring = 0; ring <= rings; ++ring)
				{
					f32 phi = glm::pi<f32>() * ring / rings;
					for (u32 seg = 0; seg <= segments; ++seg)
					{
						f32 theta = 2.0f * glm::pi<f32>() * seg / segments;
						glm::vec3 p = world_position + radius * glm::vec3(
							sin(phi) * cos(theta),
							cos(phi),
							sin(phi) * sin(theta)
						);
						m_gizmo_vertices.push_back({ p, light_color });
					}
				}

				for (u32 ring = 0; ring < rings; ++ring)
				{
					for (u32 seg = 0; seg < segments; ++seg)
					{
						u32 curr = base_index + ring * (segments + 1) + seg;
						u32 next = curr + segments + 1;

						m_gizmo_indices.push_back(curr);
						m_gizmo_indices.push_back(curr + 1);

						m_gizmo_indices.push_back(curr);
						m_gizmo_indices.push_back(next);
					}
				}
			}
			else if (light_component.type == LightType_Directional)
			{
				glm::vec3 forward = glm::normalize(glm::vec3(transform_component.world_transform * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
				f32       arrowLength = 2.0f;
				f32       arrowHeadSize = 0.5f;

				glm::vec3 arrowEnd = world_position + forward * arrowLength;

				m_gizmo_vertices.push_back({ world_position, light_color });
				m_gizmo_vertices.push_back({ arrowEnd, light_color });
				m_gizmo_indices.push_back(base_index);
				m_gizmo_indices.push_back(base_index + 1);

				glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
				glm::vec3 up = glm::cross(right, forward);

				m_gizmo_vertices.push_back({ arrowEnd, light_color });
				m_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize + right * arrowHeadSize * 0.5f, light_color });
				m_gizmo_indices.push_back(base_index + 1);
				m_gizmo_indices.push_back(base_index + 2);

				m_gizmo_vertices.push_back({ arrowEnd, light_color });
				m_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize - right * arrowHeadSize * 0.5f, light_color });
				m_gizmo_indices.push_back(base_index + 1);
				m_gizmo_indices.push_back(base_index + 3);

				m_gizmo_vertices.push_back({ arrowEnd, light_color });
				m_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize + up * arrowHeadSize * 0.5f, light_color });
				m_gizmo_indices.push_back(base_index + 1);
				m_gizmo_indices.push_back(base_index + 4);

				m_gizmo_vertices.push_back({ arrowEnd, light_color });
				m_gizmo_vertices.push_back({ arrowEnd - forward * arrowHeadSize - up * arrowHeadSize * 0.5f, light_color });
				m_gizmo_indices.push_back(base_index + 1);
				m_gizmo_indices.push_back(base_index + 5);
			}
			else if (light_component.type == LightType_Spot)
			{
				glm::vec3 forward = glm::normalize(glm::vec3(transform_component.world_transform * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
				glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
				glm::vec3 up = glm::cross(right, forward);

				f32       coneLength = light_component.range * 0.5f;
				f32       coneRadius = tan(light_component.outer_cone_angle) * coneLength;
				const u32 segments = 16;
				glm::vec3 coneBase = world_position + forward * coneLength;

				for (u32 i = 0; i < segments; ++i)
				{
					f32 angle1 = 2.0f * glm::pi<f32>() * i / segments;
					f32 angle2 = 2.0f * glm::pi<f32>() * ((i + 1) % segments) / segments;

					glm::vec3 p1 = coneBase + coneRadius * (cos(angle1) * right + sin(angle1) * up);
					glm::vec3 p2 = coneBase + coneRadius * (cos(angle2) * right + sin(angle2) * up);

					m_gizmo_vertices.push_back({ p1, light_color });
					m_gizmo_vertices.push_back({ p2, light_color });
					m_gizmo_indices.push_back(base_index + i * 2);
					m_gizmo_indices.push_back(base_index + i * 2 + 1);

					if (i % 4 == 0)
					{
						m_gizmo_vertices.push_back({ world_position, light_color });
						m_gizmo_vertices.push_back({ p1, light_color });
						u32 apexIdx = static_cast<u32>(m_gizmo_vertices.size()) - 2;
						m_gizmo_indices.push_back(apexIdx);
						m_gizmo_indices.push_back(apexIdx + 1);
					}
				}
			}
		}
	}
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

void Editor::ShowTransformGizmo()
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
	proj[1][1] *= -1.0f;

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

		static glm::quat lastQ = glm::quat(1, 0, 0, 0);
		if (glm::dot(lastQ, r) < 0.0f) r = -r;
		lastQ = r;

		auto& registry = SceneManager::GetInstance()->GetActiveScene()->GetRegistry();
		registry.patch<TransformComponent>(selected.GetHandle(), [&](TransformComponent& tc)
			{
				tc.local_position = t;
				tc.local_rotation = glm::eulerAngles(r);
				tc.local_scale = s;
				tc.is_dirty = true;
			});
	}
	else
	{
		m_viewport_panel->CanPick(true);
	}
}

void Editor::LoadLightData()
{
	Scene* active_scene = SceneManager::GetInstance()->GetActiveScene();
	if (!active_scene)
	{
		m_lights_data.light_count = 0;
		return;
	}

	auto view = active_scene->GetRegistry().view<LightComponent, TransformComponent>();

	u32 lightIndex = 0;
	for (auto& entity : view)
	{
		if (lightIndex >= MAX_LIGHTS)
			break;

		auto& light_component = view.get<LightComponent>(entity);
		auto& transform_component = view.get<TransformComponent>(entity);

		if (!light_component.enabled)
			continue;

		LightGPUData& gpuLight = m_lights_data.lights[lightIndex];

		glm::vec3 world_position = glm::vec3(transform_component.world_transform[3]);

		f32 scaleX = glm::length(glm::vec3(transform_component.world_transform[0]));
		f32 scaleY = glm::length(glm::vec3(transform_component.world_transform[1]));
		f32 scaleZ = glm::length(glm::vec3(transform_component.world_transform[2]));
		f32 maxWorldScale = glm::max(scaleX, glm::max(scaleY, scaleZ));
		f32 effectiveRange = light_component.range * glm::max(maxWorldScale, 0.0001f);

		effectiveRange = glm::max(effectiveRange, 0.5f);

		f32 lightType = static_cast<f32>(light_component.type);

		glm::vec3 forward = -glm::normalize(glm::vec3(transform_component.world_transform[2]));

		if (light_component.type == LightType_Point)
		{
			gpuLight.position_type = glm::vec4(world_position, lightType);
			gpuLight.direction_range = glm::vec4(0.0f, 0.0f, 0.0f, effectiveRange);
		}
		else if (light_component.type == LightType_Directional)
		{
			gpuLight.position_type = glm::vec4(forward, lightType);
			gpuLight.direction_range = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (light_component.type == LightType_Spot)
		{
			gpuLight.position_type = glm::vec4(world_position, lightType);
			gpuLight.direction_range = glm::vec4(forward, effectiveRange);
			gpuLight.cone_attenuation.x = glm::cos(light_component.inner_cone_angle);
			gpuLight.cone_attenuation.y = glm::cos(light_component.outer_cone_angle);
		}

		gpuLight.color_intensity = glm::vec4(light_component.color, light_component.intensity);
		gpuLight.cone_attenuation.z = light_component.attenuation;
		gpuLight.cone_attenuation.w = 1.0f;

		// Default shadow_params — overridden per light type below
		gpuLight.shadow_params.x = static_cast<f32>(lightIndex * MAX_SHADOW_CASCADES);
		gpuLight.shadow_params.y = m_editor_settings.shadow_settings.min_bias;
		gpuLight.shadow_params.z = 1.0f;
		gpuLight.shadow_params.w = light_component.cast_shadows ? 1.0f : 0.0f;

		if (light_component.cast_shadows && m_editor_settings.shadow_settings.enabled)
		{
			if (light_component.type == LightType_Directional)
			{
				LoadShadowData(forward, lightIndex, gpuLight);
				gpuLight.shadow_matrix = gpuLight.cascade_matrices[0];
			}
			else if (light_component.type == LightType_Spot)
			{
				glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
				if (glm::abs(glm::dot(up, forward)) > 0.95f)
					up = glm::vec3(1.0f, 0.0f, 0.0f);

				glm::mat4 lightView = glm::lookAt(world_position, world_position + forward, up);

				f32 fov = glm::clamp(light_component.outer_cone_angle * 2.0f, glm::radians(1.0f), glm::radians(179.0f));
				f32 nearPlane = 0.1f;
				f32 farPlane = effectiveRange;

				glm::mat4 lightProj = glm::perspectiveZO(fov, 1.0f, nearPlane, farPlane);
				lightProj[1][1] *= -1.0f;

				gpuLight.shadow_matrix = lightProj * lightView;
			}
			else if (light_component.type == LightType_Point)
			{
				gpuLight.shadow_params.x = static_cast<f32>(lightIndex);

				const glm::vec3 dirs[6] = {
					{ 1.0f,  0.0f,  0.0f },	// +X
					{-1.0f,  0.0f,  0.0f }, // -X
					{ 0.0f,  1.0f,  0.0f }, // +Y
					{ 0.0f, -1.0f,  0.0f }, // -Y
					{ 0.0f,  0.0f,  1.0f }, // +Z
					{ 0.0f,  0.0f, -1.0f }, // -Z
				};

				const glm::vec3 ups[6] = {
					{ 0.0f, -1.0f,  0.0f },  // +X
					{ 0.0f, -1.0f,  0.0f },  // -X
					{ 0.0f,  0.0f,  1.0f },  // +Y
					{ 0.0f,  0.0f, -1.0f },  // -Y
					{ 0.0f, -1.0f,  0.0f },  // +Z
					{ 0.0f, -1.0f,  0.0f },  // -Z
				};
				f32 aspect = 1.0f;
				f32 nearPlane = 0.1f;
				f32 farPlane = effectiveRange;

				glm::mat4 proj = glm::perspectiveZO(glm::radians(90.0f), aspect, nearPlane, farPlane);

				for (u32 face = 0; face < 6; ++face)
				{
					glm::mat4 faceView = glm::lookAt(world_position, world_position + dirs[face], ups[face]);
					gpuLight.point_matrices[face] = proj * faceView;
				}

				gpuLight.shadow_matrix = glm::mat4(1.0f);
			}
		}
		else
		{
			gpuLight.shadow_matrix = glm::mat4(1.0f);

			if (light_component.type == LightType_Point)
			{
				for (u32 face = 0; face < 6; ++face)
					gpuLight.point_matrices[face] = glm::mat4(1.0f);
			}
		}

		lightIndex++;
	}

	m_lights_data.light_count = lightIndex;
}

glm::mat4 Editor::ComputeCascadeMatrix(const glm::vec3& lightDir, f32 cascadeNear, f32 cascadeFar, f32 shadowMapSize)
{
	const f32 aspect = m_viewport_last_size.y > 0
		? static_cast<f32>(m_viewport_last_size.x) / static_cast<f32>(m_viewport_last_size.y)
		: 1.0f;

	glm::mat4 cameraProj = glm::perspectiveZO(
		glm::radians(m_editor_camera.GetFov()),
		aspect,
		cascadeNear,
		cascadeFar
	);
	cameraProj[1][1] *= -1.0f;
	glm::mat4 cameraView = m_editor_camera.GetView();

	std::array<glm::vec3, 8> corners = Camera::GetFrustumCornersWorldSpace(cameraProj, cameraView);
	glm::vec3 frustumCenter = Camera::GetFrustumCenter(corners);
	f32       radius = Camera::GetFrustumRadius(corners, frustumCenter);

	f32 texelSizeWorldSpace = (radius * 2.0f) / shadowMapSize;
	radius = std::ceil(radius / texelSizeWorldSpace) * texelSizeWorldSpace;

	glm::vec3 up(0.0f, 1.0f, 0.0f);
	if (glm::abs(glm::dot(up, lightDir)) > 0.95f)
		up = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 lightPos = frustumCenter - lightDir * radius;
	glm::mat4 lightView = glm::lookAt(lightPos, frustumCenter, up);

	glm::vec4 frustumCenterLS = lightView * glm::vec4(frustumCenter, 1.0f);
	frustumCenterLS.x = std::floor(frustumCenterLS.x / texelSizeWorldSpace) * texelSizeWorldSpace;
	frustumCenterLS.y = std::floor(frustumCenterLS.y / texelSizeWorldSpace) * texelSizeWorldSpace;

	glm::vec4 snappedCenterWS = glm::inverse(lightView) * frustumCenterLS;
	lightPos = glm::vec3(snappedCenterWS) - lightDir * radius;
	lightView = glm::lookAt(lightPos, glm::vec3(snappedCenterWS), up);

	f32 minZ = F32MAX;
	f32 maxZ = F32MIN;
	for (const glm::vec3& corner : corners)
	{
		glm::vec4 cornerLS = lightView * glm::vec4(corner, 1.0f);
		minZ = glm::min(minZ, cornerLS.z);
		maxZ = glm::max(maxZ, cornerLS.z);
	}

	constexpr f32 zMult = 10.0f;
	if (minZ < 0.0f) minZ *= zMult; else minZ /= zMult;
	if (maxZ < 0.0f) maxZ /= zMult; else maxZ *= zMult;

	glm::mat4 lightProj = glm::orthoZO(-radius, radius, -radius, radius, -maxZ, -minZ);
	lightProj[1][1] *= -1.0f;

	return lightProj * lightView;
}

void Editor::LoadShadowData(const glm::vec3& lightDir, u32 lightIndex, LightGPUData& gpuLight)
{
	const f32 cameraNear = m_editor_camera.GetNear();
	const f32 cameraFar = m_editor_camera.GetFar();
	const f32 lambda = m_editor_settings.shadow_settings.cascade_split_lambda;
	const f32 shadowMapSize = static_cast<f32>(m_editor_settings.shadow_settings.shadow_map_size);

	glm::vec4 splits(0.0f);
	for (u32 i = 0; i < MAX_SHADOW_CASCADES; ++i)
	{
		f32 p = static_cast<f32>(i + 1) / static_cast<f32>(MAX_SHADOW_CASCADES);
		f32 logSplit = cameraNear * std::pow(cameraFar / cameraNear, p);
		f32 uniformSplit = cameraNear + (cameraFar - cameraNear) * p;
		splits[i] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
	}

	for (u32 cascade = 0; cascade < MAX_SHADOW_CASCADES; ++cascade)
	{
		f32       cascadeNear = (cascade == 0) ? cameraNear : splits[cascade - 1];
		const f32 cascadeFar = splits[cascade];

		gpuLight.cascade_matrices[cascade] = ComputeCascadeMatrix(lightDir, cascadeNear, cascadeFar, shadowMapSize);
	}

	gpuLight.cascade_splits = splits;
	gpuLight.shadow_params.x = static_cast<f32>(lightIndex * MAX_SHADOW_CASCADES);
	gpuLight.shadow_params.z = static_cast<f32>(MAX_SHADOW_CASCADES);
}
