#include "hepch.h"
#include "multiprojection.h"

namespace hellengine
{

	namespace graphics
	{

		std::ostream& operator<<(std::ostream& os, const ProjectionType& type)
		{
			if (type == ProjectionType::Perspective)
			{
				return os << "Perspective";
			}

			if (type == ProjectionType::Orthographic)
			{
				return os << "Orthographic";
			}
		}

		void MultiProjectionCamera::CreatePerspective(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z)
		{
			m_fov = fov;
			m_aspect_ratio = aspect_ratio;
			m_near = near_z;
			m_far = far_z;

			m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
			m_right = glm::vec3(1.0f, 0.0f, 0.0f);
			m_up = glm::vec3(0.0f, 1.0f, 0.0f);
			m_global_forward = glm::vec3(0.0f, 0.0f, -1.0f);
			m_global_right = glm::vec3(1.0f, 0.0f, 0.0f);
			m_global_up = glm::vec3(0.0f, 1.0f, 0.0f);

			m_position = glm::vec3(0.0f);

			m_type = ProjectionType::Perspective;

			RecalculateProjectionMatrix();
			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::SetAspect(f32 width, f32 height)
		{
			m_aspect_ratio = width / height;
			RecalculateProjectionMatrix();
		}

		void MultiProjectionCamera::SetFov(f32 fov)
		{
			m_fov = fov;
			RecalculateProjectionMatrix();
		}

		void MultiProjectionCamera::CreateOrthographic(f32 left, f32 right, f32 bottom, f32 up, f32 near_z, f32 far_z)
		{
			m_ortho_left = left;
			m_ortho_right = right;
			m_ortho_bottom = bottom;
			m_ortho_up = up;
			m_near = near_z;
			m_far = far_z;

			m_position = glm::vec3(0.0f);

			m_type = ProjectionType::Orthographic;

			RecalculateProjectionMatrix();
			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::SetPosition(glm::vec3 position)
		{
			m_position = position;
			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::SetNear(f32 near_z)
		{
			m_near = near_z;
			RecalculateProjectionMatrix();
		}

		void MultiProjectionCamera::SetFar(f32 far_z)
		{
			m_far = far_z;
			RecalculateProjectionMatrix();
		}

		void MultiProjectionCamera::TranslateX(f32 amount)
		{
			m_position -= glm::normalize(m_right) * amount;
			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::TranslateY(f32 amount)
		{
			m_position += glm::normalize(m_global_up) * amount;
			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::TranslateZ(f32 amount)
		{
			m_position += glm::normalize(m_forward) * amount;
			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::RotateOX(f32 angle)
		{
			glm::vec4 aux = glm::rotate(glm::mat4(1.0f), glm::radians(angle), m_right) * glm::vec4(m_forward, 1.0f);
			m_forward = glm::normalize(glm::vec3(aux));
			m_up = glm::normalize(glm::cross(m_right, m_forward));

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::RotateOY(f32 angle)
		{
			glm::vec4 aux = glm::rotate(glm::mat4(1.f), glm::radians(angle), m_global_up) * glm::vec4(m_forward, 0);
			m_forward = glm::normalize(glm::vec3(aux));

			aux = glm::rotate(glm::mat4(1.f), glm::radians(angle), m_global_up) * glm::vec4(m_right, 0);
			m_right = glm::normalize(glm::vec3(aux));

			m_up = glm::cross(m_right, m_forward);

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::RotateOZ(f32 angle)
		{
			glm::vec4 aux = glm::rotate(glm::mat4(1.f), glm::radians(angle), m_global_up) * glm::vec4(m_right, 1);
			m_right = glm::normalize(glm::vec3(aux));

			aux = glm::rotate(glm::mat4(1.f), glm::radians(angle), m_forward) * glm::vec4(m_up, 0);
			m_forward = glm::normalize(glm::vec3(aux));

			m_up = glm::cross(m_right, m_forward);

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::Rotate(f32 yaw, f32 pitch, f32 roll)
		{
			glm::mat3 rotationMatrix = glm::mat3(
				glm::rotate(glm::mat4(1.0f), glm::radians(yaw), m_global_up) *
				glm::rotate(glm::mat4(1.0f), glm::radians(pitch), m_right)
			);

			m_forward = glm::normalize(rotationMatrix * m_forward);
			m_right = glm::normalize(glm::cross(m_forward, m_global_up));
			m_up = glm::normalize(glm::cross(m_right, m_forward));

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::TranslateLeft(f32 amount)
		{
			m_position.x -= amount;

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::TranslateRight(f32 amount)
		{
			m_position.x += amount;

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::TranslateBottom(f32 amount)
		{
			m_position.y -= amount;

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::TranslateUp(f32 amount)
		{
			m_position.y += amount;

			RecalculateViewMatrix();
		}

		void MultiProjectionCamera::RecalculateProjectionMatrix()
		{
			if (m_type == ProjectionType::Perspective)
			{
				m_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near, m_far);
				m_projection[1][1] *= -1;
			}
			else if (m_type == ProjectionType::Orthographic)
			{
				m_projection = glm::ortho(m_ortho_left, m_ortho_right, m_ortho_bottom, m_ortho_up, m_near, m_far);
			}
		}

		void MultiProjectionCamera::RecalculateViewMatrix()
		{
			if (m_type == ProjectionType::Perspective)
			{
				m_view = glm::lookAt(m_position, m_position + m_forward, m_up);
			}
			else if (m_type == ProjectionType::Orthographic)
			{
				m_view = glm::inverse(glm::translate(glm::mat4(1.f), m_position));
			}
		}

	} // namespace graphics

} // namespace hellengine