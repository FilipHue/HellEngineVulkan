#pragma once

// Internal
#include <hellengine/core/defines.h>
#include <hellengine/core/typedefs.h>

namespace hellengine
{

	namespace core
	{

		enum EventType
		{
			EventType_None = 0,

			// Application events
			EventType_AppTick,
			EventType_AppUpdate,
			EventType_AppRender,

			// Window events
			EventType_WindowClose,
			EventType_WindowResize,
			EventType_WindowFocus,
			EventType_WindowIconified,
			EventType_WindowMoved,

			// Keyboard events
			EventType_KeyPressed,
			EventType_KeyReleased,
			EventType_KeyTyped,

			// Mouse events
			EventType_MouseButtonPressed,
			EventType_MouseButtonReleased,
			EventType_MouseMoved,
			EventType_MouseScrolled,

			// Max event types
			EventType_Count
		};

		FORCEINLINE static const char* EventTypeToString(EventType type)
		{
			switch (type)
			{
			case EventType_None:					return "None";
			case EventType_AppTick:					return "AppTick";
			case EventType_AppUpdate:				return "AppUpdate";
			case EventType_AppRender:				return "AppRender";
			case EventType_WindowClose:				return "WindowClose";
			case EventType_WindowResize:			return "WindowResize";
			case EventType_WindowFocus:				return "WindowFocus";
			case EventType_WindowIconified:			return "WindowMinimized";
			case EventType_WindowMoved:				return "WindowMoved";
			case EventType_KeyPressed:				return "KeyPressed";
			case EventType_KeyReleased:				return "KeyReleased";
			case EventType_KeyTyped:				return "KeyTyped";
			case EventType_MouseButtonPressed:		return "MouseButtonPressed";
			case EventType_MouseButtonReleased:		return "MouseButtonReleased";
			case EventType_MouseMoved:				return "MouseMoved";
			case EventType_MouseScrolled:			return "MouseScrolled";
			default:								return "Unknown";
			}
		}

		union EventData
		{
			// Window events
			struct WindowResize
			{
				i32 width{ 0 };
				i32 height{ 0 };
			} window_resize;

			struct WindowMoved
			{
				i32 x{ 0 };
				i32 y{ 0 };
			} window_moved;

			struct WindowFocus
			{
				b8 focused{ false };
			} window_focus;

			struct WindowIconified
			{
				b8 is_iconified{ false };
			} window_iconified;

			// Keyboard events
			struct KeyEvent
			{
				i32 key{ 0 };
				i32 scancode{ 0 };
				i32 mods{ 0 };
				b8 is_pressed{ false };
			} key_event;

			// Mouse events
			struct MouseButtonEvent
			{
				i32 button{ 0 };
				i32 mods{ 0 };
				b8 is_pressed{ false };
			} mouse_button_event;

			struct MouseMoved
			{
				f32 x{ 0 };
				f32 y{ 0 };
			} mouse_moved;

			struct MouseScrolled
			{
				f32 delta_x{ 0 };
				f32 delta_y{ 0 };
			} mouse_scrolled;
		};

		struct EventContext
		{
			EventData data{};
			EventType type{ EventType_None };
			b8 handled = false;

			b8 operator==(const EventContext& other) const { return type == other.type; }
			b8 operator!=(const EventContext& other) const { return type != other.type; }

			const char* GetName() const { return EventTypeToString(type); }
		};

		using EventCallback = std::function<b8(EventContext&)>;

	} // namespace core

} // namespace hellengine