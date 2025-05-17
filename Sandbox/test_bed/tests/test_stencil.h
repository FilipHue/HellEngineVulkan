#pragma once

#include "../base.h"

class TestStencil : public TestBase
{
public:
    TestStencil(ApplicationConfiguration* configuration);
    ~TestStencil() override;

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

    void CreatePipeline();
    void CreateDescriptorSets();

private:
	ALIGN_AS(64) struct ModelDataToon
	{
		glm::mat4 model;
		glm::vec4 light_position;
	};

    ALIGN_AS(64) struct ModelDataOutline
    {
        glm::mat4 model;
		f32 outline_width;
    };

    // State
    glm::vec4 m_clear_color;

    CameraData m_camera_data;
    ModelDataToon m_object_data_toon;
	ModelDataOutline m_object_data_outline;

    // Pipeline
    Pipeline m_pipeline_toon;
    Pipeline m_pipeline_outline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
    DescriptorSet m_object_descriptor_toon;
	DescriptorSet m_object_descriptor_outline;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
    UniformBuffer m_object_buffer_toon;
	UniformBuffer m_object_buffer_outline;

    // Model
	Model* m_model;
};