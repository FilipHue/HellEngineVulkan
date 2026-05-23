#pragma once

// Internal
#include <hellengine/core/api.h>
#include <hellengine/core/typedefs.h>
#include <hellengine/core/defines.h>
#include <hellengine/math/core.h>

namespace hellengine
{

	namespace graphics
	{

		class Camera
		{
		public:
			Camera() = default;
			virtual ~Camera() = default;

			glm::mat4& GetProjection() { return m_projection; }
			const glm::mat4& GetProjection() const { return m_projection; }

			static std::array<glm::vec3, 8> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view)
			{
				const glm::mat4 inv = glm::inverse(projection * view);
				std::array<glm::vec3, 8> frustum_corners{};
				u32 index = 0;
				for (u32 x = 0; x < 2; x++)
				{
					for (u32 y = 0; y < 2; y++)
					{
						for (u32 z = 0; z < 2; z++)
						{
							const glm::vec4 pt = inv * glm::vec4(
								2.0f * x - 1.0f,
								2.0f * y - 1.0f,
								static_cast<f32>(z),
								1.0f
							);
							frustum_corners[index++] = pt / pt.w;
						}
					}
				}
				return frustum_corners;
			}

			static glm::vec3 GetFrustumCenter(const std::array<glm::vec3, 8>& corners)
			{
				glm::vec3 frustumCenter(0.0f);
				for (const glm::vec3& corner : corners)
				{
					frustumCenter += corner;
				}
				frustumCenter /= static_cast<f32>(corners.size());
				return frustumCenter;
			}

			static f32 GetFrustumRadius(const std::array<glm::vec3, 8>& corners, const glm::vec3& center)
			{
				f32 radius = 0.0f;
				for (const glm::vec3& corner : corners)
				{
					f32 distance = glm::length(corner - center);
					radius = glm::max(radius, distance);
				}
				return radius;
			}

		protected:
			glm::mat4 m_projection = glm::mat4(1.0f);
		};

	} // namespace graphics

} // namespace hellengine