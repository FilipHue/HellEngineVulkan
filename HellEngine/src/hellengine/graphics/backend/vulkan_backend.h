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
			void Init(core::Window* window);
			void Shutdown();

			// TO DO: Make them private
			HE_API void BeginFrame();
			HE_API void SubmitFrame();
			HE_API void EndFrame();

			HE_API void DrawFrame();
			HE_API void SetRecordCallback(std::function<void()> callback);
			HE_API void OnFramebufferResize();

			// State
			HE_API void SetViewport(std::initializer_list<VkViewport> viewports) const;
			HE_API void SetScissor(std::initializer_list<VkRect2D> scissors) const;

			// Rendering
			HE_API void SetClearColor(VkClearValue color);
			HE_API void SetExtent(VkExtent2D extent);

			HE_API void BeginDynamicRendering();
			HE_API void EndDynamicRendering();

			// Pipeline
			HE_API VulkanPipeline* CreatePipeline(const PipelineCreateInfo& info, const ShaderStageInfo& shader_info);

			HE_API void DestroyPipeline(VulkanPipeline* pipeline) const;

			HE_API void BindPipeline(VulkanPipeline* pipeline) const;

			// Buffer
			HE_API VulkanBuffer* CreateVertexBuffer(math::Vertex* vertices, u32 vertices_count);
			HE_API VulkanBuffer* CreateIndexBuffer(u32* indices, u32 indices_count);
			HE_API VulkanUniformBuffer * CreateUniformBuffer(u32 size);
			HE_API VulkanUniformBuffer* CreateDynamicUniformBuffer(void*& data, u32 data_size, u32 data_count);

			HE_API void UpdateUniformBuffer(VulkanUniformBuffer* ubo, void* data, u32 size);

			HE_API void DestroyBuffer(VulkanBuffer* buffer) const;
			HE_API void DestroyUniformBuffer(VulkanUniformBuffer* buffer) const;

			HE_API void BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;
			HE_API void BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const;

			// Descriptor
			HE_API void InitDescriptorPool(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets);
			HE_API VulkanDescriptorSet* CreateDescriptorSet(VulkanPipeline* pipeline, u32 set);

			HE_API void WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data);

			HE_API void BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor_set, u32 offsets_count = 0, u32* offsets = nullptr) const;

			// Texture
			HE_API VulkanTexture* CreateTexture2D(const char* path);

			HE_API void DestroyTexture(VulkanTexture* texture) const;

			// Draw
			HE_API void DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const;

			// TEMP

		private:
			VulkanContext* m_context;
		};

	} // namespace graphics

} // namespace hellengine