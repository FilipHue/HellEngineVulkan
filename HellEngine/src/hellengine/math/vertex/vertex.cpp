#include "hepch.h"
#include "vertex.h"

namespace hellengine
{

	namespace math
	{

		VkVertexInputBindingDescription VertexFormatBase::GetBindingDescription()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(VertexFormatBase);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		std::vector<VkVertexInputAttributeDescription> VertexFormatBase::GetAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {};
			attribute_descriptions.resize(4);

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[0].offset = offsetof(VertexFormatBase, position);

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(VertexFormatBase, color);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(VertexFormatBase, tex_coord);

			attribute_descriptions[3].binding = 0;
			attribute_descriptions[3].location = 3;
			attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[3].offset = offsetof(VertexFormatBase, normal);

			return attribute_descriptions;
		}

		VkVertexInputBindingDescription VertexFormatTangent::GetBindingDescription()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(VertexFormatTangent);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return binding_description;
		}

		std::vector<VkVertexInputAttributeDescription> VertexFormatTangent::GetAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {};
			attribute_descriptions.resize(5);

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[0].offset = offsetof(VertexFormatTangent, position);

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(VertexFormatTangent, color);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(VertexFormatTangent, tex_coord);

			attribute_descriptions[3].binding = 0;
			attribute_descriptions[3].location = 3;
			attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[3].offset = offsetof(VertexFormatTangent, normal);

			attribute_descriptions[4].binding = 0;
			attribute_descriptions[4].location = 4;
			attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[4].offset = offsetof(VertexFormatTangent, tangent);

			return attribute_descriptions;
		}

	} // namespace math

} // namespace hellengine