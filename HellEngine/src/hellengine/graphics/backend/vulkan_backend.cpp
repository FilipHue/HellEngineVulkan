#include "hepch.h"
#include "vulkan_backend.h"

// Internal
#include <hellengine/core/window/window.h>

namespace hellengine
{

	using namespace math;
	namespace graphics
	{

		void VulkanBackend::Init(Window* window)
		{
			m_context = new VulkanContext();
			m_context->Init(window);
		}

		void VulkanBackend::Shutdown()
		{
			m_context->Shutdown();
		}

		void VulkanBackend::CreateImGuiGraphicContext(Window* window)
		{
			m_context->CreateImGuiResources(window);
		}

		void VulkanBackend::DestroyImGuiGraphicContext()
		{
			m_context->DestroyImGuiResources();
		}

		void VulkanBackend::SubmitImGui()
		{
			m_context->SubmitImGui();
		}

		void VulkanBackend::DrawFrame()
		{
			m_context->DrawFrame();
		}

		void VulkanBackend::SetRecordCallback(std::function<void()> callback)
		{
			m_context->SetRecordCallback(callback);
		}

		void VulkanBackend::OnFramebufferResize()
		{
			m_context->OnFramebufferResize();
		}

		void VulkanBackend::InitImGuiForRenderpass(VulkanRenderPass* renderpass)
		{
			m_context->InitImGuiForRenderpass(renderpass);
		}

		void VulkanBackend::InitImGuiForDynamicRendering(VkPipelineRenderingCreateInfoKHR pipeline_create_info)
		{
			m_context->InitImGuiForDynamicRendering(pipeline_create_info);
		}

		// State
		void VulkanBackend::SetViewport(std::initializer_list<VkViewport> viewports) const
		{
			m_context->SetViewport(viewports);
		}

		void VulkanBackend::SetScissor(std::initializer_list<VkRect2D> scissors) const
		{
			m_context->SetScissor(scissors);
		}

		// Rendering
		void VulkanBackend::SetClearColor(VkClearValue color)
		{
			m_context->SetClearColor(color);
		}

		void VulkanBackend::SetExtent(VkExtent2D extent)
		{
			m_context->SetExtent(extent);
		}

		// Dynamic rendering
		void VulkanBackend::BeginDynamicRendering()
		{
			m_context->BeginDynamicRendering();
		}

		void VulkanBackend::EndDynamicRendering()
		{
			m_context->EndDynamicRendering();
		}

		void VulkanBackend::BeginDynamicRenderingWithAttachments(const DynamicRenderingInfo& info)
		{
			m_context->BeginDynamicRenderingWithAttachments(info);
		}

		void VulkanBackend::EndDynamicRenderingWithAttachments(const DynamicRenderingInfo& info)
		{
			m_context->EndDynamicRenderingWithAttachments(info);
		}

		void VulkanBackend::BeginRenderPassWithSwapchain(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const
		{
			m_context->BeginRenderPassWithSwapchain(render_pass, render_area, clear_values);
		}

		void VulkanBackend::BeginRenderPass(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const
		{
			m_context->BeginRenderPass(render_pass, render_area, clear_values);
		}

		void VulkanBackend::EndRenderPass() const
		{
			m_context->EndRenderPass();
		}

		// Renderpass
		VulkanRenderPass* VulkanBackend::CreateRenderPass(RenderPassInfo& info)
		{
			return m_context->CreateRenderPass(info);
		}

		void VulkanBackend::RecreateRenderPassFramebuffers(VulkanRenderPass* render_pass, RenderPassInfo& info)
		{
			m_context->RecreateRenderPass(render_pass, info);
		}

		void VulkanBackend::DestroyRenderPass(VulkanRenderPass* render_pass) const
		{
			m_context->DestroyRenderPass(render_pass);
		}

		// Pipeline
		VulkanPipeline* VulkanBackend::CreatePipeline(const PipelineCreateInfo& info, const ShaderStageInfo& shader_info)
		{
			return m_context->CreatePipeline(info, shader_info);
		}

		void VulkanBackend::DestroyPipeline(VulkanPipeline* pipeline) const
		{
			m_context->DestroyPipeline(pipeline);
		}

		void VulkanBackend::BindPipeline(VulkanPipeline* pipeline) const
		{
			m_context->BindPipeline(pipeline);
		}

		void VulkanBackend::BindPushConstants(VulkanPipeline* pipeline, ShaderStage stage, u32 offset, u32 size, void* data) const
		{
			return m_context->BindPushConstants(pipeline, stage, offset, size, data);
		}

		// Buffer
		VulkanBuffer* VulkanBackend::CreateVertexBufferEmpty(u32 size)
		{
			return m_context->CreateVertexBuffer(nullptr, size);
		}

		template HE_API VulkanBuffer* VulkanBackend::CreateVertexBuffer(VertexFormatBase* vertices, u32 vertices_count);
		template HE_API VulkanBuffer* VulkanBackend::CreateVertexBuffer(VertexFormatTangent* vertices, u32 vertices_count);
		template HE_API VulkanBuffer* VulkanBackend::CreateVertexBuffer(Particle* vertices, u32 vertices_count);
		template <typename T>
		VulkanBuffer* VulkanBackend::CreateVertexBuffer(T* vertices, u32 vertices_count)
		{
			return m_context->CreateVertexBuffer(static_cast<void*>(vertices), vertices_count * sizeof(T));
		}

		VulkanBuffer* VulkanBackend::CreateIndexBufferEmpty(u32 size)
		{
			return m_context->CreateIndexBuffer(nullptr, size);
		}

		template HE_API VulkanBuffer* VulkanBackend::CreateIndexBuffer(u32* indices, u32 indices_count);
		template <typename T>
		VulkanBuffer* VulkanBackend::CreateIndexBuffer(T* indices, u32 indices_count)
		{
			return m_context->CreateIndexBuffer(static_cast<void*>(indices), indices_count * sizeof(T));
		}

		VulkanUniformBuffer* hellengine::graphics::VulkanBackend::CreateUniformBufferMappedPersistent(u32 size)
		{
			return m_context->CreateUniformBufferMappedPersistent(size);
		}

		VulkanUniformBuffer* VulkanBackend::CreateUniformBufferMappedOnce(void*& data, u32 size, u32 offset)
		{
			return m_context->CreateUniformBufferMappedOnce(data, size, offset);
		}

		VulkanUniformBuffer* VulkanBackend::CreateDynamicUniformBuffer(void*& data, u32 data_size, u32 data_count)
		{
			return m_context->CreateDynamicUniformBuffer(data, data_size, data_count);
		}

		VulkanStorageBuffer* VulkanBackend::CreateStorageBuffer(void*& data, u32 size, u32 offset)
		{
			return m_context->CreateStorageBuffer(data, size, offset);
		}

		void VulkanBackend::UpdateVertexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size)
		{
			m_context->UpdateVertexBuffer(buffer, offset, data, size);
		}

		void VulkanBackend::UpdateIndexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size)
		{
			return m_context->UpdateIndexBuffer(buffer, offset, data, size);
		}

		void VulkanBackend::UpdateUniformBuffer(VulkanUniformBuffer* ubo, void* data, u32 size)
		{
			m_context->UpdateUniformBuffer(ubo, data, size);
		}

		void VulkanBackend::UpdateUniformBufferOnce(VulkanUniformBuffer* ubo, void* data, u32 size, u32 offset)
		{
			return m_context->UpdateUniformBufferOnce(ubo, data, size, offset);
		}

		void VulkanBackend::UpdateStorageBufferOnce(VulkanStorageBuffer* buffer, void* data, u32 size, u32 offset)
		{
			m_context->UpdateStorageBufferOnce(buffer, data, size, offset);
		}

		void VulkanBackend::DestroyBuffer(VulkanBuffer* buffer) const
		{
			m_context->DestroyBuffer(buffer);
		}

		void VulkanBackend::DestroyUniformBuffer(VulkanUniformBuffer* buffer) const
		{
			m_context->DestroyUniformBuffer(buffer);
		}

		void VulkanBackend::DestroyStorageBuffer(VulkanStorageBuffer* buffer) const
		{
			m_context->DestroyStorageBuffer(buffer);
		}

		void VulkanBackend::BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const
		{
			m_context->BindVertexBuffer(buffer, offset);
		}

		void VulkanBackend::BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const
		{
			m_context->BindIndexBuffer(buffer, offset);
		}

		// Descriptor
		void VulkanBackend::InitDescriptorPoolGrowable(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets)
		{
			m_context->InitDescriptorPoolGrowable(pool_sizes, max_sets);
		}

		VulkanDescriptorSet* VulkanBackend::CreateDescriptorSet(VulkanPipeline* pipeline, u32 set)
		{
			return m_context->CreateDescriptorSet(pipeline, set);
		}

		VulkanDescriptorSet* VulkanBackend::CreateDescriptorSetVariable(VulkanPipeline* pipeline, u32 set, std::vector<u32> count)
		{
			return m_context->CreateDescriptorSetVariable(pipeline, set, count);
		}

		void VulkanBackend::WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data)
		{
			m_context->WriteDescriptor(descriptor, data);
		}

		void VulkanBackend::WriteDescriptorVariable(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data, u32 count, u32 array_element)
		{
			m_context->WriteDescriptor(descriptor, data, count, array_element);
		}

		void VulkanBackend::BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor_set, u32 offsets_count, u32* offsets) const
		{
			m_context->BindDescriptorSet(pipeline, descriptor_set, offsets_count, offsets);
		}

		// Texture
		VulkanTexture2D* VulkanBackend::CreateTexture2D(const File& file)
		{
			return m_context->CreateTexture2D(file);
		}

		VulkanTexture2D* VulkanBackend::CreateTexture2D(VkFormat format, u32 width, u32 height)
		{
			return m_context->CreateTexture2D(format, width, height);
		}

		VulkanTexture2D* VulkanBackend::CreateTexture2D(VkFormat format, const void* data, i32 width, i32 height)
		{
			return m_context->CreateTexture2D(format, data, width, height);
		}

		VulkanTexture3D* VulkanBackend::CreateTexture3D(VkFormat format, const void* data, u32 width, u32 height, u32 depth)
		{
			return m_context->CreateTexture3D(format, data, width, height, depth);
		}

		VulkanTextureCubemap* VulkanBackend::CreateTextureCubemap(const File& file)
		{
			return m_context->CreateTextureCubemap(file);
		}

		VulkanTextureCubemap* VulkanBackend::CreateTextureCubemapArray(const File& file)
		{
			return m_context->CreateTextureCubemapArray(file);
		}

		void VulkanBackend::UpdateTexture(VulkanTexture* texture, const void* data)
		{
			m_context->UpdateTexture(texture, data);
		}

		void VulkanBackend::DestroyTexture(VulkanTexture* texture) const
		{
			m_context->DestroyTexture(texture);
		}

		void VulkanBackend::Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const
		{
			m_context->Draw(vertex_count, instance_count, first_vertex, first_instance);
		}

		// Draw
		void VulkanBackend::DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const
		{
			m_context->DrawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
		}

	} // namespace graphics

} // namespace hellengine