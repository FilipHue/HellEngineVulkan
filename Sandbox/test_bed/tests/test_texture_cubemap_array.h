#pragma once

// Internal
#include "../base.h"

class TestTextureCubemapArray : public TestBase
{
public:
    TestTextureCubemapArray(ApplicationConfiguration* configuration);
    ~TestTextureCubemapArray() override;

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
    struct UBOData
    {
        glm::mat4 projection;
        glm::mat4 modelView;
        glm::mat4 inverseModelview;
        f32 lodBias;
		i32 cubemapIndex;
    } m_ubo_data;

    glm::vec4 m_clear_color;

    u32 m_model_index;

    // Pipelines
    Pipeline m_pipeline_skybox;
    Pipeline m_pipeline_reflect;

    // Descriptor sets
	DescriptorSet m_ubo_descriptor;

    // Uniform buffers
    UniformBuffer m_ubo;

    // Texture
	TextureCubemap m_cubemap_texture;

    // Model
    Model* m_cube_model;
	std::vector<Model*> m_models;
	std::vector<std::string> m_model_names;
};