#include "hepch.h"
#include "scene.h"

// Internal
#include <hellengine/ecs/entity/entity.h>

namespace hellengine
{
    namespace ecs
    {
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
            // Clear hierarchy + lookup + registry
            m_hierarchy.Clear();
            m_entity_lookup.clear();
            m_registry.clear();
            m_name.clear();
        }

        Entity Scene::CreateEntity(const std::string& name)
        {
            // Always generate a valid non-zero UUID
            return CreateEntityWithUUID(UUID::Generate(), name);
        }

        Entity Scene::CreateEntityWithUUID(UUID id, const std::string& name)
        {
            // 0 is invalid sentinel for hierarchy
            if ((u64)id == (u64)INVALID_ID)
                id = UUID::Generate();

            HE_ASSERT((u64)id != (u64)INVALID_ID, "Scene::CreateEntityWithUUID produced INVALID_ID.");

            Entity entity = { m_registry.create(), this };

			entity.AddComponent<IDComponent>(id).id = id;

            auto& tag = entity.AddComponent<TagComponent>(name);
            tag.tag = name;

            entity.AddComponent<TransformComponent>();

            // Register in hierarchy as a root by default
            m_hierarchy.CreateNode(id);

            // Fast lookup
            m_entity_lookup[(u64)id] = entity.GetHandle();

            return entity;
        }

        void Scene::DestroyEntity(Entity entity)
        {
            if (!entity) return;

            UUID id = entity.GetComponent<IDComponent>().id;

            // Remove hierarchy node (CASCADE delete inside hierarchy)
            // NOTE: Scene::DestroyGameObject calls DestroyEntity bottom-up, so this will be leaf most times.
            m_hierarchy.DestroyNode(id);

            // Remove lookup
            m_entity_lookup.erase((u64)id);

            // Remove from registry
            m_registry.destroy(entity.GetHandle());
        }

        Entity Scene::CreateGameObject(const std::string& name, Entity parent)
        {
            HE_ASSERT(IsValid(parent) || parent == NULL_ENTITY, "Parent entity is not valid in the current scene");

            Entity entity = CreateEntity(name);
            UUID child_id = entity.GetComponent<IDComponent>().id;

            if (parent)
            {
                UUID parent_id = parent.GetComponent<IDComponent>().id;
                m_hierarchy.AttachNode(child_id, parent_id);
            }

            return entity;
        }

        void Scene::DestroyGameObject(Entity entity)
        {
            HE_ASSERT(IsValid(entity), "Entity is not valid in the current scene");

            UUID root_id = entity.GetComponent<IDComponent>().id;

            // Collect subtree UUIDs, then delete bottom-up
            std::vector<UUID> preorder;
            preorder.reserve(64);

            std::vector<UUID> stack;
            stack.push_back(root_id);

            while (!stack.empty())
            {
                UUID cur = stack.back();
                stack.pop_back();

                if (!m_hierarchy.Exists(cur))
                    continue;

                preorder.push_back(cur);

                // Push children
                UUID child = m_hierarchy.GetFirstChild(cur);
                while ((u64)child != (u64)INVALID_ID)
                {
                    stack.push_back(child);
                    child = m_hierarchy.GetNextSibling(child);
                }
            }

            for (auto it = preorder.rbegin(); it != preorder.rend(); ++it)
            {
                Entity e = GetEntity(*it);
                if (e) DestroyEntity(e);
            }
        }

        void Scene::ReparentGameObject(Entity child, Entity newParent)
        {
            if (!child || child == newParent)
                return;

            HE_ASSERT(IsValid(child), "Child entity is not valid in the current scene");
            HE_ASSERT(IsValid(newParent) || newParent == NULL_ENTITY, "New parent entity is not valid in the current scene");

            auto& childTc = child.GetComponent<TransformComponent>();
            glm::mat4 oldWorld = childTc.world_transform;

            UUID child_id = child.GetComponent<IDComponent>().id;

            // Hierarchy reparent
            if (!newParent)
            {
                // Move to root
                m_hierarchy.ReparentNode(child_id, UUID((u64)INVALID_ID));
            }
            else
            {
                UUID parent_id = newParent.GetComponent<IDComponent>().id;
                m_hierarchy.ReparentNode(child_id, parent_id);
            }

            // Keep world transform after reparent
            glm::mat4 parentWorld(1.0f);
            if (newParent)
                parentWorld = newParent.GetComponent<TransformComponent>().world_transform;

            glm::mat4 newLocal = glm::inverse(parentWorld) * oldWorld;

            glm::vec3 t, s, skew;
            glm::quat r;
            glm::vec4 persp;
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
                UUID      id;
                glm::mat4 parentWorld;
                bool      parentDirty;
            };

            std::vector<StackEntry> stack;
            stack.reserve(256);

            // Start from hierarchy roots
            for (UUID root : m_hierarchy.GetRootNodes())
            {
                if (!m_hierarchy.Exists(root)) continue;
                stack.push_back({ root, glm::mat4(1.0f), true });
            }

            while (!stack.empty())
            {
                StackEntry current = stack.back();
                stack.pop_back();

                Entity e = GetEntity(current.id);
                if (!e) continue;

                TransformComponent* tc = m_registry.try_get<TransformComponent>(e.GetHandle());

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

                // Traverse children through hierarchy sibling links
                UUID child = m_hierarchy.GetFirstChild(current.id);
                while ((u64)child != (u64)INVALID_ID)
                {
                    stack.push_back({ child, worldForChildren, dirtyHere });
                    child = m_hierarchy.GetNextSibling(child);
                }
            }
        }

        Entity Scene::GetEntity(UUID id)
        {
            auto it = m_entity_lookup.find((u64)id);
            if (it == m_entity_lookup.end())
                return Entity{ entt::null, this };

            entt::entity handle = it->second;
            if (!m_registry.valid(handle))
            {
                // heal stale entry
                m_entity_lookup.erase(it);
                return Entity{ entt::null, this };
            }

            return Entity{ handle, this };
        }

    } // namespace ecs
} // namespace hellengine
