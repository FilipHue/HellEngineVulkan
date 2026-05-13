#include "hepch.h"
#include "render_graph.h"
#include "../backend/vulkan_backend.h"

// STL
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace hellengine
{
    namespace graphics
    {
        RenderGraph::RenderGraph()
        {
            HE_GRAPHICS_INFO("RenderGraph created");
        }

        RenderGraph::~RenderGraph()
        {
            Clear();
        }

        /******************\
        | RESOURCE MGMT   |
        \******************/

        std::string RenderGraph::AddResource(const ResourceDescriptor& descriptor)
        {
            if (m_resource_descriptors.find(descriptor.name) != m_resource_descriptors.end())
            {
                HE_GRAPHICS_WARN("Resource '{}' already exists, replacing...", descriptor.name);
            }

            m_resource_descriptors[descriptor.name] = descriptor;
            m_is_built = false;

            HE_GRAPHICS_TRACE("Added resource: {}", descriptor.name);
            return descriptor.name;
        }

        void RenderGraph::ImportResource(const std::string& name,
                                         VkImage image,
                                         VkImageView image_view,
                                         VkFormat format,
                                         VkExtent2D extent,
                                         ResourceUsage usage)
        {
            RenderResource resource{};
            resource.name = name;
            resource.type = ResourceType::Texture2D;
            resource.primary_usage = usage;
            resource.image = image;
            resource.image_view = image_view;
            resource.format = format;
            resource.extent = extent;
            resource.is_imported = true;

            m_resources[name] = resource;
            m_is_built = false;

            HE_GRAPHICS_TRACE("Imported resource: {}", name);
        }

        RenderResource* RenderGraph::GetResource(const std::string& name)
        {
            auto it = m_resources.find(name);
            if (it != m_resources.end())
            {
                return &it->second;
            }
            return nullptr;
        }

        /******************\
        | PASS MANAGEMENT |
        \******************/

        std::string RenderGraph::AddPass(const RenderPassDescriptor& descriptor)
        {
            if (m_passes.find(descriptor.name) != m_passes.end())
            {
                HE_GRAPHICS_WARN("Pass '{}' already exists, replacing...", descriptor.name);
            }

            m_passes[descriptor.name] = descriptor;
            m_is_built = false;

            HE_GRAPHICS_TRACE("Added pass: {}", descriptor.name);
            return descriptor.name;
        }

        void RenderGraph::RemovePass(const std::string& name)
        {
            auto it = m_passes.find(name);
            if (it != m_passes.end())
            {
                m_passes.erase(it);
                m_is_built = false;
                HE_GRAPHICS_TRACE("Removed pass: {}", name);
            }
            else
            {
                HE_GRAPHICS_WARN("Cannot remove pass '{}': not found", name);
            }
        }

        void RenderGraph::SetPassEnabled(const std::string& name, b8 enabled)
        {
            auto it = m_passes.find(name);
            if (it != m_passes.end())
            {
                it->second.enabled = enabled;
                m_is_built = false;
                HE_GRAPHICS_TRACE("Pass '{}' {}", name, enabled ? "enabled" : "disabled");
            }
        }

        /******************\
        | GRAPH BUILDING  |
        \******************/

        GraphBuildResult RenderGraph::Build()
        {
            if (m_is_built)
            {
                return GraphBuildResult::Success;
            }

            HE_GRAPHICS_INFO("Building render graph...");

            // Validate resource bindings
            if (!ValidateResourceBindings())
            {
                HE_GRAPHICS_ERROR("Resource validation failed");
                return GraphBuildResult::MissingResource;
            }

            // Topologically sort passes
            if (!TopologicalSort())
            {
                HE_GRAPHICS_ERROR("Topological sort failed - cyclic dependency detected");
                return GraphBuildResult::CyclicDependency;
            }

            // Setup layout transitions (could be done here or during execution)
            SetupLayoutTransitions();

            m_is_built = true;
            HE_GRAPHICS_INFO("Render graph built successfully with {} passes", m_execution_order.size());

            return GraphBuildResult::Success;
        }

        /******************\
        | EXECUTION       |
        \******************/

        void RenderGraph::Execute(VulkanBackend* backend)
        {
            if (!m_is_built)
            {
                HE_GRAPHICS_ERROR("Cannot execute render graph: not built");
                return;
            }

            m_current_backend = backend;

            // Create/update physical resources if needed
            if (m_resources.empty() && !m_resource_descriptors.empty())
            {
                CreatePhysicalResources(backend);
            }

            // Execute passes in order
            for (const auto& pass_name : m_execution_order)
            {
                auto& pass = m_passes[pass_name];

                if (!pass.enabled)
                {
                    continue;
                }

                // Build dynamic rendering info
                DynamicRenderingInfo dri = BuildDynamicRenderingInfo(pass);

                // Begin rendering
                backend->BeginDynamicRenderingWithAttachments(dri);

                // Set viewport and scissor
                VkExtent2D viewport_extent = pass.viewport_override.value_or(dri.extent);
                backend->SetViewport({ { 0.0f, 0.0f, (f32)viewport_extent.width, (f32)viewport_extent.height, 0.0f, 1.0f } });
                backend->SetScissor({ { { 0, 0 }, viewport_extent } });

                // Execute user callback
                if (pass.execute)
                {
                    pass.execute(backend, pass);
                }

                // End rendering
                backend->EndDynamicRenderingWithAttachments(dri);
            }

            m_current_backend = nullptr;
        }

        void RenderGraph::Clear()
        {
            if (m_current_backend)
            {
                DestroyPhysicalResources(m_current_backend);
            }

            m_passes.clear();
            m_resources.clear();
            m_resource_descriptors.clear();
            m_execution_order.clear();
            m_is_built = false;

            HE_GRAPHICS_TRACE("Render graph cleared");
        }

        /******************\
        | RESIZE HANDLING |
        \******************/

        void RenderGraph::Resize(u32 width, u32 height)
        {
            if (m_frame_width == width && m_frame_height == height)
            {
                return;
            }

            m_frame_width = width;
            m_frame_height = height;

            // Recreate non-imported resources
            if (m_current_backend)
            {
                DestroyPhysicalResources(m_current_backend);
                CreatePhysicalResources(m_current_backend);
            }

            HE_GRAPHICS_TRACE("Render graph resized to {}x{}", width, height);
        }

        /******************\
        | INTROSPECTION   |
        \******************/

        std::vector<std::string> RenderGraph::GetResourceNames() const
        {
            std::vector<std::string> names;
            for (const auto& [name, _] : m_resource_descriptors)
            {
                names.push_back(name);
            }
            return names;
        }

        std::vector<std::string> RenderGraph::GetPassNames() const
        {
            std::vector<std::string> names;
            for (const auto& [name, _] : m_passes)
            {
                names.push_back(name);
            }
            return names;
        }

        void RenderGraph::PrintDebugInfo() const
        {
            HE_GRAPHICS_INFO("=== Render Graph Debug Info ===");
            HE_GRAPHICS_INFO("Resources: {}", m_resource_descriptors.size());
            for (const auto& [name, desc] : m_resource_descriptors)
            {
                HE_GRAPHICS_INFO("  - {} ({}x{}, format: {})", name, desc.width, desc.height, (u32)desc.format);
            }

            HE_GRAPHICS_INFO("Passes: {}", m_passes.size());
            for (const auto& pass_name : m_execution_order)
            {
                const auto& pass = m_passes.at(pass_name);
                HE_GRAPHICS_INFO("  [{}] {} (enabled: {})", pass.execution_order, pass_name, pass.enabled);
                HE_GRAPHICS_INFO("    Color attachments: {}", pass.color_attachments.size());
                HE_GRAPHICS_INFO("    Has depth: {}", pass.depth_attachment.has_value());
            }
        }

        /******************\
        | INTERNAL HELPERS|
        \******************/

        b8 RenderGraph::TopologicalSort()
        {
            // Build adjacency list and in-degree count
            std::unordered_map<std::string, std::vector<std::string>> adj_list;
            std::unordered_map<std::string, u32> in_degree;

            // Initialize
            for (const auto& [name, pass] : m_passes)
            {
                if (!pass.enabled) continue;
                in_degree[name] = 0;
                adj_list[name] = {};
            }

            // Build graph
            for (const auto& [name, pass] : m_passes)
            {
                if (!pass.enabled) continue;

                for (const auto& dep : pass.dependencies)
                {
                    if (m_passes.find(dep.pass_name) == m_passes.end())
                    {
                        HE_GRAPHICS_ERROR("Pass '{}' depends on non-existent pass '{}'", name, dep.pass_name);
                        return false;
                    }

                    adj_list[dep.pass_name].push_back(name);
                    in_degree[name]++;
                }
            }

            // Kahn's algorithm
            std::queue<std::string> queue;
            for (const auto& [name, degree] : in_degree)
            {
                if (degree == 0)
                {
                    queue.push(name);
                }
            }

            m_execution_order.clear();
            while (!queue.empty())
            {
                std::string current = queue.front();
                queue.pop();
                m_execution_order.push_back(current);

                for (const auto& neighbor : adj_list[current])
                {
                    in_degree[neighbor]--;
                    if (in_degree[neighbor] == 0)
                    {
                        queue.push(neighbor);
                    }
                }
            }

            // Check for cycles
            if (m_execution_order.size() != in_degree.size())
            {
                HE_GRAPHICS_ERROR("Cyclic dependency detected in render graph");
                return false;
            }

            // Sort by execution order if specified
            std::stable_sort(m_execution_order.begin(), m_execution_order.end(),
                [this](const std::string& a, const std::string& b) {
                    return m_passes.at(a).execution_order < m_passes.at(b).execution_order;
                });

            return true;
        }

        void RenderGraph::CreatePhysicalResources(VulkanBackend* backend)
        {
            // Note: Actual resource creation would require VulkanFrontend
            // For now, we track descriptors and expect resources to be imported or created externally
            HE_GRAPHICS_TRACE("Physical resources creation requested (requires frontend integration)");
        }

        void RenderGraph::DestroyPhysicalResources(VulkanBackend* backend)
        {
            // Destroy non-imported resources
            for (auto& [name, resource] : m_resources)
            {
                if (!resource.is_imported)
                {
                    // Actual destruction would happen here
                    HE_GRAPHICS_TRACE("Destroying resource: {}", name);
                }
            }
        }

        void RenderGraph::SetupLayoutTransitions()
        {
            // Track resource states across passes
            // This would insert pipeline barriers between passes as needed
            // For now, we rely on DynamicRenderingAttachmentInfo layout specifications
        }

        b8 RenderGraph::ValidateResourceBindings() const
        {
            for (const auto& [pass_name, pass] : m_passes)
            {
                if (!pass.enabled) continue;

                // Check color attachments
                for (const auto& attachment : pass.color_attachments)
                {
                    if (m_resource_descriptors.find(attachment.resource_name) == m_resource_descriptors.end() &&
                        m_resources.find(attachment.resource_name) == m_resources.end())
                    {
                        HE_GRAPHICS_ERROR("Pass '{}' references non-existent resource '{}'", pass_name, attachment.resource_name);
                        return false;
                    }
                }

                // Check depth attachment
                if (pass.depth_attachment.has_value())
                {
                    const auto& attachment = pass.depth_attachment.value();
                    if (m_resource_descriptors.find(attachment.resource_name) == m_resource_descriptors.end() &&
                        m_resources.find(attachment.resource_name) == m_resources.end())
                    {
                        HE_GRAPHICS_ERROR("Pass '{}' references non-existent depth resource '{}'", pass_name, attachment.resource_name);
                        return false;
                    }
                }
            }

            return true;
        }

        DynamicRenderingInfo RenderGraph::BuildDynamicRenderingInfo(const RenderPassDescriptor& pass)
        {
            DynamicRenderingInfo dri{};

            // Color attachments
            for (const auto& attachment_binding : pass.color_attachments)
            {
                auto* resource = GetResource(attachment_binding.resource_name);
                if (!resource)
                {
                    HE_GRAPHICS_ERROR("Resource '{}' not found", attachment_binding.resource_name);
                    continue;
                }

                DynamicRenderingAttachmentInfo attachment{};
                attachment.image = resource->image;
                attachment.image_view = resource->image_view;
                attachment.format = resource->format;
                attachment.image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.load_op = attachment_binding.load_op;
                attachment.store_op = attachment_binding.store_op;
                attachment.clear_value = resource->clear_value;
                attachment.initial_layout = attachment_binding.initial_layout;
                attachment.final_layout = attachment_binding.final_layout;

                dri.color_attachments.push_back(attachment);
            }

            // Depth attachment
            if (pass.depth_attachment.has_value())
            {
                const auto& attachment_binding = pass.depth_attachment.value();
                auto* resource = GetResource(attachment_binding.resource_name);
                if (resource)
                {
                    DynamicRenderingAttachmentInfo attachment{};
                    attachment.image = resource->image;
                    attachment.image_view = resource->image_view;
                    attachment.format = resource->format;
                    attachment.image_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    attachment.load_op = attachment_binding.load_op;
                    attachment.store_op = attachment_binding.store_op;
                    attachment.clear_value = resource->clear_value;
                    attachment.initial_layout = attachment_binding.initial_layout;
                    attachment.final_layout = attachment_binding.final_layout;

                    dri.depth_attachment = attachment;
                }
            }

            // Extent from first color attachment or pass override
            if (!dri.color_attachments.empty())
            {
                auto* resource = GetResource(pass.color_attachments[0].resource_name);
                dri.extent = resource->extent;
            }

            return dri;
        }

    } // namespace graphics

} // namespace hellengine
