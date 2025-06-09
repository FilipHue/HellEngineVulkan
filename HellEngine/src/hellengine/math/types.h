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

	} // namespace math

} // namespace hellengine