#pragma once

#include "hellengine/hellengine.h"

using namespace hellengine;
using namespace tools;
using namespace core;
using namespace math;
using namespace graphics;
using namespace resources;

#define SHADER_PATH		"assets/shaders"
#define TEXTURE_PATH	"assets/textures"
#define MODEL_PATH		"assets/models"

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

private:
	u32 m_cursor_mode;

	CameraData m_camera_data;

	PerspectiveCamera m_camera;
	PerspectiveController m_controller;

	Pipeline m_pipeline;

	DescriptorSet m_camera_descriptor;
	DescriptorSet m_descriptor_set;

	UniformBuffer m_camera_buffer;
};