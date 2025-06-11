#pragma once

// Internal
#include "hellengine/ui/shared.h"

namespace hellengine
{

	namespace ui
	{

		class Panel
		{
		public:
			HE_API Panel(const std::string& name);
			HE_API virtual ~Panel() = default;

			HE_API std::string& GetName() { return m_name; }
			HE_API void SetName(const std::string& name) { m_name = name; }

			HE_API Bounds2D& GetBounds() { return m_bounds; }
			HE_API void SetBounds(const Bounds2D& bounds) { m_bounds = bounds; }

			HE_API glm::uvec2& GetSize() { return m_size; }
			HE_API void SetSize(u32 width, u32 height) { m_size = { width, height }; }

			HE_API glm::uvec2& GetPosition() { return m_position; }
			HE_API void SetPosition(const glm::uvec2& position) { m_position = position; }

			HE_API b8 IsHovered() const { return m_is_hovered; }
			HE_API b8 IsFocused() const { return m_is_focused; }

			HE_API virtual void Draw() = 0;

		protected:
			std::string m_name;
			Bounds2D m_bounds;

			glm::uvec2 m_size;
			glm::uvec2 m_position;

			b8 m_is_hovered;
			b8 m_is_focused;
		};

	} // namespace ui

} // namespace hellengine