#include "hepch.h"
#include "multiprojection_controller.h"

namespace hellengine
{

	namespace graphics
	{

		MultiProjectionController::MultiProjectionController()
		{
			m_camera = nullptr;
			m_translation_speed = 5.0f;
			m_mouse_sensitivity = 0.1f;
		}

		MultiProjectionController::~MultiProjectionController()
		{
			delete m_camera;
		}

		void MultiProjectionController::Init()
		{
			EventDispatcher::AddListener(EventType_MouseMoved, HE_BIND_EVENTCALLBACK(OnMouseMoved));
			EventDispatcher::AddListener(EventType_MouseButtonPressed, HE_BIND_EVENTCALLBACK(OnMouseButtonPressed));
		}

		void MultiProjectionController::OnProcessUpdate(f32 delta_time)
		{
			if (!m_is_active)
			{
				return;
			}

			if (m_camera->GetType() == ProjectionType::Perspective)
			{
				if (Input::IsKeyPressed(KEY_A))
				{
					m_camera->TranslateX(m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_D))
				{
					m_camera->TranslateX(-m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_W))
				{
					m_camera->TranslateZ(m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_S))
				{
					m_camera->TranslateZ(-m_translation_speed * delta_time);
				}

				if (Input::IsKeyPressed(KEY_Q))
				{
					m_camera->TranslateY(-m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_E))
				{
					m_camera->TranslateY(m_translation_speed * delta_time);
				}
			} 
			else if (m_camera->GetType() == ProjectionType::Orthographic)
			{
				if (Input::IsKeyPressed(KEY_A))
				{
					m_camera->TranslateLeft(m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_D))
				{
					m_camera->TranslateRight(m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_W))
				{
					m_camera->TranslateUp(m_translation_speed * delta_time);
				}
				if (Input::IsKeyPressed(KEY_S))
				{
					m_camera->TranslateBottom(m_translation_speed * delta_time);
				}
			}
		}

		b8 MultiProjectionController::OnMouseMoved(EventContext& event)
		{
			if (!m_is_active)
			{
				return false;
			}

			if (m_camera->GetType() == ProjectionType::Perspective)
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
			}

			return false;
		}

		b8 MultiProjectionController::OnMouseButtonPressed(EventContext& event)
		{
			if (!m_is_active)
			{
				return false;
			}

			if (m_camera->GetType() == ProjectionType::Perspective)
			{
				m_last_mouse_x = Input::GetMouseX();
				m_last_mouse_y = Input::GetMouseY();
			}

			return false;
		}

	} // namespace graphics

} // namespace hellengine