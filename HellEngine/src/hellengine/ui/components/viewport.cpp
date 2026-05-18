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

		b8 Viewport::Begin()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			if (!ImGui::Begin(m_name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::PopStyleVar();
				ImGui::End();
				return false;
			}

			return true;
		}

		void Viewport::Draw()
		{
			ImVec2 avail_size = ImGui::GetContentRegionAvail();

			m_size = {
				static_cast<u32>(avail_size.x),
				static_cast<u32>(avail_size.y)
			};

			ImGui::Image(
				(ImTextureID)m_handle,
				ImVec2(static_cast<f32>(m_size.x), static_cast<f32>(m_size.y))
			);

			ImVec2 image_min = ImGui::GetItemRectMin();
			ImVec2 image_max = ImGui::GetItemRectMax();

			m_bounds = {
				{ static_cast<u32>(image_min.x), static_cast<u32>(image_min.y) },
				{ static_cast<u32>(image_max.x), static_cast<u32>(image_max.y) }
			};

			m_position = {
				static_cast<u32>(image_min.x),
				static_cast<u32>(image_min.y)
			};

			m_is_hovered = ImGui::IsItemHovered();
			m_is_focused = ImGui::IsWindowFocused();
		}

		void Viewport::End()
		{
			ImGui::PopStyleVar();
			ImGui::End();
		}

	} // namespace ui

} // namespace hellengine