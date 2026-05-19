#pragma once

// Internal
#include <hellengine/graphics/graphics_core.h>

// External
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <yaml-cpp/yaml.h>

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

		ALIGN_AS(16) struct CameraData
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec4 position;
		};

		ALIGN_AS(16) struct ObjectData
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

namespace YAML {

	// Specialization for glm::vec2
	template <>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsMap() || node.size() != 2)
				return false;

			rhs.x = node["x"].as<float>();
			rhs.y = node["y"].as<float>();
			return true;
		}
	};

	// Specialization for glm::vec3
	template <>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			node["z"] = rhs.z;
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsMap() || node.size() != 3)
				return false;

			rhs.x = node["x"].as<float>();
			rhs.y = node["y"].as<float>();
			rhs.z = node["z"].as<float>();
			return true;
		}
	};

	// Specialization for glm::vec4
	template <>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			node["z"] = rhs.z;
			node["w"] = rhs.w;
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsMap() || node.size() != 4)
				return false;

			rhs.x = node["x"].as<float>();
			rhs.y = node["y"].as<float>();
			rhs.z = node["z"].as<float>();
			rhs.w = node["w"].as<float>();
			return true;
		}
	};

	// Specialization for glm::ivec2
	template <>
	struct convert<glm::ivec2>
	{
		static Node encode(const glm::ivec2& rhs)
		{
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			return node;
		}

		static bool decode(const Node& node, glm::ivec2& rhs)
		{
			if (!node.IsMap() || node.size() != 2)
				return false;

			rhs.x = node["x"].as<int>();
			rhs.y = node["y"].as<int>();
			return true;
		}
	};

	// Specialization for glm::ivec3
	template <>
	struct convert<glm::ivec3>
	{
		static Node encode(const glm::ivec3& rhs)
		{
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			node["z"] = rhs.z;
			return node;
		}

		static bool decode(const Node& node, glm::ivec3& rhs)
		{
			if (!node.IsMap() || node.size() != 3)
				return false;

			rhs.x = node["x"].as<int>();
			rhs.y = node["y"].as<int>();
			rhs.z = node["z"].as<int>();
			return true;
		}
	};

	// Specialization for glm::ivec4
	template <>
	struct convert<glm::ivec4>
	{
		static Node encode(const glm::ivec4& rhs)
		{
			Node node;
			node["x"] = rhs.x;
			node["y"] = rhs.y;
			node["z"] = rhs.z;
			node["w"] = rhs.w;
			return node;
		}

		static bool decode(const Node& node, glm::ivec4& rhs)
		{
			if (!node.IsMap() || node.size() != 4)
				return false;

			rhs.x = node["x"].as<int>();
			rhs.y = node["y"].as<int>();
			rhs.z = node["z"].as<int>();
			rhs.w = node["w"].as<int>();
			return true;
		}
	};

	// Specialization for glm::mat4
	template <>
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& rhs)
		{
			Node node;
			for (int i = 0; i < 4; ++i)
			{
				node["col" + std::to_string(i)]["x"] = rhs[i].x;
				node["col" + std::to_string(i)]["y"] = rhs[i].y;
				node["col" + std::to_string(i)]["z"] = rhs[i].z;
				node["col" + std::to_string(i)]["w"] = rhs[i].w;
			}
			return node;
		}

		static bool decode(const Node& node, glm::mat4& rhs)
		{
			if (!node.IsMap())
				return false;

			for (int i = 0; i < 4; ++i)
			{
				std::string key = "col" + std::to_string(i);
				if (!node[key])
					return false;
				rhs[i].x = node[key]["x"].as<float>();
				rhs[i].y = node[key]["y"].as<float>();
				rhs[i].z = node[key]["z"].as<float>();
				rhs[i].w = node[key]["w"].as<float>();
			}
			return true;
		}
	};

} // namespace YAML