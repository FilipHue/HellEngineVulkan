#include "hepch.h"
#include "window.h"

// Internal
#include <hellengine/core/platform.h>

#if defined(HE_PLATFORM_WINDOWS)
#include <hellengine/platform/glfw/glfw_window.h>
#endif

namespace hellengine
{
	using namespace platform;
	namespace core
	{

		Window* Window::Create(WindowConfiguration configuration)
		{
#if defined(HE_PLATFORM_WINDOWS)
			return new GlfwWindow(configuration);
#endif
		}

	} // namespace core

} // namespace hellengine