#pragma once

#include "../base.h"

#if _TEST_BED_ENABLED
#define PARTICLE_COUNT 512

#define FLAME_RADIUS 8.0f

#define PARTICLE_TYPE_FLAME 0
#define PARTICLE_TYPE_SMOKE 1

class TestCpuParticles : public TestBase
{
public:
    TestCpuParticles(ApplicationConfiguration* configuration);
    ~TestCpuParticles() override;

    void Setup() override;

    void ProcessUpdate(f32 dt) override;

    void RenderBegin() override;
    void RenderUpdate() override;
    void RenderEnd() override;

    void UIRender() override;

    void Cleanup() override;

    b8 OnWindowClose(EventContext& event) override;
    b8 OnWindowResize(EventContext& event) override;
    b8 OnWindowFocus(EventContext& event) override;
    b8 OnWindowIconified(EventContext& event) override;
    b8 OnWindowMoved(EventContext& event) override;
    b8 OnKeyPressed(EventContext& event) override;
    b8 OnKeyReleased(EventContext& event) override;
    b8 OnMouseButtonPressed(EventContext& event) override;
    b8 OnMouseButtonReleased(EventContext& event) override;
    b8 OnMouseMoved(EventContext& event) override;
    b8 OnMouseScrolled(EventContext& event) override;

	void LoadResources();

    void InitParticle(Particle* particle, glm::vec3 emitter_position);
    void TransitionParticle(Particle* particle);
    void PrepareParticles();
	void UpdateParticles(f32 dt);

	void CreatePipelines();
	void CreateDescriptorSets();

private:
    struct EnviromentData
    {
        glm::mat4 projection;
        glm::mat4 modelView;
        glm::mat4 normal;
		glm::vec4 lightPos = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct ParticleData
    {
        glm::mat4 projection;
        glm::mat4 modelView;
        glm::vec2 viewportDim;
        f32 pointSize{ 10.0f };
    };

    // State
    glm::vec4 m_clear_color;

	EnviromentData m_enviroment_data;
	glm::mat4 m_enviroment_model;
	ParticleData m_particle_data;

	// Particles
    glm::vec3 emitterPos = glm::vec3(0.0f, FLAME_RADIUS + 2.0f, 0.0f);
    glm::vec3 minVel = glm::vec3(-3.0f, 0.5f, -3.0f);
    glm::vec3 maxVel = glm::vec3(3.0f, 7.0f, 3.0f);
	std::vector<Particle> m_particles;

	Buffer m_particles_vertex_buffer;

	// Pipelines
    Pipeline m_pipeline_normal;
    Pipeline m_pipeline_particles;

    // Uniform buffers
    UniformBuffer m_enviroment_buffer;
	UniformBuffer m_particle_buffer;

	// Descriptor sets
	DescriptorSet m_enviroment_descriptor;
	DescriptorSet m_particle_descriptor;

    // Textures
	Texture2D m_texture_flame;
    Texture2D m_texture_smoke;
    Texture2D m_texture_color_map;
	Texture2D m_texture_normal_map;

    // Models
    Model* m_model_fireplace;
};
#endif // _TEST_BED_ENABLED