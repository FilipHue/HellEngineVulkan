#pragma once

// Internal
#include "../base.h"

#if _TEST_BED_ENABLED
#define MAX_OBJECTS 8

class TestTextureArray : public TestBase
{
public:
    TestTextureArray(ApplicationConfiguration* configuration);
    ~TestTextureArray() override;

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
    ALIGN_AS(16)struct InstanceData
    {
        glm::mat4 model;
		f32 array_index;
	} m_instance_data;

    // State
    glm::vec4 m_clear_color;

    CameraData m_camera_data;
	std::vector<InstanceData> m_instances_data;

    // Pipelines
    Pipeline m_pipeline;

    // Descriptor sets
	DescriptorSet m_instances_descriptor;
	DescriptorSet m_texture_descriptor;

    // Uniform buffers
    UniformBuffer m_camera_buffer;
	UniformBuffer m_instance_buffer;

    // Texture
	Texture2D m_texture;

    // Mesh
    std::vector<VertexFormatBase> m_vertices;
    std::vector<u32> m_indices;
};
#endif // _TEST_BED_ENABLED