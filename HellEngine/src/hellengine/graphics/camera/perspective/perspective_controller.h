#pragma once

// Intern
#include "perspective.h"
#include <hellengine/core/input/input.h>
#include <hellengine/core/events/event.h>

namespace hellengine
{

	using namespace core;
	namespace graphics
	{

		class PerspectiveController
		{
		public:
			HE_API PerspectiveController();
			HE_API ~PerspectiveController();

			HE_API void Init();
			HE_API void SetCamera(PerspectiveCamera* camera);

			HE_API void OnProcessUpdate(f32 delta_time);

		private:
			b8 OnMouseMoved(EventContext& event);
			b8 OnMouseButtonPressed(EventContext& event);

		private:
			PerspectiveCamera* m_camera;

			f32 m_translation_speed;
			f32 m_mouse_sensitivity;

			f32 m_last_mouse_x;
			f32 m_last_mouse_y;
		};

	} // graphics

} // hellengine