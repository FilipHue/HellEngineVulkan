#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanFramebuffer
		{
		public:
			VulkanFramebuffer();
			~VulkanFramebuffer();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanRenderPass& render_pass, const FramebufferInfo& info);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device);

			VkFramebuffer GetHandle() const { return m_handle; }

		private:
			VkFramebuffer m_handle;
		};

	} // namespace graphics

} // namespace hellengine