#pragma once

#include "../base.h"

#if _TEST_BED_ENABLED
class TestGltfLoading : public TestBase
{
public:
	TestGltfLoading(ApplicationConfiguration* configuration);
	~TestGltfLoading() override;

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

    void CreatePipelines();
    void CreateDescriptorSets();

private:
    ALIGN_AS(64) struct LightData
    {
        glm::vec4 position;
        glm::vec4 view_position;
    };

    // State
    glm::vec4 m_clear_color;
	b8 m_use_wireframe = false;

    CameraData m_camera_data;
    ObjectData m_model_data;

    // Pipeline
    Pipeline m_lit_pipeline;
	Pipeline m_wireframe_pipeline;

    // Descriptor sets
    DescriptorSet m_lit_constants_descriptor;
	DescriptorSet m_wireframe_constants_descriptor;
    DescriptorSet m_lit_model_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
	UniformBuffer m_light_buffer;
	UniformBuffer m_model_buffer;

    // Model
	Model* m_model;

    // Light
	LightData m_light_data;
};
#endif // _TEST_BED_ENABLED