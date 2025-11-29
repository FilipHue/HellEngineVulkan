#pragma once

// Internal
#include "multiprojection.h"
#include <hellengine/core/input/input.h>
#include <hellengine/core/events/event.h>

namespace hellengine
{

	using namespace core;
	namespace graphics
	{

		class MultiProjectionController
		{
		public:
			HE_API MultiProjectionController();
			HE_API ~MultiProjectionController();

			HE_API void Init();

			HE_API void SetCamera(MultiProjectionCamera* camera);
			HE_API MultiProjectionCamera* GetCamera();

			HE_API void SetActive(b8 active);
			HE_API b8 IsActive();

			HE_API void OnProcessUpdate(f32 delta_time);

		private:
			b8 OnMouseMoved(EventContext& event);
			b8 OnMouseButtonPressed(EventContext& event);

		private:
			MultiProjectionCamera* m_camera;

			f32 m_translation_speed;
			f32 m_mouse_sensitivity;

			f32 m_last_mouse_x;
			f32 m_last_mouse_y;

			b8 m_is_active;
		};

	} // namespace graphics

} // namespace hellengine