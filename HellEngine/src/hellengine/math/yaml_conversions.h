//#pragma once
//
//#include <yaml-cpp/yaml.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//namespace YAML
//{
//	// GLM vec3 conversion
//	template<>
//	struct convert<glm::vec3>
//	{
//		static Node encode(const glm::vec3& rhs)
//		{
//			Node node;
//			node["x"] = rhs.x;
//			node["y"] = rhs.y;
//			node["z"] = rhs.z;
//			return node;
//		}
//
//		static bool decode(const Node& node, glm::vec3& rhs)
//		{
//			if (!node.IsMap() || node.size() != 3)
//				return false;
//
//			rhs.x = node["x"].as<float>();
//			rhs.y = node["y"].as<float>();
//			rhs.z = node["z"].as<float>();
//			return true;
//		}
//	};
//
//	// GLM vec4 conversion
//	template<>
//	struct convert<glm::vec4>
//	{
//		static Node encode(const glm::vec4& rhs)
//		{
//			Node node;
//			node["x"] = rhs.x;
//			node["y"] = rhs.y;
//			node["z"] = rhs.z;
//			node["w"] = rhs.w;
//			return node;
//		}
//
//		static bool decode(const Node& node, glm::vec4& rhs)
//		{
//			if (!node.IsMap() || node.size() != 4)
//				return false;
//
//			rhs.x = node["x"].as<float>();
//			rhs.y = node["y"].as<float>();
//			rhs.z = node["z"].as<float>();
//			rhs.w = node["w"].as<float>();
//			return true;
//		}
//	};
//
//	// GLM vec2 conversion
//	template<>
//	struct convert<glm::vec2>
//	{
//		static Node encode(const glm::vec2& rhs)
//		{
//			Node node;
//			node["x"] = rhs.x;
//			node["y"] = rhs.y;
//			return node;
//		}
//
//		static bool decode(const Node& node, glm::vec2& rhs)
//		{
//			if (!node.IsMap() || node.size() != 2)
//				return false;
//
//			rhs.x = node["x"].as<float>();
//			rhs.y = node["y"].as<float>();
//			return true;
//		}
//	};
//
//	// GLM mat4 conversion
//	template<>
//	struct convert<glm::mat4>
//	{
//		static Node encode(const glm::mat4& rhs)
//		{
//			Node node;
//			for (int i = 0; i < 4; ++i)
//			{
//				node["col" + std::to_string(i)]["x"] = rhs[i].x;
//				node["col" + std::to_string(i)]["y"] = rhs[i].y;
//				node["col" + std::to_string(i)]["z"] = rhs[i].z;
//				node["col" + std::to_string(i)]["w"] = rhs[i].w;
//			}
//			return node;
//		}
//
//		static bool decode(const Node& node, glm::mat4& rhs)
//		{
//			if (!node.IsMap())
//				return false;
//
//			for (int i = 0; i < 4; ++i)
//			{
//				std::string col_key = "col" + std::to_string(i);
//				if (node[col_key])
//				{
//					rhs[i].x = node[col_key]["x"].as<float>();
//					rhs[i].y = node[col_key]["y"].as<float>();
//					rhs[i].z = node[col_key]["z"].as<float>();
//					rhs[i].w = node[col_key]["w"].as<float>();
//				}
//			}
//			return true;
//		}
//	};
//
//} // namespace YAML
