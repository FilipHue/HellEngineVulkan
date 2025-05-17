#include "hepch.h"
#include "vulkan_device.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_surface.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanDevice::VulkanDevice()
		{
			m_physical_device = VK_NULL_HANDLE;
			m_logical_device = VK_NULL_HANDLE;

			m_features = {};
			m_properties = {};
			m_memory_properties = {};

			m_depth_format = VK_FORMAT_UNDEFINED;

			m_queue_families_indices = {};
			m_queues = {};

			m_requirements = {};
			m_requirements.graphics = true;
			m_requirements.compute = true;
			m_requirements.transfer = true;
			m_requirements.discrete = true;
			m_requirements.present_support = true;
			m_requirements.extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_MULTIVIEW_EXTENSION_NAME };
		}

		VulkanDevice::~VulkanDevice()
		{
			NO_OP;
		}

		void VulkanDevice::Create(const VulkanInstance& instance, VulkanSurface& surface)
		{
			PickPhysicalDevice(instance, surface);
			CreateLogicalDevice(instance);
			GetDeviceQueue();
		}

		void VulkanDevice::Destroy(const VulkanInstance& instance)
		{
			HE_GRAPHICS_DEBUG("Destroying vulkan device...");
			vkDestroyDevice(m_logical_device, instance.GetAllocator());

			m_physical_device = VK_NULL_HANDLE;
			m_logical_device = VK_NULL_HANDLE;

			m_features = {};
			m_properties = {};
			m_memory_properties = {};

			m_depth_format = VK_FORMAT_UNDEFINED;

			m_queue_families_indices = {};
			m_queues = {};
			HE_GRAPHICS_DEBUG("Vulkan device destroyed successfully");
		}

		void VulkanDevice::PickPhysicalDevice(const VulkanInstance& instance, VulkanSurface& surface)
		{
			HE_GRAPHICS_DEBUG("Picking vulkan physical device...");
			u32 device_count = 0;
			std::vector<VkPhysicalDevice> devices;
			VK_CHECK(vkEnumeratePhysicalDevices(instance.GetHandle(), &device_count, nullptr));
			HE_ASSERT(device_count, "Failed to find GPUs with Vulkan support!");
			devices.resize(device_count);
			VK_CHECK(vkEnumeratePhysicalDevices(instance.GetHandle(), &device_count, devices.data()));

			HE_GRAPHICS_DEBUG("\tFound {0} devices:", device_count);
			for (const auto& device : devices)
			{
				VkPhysicalDeviceProperties device_properties;
				vkGetPhysicalDeviceProperties(device, &device_properties);
				HE_GRAPHICS_DEBUG("\t\t{0}", device_properties.deviceName);
			}

			VkPhysicalDevice picked_device = VK_NULL_HANDLE;
			VkPhysicalDeviceFeatures device_features;
			VkPhysicalDeviceProperties device_properties;
			VkPhysicalDeviceMemoryProperties device_memory_properties;
			for (const auto& device : devices)
			{
				picked_device = VK_NULL_HANDLE;

				vkGetPhysicalDeviceFeatures(device, &device_features);
				vkGetPhysicalDeviceProperties(device, &device_properties);
				vkGetPhysicalDeviceMemoryProperties(device, &device_memory_properties);

				HE_GRAPHICS_DEBUG("\tChecking device: {0}", device_properties.deviceName);
				if (IsDeviceSuitable(device, surface, device_features, device_properties, device_memory_properties, m_requirements))
				{
					picked_device = device;
					break;
				}
			}

			if (picked_device == VK_NULL_HANDLE)
			{
				HE_ASSERT(false, "Failed to find a suitable GPU! Shutting down application");
			}

			m_physical_device = picked_device;
			m_features = device_features;
			m_properties = device_properties;
			m_memory_properties = device_memory_properties;

			m_queue_families_indices = FindQueueFamilies(m_physical_device, surface);
			HE_ASSERT(m_queue_families_indices.IsComplete(), "Failed to find required queue families!");
			HE_GRAPHICS_DEBUG("\t\tFound required queue families");
			HE_GRAPHICS_DEBUG("\tPicked device: {0}", m_properties.deviceName);
		}

		void VulkanDevice::CreateLogicalDevice(const VulkanInstance& instance)
		{
			HE_GRAPHICS_DEBUG("Creating vulkan logical device...");
			std::set<u32> unique_queue_families = {
				m_queue_families_indices.graphics_family.value(),
				m_queue_families_indices.compute_family.value(),
				m_queue_families_indices.transfer_family.value(),
				m_queue_families_indices.present_family.value() };

			u32 queue_families_count = static_cast<u32>(unique_queue_families.size());
			std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_families_count);
			f32 queue_priority = 1.0f;

			for (u32 i = 0; i < queue_families_count; i++)
			{
				VkDeviceQueueCreateInfo queue_create_info = {};
				queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_create_info.flags = 0;
				queue_create_info.pNext = VK_NULL_HANDLE;
				queue_create_info.queueFamilyIndex = i;
				queue_create_info.queueCount = 1;
				queue_create_info.pQueuePriorities = &queue_priority;
				queue_create_infos[i] = queue_create_info;
			}

			VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {};
			dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
			dynamic_rendering_features.pNext = VK_NULL_HANDLE;
			dynamic_rendering_features.dynamicRendering = VK_TRUE;

			VkDeviceCreateInfo device_create_info = {};
			device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			device_create_info.flags = 0;
			device_create_info.pNext = &dynamic_rendering_features;
			device_create_info.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());
			device_create_info.pQueueCreateInfos = queue_create_infos.data();
			device_create_info.enabledLayerCount = 0;
			device_create_info.ppEnabledLayerNames = VK_NULL_HANDLE;
			device_create_info.enabledExtensionCount = static_cast<u32>(m_requirements.extensions.size());
			device_create_info.ppEnabledExtensionNames = m_requirements.extensions.data();
			device_create_info.pEnabledFeatures = &m_features;

			VK_CHECK(vkCreateDevice(m_physical_device, &device_create_info, instance.GetAllocator(), &m_logical_device));
			HE_GRAPHICS_DEBUG("Vulkan logical device created successfully");
		}

		void VulkanDevice::GetDeviceQueue()
		{
			HE_GRAPHICS_DEBUG("Getting vulkan device queues...");
			vkGetDeviceQueue(m_logical_device, m_queue_families_indices.graphics_family.value(), 0, &m_queues.graphics_queue);
			vkGetDeviceQueue(m_logical_device, m_queue_families_indices.compute_family.value(), 0, &m_queues.compute_queue);
			vkGetDeviceQueue(m_logical_device, m_queue_families_indices.transfer_family.value(), 0, &m_queues.transfer_queue);
			vkGetDeviceQueue(m_logical_device, m_queue_families_indices.present_family.value(), 0, &m_queues.present_queue);
			HE_GRAPHICS_DEBUG("Vulkan device queues retrieved successfully");
		}

		void VulkanDevice::WaitForIdle() const
		{
			vkDeviceWaitIdle(m_logical_device);
		}

		u32 VulkanDevice::FindMemoryType(const VulkanDevice& device, u32 type_filter, VkMemoryPropertyFlags properties)
		{
			for (u32 i = 0; i < device.GetMemoryProperties().memoryTypeCount; i++)
			{
				if ((type_filter & (1 << i)) && (device.GetMemoryProperties().memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			HE_ASSERT(false, "Failed to find suitable memory type!");
			return UINT32_MAX;
		}

		b8 IsDeviceSuitable(const VkPhysicalDevice& device, VulkanSurface& surface, VkPhysicalDeviceFeatures& features, VkPhysicalDeviceProperties& properties, VkPhysicalDeviceMemoryProperties& memory_properties, const DeviceRequirements& requirements)
		{
			if (requirements.discrete)
			{
				if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					HE_GRAPHICS_DEBUG("\t\t\tDevice is not discrete. Skipping device");
					return false;
				}
			}

			if (!ValidateDeviceExtensions(device, requirements))
			{
				HE_GRAPHICS_DEBUG("\t\t\tDevice does not support required extensions. Skipping device");
				return false;
			}

			if (!ValidateSwapChainSupport(device, surface))
			{
				HE_GRAPHICS_DEBUG("\t\t\tDevice does not support required swap chain support. Skipping device");
				return false;
			}

			return true;
		}

		b8 ValidateDeviceExtensions(VkPhysicalDevice device, const DeviceRequirements& requirements)
		{
			u32 extensions_count;
			std::vector<VkExtensionProperties> available_extensions;
			VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, nullptr));
			available_extensions.resize(extensions_count);
			VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, available_extensions.data()));

			for (const auto& required_extension : requirements.extensions)
			{
				HE_GRAPHICS_DEBUG("\t\tChecking for required extension: {0}", required_extension);

				b8 found = false;
				for (const auto& available_extension : available_extensions)
				{
					if (strcmp(required_extension, available_extension.extensionName) == 0)
					{
						HE_GRAPHICS_DEBUG("\t\t\tFound");

						found = true;
						break;
					}
				}

				if (!found)
				{
					HE_GRAPHICS_DEBUG("\t\t\tNot found");
					return false;
				}
			}

			return true;
		}

		b8 ValidateSwapChainSupport(VkPhysicalDevice device, VulkanSurface& surface)
		{
			HE_GRAPHICS_DEBUG("\t\tQuerrying for swap chain support");

			SwapChainSupportDetails details;

			VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.GetHandle(), &details.capabilities));

			u32 format_count;
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.GetHandle(), &format_count, nullptr));
			if (format_count != 0)
			{
				details.formats.resize(format_count);
				VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.GetHandle(), &format_count, details.formats.data()));
			}

			u32 present_mode_count;
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.GetHandle(), &present_mode_count, nullptr));
			if (present_mode_count != 0)
			{
				details.present_modes.resize(present_mode_count);
				VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.GetHandle(), &present_mode_count, details.present_modes.data()));
			}

			if (details.formats.empty() || details.present_modes.empty())
			{
				HE_GRAPHICS_DEBUG("\t\t\tFailed to find suitable swap chain support");
				return false;
			}

			HE_GRAPHICS_DEBUG("\t\t\tFound suitable swap chain support");
			surface.AddSwapChainSupportDetails(details);
			return true;
		}

		QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device, VulkanSurface& surface)
		{
			HE_GRAPHICS_DEBUG("\t\tFinding queue families");

			QueueFamilyIndices indices;
			u32 queue_family_count = 0;
			std::vector<VkQueueFamilyProperties> queue_families;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
			queue_families.resize(queue_family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

			//PrintQueueFamilyProperties(queue_families);

			std::vector<f32> scores = ScorePhysicalDeviceQueueFamilies(queue_families);

			u32 best_graphics_family = UINT32_MAX;
			u32 best_compute_family = UINT32_MAX;
			u32 best_transfer_family = UINT32_MAX;
			u32 best_present_family = UINT32_MAX;

			u32 min_transfer_score = UINT32_MAX;
			for (u32 i = 0; i < queue_family_count; i++)
			{
				const auto& queue_family = queue_families[i];
				u32 transfer_score = 0;

				if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					((best_graphics_family == UINT32_MAX) || (scores[i] > scores[best_graphics_family])))
				{
					best_graphics_family = i;
					transfer_score += 1;
				}

				if ((queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
					!(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					(best_compute_family == UINT32_MAX || scores[i] > scores[best_compute_family]))
				{
					best_compute_family = i;
					transfer_score += 1;
				}

				if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT &&
					!(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					!(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					if (transfer_score < min_transfer_score)
					{
						min_transfer_score = transfer_score;
						best_transfer_family = i;
					}
				}

				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface.GetHandle(), &present_support);
				if (present_support &&
					(best_present_family == UINT32_MAX || scores[i] > scores[best_present_family]))
				{
					best_present_family = i;
				}
			}

			indices.graphics_family = best_graphics_family;
			indices.compute_family = best_compute_family;
			indices.transfer_family = best_transfer_family;
			indices.present_family = best_present_family;

			return indices;
		}

		std::string GetQueueTypeString(VkQueueFlags flags)
		{
			std::string result = "| ";
			if (flags & VK_QUEUE_GRAPHICS_BIT)
			{
				result += "Graphics | ";
			}
			if (flags & VK_QUEUE_COMPUTE_BIT)
			{
				result += "Compute | ";
			}
			if (flags & VK_QUEUE_TRANSFER_BIT)
			{
				result += "Transfer | ";
			}
			if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				result += "Sparse Binding | ";
			}
			if (flags & VK_QUEUE_PROTECTED_BIT)
			{
				result += "Protected | ";
			}
			if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			{
				result += "Video Decode | ";
			}
			if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			{
				result += "Video Encode | ";
			}
			if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
			{
				result += "Optical Flow |";
			}

			if (result == "| ")
			{
				result = " |";
			}

			return result;
		}

		void PrintQueueFamilyProperties(const std::vector<VkQueueFamilyProperties>& queue_families)
		{
			HE_GRAPHICS_DEBUG("\t\t\tFound {0} queue families", queue_families.size());
			for (u32 i = 0; i < queue_families.size(); i++)
			{
				const auto& queue_family = queue_families[i];

				HE_GRAPHICS_DEBUG("\t\t\tQueue family {0}:", i);
				HE_GRAPHICS_DEBUG("\t\t\t\tQueue count: {0}", queue_family.queueCount);
				HE_GRAPHICS_DEBUG("\t\t\t\tQueue flags: {0}", queue_family.queueFlags);
				HE_GRAPHICS_DEBUG("\t\t\t\tSupported operations: {0}", GetQueueTypeString(queue_family.queueFlags));
			}
		}

		std::vector<f32> ScorePhysicalDeviceQueueFamilies(const std::vector<VkQueueFamilyProperties>& queue_families)
		{
			f32 queue_score = 0.0f;
			std::vector<f32> scores(queue_families.size(), 0.0f);
			for (u32 i = 0; i < queue_families.size(); i++)
			{
				const auto& queue_family = queue_families[i];

				queue_score = 0.0f;

				queue_score += queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT ? 10.0f : 0.0f;
				queue_score += queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT ? 8.0f : 0.0f;
				queue_score += queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT ? 6.0f : 0.0f;
				queue_score -= 1 / (f32)queue_family.queueCount;

				scores[i] = queue_score;
			}

			std::sort(scores.begin(), scores.end(), std::greater<f32>());

			return scores;
		}

		b8 DetectDepthFormat(VulkanDevice& device)
		{
			std::vector<VkFormat> depth_formats = {
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_D16_UNORM_S8_UINT
			};

			u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
			for (const auto& format : depth_formats)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDevice(), format, &props);

				if ((props.optimalTilingFeatures & flags) == flags)
				{
					device.SetDepthFormat(format);
					return true;
				}
				else if ((props.linearTilingFeatures & flags) == flags)
				{
					device.SetDepthFormat(format);
					return true;
				}
			}

			return false;
		}

		b8 DetectStencilFormat(VulkanDevice& device)
		{
			std::vector<VkFormat> stencil_formats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT
			};

			u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
			for (const auto& format : stencil_formats)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDevice(), format, &props);
				if ((props.optimalTilingFeatures & flags) == flags)
				{
					device.SetStencilFormat(format);
					return true;
				}
				else if ((props.linearTilingFeatures & flags) == flags)
				{
					device.SetStencilFormat(format);
					return true;
				}
			}

			return false;
		}

		b8 DetectDepthStencilFormat(VulkanDevice& device)
		{
			std::vector<VkFormat> depth_stencil_formats = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT
			};

			u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
			for (const auto& format : depth_stencil_formats)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDevice(), format, &props);
				if ((props.optimalTilingFeatures & flags) == flags)
				{
					device.SetDepthStencilFormat(format);
					return true;
				}
				else if ((props.linearTilingFeatures & flags) == flags)
				{
					device.SetDepthStencilFormat(format);
					return true;
				}
			}

			return false;
		}

	} // namespace graphics

} // namespace hellengine