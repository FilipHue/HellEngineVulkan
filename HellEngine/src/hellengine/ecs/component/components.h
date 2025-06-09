#pragma once

// Internal
#include <hellengine/ecs/component/components/id.h>

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

            return "Unknown Component";
        }

	} // namespace ecs

} // namespace hellengine