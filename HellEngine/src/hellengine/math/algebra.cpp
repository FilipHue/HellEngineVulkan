#include "hepch.h"
#include "algebra.h"

namespace hellengine
{
	namespace math
	{
		std::array<glm::vec3, 8> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view)
		{
			const glm::mat4 inv = glm::inverse(projection * view);
			std::array<glm::vec3, 8> frustum_corners;
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
							2.0f * z - 1.0f,
							1.0f
						);
						frustum_corners[index++] = pt / pt.w;
					}
				}
			}
			return frustum_corners;
		}
	} // namespace math

} // namespace hellengine
