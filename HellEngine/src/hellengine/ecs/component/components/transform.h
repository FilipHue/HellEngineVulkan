#pragma once

// Internal
#include <hellengine/ecs/shared.h>

namespace hellengine
{

	namespace ecs
	{

        struct TransformComponent
        {
			DEFAULT_ALL(TransformComponent);

			glm::vec3 local_position = glm::vec3(0.0f);
			glm::vec3 local_rotation = glm::vec3(0.0f);
			glm::vec3 local_scale = glm::vec3(1.0f);

			glm::mat4 world_transform = glm::mat4(1.0f);

			b8 is_dirty = true;

			glm::mat4 GetLocalTransform() const
			{
				glm::mat4 translation = glm::translate(glm::mat4(1.0f), local_position);
				glm::mat4 rotation = 
					glm::rotate(glm::mat4(1.0f), local_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
					glm::rotate(glm::mat4(1.0f), local_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
					glm::rotate(glm::mat4(1.0f), local_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
				glm::mat4 scale = glm::scale(glm::mat4(1.0f), local_scale);
				return translation * rotation * scale;
			}
        };

	} // namespace ecs

} // namespace hellengine