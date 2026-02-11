#pragma once

// Internal
#include <hellengine/graphics/graphics_core.h>

// External
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace hellengine
{

	namespace math
	{

		struct Particle {
			glm::vec4 pos;
			glm::vec4 color;
			f32 alpha;
			f32 size;
			f32 rotation;
			u32 type;
			glm::vec4 vel;
			f32 rotationSpeed;

			HE_API static VkVertexInputBindingDescription GetBindingDescription();
			HE_API static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		ALIGN_AS(64) struct CameraData
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		ALIGN_AS(64) struct ObjectData
		{
			glm::mat4 model;
		};

		struct Bounds2D {
			glm::vec2 min;
			glm::vec2 max;
			glm::vec2 extent;
			glm::vec2 center;

			HE_API Bounds2D() : min(glm::vec2(0.0f)), max(glm::vec2(0.0f)), extent(glm::vec2(0.0f)), center(glm::vec2(0.0f)) {}
			HE_API Bounds2D(const glm::vec2& min, const glm::vec2& max) : min(min), max(max)
			{
				extent = max - min;
				center = (min + max) * 0.5f;
			}

			HE_API void Encapsulate(const glm::vec2& point)
			{
				min = glm::min(min, point);
				max = glm::max(max, point);
				extent = max - min;
				center = (min + max) * 0.5f;
			}

			HE_API void SetMinMax(const glm::vec2& new_min, const glm::vec2& new_max)
			{
				min = new_min;
				max = new_max;
				extent = max - min;
				center = (min + max) * 0.5f;
			}
		};

		struct Bounds3D
		{
			glm::vec3 min;
			glm::vec3 max;
			glm::vec3 extent;
			glm::vec3 center;
			HE_API Bounds3D() : min(glm::vec3(0.0f)), max(glm::vec3(0.0f)), extent(glm::vec3(0.0f)), center(glm::vec3(0.0f)) {}
			HE_API Bounds3D(const glm::vec3& min, const glm::vec3& max) : min(min), max(max)
			{
				extent = max - min;
				center = (min + max) * 0.5f;
			}
			HE_API void Encapsulate(const glm::vec3& point)
			{
				min = glm::min(min, point);
				max = glm::max(max, point);
				extent = max - min;
				center = (min + max) * 0.5f;
			}
			HE_API void SetMinMax(const glm::vec3& new_min, const glm::vec3& new_max)
			{
				min = new_min;
				max = new_max;
				extent = max - min;
				center = (min + max) * 0.5f;
			}
		};

	} // namespace math

} // namespace hellengine