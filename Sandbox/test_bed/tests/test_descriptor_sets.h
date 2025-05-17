#pragma once

#include "../base.h"

#define SIZE1 125
#define SIZE2 1000
#define SIZE3 10648
#define SIZE4 125000

class TestDescriptorSet : public TestBase
{
public:
    TestDescriptorSet(ApplicationConfiguration* configuration);
    ~TestDescriptorSet() override;

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

    void CreateCube();

    void CreatePipeline();
    void CreateDescriptorSets();

    void SetupCubesData();
	void UpdateCubes(f32 dt);

private:
    // State
    glm::vec4 m_clear_color;

    u32 m_objects = SIZE1;
    CameraData m_camera_data;
    std::vector<ObjectData> m_object_data;

    std::vector<glm::vec3> m_rotations;
    std::vector<glm::vec3> m_rotation_speeds;

    // Pipeline
    Pipeline m_pipeline;

    // Descriptor sets
    DescriptorSet m_camera_descriptor;
    DescriptorSet m_texture_descriptor;
    std::vector<DescriptorSet> m_object_descriptors;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
    std::vector<UniformBuffer> m_object_buffers;

    // Textures
    Texture2D m_texture1;
    Texture2D m_texture2;

    // Cube
	std::vector<VertexFormatBase> m_vertices;
	std::vector<u32> m_indices;
	Buffer m_vertex_buffer;
	Buffer m_index_buffer;
};