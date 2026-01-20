#pragma once

#include <hellengine/hellengine.h>

// STL
#include <random>

//#define SHADER_PATH		"test_bed/assets/shaders"
//#define TEXTURE_PATH	"test_bed/assets/textures"
//#define MODEL_PATH		"test_bed/assets/models"

using namespace hellengine;
using namespace core;
using namespace graphics;
using namespace math;
using namespace resources;
using namespace tools;

class TestBase : public Application
{
public:
	TestBase(ApplicationConfiguration* configuration);
	virtual ~TestBase() = default;

	void virtual Setup() = 0;

	void virtual ProcessUpdate(f32 dt) = 0;

	void virtual RenderBegin() = 0;
	void virtual RenderUpdate() = 0;
	void virtual RenderEnd() = 0;
	void virtual UIRender() {};

	void virtual Cleanup() = 0;

	b8 virtual OnWindowClose(EventContext& event)			{ return false; }
	b8 virtual OnWindowResize(EventContext& event)			{ return false; }
	b8 virtual OnWindowFocus(EventContext& event)			{ return false; }
	b8 virtual OnWindowIconified(EventContext& event)		{ return false; }
	b8 virtual OnWindowMoved(EventContext& event)			{ return false; }

	b8 virtual OnKeyPressed(EventContext& event)			{ return false; }
	b8 virtual OnKeyReleased(EventContext& event)			{ return false; }

	b8 virtual OnMouseButtonPressed(EventContext& event)	{ return false; }
	b8 virtual OnMouseButtonReleased(EventContext& event)	{ return false; }
	b8 virtual OnMouseMoved(EventContext& event)			{ return false; }
	b8 virtual OnMouseScrolled(EventContext& event)			{ return false; }

private:
	void Init() override;

	void OnProcessUpdate(f32 delta_time) override;

	void OnRenderBegin() override;
	void OnRenderUpdate() override;
	void OnRenderEnd() override;
	void OnUIRender() override;

	void Shutdown() override;

	b8 OnEventWindowClose(EventContext& event);
	b8 OnEventWindowResize(EventContext& event);
	b8 OnEventWindowFocus(EventContext& event);
	b8 OnEventWindowIconified(EventContext& event);
	b8 OnEventWindowMoved(EventContext& event);

	b8 OnEventKeyPressed(EventContext& event);
	b8 OnEventKeyReleased(EventContext& event);

	b8 OnEventMouseButtonPressed(EventContext& event);
	b8 OnEventMouseButtonReleased(EventContext& event);
	b8 OnEventMouseMoved(EventContext& event);
	b8 OnEventMouseScrolled(EventContext& event);

protected:
	// State
	u32 m_cursor_mode;
	b8 m_is_camera_active;

	b8 m_is_resizing;

	// Camera
	PerspectiveCamera m_camera;
	PerspectiveController m_controller;
};