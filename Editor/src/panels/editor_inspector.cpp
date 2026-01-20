#include "editor_inspector.h"

EditorInspector::EditorInspector() : Panel("Inspector")
{
	NO_OP;
}

void EditorInspector::Init()
{
    NO_OP;
}

b8 EditorInspector::Begin()
{
    if (!ImGui::Begin(m_name.c_str()))
    {
        ImGui::End();
        return false;
    }

    return true;
}

void EditorInspector::Draw()
{
	if (m_selected_entity == NULL_ENTITY)
	{
		return;
	}

	DrawEntityComponents();
}

void EditorInspector::End()
{
    ImGui::End();
}

Entity EditorInspector::GetSelectedEntity()
{
	return m_selected_entity;
}

void EditorInspector::SetSelectedEntity(Entity entity)
{
	if (SceneManager::GetInstance()->GetActiveScene()->GetRegistry().valid(entity.GetHandle()) == false)
    {
        m_selected_entity = NULL_ENTITY;
        return;
    }

	m_selected_entity = entity;
}

void EditorInspector::DrawEntityComponents()
{
	static bool is_enabled = true;

	auto& tag = m_selected_entity.GetComponent<TagComponent>().tag;

	ImGui::PushID("TagHeader");
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

	if (ImGui::Checkbox("##Enabled", &is_enabled))
	{

	}

	ImGui::SameLine(0.0f, 6.0f);

	static char buf[256];
	_memccpy(buf, tag.c_str(), 0, sizeof(buf));

	ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputText("##Tag", buf, sizeof(buf), flags))
    {
        tag = buf;
    }

	ImGui::PopStyleVar();
	ImGui::PopID();

	ImGui::Separator();

	DrawComponent<TransformComponent>("Transform", m_selected_entity,
		[](TransformComponent& c)
		{
			DrawTransformVec3("Position", c.local_position);

			glm::vec3 rot = glm::degrees(c.local_rotation);
			DrawTransformVec3("Rotation", rot);
			c.local_rotation = glm::radians(rot);

			DrawTransformVec3("Scale", c.local_scale, 1.f);

			c.is_dirty = true;
		});

    DrawComponent<MeshFilterComponent>("Mesh Filter", m_selected_entity,
        [&](MeshFilterComponent& c)
        {
            MeshManager& mm = *MeshManager::GetInstance();
            IDComponent& idComp = m_selected_entity.GetComponent<IDComponent>();

            const Mesh* current = c.mesh;
            const std::string fullPreview =
                (current && !current->GetName().empty()) ? current->GetName() : "None";

            static char search[128] = "";

            auto MakeEllipsisText = [&](const std::string& text, float maxTextWidth) -> std::string
                {
                    if (maxTextWidth <= 0.0f) return "...";
                    if (ImGui::CalcTextSize(text.c_str()).x <= maxTextWidth) return text;

                    const char* ell = "...";
                    const float ellW = ImGui::CalcTextSize(ell).x;

                    std::string out = text;
                    while (!out.empty() && (ImGui::CalcTextSize(out.c_str()).x + ellW) > maxTextWidth)
                        out.pop_back();

                    return out.empty() ? std::string("...") : (out + "...");
                };

            auto matches = [&](const std::string& name) -> bool {
                if (search[0] == '\0') return true;
                std::string a = name, b = search;
                std::transform(a.begin(), a.end(), a.begin(), ::tolower);
                std::transform(b.begin(), b.end(), b.begin(), ::tolower);
                return a.find(b) != std::string::npos;
                };

            ImGui::PushItemWidth(-FLT_MIN);

            // Combo width (available content region)
            const float comboW = ImGui::GetContentRegionAvail().x;

            // Ellipsis for preview
            const float arrowAndPadding =
                ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 4.0f;
            const float previewMax = comboW - arrowAndPadding;
            std::string previewEll = MakeEllipsisText(fullPreview, previewMax);

            // Width fixed to comboW. Height fixed to N items (scrollbar appears if more).
            const int   visibleItems = 10;
            const float rowH = ImGui::GetTextLineHeightWithSpacing();
            const float searchH = ImGui::GetFrameHeight();
            const float sepH = ImGui::GetStyle().ItemSpacing.y + 1.0f;

            const float popupH = (searchH + sepH) + visibleItems * rowH + ImGui::GetStyle().WindowPadding.y * 2.0f;

            ImGui::SetNextWindowSizeConstraints(
                ImVec2(comboW, popupH),
                ImVec2(comboW, popupH)
            );

            if (ImGui::BeginCombo("##Mesh", previewEll.c_str(), ImGuiComboFlags_HeightRegular))
            {
                // Search box
                ImGui::PushItemWidth(-FLT_MIN);
                ImGui::InputTextWithHint("##mesh_search", "Search...", search, IM_ARRAYSIZE(search));
                ImGui::PopItemWidth();
                ImGui::Separator();

                // Items: full-width stable rows + ellipsis + tooltip
                const float itemW = ImGui::GetContentRegionAvail().x;
                const float maxLabelW = itemW - ImGui::GetStyle().FramePadding.x * 2.0f;

                for (Mesh* mesh : mm.GetAllMeshes())
                {
                    if (!mesh) continue;

                    const std::string& fullName = mesh->GetName();
                    if (!matches(fullName)) continue;

                    ImGui::PushID(mesh);

                    bool selected = (mesh == c.mesh);

                    // Full-width selectable row (stable sizing)
                    bool clicked = ImGui::Selectable("##row", selected, 0, ImVec2(itemW, 0.0f));

                    // Draw ellipsized label on top
                    ImVec2 rmin = ImGui::GetItemRectMin();
                    ImVec2 rmax = ImGui::GetItemRectMax();
                    (void)rmax;

                    std::string labelEll = MakeEllipsisText(fullName, maxLabelW);

                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(rmin.x + ImGui::GetStyle().FramePadding.x,
                            rmin.y + ImGui::GetStyle().FramePadding.y),
                        ImGui::GetColorU32(ImGuiCol_Text),
                        labelEll.c_str()
                    );

                    // Tooltip when truncated
                    if (ImGui::IsItemHovered() && labelEll != fullName)
                        ImGui::SetTooltip("%s", fullName.c_str());

                    if (clicked)
                    {
                        c.mesh = mesh;
                        mm.SetMeshInstanceFilter(idComp.id, c.mesh);
                    }

                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }

            // Tooltip for preview when truncated
            if (ImGui::IsItemHovered() && previewEll != fullPreview)
                ImGui::SetTooltip("%s", fullPreview.c_str());

            ImGui::PopItemWidth();
        });
}

void EditorInspector::DrawTransformVec3(const std::string& label, glm::vec3& value, f32 reset_value)
{
    ImGui::PushID(label.c_str());

    if (ImGui::BeginTable("##Vec3Table", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(label.c_str());

        ImGui::TableSetColumnIndex(1);

        f32 cell_width = ImGui::GetContentRegionAvail().x;
        f32 spacing = ImGui::GetStyle().ItemSpacing.x;

        f32 button_size = ImGui::GetFrameHeight();
        f32 buttons_total_width = button_size * 3 + spacing * 3;

        f32 input_width = (cell_width - buttons_total_width) / 3.0f;
		input_width = std::max(input_width, 30.0f);

        f32 total_controls_width = (button_size + input_width + spacing) * 3;

        ImVec2 cell_pos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(cell_pos.x + cell_width - total_controls_width, cell_pos.y));

        auto DrawAxis = [&](const char* axis, f32& val, const ImVec4& color, int index)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

                if (ImGui::Button(axis, ImVec2(button_size, button_size)))
                    val = reset_value;

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                ImGui::PushItemWidth(input_width);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                std::string dragLabel = "##" + std::string(axis);
                bool changed = ImGui::DragFloat(dragLabel.c_str(), &val, 0.1f, 0.0f, 0.0f, "%.2f");
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();

                ImGui::SameLine();
            };

        DrawAxis("X", value.x, ImVec4(0.85f, 0.3f, 0.3f, 1.0f), 0);
        DrawAxis("Y", value.y, ImVec4(0.3f, 0.85f, 0.3f, 1.0f), 1);
        DrawAxis("Z", value.z, ImVec4(0.3f, 0.6f, 0.85f, 1.0f), 2);

        ImGui::NewLine();

        ImGui::EndTable();
    }

    ImGui::PopID();
}
