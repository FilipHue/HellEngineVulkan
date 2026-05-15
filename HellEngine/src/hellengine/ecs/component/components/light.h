#pragma once

// Internal
#include <hellengine/ecs/shared.h>

namespace hellengine
{

	namespace ecs
	{

		enum LightType
		{
			LightType_Point = 0,
			LightType_Directional = 1,
			LightType_Spot = 2
		};

		struct LightComponent
		{
			DEFAULT_ALL(LightComponent);

			LightType type = LightType_Point;

			glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
			f32 intensity = 20.0f;

			// Point/Spot light properties
			f32 range = 100.0f;
			f32 attenuation = 1.0f;

			// Spot light properties
			f32 inner_cone_angle = glm::radians(30.0f);
			f32 outer_cone_angle = glm::radians(45.0f);

			b8 enabled = true;
			b8 cast_shadows = false;
		};

	} // namespace ecs

} // namespace hellengine
