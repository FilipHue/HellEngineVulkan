#pragma once

// Internal
#include "hellengine/ui/shared.h"

namespace hellengine
{

	namespace ui
	{

		class ViewportPanel
		{
		public:
			HE_API ViewportPanel(const std::string& name);
			HE_API ~ViewportPanel() = default;

			HE_API void SetHandle(void* handle);
			HE_API void SetSize(u32 width, u32 height);

			HE_API glm::uvec2& GetSize() { return m_size; }
			HE_API const glm::uvec2& GetSize() const { return m_size; }

			HE_API b8 IsHovered() const { return m_is_hovered; }
			HE_API b8 IsFocused() const { return m_is_focused; }

			HE_API void Draw();

		private:
			std::string m_name;
			glm::uvec2 m_size;

			void* m_handle;

			b8 m_is_hovered;
			b8 m_is_focused;
		};

	} // namespace ui

} // namespace hellengine