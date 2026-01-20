#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <algorithm> // remove/erase

// Internal
#include <hellengine/core/uuid/uuid.h>

namespace hellengine
{
    using namespace core;
    namespace ecs
    {
        constexpr u64 INVALID_ID = 0;

        struct HierarchyNode
        {
            UUID id;

            UUID parent_id;
            UUID first_child_id;
            UUID next_sibling_id;
            UUID previous_sibling_id;
            UUID last_child_id;

            HierarchyNode()
                : id(INVALID_ID)
                , parent_id(INVALID_ID)
                , first_child_id(INVALID_ID)
                , next_sibling_id(INVALID_ID)
                , previous_sibling_id(INVALID_ID)
                , last_child_id(INVALID_ID)
            {
            }
        };

        class SceneHierarchy
        {
        public:
            SceneHierarchy();
            ~SceneHierarchy();

            void Clear();

            HierarchyNode& CreateNode(UUID node_id);

            // Policy: CASCADE delete (deletes subtree nodes from hierarchy)
            void DestroyNode(UUID node_id);

            UUID GetParent(UUID node_id);
            UUID GetFirstChild(UUID node_id);
            UUID GetNextSibling(UUID node_id);
            UUID GetPreviousSibling(UUID node_id);
            UUID GetLastChild(UUID node_id);

            b8 Exists(UUID node_id) const;

            std::vector<UUID>& GetRootNodes() { return m_root_nodes; }
            const std::vector<UUID>& GetRootNodes() const { return m_root_nodes; }

            // Structural ops
            void AttachNode(UUID child_id, UUID parent_id);              // parent_id must exist
            void DetachNode(UUID child_id);                              // becomes root
            void ReparentNode(UUID child_id, UUID new_parent_id);        // new_parent_id can be INVALID_ID => root
            void InsertNodeBefore(UUID node_id, UUID sibling_id);
            void InsertNodeAfter(UUID node_id, UUID sibling_id);

            // Utility
            b8 IsDescendant(UUID node, UUID potential_ancestor) const;   // cycle check helper

        private:
            void ClearNodeLinks(UUID node_id);

            // Removes id from roots if present (no-op otherwise)
            void RemoveFromRoots(UUID id);

            // Adds id to roots if not present
            void AddToRoots(UUID id);

            // Detach from parent list WITHOUT adding to roots (used by Destroy/Reparent internal)
            void DetachFromParentOnly(UUID child_id);

        private:
            std::unordered_map<UUID, HierarchyNode> m_nodes;
            std::vector<UUID> m_root_nodes;
        };
    } // namespace ecs
} // namespace hellengine
