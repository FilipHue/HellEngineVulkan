#include "hepch.h"
#include "perspective_controller.h"

namespace hellengine
{
	
	namespace graphics
	{

		PerspectiveController::PerspectiveController()
		{
			m_camera = nullptr;
			m_translation_speed = 5.0f;
			m_mouse_sensitivity = 0.001f;
		}

		PerspectiveController::~PerspectiveController()
		{
			delete m_camera;
		}

		void PerspectiveController::Init()
		{
			EventDispatcher::AddListener(EventType_MouseMoved, HE_BIND_EVENTCALLBACK(PerspectiveController::OnMouseMoved));
			EventDispatcher::AddListener(EventType_MouseButtonPressed, HE_BIND_EVENTCALLBACK(PerspectiveController::OnMouseButtonPressed));
		}

		void PerspectiveController::OnProcessUpdate(f32 delta_time)
		{
			if (!Input::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				return;
			}

			if (Input::IsKeyPressed(KEY_A))
			{
				m_camera->MoveLeft(m_translation_speed * delta_time);
			}
			if (Input::IsKeyPressed(KEY_D))
			{
				m_camera->MoveRight(m_translation_speed * delta_time);
			}
			if (Input::IsKeyPressed(KEY_W))
			{
				m_camera->MoveForward(m_translation_speed * delta_time);
			}
			if (Input::IsKeyPressed(KEY_S))
			{
				m_camera->MoveBackward(m_translation_speed * delta_time);
			}

			if (Input::IsKeyPressed(KEY_Q))
			{
				m_camera->MoveDown(m_translation_speed * delta_time);
			}
			if (Input::IsKeyPressed(KEY_E))
			{
				m_camera->MoveUp(m_translation_speed * delta_time);
			}
		}

		b8 PerspectiveController::OnMouseMoved(EventContext& event)
		{
			if (Input::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				f32 x_offset = event.data.mouse_moved.x - m_last_mouse_x;
				f32 y_offset = event.data.mouse_moved.y - m_last_mouse_y;

				m_last_mouse_x = (f32)event.data.mouse_moved.x;
				m_last_mouse_y = (f32)event.data.mouse_moved.y;

				x_offset *= m_mouse_sensitivity;
				y_offset *= m_mouse_sensitivity;

				m_camera->RotateOX(-y_offset);
				m_camera->RotateOY(-x_offset);
			}

			return false;
		}

		b8 PerspectiveController::OnMouseButtonPressed(EventContext& event)
		{
			m_last_mouse_x = Input::GetMouseX();
			m_last_mouse_y = Input::GetMouseY();

			return false;
		}

	} // graphics

} // hellengine