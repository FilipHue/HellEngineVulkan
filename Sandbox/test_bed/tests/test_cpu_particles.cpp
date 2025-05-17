#include "test_cpu_particles.h"

TestCpuParticles::TestCpuParticles(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}	

TestCpuParticles::~TestCpuParticles()
{
	NO_OP;
}

void TestCpuParticles::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera.SetPosition({ 75.0f, 25.0f, 75.0f });
	m_camera.RotateOX(glm::radians(-15.0f));
	m_camera.RotateOY(glm::radians(45.0f));

	m_enviroment_model = glm::mat4(1.0f);

	m_enviroment_data.projection = m_camera.GetProjection();
	m_enviroment_data.modelView = m_camera.GetView() * m_enviroment_model;
	m_enviroment_data.normal = glm::inverseTranspose(m_enviroment_data.modelView);

	m_particle_data.projection = m_camera.GetProjection();
	m_particle_data.modelView = m_camera.GetView() * m_enviroment_model;
	m_particle_data.viewportDim = { (f32)m_window->GetWidth(), (f32)m_window->GetHeight() };


	PrepareParticles();

	LoadResources();

	CreatePipelines();
	CreateDescriptorSets();

	m_model_fireplace->UploadToGPU<VertexFormatTangent>();
	m_model_fireplace->GenerateDescriptorSets(m_pipeline_normal, 1);
}

void TestCpuParticles::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_enviroment_data.modelView = m_camera.GetView() * m_enviroment_model;
		m_enviroment_data.normal = glm::inverseTranspose(m_enviroment_data.modelView);

		m_particle_data.modelView = m_camera.GetView() * m_enviroment_model;
	}

	UpdateParticles(dt);

	m_particle_data.viewportDim = { (f32)m_window->GetWidth(), (f32)m_window->GetHeight() };

	m_enviroment_data.lightPos.x = sin(dt * 2.0f * glm::pi<f32>()) * 1.5f;
	m_enviroment_data.lightPos.y = 0.0f;
	m_enviroment_data.lightPos.z = cos(dt * 2.0f * glm::pi<f32>()) * 1.5f;
}

void TestCpuParticles::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_enviroment_buffer, &m_enviroment_data, sizeof(EnviromentData));
	m_backend->UpdateUniformBuffer(m_particle_buffer, &m_particle_data, sizeof(ParticleData));
	m_backend->UpdateVertexBuffer(m_particles_vertex_buffer, 0, m_particles.data(), sizeof(Particle) * (u32)m_particles.size());
}

void TestCpuParticles::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->BindPipeline(m_pipeline_normal);

	m_backend->BindDescriptorSet(m_pipeline_normal, m_enviroment_descriptor);

	m_model_fireplace->Draw(m_pipeline_normal, 0);

	m_backend->BindPipeline(m_pipeline_particles);

	m_backend->BindDescriptorSet(m_pipeline_particles, m_particle_descriptor);

	m_backend->BindVertexBuffer(m_particles_vertex_buffer, 0);
	m_backend->Draw((u32)m_particles.size(), 1, 0, 0);

	m_backend->EndDynamicRendering();
}

void TestCpuParticles::RenderEnd()
{
}

void TestCpuParticles::UIRender()
{
}

void TestCpuParticles::Cleanup()
{
	m_backend->DestroyBuffer(m_particles_vertex_buffer);

	m_backend->DestroyBuffer(m_enviroment_buffer);
	m_backend->DestroyBuffer(m_particle_buffer);

	m_backend->DestroyPipeline(m_pipeline_normal);
	m_backend->DestroyPipeline(m_pipeline_particles);
}

b8 TestCpuParticles::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnWindowResize(EventContext& event)
{
	m_enviroment_data.projection = m_camera.GetProjection();
	m_particle_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestCpuParticles::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestCpuParticles::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestCpuParticles::LoadResources()
{
	m_texture_flame = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "particle_fire.ktx")));
	m_texture_smoke = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "particle_smoke.ktx")));

	m_texture_color_map = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "fireplace_colormap_rgba.ktx")));
	m_texture_normal_map = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "fireplace_normalmap_rgba.ktx")));

	m_model_fireplace = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "fireplace.gltf")));
}

void TestCpuParticles::InitParticle(Particle* particle, glm::vec3 emitter_position)
{
	particle->vel = glm::vec4(0.0f, minVel.y + Random::GetInRange(0.0f, maxVel.y - minVel.y), 0.0f, 0.0f);
	particle->alpha = Random::GetInRange(0.0f, 0.75f);
	particle->size = 1.0f + Random::GetInRange(0.0f, 0.5f);
	particle->color = glm::vec4(1.0f);
	particle->type = PARTICLE_TYPE_FLAME;
	particle->rotation = Random::GetInRange(0.0f, 2.0f * glm::pi<f32>());
	particle->rotationSpeed = Random::GetInRange(0.0f, 2.0f) - Random::GetInRange(0.0f, 2.0f);

	// Get random sphere point
	f32 theta = Random::GetInRange(0.0f, 2.0f * glm::pi<f32>());
	f32 phi = Random::GetInRange(0.0f, glm::pi<f32>()) - glm::pi<f32>() / 2.0f;
	f32 r = Random::GetInRange(0.0f, FLAME_RADIUS);

	particle->pos.x = r * cos(theta) * cos(phi);
	particle->pos.y = r * sin(phi);
	particle->pos.z = r * sin(theta) * cos(phi);

	particle->pos += glm::vec4(emitterPos, 0.0f);
}

void TestCpuParticles::TransitionParticle(Particle* particle)
{
	switch (particle->type)
	{
	case PARTICLE_TYPE_FLAME:
		if (Random::GetInRange(0.0f, 1.0f) < 0.05f)
		{
			particle->alpha = 0.0f;
			particle->color = glm::vec4(0.25f + Random::GetInRange(0.0f, 0.25f));
			particle->pos.x *= 0.5f;
			particle->pos.z *= 0.5f;
			particle->vel = glm::vec4(Random::GetInRange(0.0f, 1.0f) - Random::GetInRange(0.0f, 1.0f), (minVel.y * 2) + Random::GetInRange(0.0f, maxVel.y - minVel.y), Random::GetInRange(0.0f, 1.0f) - Random::GetInRange(0.0f, 1.0f), 0.0f);
			particle->size = 1.0f + Random::GetInRange(0.0f, 0.5f);
			particle->rotationSpeed = Random::GetInRange(0.0f, 1.0f) - Random::GetInRange(0.0f, 1.0f);
			particle->type = PARTICLE_TYPE_SMOKE;
		}
		else
		{
			InitParticle(particle, emitterPos);
		}
		break;
	case PARTICLE_TYPE_SMOKE:
		InitParticle(particle, emitterPos);
		break;
	}
}

void TestCpuParticles::PrepareParticles()
{
	m_particles.resize(PARTICLE_COUNT);
	for (auto& particle : m_particles)
	{
		InitParticle(&particle, emitterPos);
		particle.alpha = 1.0f - (abs(particle.pos.y) / FLAME_RADIUS * 2.0f);
	}

	m_particles_vertex_buffer = m_backend->CreateVertexBuffer(m_particles.data(), (u32)m_particles.size());
}

void TestCpuParticles::UpdateParticles(f32 dt)
{
	f32 particleTimer = dt * 0.45f;
	for (auto& particle : m_particles)
	{
		switch (particle.type)
		{
		case PARTICLE_TYPE_FLAME:
			particle.pos.y += particle.vel.y * particleTimer * 3.5f;
			particle.alpha += particleTimer * 2.5f;
			particle.size -= particleTimer * 0.5f;
			break;
		case PARTICLE_TYPE_SMOKE:
			particle.pos += particle.vel * dt * 1.0f;
			particle.alpha += particleTimer * 1.25f;
			particle.size += particleTimer * 0.125f;
			particle.color -= particleTimer * 0.05f;
			break;
		}
		particle.rotation += particleTimer * particle.rotationSpeed;

		if (particle.alpha > 2.0f)
		{
			TransitionParticle(&particle);
		}
	}
}

void TestCpuParticles::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 2, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment }
		}
	};
	pipeline_info.vertex_binding_description = VertexFormatTangent::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatTangent::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_Back;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_SRGB },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.depth_stencil_info.depth_write_enable = true;
	pipeline_info.dynamic_rendering_info.value().blend_enable = false;

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "cpu_particles_normal.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "cpu_particles_normal.frag");

	m_pipeline_normal = m_backend->CreatePipeline(pipeline_info, shader_info);

	pipeline_info.topology = PipelinePrimitiveTopology_PointList;

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 2, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};

	pipeline_info.vertex_binding_description = Particle::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = Particle::GetAttributeDescriptions();

	pipeline_info.depth_stencil_info.depth_write_enable = false;
	pipeline_info.dynamic_rendering_info.value().blend_enable = true;

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "cpu_particles_particle.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "cpu_particles_particle.frag");

	m_pipeline_particles = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline_particles->GetRenderingCreateInfo());
}

void TestCpuParticles::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPool({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Enviroment descriptor
	{
		m_enviroment_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(EnviromentData));
		m_enviroment_descriptor = m_backend->CreateDescriptorSet(m_pipeline_normal, 0);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_UniformBuffer;
		descriptor_data1.binding = 0;
		descriptor_data1.data.buffer.buffer = m_enviroment_buffer->GetHandle();
		descriptor_data1.data.buffer.offset = 0;
		descriptor_data1.data.buffer.range = sizeof(EnviromentData);

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_view = m_texture_color_map->GetImageView();
		descriptor_data2.data.image.sampler = m_texture_color_map->GetSampler();

		DescriptorSetWriteData descriptor_data3;
		descriptor_data3.type = DescriptorType_CombinedImageSampler;
		descriptor_data3.binding = 2;
		descriptor_data3.data.image.image_view = m_texture_normal_map->GetImageView();
		descriptor_data3.data.image.sampler = m_texture_normal_map->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data1, descriptor_data2, descriptor_data3 };
		m_backend->WriteDescriptor(&m_enviroment_descriptor, descriptor_data_objects);
	}

	// Particle descriptor
	{
		m_particle_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ParticleData));
		m_particle_descriptor = m_backend->CreateDescriptorSet(m_pipeline_particles, 0);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_UniformBuffer;
		descriptor_data1.binding = 0;
		descriptor_data1.data.buffer.buffer = m_particle_buffer->GetHandle();
		descriptor_data1.data.buffer.offset = 0;
		descriptor_data1.data.buffer.range = sizeof(ParticleData);

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_view = m_texture_smoke->GetImageView();
		descriptor_data2.data.image.sampler = m_texture_smoke->GetSampler();

		DescriptorSetWriteData descriptor_data3;
		descriptor_data3.type = DescriptorType_CombinedImageSampler;
		descriptor_data3.binding = 2;
		descriptor_data3.data.image.image_view = m_texture_flame->GetImageView();
		descriptor_data3.data.image.sampler = m_texture_flame->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data1, descriptor_data2, descriptor_data3 };
		m_backend->WriteDescriptor(&m_particle_descriptor, descriptor_data_objects);
	}
}
