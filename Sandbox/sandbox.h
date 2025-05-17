#pragma once

#include "hellengine/hellengine.h"

using namespace hellengine;
using namespace tools;
using namespace core;
using namespace math;
using namespace graphics;
using namespace resources;

class SandboxApplication : public Application
{
public:
	SandboxApplication(ApplicationConfiguration* configuration);

	~SandboxApplication();

	void Init() override;

	void OnProcessUpdate(f32 delta_time) override;

	void OnRenderBegin() override;
	void OnRenderUpdate() override;
	void OnRenderEnd() override;
	void OnUIRender() override;

	void Shutdown() override;

	b8 OnWindowClose(EventContext& event);
	b8 OnWindowResize(EventContext& event);
	b8 OnWindowMinimised(EventContext& event);

	b8 OnKeyPressed(EventContext& event);

private:
	void CreatePipeline();
	void CreateDescriptorSet();
	void SetMeshData();

	void CreateCubeMesh(std::string name);
	void CreateCubeDescriptorSet(std::string name);

private:
	u32 m_cursor_mode;

	Pipeline m_pipeline;

	DescriptorSet m_camera_descriptor;
	UniformBuffer m_camera_buffer;
	CameraData m_camera_data;

	PerspectiveCamera m_camera;
	PerspectiveController m_controller;

	u32 m_meshes;
	Texture2D m_texture;
	std::vector<ObjectData> m_mesh_data;
	std::vector<UniformBuffer> m_mesh_buffers;
	std::vector<DescriptorSet> m_mesh_descriptors;
	DescriptorSet m_material_descriptor;
	std::vector<VertexFormatBase> m_vertices;
	std::vector<u32> m_indices;

	Texture2D m_test_texture;
};