#pragma once

#include "../base.h"

#define FM_DIM 512

class TestOffscreen : public TestBase
{
public:
    TestOffscreen(ApplicationConfiguration* configuration);
    ~TestOffscreen() override;

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

    void CreateRenderPasses();
	void CreatePipeline();
    void CreateDescriptorSets();

private:
    ALIGN_AS(64) struct LightData
    {
        glm::vec4 position;
    };

    // State
    glm::vec4 m_clear_color;
    b8 m_resized;
    b8 m_show_render_target;
    f32 m_rotation;

    CameraData m_camera_data;
	ObjectData m_offscreen_data;
	ObjectData m_chinese_dragon_data;
    ObjectData m_plane_data;
	LightData m_light_data;

	// Renderpass
	RenderPass m_offscreen_render_pass;
	RenderPass m_swapchain_render_pass;
	RenderPassInfo m_offscreen_render_pass_info;
	RenderPassInfo m_swapchain_render_pass_info;

    Texture2D m_offscreen_color_texture;
	Texture2D m_offscreen_depth_texture;

    // Pipeline
	Pipeline m_offscreen_pipeline;
	Pipeline m_mirror_pipeline;
	Pipeline m_shaded_pipeline;
	Pipeline m_debug_pipeline;

    // Uniforms
	UniformBuffer m_camera_buffer;
    UniformBuffer m_chinese_dragon_buffer;
	UniformBuffer m_plane_buffer;
	UniformBuffer m_light_buffer;
	UniformBuffer m_offscreen_buffer;

	// Descriptor sets
	DescriptorSet m_camera_descriptor;
	DescriptorSet m_chinese_dragon_descriptor;
	DescriptorSet m_plane_descriptor;
	DescriptorSet m_light_descriptor;
	DescriptorSet m_offscreen_descriptor;
	DescriptorSet m_mirror_descriptor;
	DescriptorSet m_debug_descriptor;

    // Models
    Model* m_plane_model;
	Model* m_chinese_dragon_model;
};