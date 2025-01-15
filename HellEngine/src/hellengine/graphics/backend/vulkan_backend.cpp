#include "hepch.h"
#include "vulkan_backend.h"

// Internal
#include <hellengine/core/window/window.h>

namespace hellengine
{

	namespace graphics
	{

		void VulkanBackend::Init(core::Window* window)
		{
			m_context = new VulkanContext();
			m_context->Init(window);
		}

		void VulkanBackend::Shutdown()
		{
			m_context->Shutdown();
		}

		void VulkanBackend::BeginFrame()
		{
			m_context->BeginFrame();
		}

		void VulkanBackend::SubmitFrame()
		{
			m_context->SubmitFrame();
		}

		void VulkanBackend::EndFrame()
		{
			m_context->EndFrame();
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

		void VulkanBackend::SetViewport(std::initializer_list<VkViewport> viewports) const
		{
			m_context->SetViewport(viewports);
		}

		void VulkanBackend::SetScissor(std::initializer_list<VkRect2D> scissors) const
		{
			m_context->SetScissor(scissors);
		}

		void VulkanBackend::SetClearColor(VkClearValue color)
		{
			m_context->SetClearColor(color);
		}

		void VulkanBackend::SetExtent(VkExtent2D extent)
		{
			m_context->SetExtent(extent);
		}

		void VulkanBackend::BeginDynamicRendering()
		{
			m_context->BeginDynamicRendering();
		}

		void VulkanBackend::EndDynamicRendering()
		{
			m_context->EndDynamicRendering();
		}

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

		VulkanBuffer* VulkanBackend::CreateVertexBuffer(math::Vertex* vertices, u32 vertices_count)
		{
			return m_context->CreateVertexBuffer(vertices, vertices_count * sizeof(math::Vertex));
		}

		VulkanBuffer* VulkanBackend::CreateIndexBuffer(u32* indices, u32 indices_count)
		{
			return m_context->CreateIndexBuffer(indices, indices_count * sizeof(u32));
		}

		VulkanUniformBuffer* hellengine::graphics::VulkanBackend::CreateUniformBuffer(u32 size)
		{
			return m_context->CreateUniformBuffer(size);
		}

		VulkanUniformBuffer* VulkanBackend::CreateDynamicUniformBuffer(void*& data, u32 data_size, u32 data_count)
		{
			return m_context->CreateDynamicUniformBuffer(data, data_size, data_count);
		}

		void VulkanBackend::UpdateUniformBuffer(VulkanUniformBuffer* ubo, void* data, u32 size)
		{
			m_context->UpdateUniformBuffer(ubo, data, size);
		}

		void VulkanBackend::DestroyBuffer(VulkanBuffer* buffer) const
		{
			m_context->DestroyBuffer(buffer);
		}

		void VulkanBackend::DestroyUniformBuffer(VulkanUniformBuffer* buffer) const
		{
			m_context->DestroyUniformBuffer(buffer);
		}

		void VulkanBackend::BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const
		{
			m_context->BindVertexBuffer(buffer, offset);
		}

		void VulkanBackend::BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const
		{
			m_context->BindIndexBuffer(buffer, offset);
		}

		void VulkanBackend::InitDescriptorPool(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets)
		{
			m_context->InitDescriptorPoolGrowable(pool_sizes, max_sets);
		}

		VulkanDescriptorSet* VulkanBackend::CreateDescriptorSet(VulkanPipeline* pipeline, u32 set)
		{
			return m_context->CreateDescriptorSet(pipeline, set);
		}

		void VulkanBackend::WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data)
		{
			m_context->WriteDescriptor(descriptor, data);
		}

		void VulkanBackend::BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor_set, u32 offsets_count, u32* offsets) const
		{
			m_context->BindDescriptorSet(pipeline, descriptor_set, offsets_count, offsets);
		}

		VulkanTexture* VulkanBackend::CreateTexture2D(const char* path)
		{
			return m_context->CreateTexture2D(path);
		}

		void VulkanBackend::DestroyTexture(VulkanTexture* texture) const
		{
			m_context->DestroyTexture(texture);
		}

		void VulkanBackend::DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const
		{
			m_context->DrawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
		}

	} // namespace graphics

} // namespace hellengine