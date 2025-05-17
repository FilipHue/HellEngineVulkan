#include <hepch.h>
#include "event.h"

// External
#include <imgui.h>

namespace hellengine
{

	namespace core
	{

		void EventDispatcher::AddListener(EventType type, EventCallback callback)
		{
			for (auto& listener : m_listeners[type])
			{
				if (listener.target_type() == callback.target_type())
				{
					HE_CORE_WARN("Listener already exists for event type: {0}", EventTypeToString(type));
					return;
				}
			}

			m_listeners[type].push_back(callback);
		}

		void EventDispatcher::RemoveListener(EventType type, EventCallback callback)
		{
			for (int i = 0; i < m_listeners[type].size(); i++)
			{
				if (m_listeners[type][i].target_type() == callback.target_type())
				{
					m_listeners[type].erase(m_listeners[type].begin() + i);
					break;
				}
			}
		}

		void EventDispatcher::Dispatch(EventContext& event)
		{
			for (auto& listener : m_listeners[event.type])
			{
				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse || io.WantCaptureKeyboard)
				{
					event.handled = true;
					break;
				}

				if (listener(event))
				{
					event.handled = true;
					break;
				}
			}
		}

	} // namespace core

} // namespace hellengine