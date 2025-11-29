#pragma once

// Internal
#include "hellengine/graphics/camera/camera.h"

namespace hellengine
{

	namespace graphics
	{

		class PerspectiveCamera : public Camera
		{
		public:
			HE_API PerspectiveCamera() = default;
			HE_API ~PerspectiveCamera() = default;

			HE_API void Create(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z);

			HE_API void SetAspect(f32 width, f32 height);
			HE_API void SetFov(f32 fov);

			HE_API glm::vec3& GetPosition();
			HE_API const glm::vec3& GetPosition() const;
			HE_API void SetPosition(glm::vec3 position);

			HE_API void SetProjection(glm::mat4& projection);

			HE_API const glm::mat4& GetView() const;
			HE_API glm::mat4& GetView();
			HE_API void SetView(const glm::mat4& view);

			// Controlls
			HE_API void MoveForward(f32 amount);
			HE_API void MoveBackward(f32 amount);
			HE_API void MoveLeft(f32 amount);
			HE_API void MoveRight(f32 amount);
			HE_API void MoveUp(f32 amount);
			HE_API void MoveDown(f32 amount);

			HE_API void RotateOX(f32 angle);
			HE_API void RotateOY(f32 angle);
			HE_API void RotateOZ(f32 angle);

			HE_API void Rotate(f32 yaw, f32 pitch, f32 roll);

		private:
			void RecalculateProjectionMatrix();
			void RecalculateViewMatrix();

		private:
			f32 m_fov;
			f32 m_aspect_ratio;
			f32 m_near;
			f32 m_far;

			glm::mat4 m_view;

			glm::vec3 m_position;

			glm::vec3 m_forward;
			glm::vec3 m_right;
			glm::vec3 m_up;

			glm::vec3 m_global_forward;
			glm::vec3 m_global_right;
			glm::vec3 m_global_up;

			f32 yaw;
			f32 pitch;
			f32 roll;
		};

	} // namespace graphics

} // namespace hellengine