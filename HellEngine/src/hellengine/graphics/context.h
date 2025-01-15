#pragma once

// Internal
#include "objects/vulkan_buffer.h"
#include "objects/vulkan_command_buffer.h"
#include "objects/vulkan_device.h"
#include "objects/vulkan_descriptor.h"
#include "objects/vulkan_instance.h"
#include "objects/vulkan_pipeline.h"
#include "objects/vulkan_surface.h"
#include "objects/vulkan_swapchain.h"
#include "objects/vulkan_synchronization.h"
#include "objects/vulkan_texture.h"

namespace hellengine
{

	namespace graphics
	{

		struct FrameData
		{
			VulkanSemaphore image_available;
			VulkanSemaphore render_finished;
			VulkanFence frame_in_flight;
			VulkanCommandBuffer command_buffer;

			u32 image_index;
			b8 framebuffer_resized;
		};

		class VulkanContext
		{
		public:
			VulkanContext();
			~VulkanContext();

			void Init(core::Window* window);
			void Shutdown();

			void BeginFrame();
			void SubmitFrame();
			void EndFrame();

			void DrawFrame();
			void SetRecordCallback(std::function<void()> callback) { m_record_callback = callback; }
			void OnFramebufferResize();

			// State
			void SetViewport(std::initializer_list<VkViewport> viewports) const;
			void SetScissor(std::initializer_list<VkRect2D> scissors) const;

			// Rendering
			void SetClearColor(VkClearValue color);
			void SetExtent(VkExtent2D extent);

			void BeginDynamicRendering();
			void EndDynamicRendering();

			// Pipeline
			VulkanPipeline* CreatePipeline(const PipelineCreateInfo& info, const ShaderStageInfo& shader_info);

			void DestroyPipeline(VulkanPipeline* pipeline) const;

			void BindPipeline(VulkanPipeline* pipeline) const;

			// Buffer
			VulkanBuffer* CreateVertexBuffer(void* data, u32 size);
			VulkanBuffer* CreateIndexBuffer(void* data, u32 size);
			VulkanUniformBuffer * CreateUniformBuffer(u32 size);
			VulkanUniformBuffer* CreateDynamicUniformBuffer(void*& data, u32 data_size, u32 data_count);

			void UpdateUniformBuffer(VulkanUniformBuffer* ubo, void* data, u32 size);

			void DestroyBuffer(VulkanBuffer* buffer) const;
			void DestroyUniformBuffer(VulkanUniformBuffer* buffer) const;

			void BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;
			void BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;

			// Descriptor
			void InitDescriptorPoolGrowable(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets);
			VulkanDescriptorSet* CreateDescriptorSet(VulkanPipeline* pipeline, u32 set);

			void WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data);

			void BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor, u32 offsets_count = 0, u32* offsets = VK_NULL_HANDLE);

			// Texture
			VulkanTexture* CreateTexture2D(const char* path);

			void DestroyTexture(VulkanTexture* texture) const;

			// Draw
			void DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const;

			// TEMP

		private:
			VulkanBuffer CreateStagingBuffer(VkDeviceSize size, void* data) const;
			void FramebufferResize();

		private:
			VulkanInstance m_instance;
			VulkanSurface m_surface;
			VulkanDevice m_device;
			VulkanSwapchain m_swapchain;
			VulkanCommandPool m_command_pool;
			u32 m_current_frame;
			u32 m_frames_in_flight;

			std::vector<FrameData> m_frame_data;

			RenderData m_render_data;

			std::function<void()> m_record_callback;

			// TEMP
			VulkanDescriptorPoolGrowable m_descriptor_pool_growable;
		};

	} // namespace graphics

} // namespace hellengine