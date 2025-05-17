#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanDebugger
		{
		public:
			VulkanDebugger();
			~VulkanDebugger();

			void Create(VkInstance instance, VkAllocationCallbacks* allocator);
			void Destroy(VkInstance instance, VkAllocationCallbacks* allocator);

		private:
			void CreateMessenger(VkInstance instance, VkAllocationCallbacks* allocator);
			void DestroyMessenger(VkInstance instance, VkAllocationCallbacks* allocator) const;

		private:
			VkDebugUtilsMessengerEXT m_messenger;
			u32 m_message_severity;
			u32 m_message_type;
		};

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	} // namespace graphics

} // namespace hellengine