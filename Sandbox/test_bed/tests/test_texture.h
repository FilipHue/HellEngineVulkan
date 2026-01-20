#pragma once

// Internal
#include "../base.h"

#if _TEST_BED_ENABLED
class TestTexture : public TestBase
{
public:
    TestTexture(ApplicationConfiguration* configuration);
    ~TestTexture() override;

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
	void SetMeshData();

    void CreatePipelines();
    void CreateDescriptorSets();

private:
    struct LightData
    {
        glm::vec4 view_position;
    } m_light_data;

    // State
    glm::vec4 m_clear_color;

    CameraData m_camera_data;
	ObjectData m_model_data;

    f32 m_load_bias;

    // Pipelines
    Pipeline m_pipeline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
	DescriptorSet m_model_descriptor;
	DescriptorSet m_light_descriptor;
	DescriptorSet m_image_descriptor;
	DescriptorSet m_texture_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
	UniformBuffer m_model_buffer;
	UniformBuffer m_light_buffer;
	UniformBuffer m_image_buffer;

    // Texture
	Texture2D m_texture;

    // Mesh
    std::vector<VertexFormatBase> m_vertices;
    std::vector<u32> m_indices;
};
#endif //_TEST_BED_ENABLED