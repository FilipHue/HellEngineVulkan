#pragma once

#include "../base.h"

#if _TEST_BED_ENABLED
class TestDynamicAttachment : public TestBase
{
public:
	TestDynamicAttachment(ApplicationConfiguration* configuration);
	~TestDynamicAttachment() override;

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
    ALIGN_AS(64) struct Data
    {
        f32 brightness;
		f32 contrast;
        glm::vec2 range;
    };

    // State
    glm::vec4 m_clear_color;

    CameraData m_camera_data;
    ObjectData m_model_data;
	Data m_data;

    i32 m_texture_index;

    // Pipeline
    Pipeline m_color_pipeline;
	Pipeline m_screen_pipeline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
    DescriptorSet m_model_descriptor;
	DescriptorSet m_data_descriptor;
	DescriptorSet m_attachment_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
	UniformBuffer m_model_buffer;
	UniformBuffer m_data_buffer;

    // Model
	Model* m_model;

    // Attachments
	DynamicRenderingAttachmentInfo m_color_attachment;
	DynamicRenderingAttachmentInfo m_depth_attachment;
	DynamicRenderingInfo m_rendering_info;

	Texture2D m_color_texture;
	Texture2D m_depth_texture;
};
#endif // _TEST_BED_ENABLED