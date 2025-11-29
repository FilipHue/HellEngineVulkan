#pragma once

// Internal
#include <hellengine/core/api.h>
#include <hellengine/core/typedefs.h>
#include <hellengine/core/defines.h>
#include <hellengine/math/core.h>

namespace hellengine
{

	namespace graphics
	{

		class Camera
		{
		public:
			Camera() = default;
			virtual ~Camera() = default;

			glm::mat4& GetProjection() { return m_projection; }
			const glm::mat4& GetProjection() const { return m_projection; }

		protected:
			glm::mat4 m_projection = glm::mat4(1.0f);
		};

	} // namespace graphics

} // namespace hellengine