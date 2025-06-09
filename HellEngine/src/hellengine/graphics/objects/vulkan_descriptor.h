#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanDescriptorPool
		{
		public:
			VulkanDescriptorPool();
			~VulkanDescriptorPool();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, VkDescriptorPoolCreateFlags flags, const std::vector<VkDescriptorPoolSize>& pools, u32 max_sets);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device);

			VkDescriptorPool GetHandle() const { return m_handle; }

		private:
			VkDescriptorPool m_handle;
		};

		class VulkanDescriptorPoolGrowable
		{
		public:
			VulkanDescriptorPoolGrowable();
			~VulkanDescriptorPoolGrowable();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const std::vector<VkDescriptorPoolSize>& pool_sizes, u32 sets);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device);

			VkDescriptorPool GetPool(const VulkanInstance& instance, const VulkanDevice& device);
			VkDescriptorPool CreatePool(const VulkanInstance& instance, const VulkanDevice& device);

			void ClearPools(const VulkanInstance& instance, const VulkanDevice& device);
			void DestroyPools(const VulkanInstance& instance, const VulkanDevice& device);

			void SetReadyPool(VkDescriptorPool pool);
			void SetFullPool(VkDescriptorPool pool);

		private:
			std::vector<VkDescriptorPoolSize> m_pool_sizes;
			std::vector<VkDescriptorPool> m_ready_pools;
			std::vector<VkDescriptorPool> m_full_pools;
			u32 m_sets_per_pool;
		};

		class VulkanDescriptorSet
		{
		public:
			VulkanDescriptorSet();
			~VulkanDescriptorSet();

			void Create(const VulkanDevice& device, VkDescriptorSetLayout layout, VkDescriptorPool pool);
			void Create(const VulkanInstance& instance, const VulkanDevice& device, VkDescriptorSetLayout layout, VulkanDescriptorPoolGrowable& pool, u32 set);
			void CreateVariable(const VulkanInstance& instance, const VulkanDevice& device, VkDescriptorSetLayout layout, VulkanDescriptorPoolGrowable& pool, u32 set, std::vector<u32> counts);
			void Destroy(const VulkanDevice& device, VkDescriptorPool pool);

			void WriteBuffer(VkBuffer* buffer, VkDeviceSize* offset, VkDeviceSize* range, u32 binding, VkDescriptorType type, u32 count = 1, u32 array_element = 0);
			void WriteImage(VkImageView* image_view, VkSampler* sampler, u32 binding, VkDescriptorType type, u32 count = 1, u32 array_element = 0);

			void Update(const VulkanDevice& device);
			void Clear();

			void SetExpectedWrites(u32 count);

			VkDescriptorSet GetHandle() const { return m_handle; }

			u32 GetSet() { return m_set; }

		private:
			VkDescriptorSet m_handle;
			std::vector<VkWriteDescriptorSet> m_writes;

			std::vector<VkDescriptorBufferInfo> m_buffer_infos;
			std::vector<VkDescriptorImageInfo> m_image_infos;

			u32 m_set;
		};

		VkDescriptorSetLayout CreateDescriptorSetLayout(const VulkanInstance& instance, const VulkanDevice& device, const DescriptorSetInfo& set);

	} // namespace graphics

} // namespace hellengine