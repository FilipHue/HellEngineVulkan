#pragma once

// Internal
#include <hellengine/ecs/component/components/id.h>
#include <hellengine/ecs/component/components/mesh_filter.h>
#include <hellengine/ecs/component/components/tag.h>
#include <hellengine/ecs/component/components/transform.h>

namespace hellengine
{

	namespace ecs
	{

        template<typename T>
        INLINE HE_API const char* GetComponentTypeToString()
        {
            if constexpr (std::is_same_v<T, IDComponent>)
            {
                return "ID Component";
            }

			if constexpr (std::is_same_v<T, MeshFilterComponent>)
			{
				return "Mesh Filter Component";
			}

			if constexpr (std::is_same_v<T, TagComponent>)
			{
				return "Tag Component";
			}

			if constexpr (std::is_same_v<T, TransformComponent>)
			{
				return "Transform Component";
			}

            return "Unknown Component";
        }

	} // namespace ecs

} // namespace hellengine