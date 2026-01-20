#pragma once

#include "../base.h"

#if _TEST_BED_ENABLED
class TestTriangle : public TestBase
{
public:
    TestTriangle(ApplicationConfiguration* configuration);
    ~TestTriangle() override;

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

    void CreateTriangle();

    void CreatePipeline();
    void CreateDescriptorSets();

private:
    // State
    glm::vec4 m_clear_color;

    CameraData m_camera_data;
    ObjectData m_triangle_data;

    // Pipeline
    Pipeline m_pipeline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
    DescriptorSet m_triangle_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
    UniformBuffer m_triangle_buffer;

    // Triangle
    std::vector<VertexFormatBase> m_vertices;
    std::vector<u32> m_indices;
    Buffer m_vertex_buffer;
    Buffer m_index_buffer;
};
#endif // _TEST_BED_ENABLED