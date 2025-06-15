#pragma once

// Internal
#include <hellengine/graphics/context.h>

namespace hellengine
{

	namespace core
	{
		class Window;
	}
	
	namespace graphics
	{

		class VulkanBackend
		{
		public:
			void Init(Window* window);
			void Shutdown();

			void CreateImGuiGraphicContext(Window* window);
			void DestroyImGuiGraphicContext();

			void SubmitImGui();

			HE_API void DrawFrame();
			HE_API void SetRecordCallback(std::function<void()> callback);
			HE_API void OnFramebufferResize();

			HE_API void InitImGuiForRenderpass(VulkanRenderPass* renderpass);
			HE_API void InitImGuiForDynamicRendering(VkPipelineRenderingCreateInfoKHR pipeline_create_info);

			// State
			HE_API void SetViewport(std::initializer_list<VkViewport> viewports) const;
			HE_API void SetScissor(std::initializer_list<VkRect2D> scissors) const;

			// Rendering
			HE_API void SetClearColor(VkClearValue color);
			HE_API void SetExtent(VkExtent2D extent);

			// Dynamic rendering
			HE_API void BeginDynamicRendering();
			HE_API void EndDynamicRendering();

			HE_API void BeginDynamicRenderingWithAttachments(const DynamicRenderingInfo& info);
			HE_API void EndDynamicRenderingWithAttachments(const DynamicRenderingInfo& info);

			// Renderpass
			HE_API void BeginRenderPassWithSwapchain(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const;
			HE_API void BeginRenderPass(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const;
			HE_API void EndRenderPass() const;

			HE_API VulkanRenderPass* CreateRenderPass(RenderPassInfo& info);

			HE_API void RecreateRenderPassFramebuffers(VulkanRenderPass* render_pass, RenderPassInfo& info);

			HE_API void DestroyRenderPass(VulkanRenderPass* render_pass) const;

			// Pipeline
			HE_API VulkanPipeline* CreatePipeline(const PipelineCreateInfo& info, const ShaderStageInfo& shader_info);

			HE_API void DestroyPipeline(VulkanPipeline* pipeline) const;

			HE_API void BindPipeline(VulkanPipeline* pipeline) const;
			HE_API void BindPushConstants(VulkanPipeline* pipeline, ShaderStage stage, u32 offset, u32 size, void* data) const;

			// Buffer
			HE_API VulkanBuffer* CreateVertexBufferEmpty(u32 size);
			template <typename T> HE_API VulkanBuffer* CreateVertexBuffer(T* vertices, u32 vertices_count);
			HE_API VulkanBuffer* CreateIndexBufferEmpty(u32 size);
			template <typename T> HE_API VulkanBuffer* CreateIndexBuffer(T* indices, u32 indices_count);
			HE_API VulkanUniformBuffer* CreateUniformBufferMappedPersistent(u32 elem_size, u32 elem_count);
			HE_API VulkanUniformBuffer* CreateDynamicUniformBuffer(void*& data, u32 elem_size, u32 elem_count);
			HE_API VulkanStorageBuffer* CreateStorageBufferMappedPersistent(u32 elem_size, u32 elem_count);

			HE_API void UpdateVertexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size);
			HE_API void UpdateIndexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size);
			HE_API void UpdateUniformBuffer(VulkanUniformBuffer* buffer, void* data, u32 size, u32 offset = 0);
			HE_API void UpdateStorageBuffer(VulkanStorageBuffer* buffer, void* data, u32 size, u32 offset = 0);

			HE_API void DestroyBuffer(VulkanBuffer* buffer) const;

			HE_API void BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;
			HE_API void BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;

			// Descriptor
			HE_API void InitDescriptorPoolGrowable(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets);
			HE_API VulkanDescriptorSet* CreateDescriptorSet(VulkanPipeline* pipeline, u32 set);
			HE_API VulkanDescriptorSet* CreateDescriptorSetVariable(VulkanPipeline* pipeline, u32 set, std::vector<u32> count);

			HE_API void WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data);
			HE_API void WriteDescriptorVariable(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data, u32 count, u32 array_element);

			HE_API void BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor_set, u32 offsets_count = 0, u32* offsets = nullptr) const;

			// Texture
			HE_API VulkanTexture2D* CreateTexture2D(const File& file);
			HE_API VulkanTexture2D* CreateTexture2D(VkFormat format, u32 width, u32 height);
			HE_API VulkanTexture2D* CreateTexture2D(VkFormat format, const void* data, i32 width, i32 height);

			HE_API VulkanTexture3D* CreateTexture3D(VkFormat format, const void* data, u32 width, u32 height, u32 depth);

			HE_API VulkanTextureCubemap* CreateTextureCubemap(const File& file);
			HE_API VulkanTextureCubemap* CreateTextureCubemapArray(const File& file);

			HE_API void UpdateTexture(VulkanTexture* texture, const void* data);

			template<typename T>
			HE_API T ReadPixel(VulkanTexture* texture, u32 x, u32 y, u32 layer = 0, u32 face = 0);

			HE_API void DestroyTexture(VulkanTexture* texture) const;

			// Draw
			HE_API void Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const;
			HE_API void DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const;

		private:
			VulkanContext* m_context;
		};

	} // namespace graphics

} // namespace hellengine