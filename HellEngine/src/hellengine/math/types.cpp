#include "hepch.h"
#include "types.h"

namespace hellengine
{

	namespace math
	{

		VkVertexInputBindingDescription Particle::GetBindingDescription()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Particle);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		std::vector<VkVertexInputAttributeDescription> Particle::GetAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {};
			attribute_descriptions.resize(6);

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attribute_descriptions[0].offset = offsetof(Particle, pos);

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(Particle, color);

			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32_SFLOAT;
			attribute_descriptions[2].offset = offsetof(Particle, alpha);

			attribute_descriptions[3].binding = 0;
			attribute_descriptions[3].location = 3;
			attribute_descriptions[3].format = VK_FORMAT_R32_SFLOAT;
			attribute_descriptions[3].offset = offsetof(Particle, size);

			attribute_descriptions[4].binding = 0;
			attribute_descriptions[4].location = 4;
			attribute_descriptions[4].format = VK_FORMAT_R32_SFLOAT;
			attribute_descriptions[4].offset = offsetof(Particle, rotation);

			attribute_descriptions[5].binding = 0;
			attribute_descriptions[5].location = 5;
			attribute_descriptions[5].format = VK_FORMAT_R32_SINT;
			attribute_descriptions[5].offset = offsetof(Particle, type);

			return attribute_descriptions;
		}

	} // namespace math

} // namespace hellengine