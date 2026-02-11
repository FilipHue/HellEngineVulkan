#include "sandbox.h"

void ShapeManager::CreateCube(f32 size)
{
	ShapeData cube{};
	const glm::vec4 baseColor = RandomColor();
	const f32 h = size * 0.5f;

	auto push_face = [&](glm::vec3 n, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
		{
			const u32 base = (u32)cube.vertices.size();

			cube.vertices.emplace_back(v0, baseColor, glm::vec2(1.0f, 0.0f), n);
			cube.vertices.emplace_back(v1, baseColor, glm::vec2(1.0f, 0.0f), n);
			cube.vertices.emplace_back(v2, baseColor, glm::vec2(1.0f, 1.0f), n);
			cube.vertices.emplace_back(v3, baseColor, glm::vec2(0.0f, 1.0f), n);

			cube.indices.insert(cube.indices.end(),
				{
					base + 0, base + 1, base + 2,
					base + 0, base + 2, base + 3
				});
		};

	/* +Z */ push_face({ 0,0,1 }, { -h,-h, h }, { h,-h, h }, { h, h, h }, { -h, h, h });
	/* -Z */ push_face({ 0,0,-1 }, { h,-h,-h }, { -h,-h,-h }, { -h, h,-h }, { h, h,-h });
	/* +X */ push_face({ 1,0,0 }, { h,-h, h }, { h,-h,-h }, { h, h,-h }, { h, h, h });
	/* -X */ push_face({ -1,0,0 }, { -h,-h,-h }, { -h,-h, h }, { -h, h, h }, { -h, h,-h });
	/* +Y */ push_face({ 0,1,0 }, { -h, h, h }, { h, h, h }, { h, h,-h }, { -h, h,-h });
	/* -Y */ push_face({ 0,-1,0 }, { -h,-h,-h }, { h,-h,-h }, { h,-h, h }, { -h,-h, h });

	shapes.push_back(cube);
}

void ShapeManager::CreateUVSphere(f32 radius, u32 sector_count, u32 stack_count)
{
	ShapeData sphere{};
	const glm::vec4 baseColor = RandomColor();
	const f32 PI = glm::pi<f32>();

	for (u32 i = 0; i <= stack_count; ++i)
	{
		f32 v = (f32)i / stack_count;
		f32 phi = PI * 0.5f - v * PI;

		for (u32 j = 0; j <= sector_count; ++j)
		{
			f32 u = (f32)j / sector_count;
			f32 theta = u * 2.0f * PI;

			glm::vec3 pos{
				radius * cos(phi) * cos(theta),
				radius * sin(phi),
				radius * cos(phi) * sin(theta)
			};

			glm::vec3 normal = glm::normalize(pos);
			glm::vec4 color = baseColor * glm::vec4(normal * 0.5f + 0.5f, 1.0f);

			sphere.vertices.emplace_back(pos, color, glm::vec2(u, v), normal);
		}
	}

	for (u32 i = 0; i < stack_count; ++i)
	{
		u32 k1 = i * (sector_count + 1);
		u32 k2 = k1 + sector_count + 1;

		for (u32 j = 0; j < sector_count; ++j)
		{
			if (i != 0)
				sphere.indices.insert(sphere.indices.end(), { k1 + j, k2 + j, k2 + j + 1 });

			if (i != stack_count - 1)
				sphere.indices.insert(sphere.indices.end(), { k1 + j, k2 + j + 1, k1 + j + 1 });
		}
	}

	shapes.push_back(sphere);
}

void ShapeManager::CreateCone(f32 radius, f32 height, u32 sector_count)
{
	ShapeData cone{};
	const glm::vec4 baseColor = RandomColor();
	const f32 PI = glm::pi<f32>();
	const f32 halfH = height * 0.5f;
	const f32 ny = radius / height;

	for (u32 i = 0; i <= sector_count; ++i)
	{
		f32 u = (f32)i / sector_count;
		f32 theta = u * 2.0f * PI;

		f32 x = radius * cos(theta);
		f32 z = radius * sin(theta);

		glm::vec3 pos(x, -halfH, z);
		glm::vec3 normal = glm::normalize(glm::vec3(x, ny, z));
		glm::vec4 color = baseColor * glm::vec4(normal * 0.5f + 0.5f, 1.0f);

		cone.vertices.emplace_back(pos, color, glm::vec2{ u, 0.0f }, normal);
	}

	u32 apexIndex = (u32)cone.vertices.size();
	cone.vertices.emplace_back(
		glm::vec3(0, halfH, 0),
		baseColor,
		glm::vec2(0.5f, 1.0f),
		glm::vec3(0, 1, 0)
	);

	u32 baseRingStart = (u32)cone.vertices.size();
	for (u32 i = 0; i <= sector_count; ++i)
	{
		f32 u = (f32)i / sector_count;
		f32 theta = u * 2.0f * PI;

		f32 x = radius * cos(theta);
		f32 z = radius * sin(theta);

		cone.vertices.emplace_back(
			glm::vec3(x, -halfH, z),
			baseColor,
			glm::vec2{ 0.5f + x / (2 * radius), 0.5f + z / (2 * radius) },
			glm::vec3{ 0,-1,0 }
		);
	}

	u32 baseCenter = (u32)cone.vertices.size();
	cone.vertices.emplace_back(glm::vec3(0, -halfH, 0), baseColor, glm::vec2{ 0.5f,0.5f }, glm::vec3{ 0,-1,0 });

	for (u32 i = 0; i < sector_count; ++i)
	{
		cone.indices.insert(cone.indices.end(), { i, i + 1, apexIndex });
		cone.indices.insert(cone.indices.end(), { baseRingStart + i, baseCenter, baseRingStart + i + 1 });
	}

	shapes.push_back(cone);
}

SandboxApplication::SandboxApplication(ApplicationConfiguration* configuration) : Application(configuration)
{
	m_cursor_mode = GLFW_CURSOR_NORMAL;
}

SandboxApplication::~SandboxApplication()
{
	NO_OP;
}

void SandboxApplication::Init()
{
	EventDispatcher::AddListener(EventType_KeyPressed, HE_BIND_EVENTCALLBACK(OnKeyPressed));
	EventDispatcher::AddListener(EventType_WindowClose, HE_BIND_EVENTCALLBACK(OnWindowClose));
	EventDispatcher::AddListener(EventType_WindowResize, HE_BIND_EVENTCALLBACK(OnWindowResize));
	EventDispatcher::AddListener(EventType_WindowIconified, HE_BIND_EVENTCALLBACK(OnWindowMinimised));

	SetCursorMode(m_cursor_mode);

	m_camera = PerspectiveCamera();
	m_camera.Create(45.0f, (f32)m_window->GetWidth() / (f32)m_window->GetHeight(), 0.001f, 1000.0f);
	m_camera.SetPosition({ 0.0f, 0.0f, 50.0f });

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_controller = PerspectiveController();
	m_controller.Init();
	m_controller.SetCamera(&m_camera);

	m_first_frame = true;

	CreateResources();
}

void SandboxApplication::Shutdown()
{
	if (m_camera_buffer) m_backend->DestroyBuffer(m_camera_buffer);

	if (m_wireframe_cube)
	{
		m_wireframe_cube->DestroyBuffers(m_backend);
		delete m_wireframe_cube;
		m_wireframe_cube = nullptr;
	}

	if (m_shape_manager)
	{
		m_shape_manager->DestroyBuffers(m_backend);
		delete m_shape_manager;
		m_shape_manager = nullptr;
	}

	if (m_physics_world)
	{
		m_physics_world->DestroyBuffers(m_backend);
		delete m_physics_world;
		m_physics_world = nullptr;
	}
}

void SandboxApplication::OnProcessUpdate(f32 delta_time)
{
	m_dt = delta_time;

	if (m_cursor_mode == GLFW_CURSOR_DISABLED)
	{
		m_controller.OnProcessUpdate(delta_time);
		m_camera_data.view = m_camera.GetView();
	}

	m_physics_world->SyncBodiesFromMappedGPU();
}

void SandboxApplication::OnRenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(m_camera_data));

	m_wireframe_cube->UpdateBuffers(m_backend);

	if (m_request_apply_instance_count)
	{
		m_request_apply_instance_count = false;
		m_first_frame = true;

		m_instances_per_shape_ui = glm::clamp(m_instances_per_shape_ui, m_instances_per_shape_min, m_instances_per_shape_max);
		m_instances_per_shape_active = m_instances_per_shape_ui;

		m_shape_manager->instance_count = m_instances_per_shape_active;

		m_physics_world->Spawn(m_instances_per_shape_active);
	}

	m_physics_world->UploadSpawnDataIfNeeded(m_backend, m_shape_manager);

	bool do_sim = !m_simulation_paused || m_use_step_simulation || m_first_frame;
	if (do_sim)
	{
		m_physics_world->RunCompute(m_backend, m_wireframe_cube->aabb, m_dt);

		if (m_use_step_simulation)
		{
			m_use_step_simulation = false;
		}

		if (m_first_frame)
		{
			m_first_frame = false;
		}
	}

	m_physics_world->SyncBodiesFromMappedGPU();
	m_physics_world->RebuildBVHAndUploadDebug(m_backend, true);

	m_backend->BeginDynamicRendering();
}

void SandboxApplication::OnRenderUpdate()
{
	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	// Spawn cube draw
	{
		auto pipe = PipelineManager::GetInstance()->GetPipeline(WIREFRAME_PIPELINE_NAME);
		m_backend->BindPipeline(pipe);
		m_backend->BindDescriptorSet(pipe, m_camera_descriptor_wireframe, 0);
		m_wireframe_cube->Draw(m_backend);
	}

	// Shapes draw
	{
		auto pipe = PipelineManager::GetInstance()->GetPipeline(GRAPHICS_PIPELINE_NAME);
		m_backend->BindPipeline(pipe);
		m_backend->BindDescriptorSet(pipe, m_camera_descriptor_graphics, 0);
		m_shape_manager->Draw(m_backend);
	}

	// BVH debug draw
	{
		if (m_show_debug)
		{
			auto pipe = PipelineManager::GetInstance()->GetPipeline(DEBUG_LINES_PIPELINE_NAME);
			m_backend->BindPipeline(pipe);
			m_backend->BindDescriptorSet(pipe, m_camera_descriptor_debug_lines, 0);
			m_physics_world->DrawDebug(m_backend, m_camera_descriptor_debug_lines);
		}
	}
}

void SandboxApplication::OnRenderEnd()
{
	m_backend->EndDynamicRendering();
}

void SandboxApplication::OnUIRender()
{
	ImGui::Begin("Physics World", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Separator();

	ImGui::Text("Bodies: %u", (unsigned)m_physics_world->bodies.size());

	ImGui::PushItemWidth(220.0f);

	bool changed = false;

	changed |= ImGui::DragFloat("Separation slop", &m_physics_world->m_params.separation_slop, 0.0001f, 0.0f, 0.1f, "%.6f");
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset##slop")) { m_physics_world->m_params.separation_slop = 0.001f; changed = true; }

	changed |= ImGui::DragFloat("Push strength", &m_physics_world->m_params.push_strength, 0.01f, 0.0f, 5.0f, "%.3f");
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset##push")) { m_physics_world->m_params.push_strength = 0.85f; changed = true; }

	changed |= ImGui::DragFloat("Damping", &m_physics_world->m_params.damping, 0.001f, 0.0f, 1.0f, "%.4f");
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset##damp")) { m_physics_world->m_params.damping = 0.985f; changed = true; }

	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::SeparatorText("Population");

	ImGui::Text("Active instances/shape: %u", m_instances_per_shape_active);
	ImGui::Text("Min: %u   Max: %u", m_instances_per_shape_min, m_instances_per_shape_max);

	ImGui::PushItemWidth(220.0f);

	i32 ui_count = (i32)m_instances_per_shape_ui;
	if (ImGui::SliderInt("Instances / shape", &ui_count, (i32)m_instances_per_shape_min, (i32)m_instances_per_shape_max))
	{
		u32 new_value = (u32)ui_count;
		new_value = glm::clamp(new_value, m_instances_per_shape_min, m_instances_per_shape_max);

		if (new_value != m_instances_per_shape_active)
		{
			m_instances_per_shape_ui = new_value;
			m_request_apply_instance_count = true;
		}
	}

	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Checkbox("Pause simulation", &m_simulation_paused) || m_simulation_paused)
	{
		if (ImGui::Button("Step simulation"))
		{
			m_use_step_simulation = true;
		}
	}

	ImGui::Checkbox("Show BVH debug", &m_show_debug);

	if (ImGui::Button("Restart simulation"))
	{
		RestartSimulation();
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Button("Soft"))
		{
			m_physics_world->m_params.separation_slop = 0.0025f;
			m_physics_world->m_params.push_strength = 0.35f;
			m_physics_world->m_params.damping = 0.995f;
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Default"))
		{
			m_physics_world->m_params.separation_slop = 0.001f;
			m_physics_world->m_params.push_strength = 0.85f;
			m_physics_world->m_params.damping = 0.985f;
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stiff"))
		{
			m_physics_world->m_params.separation_slop = 0.00025f;
			m_physics_world->m_params.push_strength = 1.75f;
			m_physics_world->m_params.damping = 0.975f;
			changed = true;
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	f32 cube_size = m_wireframe_cube->aabb.max.x - m_wireframe_cube->aabb.min.x;
	changed |= ImGui::DragFloat("Spawn volume size", &cube_size, 0.1f, 1.0f, 100.0f, "%.1f");
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset##spawn")) { cube_size = 25.0f; changed = true; }
	if (changed)
	{
		m_wireframe_cube->SetScale(glm::vec3(cube_size));
		m_physics_world->SetSpawnBox(m_wireframe_cube->aabb.min, m_wireframe_cube->aabb.max);
		m_first_frame = true;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Controls:");
	ImGui::BulletText("WASD to move, right mouse click to look around");
	ImGui::TextDisabled("Tip: use F1 to toggle camera controls");

	ImGui::End();
}

b8 SandboxApplication::OnWindowClose(EventContext& event)
{
	m_running = false;
	return true;
}

b8 SandboxApplication::OnWindowResize(EventContext& event)
{
	if (event.data.window_resize.width == 0 || event.data.window_resize.height == 0)
	{
		return false;
	}

	m_window->SetSize(event.data.window_resize.width, event.data.window_resize.height);

	m_camera.SetAspect((f32)event.data.window_resize.width, (f32)event.data.window_resize.height);
	m_camera_data.projection = m_camera.GetProjection();

	m_backend->OnFramebufferResize();
	return false;
}

b8 SandboxApplication::OnWindowMinimised(EventContext& event)
{
	m_suspended = event.data.window_iconified.is_iconified;
	return false;
}

b8 SandboxApplication::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_F1)
	{
		m_cursor_mode = (m_cursor_mode == GLFW_CURSOR_DISABLED) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		SetCursorMode(m_cursor_mode);
	}

	if (event.data.key_event.key == KEY_KP_ADD)
	{
		m_wireframe_cube->transform = glm::scale(m_wireframe_cube->transform, glm::vec3(1.1f));
		m_wireframe_cube->aabb.min *= 1.1f;
		m_wireframe_cube->aabb.max *= 1.1f;
	}
	else if (event.data.key_event.key == KEY_KP_SUBTRACT)
	{
		m_wireframe_cube->transform = glm::scale(m_wireframe_cube->transform, glm::vec3(0.9f));
		m_wireframe_cube->aabb.min *= 0.9f;
		m_wireframe_cube->aabb.max *= 0.9f;
	}

	if (event.data.key_event.key == KEY_ESCAPE)
	{
		m_running = false;
	}

	return false;
}

void SandboxApplication::RestartSimulation()
{
	m_physics_world->SetSpawnBox(m_wireframe_cube->aabb.min, m_wireframe_cube->aabb.max);
	m_physics_world->SetShapePrototypes(m_shape_prototypes);
	m_physics_world->Spawn(m_shape_manager->instance_count);

	m_first_frame = true;
}

void SandboxApplication::CreateResources()
{
	CreatePipelines();
	CreateDescriptorSets();

	// Spawn cube
	m_wireframe_cube = new WireframeCubeData();
	m_wireframe_cube->CreateBuffers(m_backend);
	m_wireframe_cube->SetScale(glm::vec3(25.0f));

	// Shapes
	m_shape_manager = new ShapeManager(10000);
	m_shape_manager->CreateCube(1.0f);
	m_shape_manager->CreateUVSphere(0.5f, 36, 18);
	m_shape_manager->CreateCone(0.5f, 1.0f, 36);
	m_shape_manager->BuildInitialInstances();
	m_shape_manager->CreateBuffers(m_backend);

	// Physics
	m_physics_world = new PhysicsWorld();
	m_physics_world->SetSpawnBox(m_wireframe_cube->aabb.min, m_wireframe_cube->aabb.max);

	m_shape_prototypes.clear();
	m_shape_prototypes.reserve(3);

	// Cube
	{
		Body b{};
		b.type = Collider_AABB;
		b.half_extents = glm::vec3(0.5f);
		b.inv_mass = 1.0f;
		m_shape_prototypes.push_back(b);
	}

	// Sphere
	{
		Body b{};
		b.type = Collider_Sphere;
		b.radius = 0.5f;
		b.inv_mass = 1.0f;
		m_shape_prototypes.push_back(b);
	}

	// Cone
	{
		Body b{};
		b.type = Collider_ConeSphereApprox;
		b.radius = glm::sqrt(0.5f * 0.5f + 0.5f * 0.5f);
		b.inv_mass = 1.0f;
		m_shape_prototypes.push_back(b);
	}

	m_physics_world->SetShapePrototypes(m_shape_prototypes);
	m_physics_world->CreateBuffers(m_backend, m_shape_manager);

	m_instances_per_shape_min = 1;
	m_instances_per_shape_max = m_shape_manager->instance_count;
	m_instances_per_shape_active = 1;
	m_instances_per_shape_ui = m_instances_per_shape_active;
	m_request_apply_instance_count = true;
}

void SandboxApplication::CreatePipelines()
{
	// Graphics pipeline (instanced)
	{
		PipelineCreateInfo pipeline_info{};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
		pipeline_info.cull_mode = PipelineCullMode_None;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
		pipeline_info.line_width = 1.0f;
		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

		pipeline_info.layout = {
			{
				{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			},
			{
				{ { 0, DescriptorType_StorageBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			}
		};

		pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

		pipeline_info.dynamic_rendering_info = {
			false,
			{ VK_FORMAT_B8G8R8A8_UNORM },
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		ShaderStageInfo shader_info{};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, SHADER_UNLIT[0]);
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, SHADER_UNLIT[1]);

		PipelineManager::GetInstance()->CreatePipeline(GRAPHICS_PIPELINE_NAME, pipeline_info, shader_info);

		m_backend->InitImGuiForDynamicRendering(
			PipelineManager::GetInstance()->GetPipeline(GRAPHICS_PIPELINE_NAME)->GetRenderingCreateInfo());
	}

	// Wireframe pipeline
	{
		PipelineCreateInfo pipeline_info{};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_LineList;
		pipeline_info.cull_mode = PipelineCullMode_None;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.polygon_mode = PipelinePolygonMode_Line;
		pipeline_info.line_width = 1.0f;
		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

		pipeline_info.layout = {
			{
				{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			},
			{
				{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			}
		};

		pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

		pipeline_info.dynamic_rendering_info = {
			false,
			{ VK_FORMAT_B8G8R8A8_UNORM },
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		ShaderStageInfo shader_info{};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, SHADER_WIREFRAME[0]);
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, SHADER_WIREFRAME[1]);

		PipelineManager::GetInstance()->CreatePipeline(WIREFRAME_PIPELINE_NAME, pipeline_info, shader_info);
	}

	// Debug lines pipeline
	{
		PipelineCreateInfo pipeline_info{};
		pipeline_info.type = PipelineType_Graphics;
		pipeline_info.topology = PipelinePrimitiveTopology_LineList;
		pipeline_info.cull_mode = PipelineCullMode_None;
		pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
		pipeline_info.polygon_mode = PipelinePolygonMode_Line;
		pipeline_info.line_width = 1.0f;
		pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

		pipeline_info.layout = {
			{
				{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			}
		};

		pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
		pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

		pipeline_info.dynamic_rendering_info = {
			false,
			{ VK_FORMAT_B8G8R8A8_UNORM },
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		ShaderStageInfo shader_info{};
		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, SHADER_DEBUG_LINES[0]);
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, SHADER_DEBUG_LINES[1]);

		PipelineManager::GetInstance()->CreatePipeline(DEBUG_LINES_PIPELINE_NAME, pipeline_info, shader_info);
	}

	// Compute pipeline (physics)
	{
		PipelineCreateInfo pipeline_info{};
		pipeline_info.type = PipelineType_Compute;

		pipeline_info.layout = {
			{
				{ { 0, DescriptorType_StorageBuffer, 1, ShaderStage_Compute, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			},
			{
				{ { 0, DescriptorType_StorageBuffer, 1, ShaderStage_Compute, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			},
			{
				{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Compute, DescriptorBindingFlags_None } },
				DescriptorSetFlags_None
			}
		};

		ShaderStageInfo shader_info{};
		shader_info.sources[ShaderType_Compute] = CONCAT_PATHS(SHADER_PATH, SHADER_PHYSICS_COMPUTE);

		PipelineManager::GetInstance()->CreatePipeline(PHYSICS_COMPUTE_PIPELINE_NAME, pipeline_info, shader_info);
	}
}

void SandboxApplication::CreateDescriptorSets()
{
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 32 },
		{ DescriptorType_StorageBuffer, 32 }
		}, 1);

	m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData), 1);

	auto write_camera = [&](DescriptorSet** out_set, Pipeline* pipe)
		{
			DescriptorSet* set = m_backend->CreateDescriptorSet(pipe, 0);

			DescriptorSetWriteData w{};
			w.type = DescriptorType_UniformBuffer;
			w.binding = 0;
			w.data.buffer.buffers = new VkBuffer[1]{ m_camera_buffer->GetHandle() };
			w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(CameraData) };

			std::vector<DescriptorSetWriteData> writes = { w };
			m_backend->WriteDescriptor(&set, writes);

			*out_set = set;
		};

	write_camera(&m_camera_descriptor_graphics, PipelineManager::GetInstance()->GetPipeline(GRAPHICS_PIPELINE_NAME));
	write_camera(&m_camera_descriptor_wireframe, PipelineManager::GetInstance()->GetPipeline(WIREFRAME_PIPELINE_NAME));
	write_camera(&m_camera_descriptor_debug_lines, PipelineManager::GetInstance()->GetPipeline(DEBUG_LINES_PIPELINE_NAME));
}
