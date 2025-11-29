#pragma once

// Internal
#include "objects/vulkan_buffer.h"
#include "objects/vulkan_command_buffer.h"
#include "objects/vulkan_descriptor.h"
#include "objects/vulkan_device.h"
#include "objects/vulkan_instance.h"
#include "objects/vulkan_pipeline.h"
#include "objects/vulkan_renderpass.h"
#include "objects/vulkan_surface.h"
#include "objects/vulkan_swapchain.h"
#include "objects/vulkan_synchronization.h"
#include "objects/vulkan_texture.h"

#include <hellengine/resources/file_manager.h>

namespace hellengine
{

	using namespace core;
	using namespace math;
	using namespace resources;
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

			void Init(Window* window);
			void Shutdown();

			void InitImGuiForRenderpass(VulkanRenderPass* renderpass);
			void InitImGuiForDynamicRendering(VkPipelineRenderingCreateInfoKHR pipeline_create_info);

			void SubmitImGui();

			void DrawFrame();
			void SetRecordCallback(std::function<void()> callback) { m_record_callback = callback; }
			void OnFramebufferResize();

			// State
			void SetViewport(std::initializer_list<VkViewport> viewports) const;
			void SetScissor(std::initializer_list<VkRect2D> scissors) const;

			// Rendering
			void SetClearColor(VkClearValue color);
			void SetExtent(VkExtent2D extent);

			// Dynamic rendering
			void BeginDynamicRendering();
			void EndDynamicRendering();

			void BeginDynamicRenderingWithAttachments(const DynamicRenderingInfo& info);
			void EndDynamicRenderingWithAttachments(const DynamicRenderingInfo& info);

			// Renderpass
			void BeginRenderPassWithSwapchain(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const;
			void BeginRenderPass(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const;
			void EndRenderPass() const;

			VulkanRenderPass* CreateRenderPass(RenderPassInfo& info);

			void RecreateRenderPass(VulkanRenderPass* render_pass, RenderPassInfo& info);

			void DestroyRenderPass(VulkanRenderPass* render_pass) const;

			// Pipeline
			VulkanPipeline* CreatePipeline(const PipelineCreateInfo& info, const ShaderStageInfo& shader_info);

			void DestroyPipeline(VulkanPipeline* pipeline) const;

			void BindPipeline(VulkanPipeline* pipeline) const;
			void BindPushConstants(VulkanPipeline* pipeline, ShaderStage stage, u32 offset, u32 size, const void* data) const;

			// Buffer
			VulkanBuffer* CreateVertexBuffer(void* data, u32 size);
			VulkanBuffer* CreateIndexBuffer(void* data, u32 size);
			VulkanUniformBuffer* CreateUniformBufferMappedPersistent(u32 elem_size, u32 elem_count);
			VulkanUniformBuffer* CreateDynamicUniformBuffer(void*& data, u32 elem_size, u32 elem_count);
			VulkanStorageBuffer* CreateStorageBufferMappedPersistent(u32 elem_size, u32 elem_count);
			VulkanBuffer* CreateDrawIndirectBuffer(u32 elem_size, u32 elem_count);

			void UpdateVertexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size);
			void UpdateIndexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size);
			void UpdateUniformBuffer(VulkanUniformBuffer* buffer, void* data, u32 size, u32 offset);
			void UpdateStorageBuffer(VulkanStorageBuffer* buffer, void* data, u32 size, u32 offset);
			void UpdateDrawIndirectBuffer(VulkanBuffer* buffer, void* data, u32 size, u32 offset);

			void DestroyBuffer(VulkanBuffer* buffer) const;

			void BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;
			void BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;

			// Descriptor
			void InitDescriptorPoolGrowable(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets);
			VulkanDescriptorSet* CreateDescriptorSet(VulkanPipeline* pipeline, u32 set);
			VulkanDescriptorSet* CreateDescriptorSetVariable(VulkanPipeline* pipeline, u32 set, std::vector<u32> count);

			void WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data);
			void WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data, u32 count, u32 array_element);

			void BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor, u32 offsets_count = 0, u32* offsets = VK_NULL_HANDLE);

			// Texture
			VulkanTexture2D* CreateTexture2D(VkFormat format, i32 width, i32 height);
			VulkanTexture2D* CreateTexture2D(const File& file);
			VulkanTexture2D* CreateTexture2D(VkFormat format, const void* data, i32 width, i32 height);

			VulkanTexture3D* CreateTexture3D(VkFormat format, const void* data, i32 width, i32 height, i32 depth);

			VulkanTextureCubemap* CreateTextureCubemap(const File& file);
			VulkanTextureCubemap* CreateTextureCubemapArray(const File& file);

			void UpdateTexture(VulkanTexture* texture, const void* data);

			template<typename T> T ReadPixel(VulkanTexture* texture, u32 x, u32 y, u32 layer = 0, u32 face = 0);

			void DestroyTexture(VulkanTexture* texture) const;

			// Draw
			void Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const;
			void DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const;
			void DrawIndexedIndirect(VulkanBuffer* buffer, u32 offset, u32 draw_count, u32 stride) const;
			
			// UI
			void CreateImGuiResources(Window* window);
			void DestroyImGuiResources();

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

			VulkanDescriptorPool m_imgui_descriptor_pool;
			VulkanDescriptorPoolGrowable m_descriptor_pool_growable;
		};

	} // namespace graphics

} // namespace hellengine