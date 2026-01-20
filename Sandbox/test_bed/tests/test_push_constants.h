#pragma once

// Internal
#include "../base.h"

#if _TEST_BED_ENABLED
class TestPushConstants : public TestBase
{
public:
	TestPushConstants(ApplicationConfiguration* configuration);
	~TestPushConstants() override;

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
	void SetupSpheres();

    void CreatePipelines();
    void CreateDescriptorSets();

private:
    struct SphereData
    {
        glm::vec4 color;
        glm::vec4 position;
    };

    // State
    glm::vec4 m_clear_color;

    CameraData m_camera_data;
    ObjectData m_model_data;
	static const u32 m_model_count = 16;
	std::array<SphereData, m_model_count> m_spheres;


    // Pipeline
	Pipeline m_pipeline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
	DescriptorSet m_model_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
	UniformBuffer m_model_buffer;

    // Model
    Model* m_model;
};
#endif //_TEST_BED_ENABLED