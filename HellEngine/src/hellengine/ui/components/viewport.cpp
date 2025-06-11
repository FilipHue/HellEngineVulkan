#include "hepch.h"
#include "viewport.h"

namespace hellengine
{

	namespace ui
	{

		Viewport::Viewport(const std::string& name) : Panel(name)
		{
			m_handle = nullptr;
		}

		void Viewport::Draw()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin(m_name.c_str());

			ImVec2 avail_size = ImGui::GetContentRegionAvail();
			glm::uvec2 new_size = { (uint32_t)avail_size.x, (uint32_t)avail_size.y };

			auto viweport_offset = ImGui::GetWindowPos();
			auto viewport_min_region = ImGui::GetWindowContentRegionMin();
			auto viewport_max_region = ImGui::GetWindowContentRegionMax();

			ImVec2 viewport_min = { viweport_offset.x + viewport_min_region.x, viweport_offset.y + viewport_min_region.y };
			ImVec2 viewport_max = { viweport_offset.x + viewport_max_region.x, viweport_offset.y + viewport_max_region.y };

			m_bounds = {
				{ (u32)viewport_min.x, (u32)viewport_min.y },
				{ (u32)viewport_max.x, (u32)viewport_max.y }
			};

			m_size = new_size;

			m_position = {
				(u32)viewport_min.x,
				(u32)viewport_min.y
			};

			m_is_hovered = ImGui::IsWindowHovered();
			m_is_focused = ImGui::IsWindowFocused();

			ImGui::Image((ImTextureID)m_handle, ImVec2((f32)m_size.x, (f32)m_size.y));

			ImGui::End();
			ImGui::PopStyleVar();
		}

	} // namespace ui

} // namespace hellengine