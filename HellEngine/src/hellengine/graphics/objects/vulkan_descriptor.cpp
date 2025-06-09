#include "hepch.h"
#include "vulkan_descriptor.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanDescriptorPool::VulkanDescriptorPool()
		{
			m_handle = VK_NULL_HANDLE;
		}

		VulkanDescriptorPool::~VulkanDescriptorPool()
		{
			NO_OP;
		}

		void VulkanDescriptorPool::Create(const VulkanInstance& instance, const VulkanDevice& device, VkDescriptorPoolCreateFlags flags, const std::vector<VkDescriptorPoolSize>& pool_sizes, u32 max_sets)
		{
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.pNext = VK_NULL_HANDLE;
			pool_info.flags = flags;
			pool_info.maxSets = max_sets;
			pool_info.poolSizeCount = (u32)pool_sizes.size();
			pool_info.pPoolSizes = pool_sizes.data();

			VK_CHECK(vkCreateDescriptorPool(device.GetLogicalDevice(), &pool_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanDescriptorPool::Destroy(const VulkanInstance& instance, const VulkanDevice& device)
		{
			vkDestroyDescriptorPool(device.GetLogicalDevice(), m_handle, instance.GetAllocator());

			m_handle = VK_NULL_HANDLE;
		}

		VulkanDescriptorPoolGrowable::VulkanDescriptorPoolGrowable()
		{
			m_ready_pools = {};
			m_full_pools = {};
			m_sets_per_pool = 0;
		}

		VulkanDescriptorPoolGrowable::~VulkanDescriptorPoolGrowable()
		{
			NO_OP;
		}

		void VulkanDescriptorPoolGrowable::Create(const VulkanInstance& instance, const VulkanDevice& device, const std::vector<VkDescriptorPoolSize>& pool_sizes, u32 sets)
		{
			m_sets_per_pool = sets;

			for (u32 i = 0; i < (u32)pool_sizes.size(); i++)
			{
				m_pool_sizes.push_back(pool_sizes[i]);
			}

			VkDescriptorPool pool = CreatePool(instance, device);

			m_sets_per_pool = 2 * m_sets_per_pool;
			m_ready_pools.push_back(pool);
		}

		void VulkanDescriptorPoolGrowable::Destroy(const VulkanInstance& instance, const VulkanDevice& device)
		{
			DestroyPools(instance, device);
		}

		VkDescriptorPool VulkanDescriptorPoolGrowable::CreatePool(const VulkanInstance& instance, const VulkanDevice& device)
		{
			for (u32 i = 0; i < (u32)m_pool_sizes.size(); i++)
			{
				m_pool_sizes[i].descriptorCount = m_sets_per_pool;
			}

			VkDescriptorPool pool = VK_NULL_HANDLE;

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.pNext = VK_NULL_HANDLE;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
			pool_info.maxSets = m_sets_per_pool;
			pool_info.poolSizeCount = (u32)m_pool_sizes.size();
			pool_info.pPoolSizes = m_pool_sizes.data();

			VK_CHECK(vkCreateDescriptorPool(device.GetLogicalDevice(), &pool_info, instance.GetAllocator(), &pool));
			
			return pool;
		}

		void VulkanDescriptorPoolGrowable::ClearPools(const VulkanInstance& instance, const VulkanDevice& device)
		{
			for (u32 i = 0; i < (u32)m_ready_pools.size(); i++)
			{
				vkResetDescriptorPool(device.GetLogicalDevice(), m_ready_pools[i], 0);
			}

			for (u32 i = 0; i < (u32)m_full_pools.size(); i++)
			{
				vkResetDescriptorPool(device.GetLogicalDevice(), m_full_pools[i], 0);
				m_ready_pools.push_back(m_full_pools[i]);
			}

			m_full_pools.clear();
		}

		void VulkanDescriptorPoolGrowable::DestroyPools(const VulkanInstance& instance, const VulkanDevice& device)
		{
			for (u32 i = 0; i < (u32)m_ready_pools.size(); i++)
			{
				vkDestroyDescriptorPool(device.GetLogicalDevice(), m_ready_pools[i], instance.GetAllocator());
			}
			for (u32 i = 0; i < (u32)m_full_pools.size(); i++)
			{
				vkDestroyDescriptorPool(device.GetLogicalDevice(), m_full_pools[i], instance.GetAllocator());
			}
			m_ready_pools.clear();
			m_full_pools.clear();
		}

		void VulkanDescriptorPoolGrowable::SetReadyPool(VkDescriptorPool pool)
		{
			m_ready_pools.push_back(pool);
		}

		void VulkanDescriptorPoolGrowable::SetFullPool(VkDescriptorPool pool)
		{
			m_full_pools.push_back(pool);
		}

		VkDescriptorPool VulkanDescriptorPoolGrowable::GetPool(const VulkanInstance& instance, const VulkanDevice& device)
		{
			VkDescriptorPool pool = VK_NULL_HANDLE;
			if (m_ready_pools.size() != 0)
			{
				pool = m_ready_pools.back();
				m_ready_pools.pop_back();
			}
			else
			{
				pool = CreatePool(instance, device);
				m_sets_per_pool = 2 * m_sets_per_pool;
			}

			return pool;
		}

		VulkanDescriptorSet::VulkanDescriptorSet()
		{
			m_handle = VK_NULL_HANDLE;
			m_writes = {};

			m_set = 0;
		}

		VulkanDescriptorSet::~VulkanDescriptorSet()
		{
			NO_OP;
		}

		void VulkanDescriptorSet::Create(const VulkanDevice& device, VkDescriptorSetLayout layout, VkDescriptorPool pool)
		{
			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.pNext = VK_NULL_HANDLE;
			alloc_info.descriptorPool = pool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &layout;

			VK_CHECK(vkAllocateDescriptorSets(device.GetLogicalDevice(), &alloc_info, &m_handle));
		}

		void VulkanDescriptorSet::Create(const VulkanInstance& instance, const VulkanDevice& device, VkDescriptorSetLayout layout, VulkanDescriptorPoolGrowable& pool, u32 set)
		{
			m_set = set;

			VkDescriptorPool pool_handle = pool.GetPool(instance, device);

			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.pNext = VK_NULL_HANDLE;
			alloc_info.descriptorPool = pool_handle;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &layout;

			VkResult result = vkAllocateDescriptorSets(device.GetLogicalDevice(), &alloc_info, &m_handle);

			if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
			{
				pool.SetFullPool(pool_handle);
				pool_handle = pool.GetPool(instance, device);
				alloc_info.descriptorPool = pool_handle;
				VK_CHECK(vkAllocateDescriptorSets(device.GetLogicalDevice(), &alloc_info, &m_handle));
			}

			pool.SetReadyPool(pool_handle);
		}

		void VulkanDescriptorSet::CreateVariable(const VulkanInstance& instance, const VulkanDevice& device, VkDescriptorSetLayout layout, VulkanDescriptorPoolGrowable& pool, u32 set, std::vector<u32> counts)
		{
			m_set = set;

			VkDescriptorPool pool_handle = pool.GetPool(instance, device);

			VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = {};
			set_counts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			set_counts.descriptorSetCount = (u32)counts.size();
			set_counts.pDescriptorCounts = counts.data();

			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.pNext = &set_counts;
			alloc_info.descriptorPool = pool_handle;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &layout;

			VkResult result = vkAllocateDescriptorSets(device.GetLogicalDevice(), &alloc_info, &m_handle);

			if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
			{
				pool.SetFullPool(pool_handle);
				pool_handle = pool.GetPool(instance, device);
				alloc_info.descriptorPool = pool_handle;
				VK_CHECK(vkAllocateDescriptorSets(device.GetLogicalDevice(), &alloc_info, &m_handle));
			}

			pool.SetReadyPool(pool_handle);
		}

		void VulkanDescriptorSet::Destroy(const VulkanDevice& device, VkDescriptorPool pool)
		{
			VK_CHECK(vkFreeDescriptorSets(device.GetLogicalDevice(), pool, 1, &m_handle));

			m_handle = VK_NULL_HANDLE;
		}

		void VulkanDescriptorSet::WriteBuffer(VkBuffer* buffer, VkDeviceSize* offset, VkDeviceSize* range, u32 binding, VkDescriptorType type, u32 count, u32 array_element)
		{
			for (u32 i = 0; i < count; i++)
			{
				m_buffer_infos.push_back({
					buffer[i],
					offset[i],
					range[i]
					});
			}

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = m_handle;	
			write.dstBinding = binding;
			write.dstArrayElement = array_element;
			write.descriptorType = type;
			write.descriptorCount = count;
			write.pBufferInfo = &m_buffer_infos.back();

			m_writes.push_back(write);
		}

		void VulkanDescriptorSet::WriteImage(VkImageView* image_view, VkSampler* sampler, u32 binding, VkDescriptorType type, u32 count, u32 array_element)
		{
			for (u32 i = 0; i < count; i++)
			{
				m_image_infos.push_back({
						sampler[i],
						image_view[i],
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				});
			}

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = m_handle;
			write.dstBinding = binding;
			write.dstArrayElement = array_element;
			write.descriptorType = type;
			write.descriptorCount = count;
			write.pImageInfo = m_image_infos.data();

			m_writes.push_back(write);
		}

		void VulkanDescriptorSet::Update(const VulkanDevice& device)
		{
			vkUpdateDescriptorSets(device.GetLogicalDevice(), (u32)m_writes.size(), m_writes.data(), 0, VK_NULL_HANDLE);
		}

		void VulkanDescriptorSet::Clear()
		{
			m_writes.clear();
			m_buffer_infos.clear();
			m_image_infos.clear();
		}

		void VulkanDescriptorSet::SetExpectedWrites(u32 count)
		{
			m_buffer_infos.reserve(count);
			m_image_infos.reserve(count);
			m_writes.reserve(count);
		}

		VkDescriptorSetLayout CreateDescriptorSetLayout(const VulkanInstance& instance, const VulkanDevice& device, const DescriptorSetInfo& set)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings_layout;
			std::vector<VkDescriptorBindingFlags> binding_flags;

			for (const auto& binding : set.bindings)
			{
				VkDescriptorSetLayoutBinding layout_binding{};
				layout_binding.binding = binding.binding;
				layout_binding.descriptorType = GetVulkanDescriptorType(binding.type);
				layout_binding.descriptorCount = binding.count;
				layout_binding.stageFlags = GetVulkanStageFlags(binding.stage);
				layout_binding.pImmutableSamplers = nullptr;

				bindings_layout.push_back(layout_binding);

				binding_flags.push_back(GetVulkanDescriptorSetBindingFlags(binding.flags));
			}

			VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{};
			binding_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			binding_flags_info.bindingCount = (u32)binding_flags.size();
			binding_flags_info.pBindingFlags = binding_flags.data();

			VkDescriptorSetLayoutCreateInfo layout_info{};
			layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout_info.pNext = &binding_flags_info;
			layout_info.flags = GetVulkanDescriptorSetFlags(set.flags);
			layout_info.bindingCount = static_cast<uint32_t>(bindings_layout.size());
			layout_info.pBindings = bindings_layout.data();

			VkDescriptorSetLayout layout = VK_NULL_HANDLE;
			VK_CHECK(vkCreateDescriptorSetLayout(device.GetLogicalDevice(), &layout_info, instance.GetAllocator(), &layout));

			return layout;
		}

	} // namespace graphics

} // namespace hellengine