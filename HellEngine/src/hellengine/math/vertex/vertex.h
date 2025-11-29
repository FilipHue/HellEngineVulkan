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
			std::vector<glm::vec3> positions;
			std::vector<glm::vec4> colors;
			std::vector<glm::vec2> tex_coords;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec3> tangents;
			std::vector<glm::vec3> bitangents;
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
			glm::vec3 bitangent;

			VertexFormatTangent() : position(0.0f), color(0.0f), tex_coord(0.0f), normal(0.0f), tangent(0.0f), bitangent(0.0f) {}
			VertexFormatTangent(glm::vec3 position, glm::vec4 color, glm::vec2 tex_coord, glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 bitangent = glm::vec3(0.0f)) : position(position), color(color), tex_coord(tex_coord), normal(normal), tangent(tangent), bitangent(bitangent) {}

			HE_API static VkVertexInputBindingDescription GetBindingDescription();
			HE_API static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

	} // namespace math

} // namespace hellengine