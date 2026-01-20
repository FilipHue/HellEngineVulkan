#pragma once

#include "../base.h"

#if _TEST_BED_ENABLED
class TestGBuffer : public TestBase
{
public:
    TestGBuffer(ApplicationConfiguration* configuration);
    ~TestGBuffer() override;

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
	void CreateAttachments();
    void CreateLightsData();

    void CreatePipeline();
    void CreateDescriptorSets();

	void UpdateAttachments();

private:
    struct Light {
        glm::vec4 position;
        glm::vec3 color;
        float radius;
    };

    // State
    glm::vec4 m_clear_color;

	std::array<Light, 64> m_lights;

    CameraData m_camera_data;
	ObjectData m_object_data;

	// Buffers
	UniformBuffer m_camera_buffer;
	UniformBuffer m_object_buffer;
	StorageBuffer m_lights_buffer;

	// Descriptor sets
    DescriptorSet m_camera_descriptor;
	DescriptorSet m_object_descriptor;
	DescriptorSet m_composition_descriptor;
    DescriptorSet m_glass_descriptor;
    DescriptorSet m_final_descriptor;

	// Pipeline
	Pipeline m_gbuffer_pipeline;
    Pipeline m_composition_pipeline;
	Pipeline m_glass_pipeline;
	Pipeline m_final_pipeline;

	// Attachments
	DynamicRenderingAttachmentInfo m_gbuffer_color_attachment;
	DynamicRenderingAttachmentInfo m_gbuffer_normal_attachment;
	DynamicRenderingAttachmentInfo m_gbuffer_position_attachment;
	DynamicRenderingAttachmentInfo m_gbuffer_albedo_attachment;
	DynamicRenderingAttachmentInfo m_gbuffer_depth_attachment;

    DynamicRenderingAttachmentInfo m_composition_color_attachment;

	DynamicRenderingAttachmentInfo m_glass_color_attachment;
	DynamicRenderingAttachmentInfo m_glass_depth_attachment;

	DynamicRenderingAttachmentInfo m_final_color_attachment;

	Texture2D m_gbuffer_color_texture;
	Texture2D m_gbuffer_normal_texture;
	Texture2D m_gbuffer_position_texture;
	Texture2D m_gbuffer_albedo_texture;
	Texture2D m_gbuffer_depth_texture;

    Texture2D m_composition_color_texture;

	Texture2D m_glass_color_texture;
	Texture2D m_glass_depth_texture;

	Texture2D m_final_color_texture;

	DynamicRenderingInfo m_gbuffer_rendering_info;
	DynamicRenderingInfo m_composition_rendering_info;
	DynamicRenderingInfo m_glass_rendering_info;
	DynamicRenderingInfo m_final_rendering_info;

    // Model
    Model* m_scene_model;
    Model* m_glass_model;

    // Texture
    Texture2D m_glass_texture;
};
#endif // _TEST_BED_ENABLED