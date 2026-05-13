#pragma once

// Internal
#include "types.h"
#include "../backend/vulkan_backend.h"

namespace hellengine
{
    namespace graphics
    {
        /**
         * @brief RenderGraph - Declarative frame graph for dynamic rendering
         * 
         * Manages render passes, resources, and their dependencies automatically.
         * Handles resource lifetime, layout transitions, and execution ordering.
         * 
         * Usage:
         *   RenderGraph graph;
         *   graph.AddResource(...);
         *   graph.AddPass(...);
         *   graph.Build();
         *   graph.Execute(backend);
         */
        class RenderGraph
        {
        public:
            RenderGraph();
            ~RenderGraph();

            /******************\
            | RESOURCE MGMT   |
            \******************/

            /**
             * @brief Declare a render target resource
             * @param descriptor Resource specification
             * @return Resource name for referencing in passes
             */
            std::string AddResource(const ResourceDescriptor& descriptor);

            /**
             * @brief Import an external resource (e.g., swapchain image, existing texture)
             * @param name Unique name for the resource
             * @param image Vulkan image handle
             * @param image_view Vulkan image view handle
             * @param format Image format
             * @param extent Image dimensions
             * @param usage Primary usage
             */
            void ImportResource(const std::string& name,
                              VkImage image,
                              VkImageView image_view,
                              VkFormat format,
                              VkExtent2D extent,
                              ResourceUsage usage);

            /**
             * @brief Get a resource by name (for reading outputs)
             */
            RenderResource* GetResource(const std::string& name);

            /******************\
            | PASS MANAGEMENT |
            \******************/

            /**
             * @brief Add a render pass to the graph
             * @param descriptor Pass specification with resource bindings and execution callback
             * @return Pass name
             */
            std::string AddPass(const RenderPassDescriptor& descriptor);

            /**
             * @brief Remove a pass from the graph
             */
            void RemovePass(const std::string& name);

            /**
             * @brief Enable/disable a pass without removing it
             */
            void SetPassEnabled(const std::string& name, b8 enabled);

            /******************\
            | GRAPH BUILDING  |
            \******************/

            /**
             * @brief Build the render graph (resolves dependencies, orders passes)
             * @return Build result status
             */
            GraphBuildResult Build();

            /**
             * @brief Check if graph is built and ready to execute
             */
            b8 IsBuilt() const { return m_is_built; }

            /**
             * @brief Rebuild the graph (call after adding/removing passes)
             */
            void Rebuild() { m_is_built = false; Build(); }

            /******************\
            | EXECUTION       |
            \******************/

            /**
             * @brief Execute all passes in dependency order
             * @param backend Vulkan backend for rendering commands
             */
            void Execute(VulkanBackend* backend);

            /**
             * @brief Clear all passes and resources
             */
            void Clear();

            /******************\
            | RESIZE HANDLING |
            \******************/

            /**
             * @brief Resize all non-imported resources
             * @param width New width
             * @param height New height
             */
            void Resize(u32 width, u32 height);

            /******************\
            | INTROSPECTION   |
            \******************/

            /**
             * @brief Get ordered list of pass names (post-build)
             */
            const std::vector<std::string>& GetExecutionOrder() const { return m_execution_order; }

            /**
             * @brief Get all registered resource names
             */
            std::vector<std::string> GetResourceNames() const;

            /**
             * @brief Get all registered pass names
             */
            std::vector<std::string> GetPassNames() const;

            /**
             * @brief Debug: Print graph structure
             */
            void PrintDebugInfo() const;

        private:
            /******************\
            | INTERNAL HELPERS|
            \******************/

            /**
             * @brief Topologically sort passes based on dependencies
             */
            b8 TopologicalSort();

            /**
             * @brief Create physical resources from descriptors
             */
            void CreatePhysicalResources(VulkanBackend* backend);

            /**
             * @brief Destroy physical resources
             */
            void DestroyPhysicalResources(VulkanBackend* backend);

            /**
             * @brief Setup image layout transitions between passes
             */
            void SetupLayoutTransitions();

            /**
             * @brief Validate resource bindings
             */
            b8 ValidateResourceBindings() const;

            /**
             * @brief Build DynamicRenderingInfo for a pass
             */
            DynamicRenderingInfo BuildDynamicRenderingInfo(const RenderPassDescriptor& pass);

        private:
            // Resources
            std::unordered_map<std::string, RenderResource> m_resources;
            std::unordered_map<std::string, ResourceDescriptor> m_resource_descriptors;

            // Passes
            std::unordered_map<std::string, RenderPassDescriptor> m_passes;
            std::vector<std::string> m_execution_order;

            // Graph state
            b8 m_is_built{ false };
            u32 m_frame_width{ 0 };
            u32 m_frame_height{ 0 };

            // Backend reference (stored during execution)
            VulkanBackend* m_current_backend{ nullptr };
        };

    } // namespace graphics

} // namespace hellengine
