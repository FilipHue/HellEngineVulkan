#pragma once

// Internal
#include <hellengine/graphics/core.h>

// External
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace hellengine
{

	namespace math
	{

		struct Vertex
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 tex_coord;
			glm::vec3 normal;

			Vertex() : position(glm::vec3(0.0f)), color(glm::vec4(0.0f)), tex_coord(0.0f), normal(glm::vec3(0.0f)) {}
			Vertex(glm::vec3 position, glm::vec4 color, glm::vec2 tex_coord, glm::vec3 normal = glm::vec3(0, 1, 0)) : position(position), color(color), tex_coord(tex_coord), normal(normal) {}

			HE_API static VkVertexInputBindingDescription GetBindingDescription();
			HE_API static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		struct CameraData
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct ObjectData
		{
			glm::mat4 model;
		};

	} // namespace math

} // namespace hellengine