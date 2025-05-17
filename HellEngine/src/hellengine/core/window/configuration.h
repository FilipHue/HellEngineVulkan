#pragma once

// Internal
#include <hellengine/containers/tuple.h>
#include <hellengine/core/typedefs.h>

namespace hellengine
{

	using namespace containers;
	namespace core
	{

		struct WindowConfiguration
		{
			u16 width{ 0 };
			u16 height{ 0 };
			const char* title{ "" };

			Tuple<u16, u16> position{ 0, 0 };

			WindowConfiguration() = default;
			WindowConfiguration(u16 width, u16 height, const char* title, Tuple<u16, u16> position) : width(width), height(height), title(title), position(position) {}
		};

		struct WindowState
		{
			b8 is_running{ true };
			b8 is_suspended{ false };
			b8 is_fullscreen{ false };

			b8 is_cursor_locked{ false };

			void* data{ nullptr };

			WindowState() : is_running(true), is_suspended(false), is_fullscreen(false), is_cursor_locked(false) {};
		};

	} // namespace core

} // namespace hellengine