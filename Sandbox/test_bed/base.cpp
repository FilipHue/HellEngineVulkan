#include "base.h"

TestBase::TestBase(ApplicationConfiguration* configuration) : Application(configuration)
{
	m_cursor_mode = GLFW_CURSOR_NORMAL;
	m_is_camera_active = false;
	m_is_resizing = false;
}

void TestBase::Init()
{
    EventDispatcher::AddListener(EventType_WindowClose,				HE_BIND_EVENTCALLBACK(TestBase::OnEventWindowClose));
    EventDispatcher::AddListener(EventType_WindowResize,			HE_BIND_EVENTCALLBACK(TestBase::OnEventWindowResize));
    EventDispatcher::AddListener(EventType_WindowFocus,				HE_BIND_EVENTCALLBACK(TestBase::OnEventWindowFocus));
    EventDispatcher::AddListener(EventType_WindowIconified,			HE_BIND_EVENTCALLBACK(TestBase::OnEventWindowIconified));
    EventDispatcher::AddListener(EventType_WindowMoved,				HE_BIND_EVENTCALLBACK(TestBase::OnEventWindowMoved));

    EventDispatcher::AddListener(EventType_KeyPressed,				HE_BIND_EVENTCALLBACK(TestBase::OnEventKeyPressed));
    EventDispatcher::AddListener(EventType_KeyReleased,				HE_BIND_EVENTCALLBACK(TestBase::OnEventKeyReleased));

    EventDispatcher::AddListener(EventType_MouseButtonPressed,		HE_BIND_EVENTCALLBACK(TestBase::OnEventMouseButtonPressed));
    EventDispatcher::AddListener(EventType_MouseButtonReleased,		HE_BIND_EVENTCALLBACK(TestBase::OnEventMouseButtonReleased));
    EventDispatcher::AddListener(EventType_MouseMoved,				HE_BIND_EVENTCALLBACK(TestBase::OnEventMouseMoved));
    EventDispatcher::AddListener(EventType_MouseScrolled,			HE_BIND_EVENTCALLBACK(TestBase::OnEventMouseScrolled));

	SetCursorMode(m_cursor_mode);

	m_camera = PerspectiveCamera();
	m_camera.Create(45.0f, (f32)m_window->GetWidth() / m_window->GetHeight(), 0.1f, 1000.0f);

	m_controller = PerspectiveController();
	m_controller.Init();
	m_controller.SetCamera(&m_camera);

	Setup();
}										 

void TestBase::OnProcessUpdate(f32 delta_time)
{
	if (m_is_camera_active)
	{
		m_controller.OnProcessUpdate(delta_time);
	}

	ProcessUpdate(delta_time);
}

void TestBase::OnRenderBegin()
{
	RenderBegin();
}

void TestBase::OnRenderUpdate()
{
	RenderUpdate();
}

void TestBase::OnRenderEnd()
{
	RenderEnd();

	m_is_resizing = false;
}

void TestBase::OnUIRender()
{
	UIRender();
}

void TestBase::Shutdown()
{
	Cleanup();
}

b8 TestBase::OnEventWindowClose(EventContext& event)
{
	m_running = false;

	return OnWindowClose(event);
}

b8 TestBase::OnEventWindowResize(EventContext& event)
{
	if (event.data.window_resize.width == 0 || event.data.window_resize.height == 0)
	{
		return false;
	}
	m_window->SetSize(event.data.window_resize.width, event.data.window_resize.height);

	m_camera.SetAspect((f32)event.data.window_resize.width, (f32)event.data.window_resize.height);

	m_backend->OnFramebufferResize();

	return OnWindowResize(event);
}

b8 TestBase::OnEventWindowFocus(EventContext& event)
{
	return OnWindowFocus(event);
}

b8 TestBase::OnEventWindowIconified(EventContext& event)
{
	m_suspended = event.data.window_iconified.is_iconified;

	return OnWindowIconified(event);
}

b8 TestBase::OnEventWindowMoved(EventContext& event)
{
	return OnWindowMoved(event);
}

b8 TestBase::OnEventKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_F1)
	{
		m_cursor_mode = GLFW_CURSOR_NORMAL + GLFW_CURSOR_DISABLED - m_cursor_mode;
		m_is_camera_active = m_cursor_mode == GLFW_CURSOR_DISABLED;

		SetCursorMode(m_cursor_mode);
	}

	if (event.data.key_event.key == KEY_ESCAPE)
	{
		m_running = false;
	}

	return OnKeyPressed(event);
}

b8 TestBase::OnEventKeyReleased(EventContext& event)
{
	return OnKeyReleased(event);
}

b8 TestBase::OnEventMouseButtonPressed(EventContext& event)
{
	return OnMouseButtonPressed(event);
}

b8 TestBase::OnEventMouseButtonReleased(EventContext& event)
{
	return OnMouseButtonReleased(event);
}

b8 TestBase::OnEventMouseMoved(EventContext& event)
{
	return OnMouseMoved(event);
}

b8 TestBase::OnEventMouseScrolled(EventContext& event)
{
	return OnMouseScrolled(event);
}
