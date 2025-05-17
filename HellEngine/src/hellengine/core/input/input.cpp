#include "hepch.h"
#include "input.h"

// Internal
#include <hellengine/core/events/event.h>

namespace hellengine
{

	namespace core
	{

		b8 Input::IsKeyPressed(keys key)
		{
			return m_keys[key];
		}

		b8 Input::IsKeyReleased(keys key)
		{
			return !m_keys[key];
		}

		b8 Input::IsMouseButtonPressed(mouse_buttons button)
		{
			return m_mouse_buttons[button];
		}

		b8 Input::IsMouseButtonReleased(mouse_buttons button)
		{
			return !m_mouse_buttons[button];
		}

		Tuple<f32, f32> Input::GetMousePosition()
		{
			return { m_mouse_x, m_mouse_y };
		}

		f32 Input::GetMouseX()
		{
			return m_mouse_x;
		}

		f32 Input::GetMouseY()
		{
			return m_mouse_y;
		}

		Tuple<f32, f32> Input::GetMouseDelta()
		{
			return { m_mouse_x - m_mouse_x_last_frame, m_mouse_y - m_mouse_y_last_frame };
		}

		Tuple<f32, f32> Input::GetMouseScroll()
		{
			return { m_mouse_scroll_delta_x, m_mouse_scroll_delta_y };
		}

		void Input::ProcessKey(keys key, b8 pressed)
		{
			if (m_keys[key] != pressed)
			{
				m_keys[key] = pressed;
			}
		}

		void Input::ProcessMouseButton(mouse_buttons button, b8 pressed)
		{
			if (m_mouse_buttons[button] != pressed)
			{
				m_mouse_buttons[button] = pressed;
			}
		}

		void hellengine::core::Input::ProcessMousePosition(f32 x, f32 y)
		{
			if (x != m_mouse_x && y != m_mouse_y)
			{
				m_mouse_x_last_frame = m_mouse_x;
				m_mouse_y_last_frame = m_mouse_y;

				m_mouse_x = x;
				m_mouse_y = y;
			}
		}

		void Input::ProcessMouseScroll(f32 x, f32 y)
		{
			m_mouse_scroll_delta_x = x;
			m_mouse_scroll_delta_y = y;
		}

		void Input::ProcessWindowResize(u32 width, u32 height)
		{
		}

		void Input::Update()
		{
			m_mouse_x_last_frame = m_mouse_x;
			m_mouse_y_last_frame = m_mouse_y;
			m_keys_last_frame = m_keys;
			m_mouse_buttons_last_frame = m_mouse_buttons;
			m_mouse_scroll_delta_x = 0.0f;
			m_mouse_scroll_delta_y = 0.0f;
		}

	} // namespace core

} // namespace hellengine