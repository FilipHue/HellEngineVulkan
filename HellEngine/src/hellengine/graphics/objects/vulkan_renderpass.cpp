#include "hepch.h"
#include "vulkan_renderpass.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"

namespace hellengine
{
	namespace graphics
	{

		VulkanRenderPass::VulkanRenderPass()
		{
			m_handle = VK_NULL_HANDLE;
		}

		VulkanRenderPass::~VulkanRenderPass()
		{
			NO_OP;
		}

		void VulkanRenderPass::Create(const VulkanInstance& instance, const VulkanDevice& device, VulkanSwapchain& swapchain, const RenderPassInfo& info)
		{
			VkRenderPassCreateInfo render_pass_info = {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.flags = 0;

			std::vector<VkAttachmentDescription> attachments;
			attachments.reserve(info.attachments.size());
			for (const auto& attachment : info.attachments)
			{
				VkAttachmentDescription attachment_description = {};
				attachment_description.flags = 0;
				attachment_description.format = attachment.format;
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = GetVulkanAttachmentLoadOp(attachment.load_op);
				attachment_description.storeOp = GetVulkanAttachmentStoreOp(attachment.store_op);
				attachment_description.stencilLoadOp = GetVulkanAttachmentLoadOp(attachment.stencil_load_op);
				attachment_description.stencilStoreOp = GetVulkanAttachmentStoreOp(attachment.stencil_store_op);
				attachment_description.initialLayout = attachment.initial_layout;
				attachment_description.finalLayout = attachment.final_layout;

				attachments.push_back(attachment_description);
			}

			std::vector<VkSubpassDescription> subpasses;
			subpasses.reserve(info.subpasses.size());
			for (const auto& subpass : info.subpasses)
			{
				VkSubpassDescription subpass_description = {};
				subpass_description.pipelineBindPoint = subpass.pipeline_bind_point;
				subpass_description.inputAttachmentCount = 0;
				subpass_description.pInputAttachments = nullptr;
				subpass_description.colorAttachmentCount = static_cast<u32>(subpass.color_attachments.size());
				subpass_description.pColorAttachments = subpass.color_attachments.data();
				subpass_description.pResolveAttachments = nullptr;
				subpass_description.pDepthStencilAttachment = subpass.depth_attachment.has_value() ? &subpass.depth_attachment.value() : nullptr;
				subpass_description.preserveAttachmentCount = 0;
				subpass_description.pPreserveAttachments = nullptr;

				subpasses.push_back(subpass_description);
			}

			std::vector<VkSubpassDependency> dependencies;
			dependencies.reserve(info.dependencies.size());
			for (const auto& dependency : info.dependencies)
			{
				VkSubpassDependency dependency_description = {};
				dependency_description.srcSubpass = dependency.src_subpass;
				dependency_description.dstSubpass = dependency.dst_subpass;
				dependency_description.srcStageMask = dependency.src_stage_mask;
				dependency_description.dstStageMask = dependency.dst_stage_mask;
				dependency_description.srcAccessMask = dependency.src_access_mask;
				dependency_description.dstAccessMask = dependency.dst_access_mask;
				dependency_description.dependencyFlags = dependency.dependency_flags;

				dependencies.push_back(dependency_description);
			}

			render_pass_info.attachmentCount = static_cast<u32>(attachments.size());
			render_pass_info.pAttachments = attachments.data();
			render_pass_info.subpassCount = static_cast<u32>(subpasses.size());
			render_pass_info.pSubpasses = subpasses.data();
			render_pass_info.dependencyCount = static_cast<u32>(dependencies.size());
			render_pass_info.pDependencies = dependencies.data();

			VK_CHECK(vkCreateRenderPass(device.GetLogicalDevice(), &render_pass_info, instance.GetAllocator(), &m_handle));

			CreateFramebuffers(instance, device, swapchain, info);
		}

		void VulkanRenderPass::Destroy(const VulkanInstance& instance, const VulkanDevice& device)
		{
			vkDestroyRenderPass(device.GetLogicalDevice(), m_handle, instance.GetAllocator());

			DestroyFramebuffers(instance, device);
		}

		void VulkanRenderPass::RecreateFramebuffers(const VulkanInstance& instance, const VulkanDevice& device, VulkanSwapchain& swapchain, const RenderPassInfo& info)
		{
			DestroyFramebuffers(instance, device);
			CreateFramebuffers(instance, device, swapchain, info);
		}

		void VulkanRenderPass::CreateFramebuffers(const VulkanInstance& instance, const VulkanDevice& device, VulkanSwapchain& swapchain, const RenderPassInfo& info)
		{
			if (info.type == RenderPassType_Swapchain)
			{
				for (size_t i = 0; i < swapchain.GetImageViews().size(); ++i)
				{
					std::vector<VkImageView> views;

					for (const auto& attachment : info.attachments)
					{
						if (attachment.is_swapchain_attachment)
						{
							views.push_back(swapchain.GetImageViews()[i]);
						}
						else if (attachment.is_depth_attachment)
						{
							views.push_back(swapchain.GetDepthImage().GetImageView());
						}
						else if (attachment.is_stencil_attachment)
						{
							views.push_back(swapchain.GetStencilImage().GetImageView());
						}
						else if (attachment.is_depth_stencil_attachment) 
						{
							views.push_back(swapchain.GetDepthStencilImage().GetImageView());
						}
						else 
						{
							views.push_back(attachment.image_view);
						}
					}

					FramebufferInfo framebuffer_info = {};
					framebuffer_info.attachment_count = static_cast<u32>(views.size());
					framebuffer_info.attachments = views.data();
					framebuffer_info.width = swapchain.GetExtent().width;
					framebuffer_info.height = swapchain.GetExtent().height;
					framebuffer_info.layers = 1;

					VulkanFramebuffer framebuffer;
					framebuffer.Create(instance, device, *this, framebuffer_info);
					m_framebuffers.push_back(framebuffer);
				}
			}
			else
			{
				std::vector<VkImageView> views = info.custom_attachment_views;

				FramebufferInfo framebuffer_info = {};
				framebuffer_info.attachment_count = static_cast<u32>(views.size());
				framebuffer_info.attachments = views.data();
				framebuffer_info.width = info.width;
				framebuffer_info.height = info.height;
				framebuffer_info.layers = 1;

				VulkanFramebuffer framebuffer;
				framebuffer.Create(instance, device, *this, framebuffer_info);
				m_framebuffers.push_back(framebuffer);
			}
		}

		void VulkanRenderPass::DestroyFramebuffers(const VulkanInstance& instance, const VulkanDevice& device)
		{
			for (auto& framebuffer : m_framebuffers)
			{
				framebuffer.Destroy(instance, device);
			}
			m_framebuffers.clear();
		}

	} // namespace graphics

} // namespace hellengine