#include "editor_hierarchy.h"

EditorHierarchy::EditorHierarchy() : Panel("Hierarchy")
{
}

void EditorHierarchy::Init()
{
}

void EditorHierarchy::Draw()
{
	ImGui::Begin(m_name.c_str());

	ImGui::End();
}
