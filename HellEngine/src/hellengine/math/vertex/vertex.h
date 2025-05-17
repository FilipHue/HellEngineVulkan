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

		struct RawVertexData
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 tex_coord;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
		};

		struct VertexFormatBase
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 tex_coord;
			glm::vec3 normal;

			VertexFormatBase() : position(0.0f), color(0.0f), tex_coord(0.0f), normal(0.0f) {}
			VertexFormatBase(glm::vec3 position, glm::vec4 color, glm::vec2 tex_coord, glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f)) : position(position), color(color), tex_coord(tex_coord), normal(normal) {}

			HE_API static VkVertexInputBindingDescription GetBindingDescription();
			HE_API static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		struct VertexFormatTangent
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 tex_coord;
			glm::vec3 normal;
			glm::vec3 tangent;

			VertexFormatTangent() : position(0.0f), color(0.0f), tex_coord(0.0f), normal(0.0f), tangent(0.0f) {}
			VertexFormatTangent(glm::vec3 position, glm::vec4 color, glm::vec2 tex_coord, glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 tangent = glm::vec3(0.0f)) : position(position), color(color), tex_coord(tex_coord), normal(normal), tangent(tangent) {}

			HE_API static VkVertexInputBindingDescription GetBindingDescription();
			HE_API static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

	} // namespace math

} // namespace hellengine