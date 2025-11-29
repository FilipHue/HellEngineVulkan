#pragma once

// Internal
#include "input_codes.h"
#include <hellengine/containers/tuple.h>
#include <hellengine/core/api.h>
#include <hellengine/core/defines.h>
#include <hellengine/core/typedefs.h>

namespace hellengine
{

	using namespace containers;
	namespace core
	{

		class Input
		{
		public:
			HE_API static b8 IsKeyPressed(keys key);
			HE_API static b8 IsKeyReleased(keys key);

			HE_API static b8 IsMouseButtonPressed(mouse_buttons button);
			HE_API static b8 IsMouseButtonReleased(mouse_buttons button);

			HE_API static Tuple<f32, f32> GetMousePosition();
			HE_API static f32 GetMouseX();
			HE_API static f32 GetMouseY();
			HE_API static Tuple<f32, f32> GetMouseDelta();
			HE_API static Tuple<f32, f32> GetMouseScroll();

			static void ProcessKey(keys key, b8 pressed);

			static void ProcessMouseButton(mouse_buttons button, b8 pressed);
			static void ProcessMousePosition(f32 x, f32 y);
			static void ProcessMouseScroll(f32 x, f32 y);

			static void ProcessWindowResize(u32 width, u32 height);

			static void Update();

		private:
			Input() = delete;

		private:
			INLINE static std::array<b8, MAX_KEY_COUNT> m_keys{ 0 };
			INLINE static std::array<b8, MAX_KEY_COUNT> m_keys_last_frame{ 0 };

			INLINE static std::array<b8, MAX_MOUSE_BUTTON_COUNT> m_mouse_buttons{ 0 };
			INLINE static std::array<b8, MAX_MOUSE_BUTTON_COUNT> m_mouse_buttons_last_frame{ 0 };

			INLINE static f32 m_mouse_x{ 0 };
			INLINE static f32 m_mouse_y{ 0 };
			INLINE static f32 m_mouse_x_last_frame{ 0 };
			INLINE static f32 m_mouse_y_last_frame{ 0 };
			INLINE static f32 m_mouse_scroll_delta_x{ 0 };
			INLINE static f32 m_mouse_scroll_delta_y{ 0 };
		};

	} // namespace core

} // namespace hellengine