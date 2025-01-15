#include "hepch.h"
#include "vulkan_debugger.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanDebugger::VulkanDebugger()
		{
			m_messenger = VK_NULL_HANDLE;
			m_message_severity = 0;
			m_message_type = 0;
		}

		VulkanDebugger::~VulkanDebugger()
		{
			NO_OP;
		}

		void VulkanDebugger::Create(VkInstance instance, VkAllocationCallbacks* allocator)
		{
			HE_GRAPHICS_DEBUG("\tCreating Vulkan Debugger...");
			m_message_severity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			m_message_type =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			CreateMessenger(instance, allocator);
			HE_GRAPHICS_DEBUG("\tVulkan Debugger created  successfully");
		}

		void VulkanDebugger::Destroy(VkInstance instance, VkAllocationCallbacks* allocator)
		{
			HE_GRAPHICS_DEBUG("\tDestroying Vulkan Debugger...");
			DestroyMessenger(instance, allocator);

			m_messenger = VK_NULL_HANDLE;
			HE_GRAPHICS_DEBUG("\tVulkan Debugger destroyed successfully");
		}

		void VulkanDebugger::CreateMessenger(VkInstance instance, VkAllocationCallbacks* allocator)
		{
			VkDebugUtilsMessengerCreateInfoEXT create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.flags = 0;
			create_info.pNext = VK_NULL_HANDLE;
			create_info.messageSeverity = m_message_severity;
			create_info.messageType = m_message_type;
			create_info.pfnUserCallback = DebugCallback;
			create_info.pUserData = VK_NULL_HANDLE;

			PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func == VK_NULL_HANDLE)
			{
				HE_GRAPHICS_DEBUG("Failed to get vkCreateDebugUtilsMessengerEXT function pointer. Shutting down appliaction");
				exit(EXIT_FAILURE);
			}

			VK_CHECK(func(instance, &create_info, allocator, &m_messenger));
		}

		void VulkanDebugger::DestroyMessenger(VkInstance instance, VkAllocationCallbacks* allocator) const
		{
			PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != VK_NULL_HANDLE)
			{
				func(instance, m_messenger, allocator);
			}
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			std::string message = pCallbackData->pMessage;
			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				HE_GRAPHICS_DEBUG(message.c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				HE_GRAPHICS_INFO(message.c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				HE_GRAPHICS_WARN(message.c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				message += "\n";
				HE_GRAPHICS_ERROR(message.c_str());
				break;
			default:
				break;
			}

			return VK_FALSE;
		}

	} // namespace graphics

} // namespace hellengine