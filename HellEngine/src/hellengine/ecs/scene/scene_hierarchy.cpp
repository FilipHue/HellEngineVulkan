#include "hepch.h"
#include "scene_hierarchy.h"

// Internal
#include <hellengine/core/logging/logger.h>

namespace hellengine
{
    namespace ecs
    {
        SceneHierarchy::SceneHierarchy() { NO_OP; }
        SceneHierarchy::~SceneHierarchy() { NO_OP; }

        void SceneHierarchy::Clear()
        {
            m_nodes.clear();
            m_root_nodes.clear();
        }

        b8 SceneHierarchy::Exists(UUID node_id) const
        {
            return m_nodes.find(node_id) != m_nodes.end();
        }

        void SceneHierarchy::RemoveFromRoots(UUID id)
        {
            auto& v = m_root_nodes;
            v.erase(std::remove(v.begin(), v.end(), id), v.end());
        }

        void SceneHierarchy::AddToRoots(UUID id)
        {
            if ((u64)id == INVALID_ID) return;
            if (std::find(m_root_nodes.begin(), m_root_nodes.end(), id) == m_root_nodes.end())
                m_root_nodes.push_back(id);
        }

        HierarchyNode& SceneHierarchy::CreateNode(UUID node_id)
        {
            HE_ASSERT((u64)node_id != INVALID_ID, "CreateNode: INVALID_ID (0) is not allowed.");
            HE_ASSERT(!Exists(node_id), "Node with ID {0} already exists.", (u64)node_id);

            HierarchyNode node{};
            node.id = node_id;

            auto [it, ok] = m_nodes.emplace(node_id, node);
            HE_ASSERT(ok, "CreateNode: failed to emplace node.");

            AddToRoots(node_id);
            return it->second;
        }

        UUID SceneHierarchy::GetParent(UUID node_id)
        {
            HE_ASSERT(Exists(node_id), "Node with ID {0} does not exist.", (u64)node_id);
            return m_nodes[node_id].parent_id;
        }

        UUID SceneHierarchy::GetFirstChild(UUID node_id)
        {
            HE_ASSERT(Exists(node_id), "Node with ID {0} does not exist.", (u64)node_id);
            return m_nodes[node_id].first_child_id;
        }

        UUID SceneHierarchy::GetNextSibling(UUID node_id)
        {
            HE_ASSERT(Exists(node_id), "Node with ID {0} does not exist.", (u64)node_id);
            return m_nodes[node_id].next_sibling_id;
        }

        UUID SceneHierarchy::GetPreviousSibling(UUID node_id)
        {
            HE_ASSERT(Exists(node_id), "Node with ID {0} does not exist.", (u64)node_id);
            return m_nodes[node_id].previous_sibling_id;
        }

        UUID SceneHierarchy::GetLastChild(UUID node_id)
        {
            HE_ASSERT(Exists(node_id), "Node with ID {0} does not exist.", (u64)node_id);
            return m_nodes[node_id].last_child_id;
        }

        void SceneHierarchy::ClearNodeLinks(UUID node_id)
        {
            HE_ASSERT(Exists(node_id), "Node with ID {0} does not exist.", (u64)node_id);

            auto& n = m_nodes[node_id];
            n.parent_id = INVALID_ID;
            n.first_child_id = INVALID_ID;
            n.next_sibling_id = INVALID_ID;
            n.previous_sibling_id = INVALID_ID;
            n.last_child_id = INVALID_ID;
        }

        // True if `node` is under `potential_ancestor`
        b8 SceneHierarchy::IsDescendant(UUID node, UUID potential_ancestor) const
        {
            if ((u64)node == INVALID_ID || (u64)potential_ancestor == INVALID_ID) return false;
            if (node == potential_ancestor) return true;

            auto it = m_nodes.find(node);
            if (it == m_nodes.end()) return false;

            UUID p = it->second.parent_id;
            while ((u64)p != INVALID_ID)
            {
                if (p == potential_ancestor) return true;
                auto pit = m_nodes.find(p);
                if (pit == m_nodes.end()) break;
                p = pit->second.parent_id;
            }
            return false;
        }

        // Detach from parent child list but DO NOT add to roots
        void SceneHierarchy::DetachFromParentOnly(UUID child_id)
        {
            auto& child = m_nodes[child_id];
            UUID parent_id = child.parent_id;
            if ((u64)parent_id == INVALID_ID) return;

            auto& parent = m_nodes[parent_id];

            if (parent.first_child_id == child_id)
                parent.first_child_id = child.next_sibling_id;

            if (parent.last_child_id == child_id)
                parent.last_child_id = child.previous_sibling_id;

            if ((u64)child.previous_sibling_id != INVALID_ID)
                m_nodes[child.previous_sibling_id].next_sibling_id = child.next_sibling_id;

            if ((u64)child.next_sibling_id != INVALID_ID)
                m_nodes[child.next_sibling_id].previous_sibling_id = child.previous_sibling_id;

            child.parent_id = INVALID_ID;
            child.previous_sibling_id = INVALID_ID;
            child.next_sibling_id = INVALID_ID;
        }

        void SceneHierarchy::AttachNode(UUID child_id, UUID parent_id)
        {
            HE_ASSERT((u64)child_id != INVALID_ID, "AttachNode: child_id is INVALID_ID.");
            HE_ASSERT((u64)parent_id != INVALID_ID, "AttachNode: parent_id is INVALID_ID.");
            HE_ASSERT(Exists(child_id), "Child node with ID {0} does not exist.", (u64)child_id);
            HE_ASSERT(Exists(parent_id), "Parent node with ID {0} does not exist.", (u64)parent_id);

            // cycle check: parent cannot be inside child's subtree
            HE_ASSERT(!IsDescendant(parent_id, child_id), "AttachNode: would create cycle.");

            // If child already has a parent, detach first (but do not leave it in roots)
            if ((u64)m_nodes[child_id].parent_id != INVALID_ID)
                DetachFromParentOnly(child_id);
            else
                RemoveFromRoots(child_id); // if it was root, stop being root

            auto& child = m_nodes[child_id];
            auto& parent = m_nodes[parent_id];

            // Reset sibling links before inserting
            child.previous_sibling_id = INVALID_ID;
            child.next_sibling_id = INVALID_ID;

            child.parent_id = parent_id;

            if ((u64)parent.first_child_id == INVALID_ID)
            {
                parent.first_child_id = child_id;
                parent.last_child_id = child_id;
                return;
            }

            UUID last = parent.last_child_id;
            m_nodes[last].next_sibling_id = child_id;
            child.previous_sibling_id = last;
            parent.last_child_id = child_id;
        }

        void SceneHierarchy::DetachNode(UUID child_id)
        {
            HE_ASSERT((u64)child_id != INVALID_ID, "DetachNode: child_id is INVALID_ID.");
            HE_ASSERT(Exists(child_id), "Child node with ID {0} does not exist.", (u64)child_id);

            auto& child = m_nodes[child_id];
            if ((u64)child.parent_id == INVALID_ID)
            {
                // already root; ensure it is actually present in roots
                AddToRoots(child_id);
                return;
            }

            DetachFromParentOnly(child_id);
            AddToRoots(child_id);
        }

        void SceneHierarchy::ReparentNode(UUID child_id, UUID new_parent_id)
        {
            HE_ASSERT((u64)child_id != INVALID_ID, "ReparentNode: child_id is INVALID_ID.");
            HE_ASSERT(Exists(child_id), "Child node with ID {0} does not exist.", (u64)child_id);

            // Reparent to root
            if ((u64)new_parent_id == INVALID_ID)
            {
                DetachNode(child_id);
                return;
            }

            HE_ASSERT(Exists(new_parent_id), "New parent node with ID {0} does not exist.", (u64)new_parent_id);
            HE_ASSERT(!IsDescendant(new_parent_id, child_id), "ReparentNode: would create cycle.");

            // If currently root, remove from roots now (before attaching)
            RemoveFromRoots(child_id);

            // detach from old parent if needed, then attach
            if ((u64)m_nodes[child_id].parent_id != INVALID_ID)
                DetachFromParentOnly(child_id);

            AttachNode(child_id, new_parent_id);
        }

        void SceneHierarchy::InsertNodeBefore(UUID node_id, UUID sibling_id)
        {
            HE_ASSERT(Exists(node_id) && Exists(sibling_id), "InsertNodeBefore: invalid ids.");
            if (node_id == sibling_id) return;

            auto& node = m_nodes[node_id];
            auto& sib = m_nodes[sibling_id];

            HE_ASSERT(node.parent_id == sib.parent_id, "InsertNodeBefore: nodes must share the same parent.");

            UUID parent_id = node.parent_id;

            // Detach node from its current position in parent's list (but don't add to roots)
            if ((u64)parent_id != INVALID_ID)
                DetachFromParentOnly(node_id);
            else
                RemoveFromRoots(node_id); // reorder roots list requires a different structure; skip for now

            // Insert before sibling in parent's list
            auto& parent = m_nodes[parent_id];

            UUID prev = sib.previous_sibling_id;

            node.parent_id = parent_id;
            node.next_sibling_id = sibling_id;
            node.previous_sibling_id = prev;
            sib.previous_sibling_id = node_id;

            if ((u64)prev != INVALID_ID)
                m_nodes[prev].next_sibling_id = node_id;
            else
                parent.first_child_id = node_id;
        }

        void SceneHierarchy::InsertNodeAfter(UUID node_id, UUID sibling_id)
        {
            HE_ASSERT(Exists(node_id) && Exists(sibling_id), "InsertNodeAfter: invalid ids.");
            if (node_id == sibling_id) return;

            auto& node = m_nodes[node_id];
            auto& sib = m_nodes[sibling_id];

            HE_ASSERT(node.parent_id == sib.parent_id, "InsertNodeAfter: nodes must share the same parent.");

            UUID parent_id = node.parent_id;

            if ((u64)parent_id != INVALID_ID)
                DetachFromParentOnly(node_id);
            else
                RemoveFromRoots(node_id); // root reorder not supported with vector roots

            auto& parent = m_nodes[parent_id];

            UUID next = sib.next_sibling_id;

            node.parent_id = parent_id;
            node.previous_sibling_id = sibling_id;
            node.next_sibling_id = next;
            sib.next_sibling_id = node_id;

            if ((u64)next != INVALID_ID)
                m_nodes[next].previous_sibling_id = node_id;
            else
                parent.last_child_id = node_id;
        }

        // CASCADE delete subtree from hierarchy and clean roots/links
        void SceneHierarchy::DestroyNode(UUID node_id)
        {
            if (!Exists(node_id)) return;

            // Collect subtree nodes (DFS)
            std::vector<UUID> stack;
            std::vector<UUID> order;
            stack.push_back(node_id);

            while (!stack.empty())
            {
                UUID cur = stack.back();
                stack.pop_back();

                if (!Exists(cur)) continue;
                order.push_back(cur);

                UUID c = m_nodes[cur].first_child_id;
                while ((u64)c != INVALID_ID)
                {
                    stack.push_back(c);
                    c = m_nodes[c].next_sibling_id;
                }
            }

            // Delete bottom-up
            for (auto it = order.rbegin(); it != order.rend(); ++it)
            {
                UUID cur = *it;
                if (!Exists(cur)) continue;

                // Remove from roots if it is there
                RemoveFromRoots(cur);

                // Detach from parent if needed (no re-rooting)
                if ((u64)m_nodes[cur].parent_id != INVALID_ID)
                    DetachFromParentOnly(cur);

                // Clear links then erase
                ClearNodeLinks(cur);
                m_nodes.erase(cur);
            }
        }

    } // namespace ecs
} // namespace hellengine
