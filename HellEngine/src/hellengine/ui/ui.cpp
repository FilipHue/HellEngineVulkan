#include "hepch.h"
#include "ui.h"

// Internal
#include "hellengine/core/events/event.h"

namespace hellengine
{
	namespace ui
	{

		void UI::Init(Window* window, VulkanBackend* backend)
		{
			m_backend = new UIBackend_ImGui();
			m_backend->Init(window, backend);

			//EventDispatcher::AddListener(EventType_WindowClose,			HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_WindowResize,		HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_WindowFocus,			HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_WindowIconified,		HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_WindowMoved,			HE_BIND_EVENTCALLBACK(UI::HandleEvents));

			//EventDispatcher::AddListener(EventType_KeyPressed,			HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_KeyReleased,			HE_BIND_EVENTCALLBACK(UI::HandleEvents));

			//EventDispatcher::AddListener(EventType_MouseButtonPressed,	HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_MouseButtonReleased, HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_MouseMoved,			HE_BIND_EVENTCALLBACK(UI::HandleEvents));
			//EventDispatcher::AddListener(EventType_MouseScrolled,		HE_BIND_EVENTCALLBACK(UI::HandleEvents));
		}

		void UI::Shutdown()
		{
			m_backend->Shutdown();
			delete m_backend;
		}

		b8 UI::HandleEvents(EventContext& event)
		{
			return m_backend->HandleEvents(event);
		}

		void UI::ShowDemoWindow(b8* p_open)
		{
			ImGui::ShowDemoWindow(p_open);
		}

	} // namespace ui

} // namespace hellengine
