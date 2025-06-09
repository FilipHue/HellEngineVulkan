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
		}

		void UI::Shutdown()
		{
			m_backend->Shutdown();
			delete m_backend;
		}

		void UI::ShowDemoWindow(b8* p_open)
		{
			ImGui::ShowDemoWindow(p_open);
		}

	} // namespace ui

} // namespace hellengine
