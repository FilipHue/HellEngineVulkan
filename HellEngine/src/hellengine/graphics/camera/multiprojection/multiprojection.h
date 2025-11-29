#pragma once

// Internal
#include "hellengine/graphics/camera/camera.h"

namespace hellengine
{

	namespace graphics
	{

		enum class ProjectionType
		{
			Perspective,
			Orthographic
		};

		class MultiProjectionCamera : public Camera
		{
		public:
			HE_API MultiProjectionCamera() = default;
			HE_API ~MultiProjectionCamera() = default;

			// Perspective
			HE_API void CreatePerspective(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z);

			HE_API void SetAspect(f32 width, f32 height);
			HE_API void SetFov(f32 fov);

			// Orthographic
			HE_API void CreateOrthographic(f32 left, f32 right, f32 bottom, f32 up, f32 near_z, f32 far_z);

			// Global
			HE_API const glm::mat4& GetView() const;
			HE_API glm::mat4& GetView();
			HE_API void SetView(const glm::mat4& view);

			HE_API glm::vec3& GetPosition();
			HE_API const glm::vec3& GetPosition() const;
			HE_API void SetPosition(glm::vec3 position);

			HE_API void SetNear(f32 near_z);
			HE_API void SetFar(f32 far_z);

			HE_API ProjectionType GetType() const;

			// Controls perspective
			HE_API void TranslateX(f32 amount);
			HE_API void TranslateY(f32 amount);
			HE_API void TranslateZ(f32 amount);

			HE_API void RotateOX(f32 angle);
			HE_API void RotateOY(f32 angle);
			HE_API void RotateOZ(f32 angle);

			HE_API void Rotate(f32 yaw, f32 pitch, f32 roll);

			// Controls orthographic
			HE_API void TranslateLeft(f32 amount);
			HE_API void TranslateRight(f32 amount);
			HE_API void TranslateBottom(f32 amount);
			HE_API void TranslateUp(f32 amount);

		private:
			void RecalculateProjectionMatrix();
			void RecalculateViewMatrix();

		private:
			// Perspective
			f32 m_aspect_ratio;
			f32 m_fov;

			glm::vec3 m_forward;
			glm::vec3 m_right;
			glm::vec3 m_up;

			glm::vec3 m_global_forward;
			glm::vec3 m_global_right;
			glm::vec3 m_global_up;

			// Orthographic
			f32 m_ortho_left;
			f32 m_ortho_right;
			f32 m_ortho_bottom;
			f32 m_ortho_up;

			// Global
			glm::mat4 m_view;

			glm::vec3 m_position;

			f32 m_near;
			f32 m_far;

			f32 yaw;
			f32 pitch;
			f32 roll;

			ProjectionType m_type;
		};
		 
	} // namespace graphics

} // namespace hellengine