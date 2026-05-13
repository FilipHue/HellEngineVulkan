#pragma once

// STL
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>

// Internal
#include "../objects/types.h"

namespace hellengine
{
    namespace graphics
    {
        // Forward declarations
        class RenderGraph;
        struct RenderResource;
        class VulkanBackend;

        /******************\
        | RESOURCE TYPES  |
        \******************/

        enum class ResourceType
        {
            Texture2D,
            TextureCube,
            Buffer,
            UniformBuffer
        };

        enum class ResourceUsage
        {
            ColorAttachment,
            DepthAttachment,
            Sampled,
            Storage,
            TransferSrc,
            TransferDst
        };

        struct ResourceDescriptor
        {
            std::string name;
            ResourceType type;
            u32 width{};
            u32 height{};
            VkFormat format{};
            std::vector<ResourceUsage> usages;
            VkClearValue clear_value{};

            // For buffers
            u64 buffer_size{};

            // Transient resources are temporary and can be aliased
            b8 transient{ false };
        };

        struct RenderResource
        {
            std::string name;
            ResourceType type;
            ResourceUsage primary_usage;

            // Vulkan handles
            VkImage image{ VK_NULL_HANDLE };
            VkImageView image_view{ VK_NULL_HANDLE };
            VkBuffer buffer{ VK_NULL_HANDLE };
            VkFormat format{};
            VkExtent2D extent{};

            // Metadata
            b8 is_imported{ false };      // External resource (e.g., swapchain)
            b8 is_transient{ false };     // Can be reused/aliased
            u32 last_used_pass{ 0 };      // For resource lifetime tracking

            // Clear value
            VkClearValue clear_value{};
        };

        /*****************\
        | RENDER PASS    |
        \*****************/

        struct PassDependency
        {
            std::string pass_name;
            VkPipelineStageFlags stage_mask;
            VkAccessFlags access_mask;
        };

        struct PassResourceBinding
        {
            std::string resource_name;
            ResourceUsage usage;
            VkAttachmentLoadOp load_op{ VK_ATTACHMENT_LOAD_OP_DONT_CARE };
            VkAttachmentStoreOp store_op{ VK_ATTACHMENT_STORE_OP_STORE };
            VkImageLayout initial_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
            VkImageLayout final_layout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        };

        struct RenderPassDescriptor
        {
            std::string name;
            std::vector<PassResourceBinding> color_attachments;
            std::optional<PassResourceBinding> depth_attachment;
            std::vector<PassResourceBinding> input_attachments;
            std::vector<PassDependency> dependencies;

            // Execution callback
            std::function<void(VulkanBackend*, const RenderPassDescriptor&)> execute;

            // Viewport dimensions (if different from attachments)
            std::optional<VkExtent2D> viewport_override;

            // Flags
            b8 enabled{ true };
            u32 execution_order{ 0 };  // For manual ordering
        };

        /*****************\
        | RENDER GRAPH   |
        \*****************/

        enum class GraphBuildResult
        {
            Success,
            CyclicDependency,
            MissingResource,
            MissingDependency,
            InvalidConfiguration
        };

    } // namespace graphics

} // namespace hellengine
