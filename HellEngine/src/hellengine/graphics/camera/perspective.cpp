#include "hepch.h"
#include "perspective.h"

namespace hellengine
{

	namespace graphics
	{

		void PerspectiveCamera::Create(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z)
		{
			m_fov = fov;
			m_aspect_ratio = aspect_ratio;
			m_near = near_z;
			m_far = far_z;

			m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
			m_right = glm::vec3(1.0f, 0.0f, 0.0f);
			m_up = glm::vec3(0.0f, 1.0f, 0.0f);
			m_global_up = glm::vec3(0.0f, 1.0f, 0.0f);

			m_position = glm::vec3(0.0f);

			RecalculateProjectionMatrix();
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::SetAspect(f32 width, f32 height)
		{
			m_aspect_ratio = width / height;
			RecalculateProjectionMatrix();
		}

		void PerspectiveCamera::SetFov(f32 fov)
		{
			m_fov = fov;
			RecalculateProjectionMatrix();
		}

		void PerspectiveCamera::SetPosition(glm::vec3 position)
		{
			m_position = position;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::MoveForward(f32 amount)
		{
			m_position += glm::normalize(m_forward) * amount;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::MoveBackward(f32 amount)
		{
			m_position -= glm::normalize(m_forward) * amount;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::MoveLeft(f32 amount)
		{
			m_position -= glm::normalize(m_right) * amount;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::MoveRight(f32 amount)
		{
			m_position += glm::normalize(m_right) * amount;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::MoveUp(f32 amount)
		{
			m_position += glm::normalize(m_global_up) * amount;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::MoveDown(f32 amount)
		{
			m_position -= glm::normalize(m_global_up) * amount;
			RecalculateViewMatrix();
		}

		void PerspectiveCamera::RotateOX(f32 angle)
		{
			glm::vec4 aux = glm::rotate(glm::mat4(1.0f), angle, m_right) * glm::vec4(m_forward, 1.0f);
			m_forward = glm::normalize(glm::vec3(aux));
			m_up = glm::normalize(glm::cross(m_right, m_forward));

			RecalculateViewMatrix();
		}

		void PerspectiveCamera::RotateOY(f32 angle)
		{
			glm::vec4 aux = glm::rotate(glm::mat4(1.f), angle, m_global_up) * glm::vec4(m_forward, 0);
			m_forward = glm::normalize(glm::vec3(aux));

			aux = glm::rotate(glm::mat4(1.f), angle, m_global_up) * glm::vec4(m_right, 0);
			m_right = glm::normalize(glm::vec3(aux));

			m_up = glm::cross(m_right, m_forward);

			RecalculateViewMatrix();
		}

		void PerspectiveCamera::RotateOZ(f32 angle)
		{
			glm::vec4 aux = glm::rotate(glm::mat4(1.f), angle, m_global_up) * glm::vec4(m_right, 1);
			m_right = glm::normalize(glm::vec3(aux));

			aux = glm::rotate(glm::mat4(1.f), angle, m_forward) * glm::vec4(m_up, 0);
			m_forward = glm::normalize(glm::vec3(aux));

			m_up = glm::cross(m_right, m_forward);

			RecalculateViewMatrix();
		}

		void PerspectiveCamera::Rotate(f32 yaw, f32 pitch, f32 roll)
		{
			glm::mat3 rotationMatrix = glm::mat3(
				glm::rotate(glm::mat4(1.0f), yaw, m_global_up) *
				glm::rotate(glm::mat4(1.0f), pitch, m_right)
			);

			m_forward = glm::normalize(rotationMatrix * m_forward);
			m_right = glm::normalize(glm::cross(m_forward, m_global_up));
			m_up = glm::normalize(glm::cross(m_right, m_forward));

			RecalculateViewMatrix();
		}

		void PerspectiveCamera::RecalculateProjectionMatrix()
		{
			m_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near, m_far);
			m_projection[1][1] *= -1; // Flip the y axis
		}

		void PerspectiveCamera::RecalculateViewMatrix()
		{
			m_view = glm::lookAt(m_position, m_position + m_forward, m_up);
		}

	} // namespace graphics

} // namespace hellengine