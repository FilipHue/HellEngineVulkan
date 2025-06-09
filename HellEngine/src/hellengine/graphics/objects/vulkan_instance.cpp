#include "hepch.h"
#include "vulkan_instance.h"

// Internal
#include <hellengine/core/window/window.h>

namespace hellengine
{

	namespace graphics
	{

		VulkanInstance::VulkanInstance()
		{
			m_handle = VK_NULL_HANDLE;
			m_allocator = VK_NULL_HANDLE;
#if defined(HE_DEBUG)
			m_debugger = {};
#endif
		}

		VulkanInstance::~VulkanInstance()
		{
			NO_OP;
		}

		void VulkanInstance::Create(Window* window)
		{
			HE_GRAPHICS_DEBUG("Creating vulkan allocator...");
			m_allocator = VK_NULL_HANDLE;
			HE_GRAPHICS_DEBUG("Vulkan allocator created successfully");

			HE_GRAPHICS_DEBUG("Creating vulkan instance...");
			VkApplicationInfo app_info = {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = window->GetTitle();
			app_info.applicationVersion = HELL_MAKE_VERSION(1, 0, 0);
			app_info.pEngineName = HELL_VERSION_NAME;
			app_info.engineVersion = HELL_VERSION_NUMBER;
			app_info.apiVersion = VK_API_VERSION_1_3;

			VkInstanceCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pNext = VK_NULL_HANDLE;
			create_info.flags = 0;
			create_info.pApplicationInfo = &app_info;

			HE_GRAPHICS_DEBUG("\tValidating extensions");
			std::vector<const char*> extensions = GetVulkanExtensions();
			if (!ValidateVulkanExtensions(extensions))
			{
				HE_GRAPHICS_ERROR("Failed to validate extensions. Shutting down the application");
				exit(EXIT_FAILURE);
			}

			create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
			create_info.ppEnabledExtensionNames = extensions.data();

#if defined(HE_DEBUG)
			HE_GRAPHICS_DEBUG("\tValidating layers");
			std::vector<const char*> layers = GetVulkanLayers();
			if (!ValidateVulkanLayers(layers))
			{
				HE_GRAPHICS_ERROR("Failed to validate layers. Shutting down the application");
				exit(EXIT_FAILURE);
			}

			create_info.enabledLayerCount = static_cast<u32>(layers.size());
			create_info.ppEnabledLayerNames = layers.data();
#else
			create_info.enabledLayerCount = 0;
			create_info.ppEnabledLayerNames = VK_NULL_HANDLE;
#endif
			VK_CHECK(vkCreateInstance(&create_info, m_allocator, &m_handle));

#if defined(HE_DEBUG)
			m_debugger.Create(m_handle, m_allocator);
#endif
			HE_GRAPHICS_DEBUG("Vulkan instance created successfully");
		}

		void VulkanInstance::Destroy()
		{
			HE_GRAPHICS_DEBUG("Destroying vulkan instance...");
#if defined(HE_DEBUG)
			m_debugger.Destroy(m_handle, m_allocator);
#endif
			vkDestroyInstance(m_handle, VK_NULL_HANDLE);

			m_handle = VK_NULL_HANDLE;
			HE_GRAPHICS_DEBUG("Vulkan instance destroyed successfully");
		}

		std::vector<const char*> GetVulkanExtensions()
		{
			return {
				VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(HE_PLATFORM_WINDOWS)
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(HE_DEBUG)
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
			};
		}

		b8 ValidateVulkanExtensions(std::vector<const char*> required_extensions)
		{
			u32 extension_count = 0;
			VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
			std::vector<VkExtensionProperties> extensions(extension_count);
			VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));

			for (const char* required_extension : required_extensions)
			{
				HE_GRAPHICS_DEBUG("\t\tLooking for extension: {0}", required_extension);

				b8 found = false;
				for (const auto& extension : extensions)
				{
					if (strcmp(required_extension, extension.extensionName) == 0)
					{
						HE_GRAPHICS_DEBUG("\t\t\tFound");
						found = true;
						break;
					}
				}

				if (!found)
				{
					HE_GRAPHICS_ERROR("\t\t\tNot found");
					return false;
				}
			}

			return true;
		}

		std::vector<const char*> GetVulkanLayers()
		{
			return { 
				"VK_LAYER_KHRONOS_validation"
			};
		}

		b8 ValidateVulkanLayers(std::vector<const char*> required_layers)
		{
			u32 layer_count = 0;
			VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
			std::vector<VkLayerProperties> layers(layer_count);
			VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, layers.data()));

			for (const char* required_layer : required_layers)
			{
				HE_GRAPHICS_DEBUG("\t\tLooking for layer: {0}", required_layer);

				b8 found = false;
				for (const auto& layer : layers)
				{
					if (strcmp(required_layer, layer.layerName) == 0)
					{
						HE_GRAPHICS_DEBUG("\t\t\tFound");
						found = true;
						break;
					}
				}

				if (!found)
				{
					HE_GRAPHICS_ERROR("\t\t\tNot found");
					return false;
				}
			}

			return true;
		}

	} // namespace graphics

} // namespace hellengine