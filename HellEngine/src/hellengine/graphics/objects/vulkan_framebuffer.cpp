#include "hepch.h"
#include "vulkan_framebuffer.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_renderpass.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanFramebuffer::VulkanFramebuffer()
		{
			m_handle = VK_NULL_HANDLE;
		}

		VulkanFramebuffer::~VulkanFramebuffer()
		{
			NO_OP;
		}

		void VulkanFramebuffer::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanRenderPass& render_pass, const FramebufferInfo& info)
		{
			VkFramebufferCreateInfo framebuffer_info = {};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.flags = 0;
			framebuffer_info.renderPass = render_pass.GetHandle();
			framebuffer_info.attachmentCount = info.attachment_count;
			framebuffer_info.pAttachments = info.attachments;
			framebuffer_info.width = info.width;
			framebuffer_info.height = info.height;
			framebuffer_info.layers = info.layers;

			VK_CHECK(vkCreateFramebuffer(device.GetLogicalDevice(), &framebuffer_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanFramebuffer::Destroy(const VulkanInstance& instance, const VulkanDevice& device)
		{
			vkDestroyFramebuffer(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

	} // namespace graphics

} // namespace hellengine