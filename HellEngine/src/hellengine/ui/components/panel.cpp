#include "hepch.h"
#include "panel.h"

namespace hellengine
{

	namespace ui
	{

		Panel::Panel(const std::string& name)
		{
			m_name = name;
			m_bounds = Bounds2D{ { 0, 0 }, { 0, 0 } };

			m_size = glm::uvec2(0);
			m_position = glm::uvec2(0, 0);

			m_is_hovered = false;
			m_is_focused = false;
		}

	} // namespace ui

} // namespace hellengine