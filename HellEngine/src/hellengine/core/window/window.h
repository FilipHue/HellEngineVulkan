#pragma once

// Internal
#include "configuration.h"

namespace hellengine
{

	namespace core
	{


		class Window
		{
		public:
			Window() = default;
			Window(WindowConfiguration configuration) : m_configuration(configuration) {}
			virtual ~Window() = default;

			virtual void* GetHandle() = 0;

			virtual void PollEvents() = 0;

			virtual void UpdateInternalState() = 0;

			virtual void SetCursorMode(u32 mode) = 0;

			b8 IsRunning() const { return m_state.is_running; }
			b8 IsIconic() const { return m_state.is_suspended; }

			u16 GetWidth() const { return m_configuration.width; }
			u16 GetHeight() const { return m_configuration.height; }
			const char* GetTitle() const { return m_configuration.title; }

			Tuple<u16, u16>& GetPosition() { return m_configuration.position; }
			const Tuple<u16, u16>& GetPosition() const { return m_configuration.position; }

			Tuple<u16, u16> GetSize() { return { m_configuration.width, m_configuration.height }; }
			const Tuple<u16, u16> GetSize() const { return { m_configuration.width, m_configuration.height }; }

			virtual void SetTitle(const char* title) = 0;
			void SetSize(u16 width, u16 height) { m_configuration.width = width; m_configuration.height = height; }

			const WindowState& GetState() const { return m_state; }

			static Window* Create(WindowConfiguration configuration);

		protected:
			WindowConfiguration m_configuration{};
			WindowState m_state{};
		};

	} // namespace core

} // namespace hellengine