#include "hepch.h"
#include "viewport_panel.h"

namespace hellengine
{

	namespace ui
	{

		ViewportPanel::ViewportPanel(const std::string& name)
		{
			m_name = name;
			m_size = glm::uvec2(0);

			m_handle = nullptr;

			m_is_hovered = false;
			m_is_focused = false;
		}

		void ViewportPanel::SetHandle(void* handle)
		{
			m_handle = handle;
		}

		void ViewportPanel::SetSize(u32 width, u32 height)
		{
			m_size = { width, height };
		}

		void ViewportPanel::Draw()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin(m_name.c_str());

			m_is_hovered = ImGui::IsWindowHovered();
			m_is_focused = ImGui::IsWindowFocused();

			ImVec2 availSize = ImGui::GetContentRegionAvail();
			glm::uvec2 newSize = { (uint32_t)availSize.x, (uint32_t)availSize.y };

			if (m_size != newSize)
			{
				m_size = newSize;
			}

			if (m_handle)
			{
				ImGui::Image((ImTextureID)m_handle, ImVec2((f32)m_size.x, (f32)m_size.y));
			}

			ImGui::End();
			ImGui::PopStyleVar();
		}

	} // namespace ui

} // namespace hellengine