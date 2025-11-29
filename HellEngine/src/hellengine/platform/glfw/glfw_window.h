#pragma once

// Internal
#include "glfw_types.h"
#include <hellengine/core/window/window.h>

// External
#include <GLFW/glfw3.h>
#if defined(HE_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif // HE_PLATFORM_WINDOWS

namespace hellengine
{

	using namespace core;
	namespace platform
	{

		class GlfwWindow : public Window
		{
		public:
			GlfwWindow() = default;
			GlfwWindow(const WindowConfiguration& config);
			~GlfwWindow();

			void* GetHandle() override;

			void PollEvents() override;
			void UpdateInternalState() override;

			void SetCursorMode(u32 mode) override;

			void SetTitle(const char* title) override;

			void CenterWindow();
			void CenterCursor();

		private:
			void SetWindowHints();

			void SetWindowCallbacks();
			void SetKeyCallbacks();
			void SetMouseCallbacks();

		private:
			GLFWwindow* m_handle;
			GLFWmonitor* m_monitor;
			const GLFWvidmode* m_vidmode;
		};

	} // namespace platform

} // namespace hellengine