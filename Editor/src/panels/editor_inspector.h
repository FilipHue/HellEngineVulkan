#pragma once

// Internal
#include "hellengine/hellengine.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;

class EditorInspector : public Panel
{
public:
	EditorInspector();
	virtual ~EditorInspector() = default;

	void Init();

	b8 Begin() override;
	void Draw() override;
	void End() override;

	Entity GetSelectedEntity();
	void SetSelectedEntity(Entity entity);

private:
	void DrawEntityComponents();

	template<typename ComponentT, typename UIFunctionT>
	static void DrawComponent(const std::string& name, Entity entity, UIFunctionT ui_function)
	{
		if (entity == Entity{})
		{
			return;
		}

		ImGui::PushID(name.c_str());

		if (entity.HasComponent<ComponentT>())
		{
			auto& component = entity.GetComponent<ComponentT>();
			
			ImGui::Separator();

			ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
			b8 open = ImGui::TreeNodeEx("##Component", tree_node_flags, name.c_str());

			if (open) {
				ui_function(component);

				ImGui::TreePop();
			}
		}

		ImGui::PopID();
	}

	static void DrawTransformVec3(const std::string& label, glm::vec3& value, f32 reset_value = 0.0f);

private:
	Entity m_selected_entity;
};