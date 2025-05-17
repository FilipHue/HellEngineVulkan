#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanDevice
		{
		public:
			VulkanDevice();
			~VulkanDevice();

			void Create(const VulkanInstance& instance, VulkanSurface& surface);
			void Destroy(const VulkanInstance& instance);

			void PickPhysicalDevice(const VulkanInstance& instance, VulkanSurface& surface);
			void CreateLogicalDevice(const VulkanInstance& instance);
			void GetDeviceQueue();

			void WaitForIdle() const;

			const VkPhysicalDevice& GetPhysicalDevice() const { return m_physical_device; }
			const VkDevice& GetLogicalDevice() const { return m_logical_device; }

			const VkPhysicalDeviceFeatures& GetFeatures() const { return m_features; }
			const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
			const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_memory_properties; }

			const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_queue_families_indices; }
			const DeviceQueues& GetQueues() const { return m_queues; }

			const u32& GetGraphicsFamilyIndex() const { return m_queue_families_indices.graphics_family.value(); }
			const u32& GetComputeFamilyIndex() const { return m_queue_families_indices.compute_family.value(); }
			const u32& GetTransferFamilyIndex() const { return m_queue_families_indices.transfer_family.value(); }
			const u32& GetPresentFamilyIndex() const { return m_queue_families_indices.present_family.value(); }

			const VkQueue& GetGraphicsQueue() const { return m_queues.graphics_queue; }
			const VkQueue& GetComputeQueue() const { return m_queues.compute_queue; }
			const VkQueue& GetTransferQueue() const { return m_queues.transfer_queue; }
			const VkQueue& GetPresentQueue() const { return m_queues.present_queue; }

			const VkFormat& GetDepthFormat() const { return m_depth_format; }
			const VkFormat& GetStencilFormat() const { return m_stencil_format; }
			const VkFormat& GetDepthStencilFormat() const { return m_depth_stencil_format; }

			void SetDepthFormat(VkFormat format) { m_depth_format = format; }
			void SetStencilFormat(VkFormat format) { m_stencil_format = format; }
			void SetDepthStencilFormat(VkFormat format) { m_depth_stencil_format = format; }

			static u32 FindMemoryType(const VulkanDevice& device, u32 type_filter, VkMemoryPropertyFlags properties);

		private:
			VkPhysicalDevice m_physical_device;
			VkDevice m_logical_device;

			VkPhysicalDeviceFeatures m_features;
			VkPhysicalDeviceProperties m_properties;
			VkPhysicalDeviceMemoryProperties m_memory_properties;

			VkFormat m_depth_format;
			VkFormat m_stencil_format;
			VkFormat m_depth_stencil_format;

			QueueFamilyIndices m_queue_families_indices;
			DeviceQueues m_queues;

			DeviceRequirements m_requirements;
		};

		static b8 IsDeviceSuitable(
			const VkPhysicalDevice& device,
			VulkanSurface& surface,
			VkPhysicalDeviceFeatures& features,
			VkPhysicalDeviceProperties& properties,
			VkPhysicalDeviceMemoryProperties& memory_properties,
			const DeviceRequirements& requirements);

		static b8 ValidateDeviceExtensions(
			VkPhysicalDevice device,
			const DeviceRequirements& requirements);

		b8 ValidateSwapChainSupport(
			VkPhysicalDevice device,
			VulkanSurface& surface);

		static QueueFamilyIndices FindQueueFamilies(
			const VkPhysicalDevice& device,
			VulkanSurface& surface);

		static std::string GetQueueTypeString(VkQueueFlags flags);
		static void PrintQueueFamilyProperties(const std::vector<VkQueueFamilyProperties>& queue_families);
		static std::vector<f32> ScorePhysicalDeviceQueueFamilies(const std::vector< VkQueueFamilyProperties>& queue_families);

		b8 DetectDepthFormat(VulkanDevice& device);
		b8 DetectStencilFormat(VulkanDevice& device);
		b8 DetectDepthStencilFormat(VulkanDevice& device);

	} // namespace graphics

}