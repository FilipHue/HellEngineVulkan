#include "hepch.h"
#include "scene.h"

// Internal
#include <hellengine/ecs/entity/entity.h>
#include <hellengine/graphics/managers/mesh_manager.h>

namespace hellengine
{

	namespace ecs
	{

		static entt::entity GetFirstChild(const RelationshipComponent& rc) { return rc.first; }
		static entt::entity GetNextSibling(const RelationshipComponent& rc) { return rc.next; }
		static bool         IsRoot(const RelationshipComponent& rc) { return rc.parent == entt::null; }

		Scene::Scene()
		{
			NO_OP;
		}

		Scene::~Scene()
		{
			NO_OP;
		}

		void Scene::Create(std::string& name)
		{
			m_name = name;
			m_uuid = UUID::Generate();
		}

		void Scene::Destroy()
		{
			m_registry.clear();
			m_name.clear();
		}

		Entity Scene::CreateEntity(const std::string& name)
		{
			return CreateEntityWithUUID(UUID(), name);
		}

		Entity Scene::CreateEntityWithUUID(UUID id, const std::string& name)
		{
			Entity entity = { m_registry.create(), this };

			entity.AddComponent<IDComponent>(id);
			entity.AddComponent<RelationshipComponent>();
			auto& tag = entity.AddComponent<TagComponent>(name);
			tag.tag = name;
			entity.AddComponent<TransformComponent>();

			return entity;
		}

		void Scene::DestroyEntity(Entity entity)
		{
			m_registry.destroy(entity.GetHandle());
		}

		Entity Scene::CreateGameObject(const std::string& name, Entity parent)
		{
			HE_ASSERT(IsValid(parent) || parent == NULL_ENTITY, "Parent entity is not valid in the current scene");

			Entity entity = CreateEntity(name);
			auto& childRel = entity.GetComponent<RelationshipComponent>();

			if (parent.GetHandle() == entt::null)
			{
				return entity;
			}

			childRel.parent = parent.GetHandle();

			auto& parentRel = parent.GetComponent<RelationshipComponent>();

			if (parentRel.first == entt::null)
			{
				parentRel.first = entity.GetHandle();
				parentRel.last = entity.GetHandle();
				parentRel.child_count = 1;
			}
			else
			{
				entt::entity last = parentRel.last;
				auto& lastRel = m_registry.get<RelationshipComponent>(last);

				lastRel.next = entity.GetHandle();
				parentRel.last = entity.GetHandle();
				childRel.prev = last;

				++parentRel.child_count;
			}

			return entity;
		}

		void Scene::DestroyGameObject(Entity entity)
		{
			HE_ASSERT(IsValid(entity), "Entity is not valid in the current scene");

			auto& relationship = entity.GetComponent<RelationshipComponent>();
			entt::entity child = relationship.first;
			while (child != entt::null)
			{
				Entity child_entity(child, this);
				child = child_entity.GetComponent<RelationshipComponent>().next;
				DestroyGameObject(child_entity);
			}
			if (relationship.parent != entt::null)
			{
				Entity parent_entity(relationship.parent, this);
				auto& parent_rel = parent_entity.GetComponent<RelationshipComponent>();
				if (parent_rel.first == entity.GetHandle())
				{
					parent_rel.first = relationship.next;
				}
				else if (relationship.next == entt::null)
				{
					auto& prev_rel = m_registry.get<RelationshipComponent>(relationship.prev);
					prev_rel.next = entt::null;
				}
				else
				{
					entt::entity current = parent_rel.first;
					while (current != entt::null)
					{
						auto& curr_rel = m_registry.get<RelationshipComponent>(current);
						if (curr_rel.next == entity.GetHandle())
						{
							curr_rel.next = relationship.next;
							break;
						}
						current = curr_rel.next;
					}
				}
				parent_rel.child_count = std::max(0u, parent_rel.child_count - 1);
			}

			DestroyEntity(entity);
		}

		void Scene::ReparentGameObject(Entity child, Entity newParent)
		{
			if (!child || child == newParent)
			{
				return;
			}

			HE_ASSERT(IsValid(child), "Child entity is not valid in the current scene");
			HE_ASSERT(IsValid(newParent) || newParent == NULL_ENTITY, "New parent entity is not valid in the current scene");

			auto& childTc = child.GetComponent<TransformComponent>();
			glm::mat4 oldWorld = childTc.world_transform;

			auto& childRel = child.GetComponent<RelationshipComponent>();
			if (childRel.parent != entt::null)
			{
				Entity oldParent(childRel.parent, this);
				auto& opRel = oldParent.GetComponent<RelationshipComponent>();

				if (opRel.first == child.GetHandle())
				{
					opRel.first = childRel.next;
				}
				else
				{
					entt::entity cur = opRel.first;
					while (cur != entt::null)
					{
						auto& curRel = m_registry.get<RelationshipComponent>(cur);
						if (curRel.next == child.GetHandle())
						{
							curRel.next = childRel.next;
							break;
						}
						cur = curRel.next;
					}
				}
				--opRel.child_count;
			}

			childRel.parent = newParent.GetHandle();
			childRel.next = entt::null;

			if (newParent)
			{
				auto& pRel = newParent.GetComponent<RelationshipComponent>();

				childRel.next = pRel.first;
				pRel.first = child.GetHandle();
				++pRel.child_count;
			}

			glm::mat4 parentWorld(1.0f);
			if (newParent)
			{
				parentWorld = newParent.GetComponent<TransformComponent>().world_transform;
			}

			glm::mat4 newLocal = glm::inverse(parentWorld) * oldWorld;

			glm::vec3 t, s, skew;  glm::quat r;  glm::vec4 persp;
			glm::decompose(newLocal, s, r, t, skew, persp);

			childTc.local_position = t;
			childTc.local_rotation = glm::eulerAngles(r);
			childTc.local_scale = s;
			childTc.is_dirty = true;
		}

		b8 Scene::IsValid(Entity entity) const
		{
			return m_registry.valid(entity.GetHandle());
		}

		void Scene::UpdateTransforms()
		{
			struct StackEntry
			{
				entt::entity e;
				glm::mat4    parentWorld;
				bool         parentDirty;
			};

			std::vector<StackEntry> stack;
			stack.reserve(256);

			m_registry.view<RelationshipComponent>().each(
				[&](entt::entity entity, const RelationshipComponent& rel)
				{
					if (rel.parent == entt::null)
						stack.push_back({ entity, glm::mat4(1.0f), true });
				});

			while (!stack.empty())
			{
				StackEntry current = stack.back();
				stack.pop_back();

				auto& rel = m_registry.get<RelationshipComponent>(current.e);

				TransformComponent* tc = m_registry.try_get<TransformComponent>(current.e);

				bool dirtyHere = current.parentDirty;
				if (tc)
				{
					dirtyHere |= tc->is_dirty;
					if (dirtyHere)
					{
						tc->world_transform = current.parentWorld * tc->GetLocalTransform();
						tc->is_dirty = false;
					}
				}

				glm::mat4 worldForChildren = tc ? tc->world_transform : current.parentWorld;

				entt::entity child = rel.first;
				while (child != entt::null)
				{
					const auto& childRel = m_registry.get<RelationshipComponent>(child);
					stack.push_back({ child, worldForChildren, dirtyHere });
					child = childRel.next;
				}
			}
		}

		Entity Scene::GetEntity(std::string name)
		{
			auto view = m_registry.view<TagComponent>();
			for (auto entity : view)
			{
				auto& tag = view.get<TagComponent>(entity);
				if (tag.tag == name)
				{
					return Entity(entity, this);
				}
			}
			return NULL_ENTITY;
		}

	} // namespace ecs

} // namespace hellengine