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

	void Shutdown() override;

	b8 OnWindowClose(EventContext& event);
	b8 OnWindowResize(EventContext& event);
	b8 OnWindowMinimised(EventContext& event);

	b8 OnKeyPressed(EventContext& event);

private:
	void CreatePipeline();
	void CreateDescriptorSet();
	void CreateCubeMesh();

private:
	u32 m_cursor_mode;

	Pipeline m_pipeline;

	DescriptorSet m_camera_descriptor;
	UniformBuffer m_camera_buffer;
	CameraData m_camera_data;

	DescriptorSet m_texture_descriptor;
	Texture m_texture1;
	Texture m_texture2;

	DescriptorSet m_object_descriptor;
	UniformBuffer m_object_buffer;
	ObjectData m_object_data;

	PerspectiveCamera m_camera;
	PerspectiveController m_controller;

	Buffer m_vertex_buffer;
	Buffer m_index_buffer;
};