#pragma once

#include "../base.h"

class TestPipelines : public TestBase
{
public:
    TestPipelines(ApplicationConfiguration* configuration);
    ~TestPipelines() override;

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

    void CreatePipelines();
    void CreateDescriptorSets();

private:
    ALIGN_AS(64) struct LightData
    {
        glm::vec3 position;
    };

    // State
    glm::vec4 m_clear_color;
    f32 m_light_move_speed;

    CameraData m_camera_data;
	ObjectData m_model_data;
	LightData m_light_data;

    f32 m_width;
    f32 m_height;

    // Pipelinex
    Pipeline m_phong_pipeline;
	Pipeline m_toon_pipeline;
	Pipeline m_wireframe_pipeline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
	DescriptorSet m_model_descriptor;
	DescriptorSet m_light_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
	UniformBuffer m_model_buffer;
	UniformBuffer m_light_buffer;

    // Model
	Model* m_model;
};