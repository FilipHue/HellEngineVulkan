#include "hepch.h"
#include "context.h"

// Internal
#include <hellengine/core/window/window.h>

// TEMP
#include <hellengine/math/types.h>

namespace hellengine
{

	namespace graphics
	{

		VulkanContext::VulkanContext()
		{
			m_instance = VulkanInstance();
			m_surface = VulkanSurface();
			m_device = VulkanDevice();
			m_swapchain = VulkanSwapchain();

			m_current_frame = 0;
			m_frames_in_flight = MAX_FRAMES_IN_FLIGHT;

			m_frame_data.resize(m_frames_in_flight);

			m_render_data.depth_clear_value = { 1.0f, 0 };
		}

		VulkanContext::~VulkanContext()
		{
		}

		void VulkanContext::Init(core::Window* window)
		{
			m_instance.Create(window);
			m_surface.Create(m_instance, window);
			m_device.Create(m_instance, m_surface);

			m_swapchain.SetFramesInFlight(m_frames_in_flight);
			m_swapchain.Create(m_instance, m_device, m_surface, { window->GetWidth(), window->GetHeight() });
			m_swapchain.SetResizeCallback(HE_BIND_EVENTCALLBACK(VulkanContext::FramebufferResize));

			m_command_pool.Create(m_instance, m_device, m_device.GetGraphicsFamilyIndex());

			for (u32 i = 0; i < m_frames_in_flight; i++)
			{
				m_frame_data[i].command_buffer.Allocate(m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
				m_frame_data[i].image_available.Create(m_instance, m_device);
				m_frame_data[i].render_finished.Create(m_instance, m_device);
				m_frame_data[i].frame_in_flight.Create(m_instance, m_device, true);

				m_frame_data[i].image_index = 0;
			}
		}

		void VulkanContext::Shutdown()
		{
			m_device.WaitForIdle();

			for (u32 i = 0; i < m_frames_in_flight; i++)
			{
				m_frame_data[i].command_buffer.Free(m_device, m_command_pool);
				m_frame_data[i].image_available.Destroy(m_instance, m_device);
				m_frame_data[i].render_finished.Destroy(m_instance, m_device);
				m_frame_data[i].frame_in_flight.Destroy(m_instance, m_device);

				m_frame_data[i].image_index = 0;
			}

			m_descriptor_pool_growable.Destroy(m_instance, m_device);
			m_command_pool.Destroy(m_instance, m_device);
			m_swapchain.Destroy(m_instance, m_device);
			m_device.Destroy(m_instance);
			m_surface.Destroy(m_instance);
			m_instance.Destroy();
		}

		void VulkanContext::BeginFrame()
		{
			m_swapchain.AcquireNextImage(m_instance, m_device, m_surface, m_frame_data[m_current_frame]);

			VkImageMemoryBarrier image_memory_barrier = {};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.pNext = VK_NULL_HANDLE;
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			image_memory_barrier.image = m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index];
			image_memory_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			vkCmdPipelineBarrier(m_frame_data[m_current_frame].command_buffer.GetHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &image_memory_barrier);
		
			VulkanImage::Transition(m_device, m_command_pool, m_swapchain.GetDepthImage().GetHandle(), m_device.GetDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}

		void VulkanContext::SubmitFrame()
		{
			VkImageMemoryBarrier image_memory_barrier = {};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.pNext = VK_NULL_HANDLE;
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			image_memory_barrier.image = m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index];
			image_memory_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			vkCmdPipelineBarrier(m_frame_data[m_current_frame].command_buffer.GetHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &image_memory_barrier);

			m_swapchain.Submit(m_device, m_surface, m_frame_data[m_current_frame]);
		}

		void VulkanContext::EndFrame()
		{
			m_swapchain.Present(m_instance, m_device, m_surface, m_frame_data[m_current_frame]);

			m_current_frame = (m_current_frame + 1) % m_frames_in_flight;
		}

		void VulkanContext::DrawFrame()
		{
			if (!m_swapchain.AcquireNextImage(m_instance, m_device, m_surface, m_frame_data[m_current_frame]))
			{
				return;
			}

			VkImageMemoryBarrier image_memory_barrier = {};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.pNext = VK_NULL_HANDLE;
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			image_memory_barrier.image = m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index];
			image_memory_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			vkCmdPipelineBarrier(m_frame_data[m_current_frame].command_buffer.GetHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &image_memory_barrier);

			VulkanImage::Transition(m_device, m_command_pool, m_swapchain.GetDepthImage().GetHandle(), m_device.GetDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		
			m_record_callback();

			image_memory_barrier = {};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.pNext = VK_NULL_HANDLE;
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			image_memory_barrier.image = m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index];
			image_memory_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			vkCmdPipelineBarrier(m_frame_data[m_current_frame].command_buffer.GetHandle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &image_memory_barrier);

			m_swapchain.Submit(m_device, m_surface, m_frame_data[m_current_frame]);

			m_swapchain.Present(m_instance, m_device, m_surface, m_frame_data[m_current_frame]);

			m_current_frame = (m_current_frame + 1) % m_frames_in_flight;
		}

		void VulkanContext::OnFramebufferResize()
		{
			m_frame_data[m_current_frame].framebuffer_resized = true;
		}

		void VulkanContext::SetViewport(std::initializer_list<VkViewport> viewports) const
		{
			vkCmdSetViewport(m_frame_data[m_current_frame].command_buffer.GetHandle(), 0, (u32)viewports.size(), &*viewports.begin());
		}

		void VulkanContext::SetScissor(std::initializer_list<VkRect2D> scissors) const
		{
			vkCmdSetScissor(m_frame_data[m_current_frame].command_buffer.GetHandle(), 0, (u32)scissors.size(), &*scissors.begin());
		}

		void VulkanContext::SetClearColor(VkClearValue color)
		{
			m_render_data.clear_value = color;
		}

		void VulkanContext::SetExtent(VkExtent2D extent)
		{
			if (m_swapchain.GetExtent().width < extent.width || m_swapchain.GetExtent().height < extent.height)
			{
				FramebufferResize();
			}
			m_render_data.extent = extent;
		}

		void VulkanContext::BeginDynamicRendering()
		{
			VkRenderingAttachmentInfo attachment_info = {};
			attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			attachment_info.pNext = VK_NULL_HANDLE;
			attachment_info.imageView = m_swapchain.GetImageViews()[m_frame_data[m_current_frame].image_index];
			attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_info.clearValue = m_render_data.clear_value;

			VkRenderingAttachmentInfo depth_attachment_info = {};
			depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			depth_attachment_info.pNext = VK_NULL_HANDLE;
			depth_attachment_info.imageView = m_swapchain.GetDepthImage().GetImageView();
			depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment_info.clearValue = m_render_data.depth_clear_value;

			VkRenderingInfo rendering_info = {};
			rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
			rendering_info.pNext = VK_NULL_HANDLE;
			rendering_info.flags = 0;
			rendering_info.renderArea = { {0, 0}, m_render_data.extent };
			rendering_info.layerCount = 1;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments = &attachment_info;
			rendering_info.pDepthAttachment = &depth_attachment_info;

			vkCmdBeginRendering(m_frame_data[m_current_frame].command_buffer.GetHandle(), &rendering_info);
		}

		void VulkanContext::EndDynamicRendering()
		{
			vkCmdEndRendering(m_frame_data[m_current_frame].command_buffer.GetHandle());
		}

		VulkanPipeline* VulkanContext::CreatePipeline(const PipelineCreateInfo& info, const ShaderStageInfo& shader_info)
		{
			VulkanPipeline* pipeline = new VulkanPipeline();
			pipeline->Create(m_instance, m_device, m_swapchain, info, shader_info);

			return pipeline;
		}

		void VulkanContext::DestroyPipeline(VulkanPipeline* pipeline) const
		{
			m_device.WaitForIdle();

			pipeline->Destroy(m_instance, m_device);
		}

		void VulkanContext::BindPipeline(VulkanPipeline* pipeline) const
		{
			pipeline->Bind(m_device, m_frame_data[m_current_frame].command_buffer.GetHandle());
		}

		VulkanBuffer* VulkanContext::CreateVertexBuffer(void* data, u32 size)
		{
			std::array<u32, 2> queue_families = { m_device.GetGraphicsFamilyIndex(), m_device.GetTransferFamilyIndex() };

			VulkanBuffer stagging_buffer = VulkanBuffer();
			stagging_buffer = CreateStagingBuffer(size, data);

			VulkanBuffer* vertex_buffer = new VulkanBuffer();
			vertex_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, 2, queue_families.data());
		
			vertex_buffer->CopyFromBuffer(m_device, m_command_pool, stagging_buffer.GetHandle());
		
			stagging_buffer.Destroy(m_instance, m_device);

			return vertex_buffer;
		}

		VulkanBuffer* VulkanContext::CreateIndexBuffer(void* data, u32 size)
		{
			std::array<u32, 2> queue_families = { m_device.GetGraphicsFamilyIndex(), m_device.GetTransferFamilyIndex() };

			VulkanBuffer stagging_buffer = VulkanBuffer();
			stagging_buffer = CreateStagingBuffer(size, data);

			VulkanBuffer* index_buffer = new VulkanBuffer();
			index_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, 2, queue_families.data());

			index_buffer->CopyFromBuffer(m_device, m_command_pool, stagging_buffer.GetHandle());

			stagging_buffer.Destroy(m_instance, m_device);

			return index_buffer;
		}

		VulkanUniformBuffer* hellengine::graphics::VulkanContext::CreateUniformBuffer(u32 size)
		{
			VulkanUniformBuffer* uniform_buffer = new VulkanUniformBuffer();
			uniform_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			
			uniform_buffer->Map(m_device, size, 0, uniform_buffer->GetMappedMemory());

			return uniform_buffer;
		}

		VulkanUniformBuffer* VulkanContext::CreateDynamicUniformBuffer(void*& data, u32 data_size, u32 data_count)
		{
			u32 dynamic_alignment = data_size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			if (min_ubo_alignment > 0)
			{
				dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
			}

			HE_GRAPHICS_DEBUG("Min ubo alignment: {0}", min_ubo_alignment);
			HE_GRAPHICS_DEBUG("Dynamic alignment: {0}", dynamic_alignment);

			u32 size = data_count * dynamic_alignment;
			data = AlignedAllocation(size, dynamic_alignment);

			VulkanUniformBuffer* dynamic_buffer = new VulkanUniformBuffer();
			dynamic_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			dynamic_buffer->SetType(BufferType_UniformDynamic);
			dynamic_buffer->SetDynamicAlignment(dynamic_alignment);

			dynamic_buffer->Map(m_device, size, 0, dynamic_buffer->GetMappedMemory());

			return dynamic_buffer;
		}

		void VulkanContext::UpdateUniformBuffer(VulkanUniformBuffer* ubo, void* data, u32 size)
		{
			memcpy(ubo->GetMappedMemory(), data, size);

			// FOR MEMEORY COHERENCY
			VkMappedMemoryRange memory_range{};
			memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memory_range.memory = ubo->GetDeviceMemory();
			memory_range.offset = 0;
			memory_range.size = size;

			VK_CHECK(vkFlushMappedMemoryRanges(m_device.GetLogicalDevice(), 1, &memory_range));

			VkBufferMemoryBarrier bufferBarrier{};
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			bufferBarrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.buffer = ubo->GetHandle();
			bufferBarrier.offset = 0;
			bufferBarrier.size = VK_WHOLE_SIZE;

			// Add the barrier to the command buffer
			vkCmdPipelineBarrier(
				m_frame_data[m_current_frame].command_buffer.GetHandle(),
				VK_PIPELINE_STAGE_HOST_BIT,          // Source stage: Host writes
				VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, // Destination stage: Vertex shader reads
				0,                                   // Dependency flags
				0, nullptr,                          // No global memory barriers
				1, &bufferBarrier,                   // Buffer memory barrier
				0, nullptr                           // No image memory barriers
			);
		}

		void VulkanContext::DestroyBuffer(VulkanBuffer* buffer) const
		{
			m_device.WaitForIdle();

			buffer->Destroy(m_instance, m_device);
		}

		void VulkanContext::DestroyUniformBuffer(VulkanUniformBuffer* buffer) const
		{
			m_device.WaitForIdle();

			buffer->Destroy(m_instance, m_device);
		}

		void VulkanContext::BindVertexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const
		{
			VkBuffer buffer_handle = buffer->GetHandle();
			vkCmdBindVertexBuffers(m_frame_data[m_current_frame].command_buffer.GetHandle(), 0, 1, &buffer_handle, &offset);
		}

		void VulkanContext::BindIndexBuffer(VulkanBuffer* buffer, VkDeviceSize offset) const
		{
			VkBuffer buffer_handle = buffer->GetHandle();
			vkCmdBindIndexBuffer(m_frame_data[m_current_frame].command_buffer.GetHandle(), buffer_handle, offset, VK_INDEX_TYPE_UINT32);
		}

		VulkanDescriptorSet* VulkanContext::CreateDescriptorSet(VulkanPipeline* pipeline, u32 set)
		{
			VulkanDescriptorSet* descriptor_set = new VulkanDescriptorSet();
			descriptor_set->Create(m_instance, m_device, pipeline->GetDescriptorSetLayout(set), m_descriptor_pool_growable, set);

			return descriptor_set;
		}

		void VulkanContext::WriteDescriptor(VulkanDescriptorSet** descriptor, std::vector<DescriptorSetWriteData>& data)
		{
			(*descriptor)->SetExpectedWrites((u32)data.size());
			for (const auto& write_data : data)
			{
				if (ToVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || ToVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
				{
					(*descriptor)->WriteBuffer(write_data.data.buffer.buffer, write_data.data.buffer.offset, write_data.data.buffer.range, write_data.binding, ToVulkanDescriptorType(write_data.type));
				}
				else if (ToVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					(*descriptor)->WriteImage(write_data.data.image.image_view, write_data.data.image.sampler, write_data.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				}
			}
			(*descriptor)->Update(m_device);
		}

		void VulkanContext::BindDescriptorSet(VulkanPipeline* pipeline, VulkanDescriptorSet* descriptor, u32 offsets_count, u32* offsets)
		{
			VkDescriptorSet descriptors[] = { descriptor->GetHandle() };
			vkCmdBindDescriptorSets(m_frame_data[m_current_frame].command_buffer.GetHandle(), pipeline->GetBindPoint(), pipeline->GetLayout(), descriptor->GetSet(), 1, descriptors, offsets_count, offsets);
		}

		VulkanTexture* VulkanContext::CreateTexture2D(const char* path)
		{
			VulkanTexture* texture = new VulkanTexture();
			texture->Create(m_instance, m_device, m_command_pool, path);

			return texture;
		}

		void VulkanContext::DestroyTexture(VulkanTexture* texture) const
		{
			m_device.WaitForIdle();

			texture->Destroy(m_instance, m_device);
			delete texture;
		}

		void VulkanContext::DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) const
		{
			vkCmdDrawIndexed(m_frame_data[m_current_frame].command_buffer.GetHandle(), index_count, instance_count, first_index, vertex_offset, first_instance);
		}

		void VulkanContext::InitDescriptorPoolGrowable(const std::vector<DescriptorPoolSizeInfo>& pool_sizes, u32 max_sets)
		{
			std::vector<VkDescriptorPoolSize> pool_sizes_vk;
			for (const auto& pool_size : pool_sizes)
			{
				pool_sizes_vk.push_back({ ToVulkanDescriptorType(pool_size.type), pool_size.count});
			}
			m_descriptor_pool_growable.Create(m_instance, m_device, pool_sizes_vk, max_sets);
		}

		VulkanBuffer VulkanContext::CreateStagingBuffer(VkDeviceSize size, void* data) const
		{
			std::array<u32, 2> queue_families = { m_device.GetGraphicsFamilyIndex(), m_device.GetTransferFamilyIndex() };

			VulkanBuffer staging_buffer = VulkanBuffer();
			staging_buffer.Create(m_instance, m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true, 2, queue_families.data());
			staging_buffer.MapUnmap(m_device, size, 0, data);

			return staging_buffer;
		}

		void VulkanContext::FramebufferResize()
		{
			m_device.WaitForIdle();

			m_swapchain.Recreate(m_instance, m_device, m_surface);

			for (u32 i = 0; i < m_frames_in_flight; i++)
			{
				m_frame_data[i].command_buffer.Free(m_device, m_command_pool);
				m_frame_data[i].command_buffer.Allocate(m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

				m_frame_data[i].image_available.Destroy(m_instance, m_device);
				m_frame_data[i].image_available.Create(m_instance, m_device);

				m_frame_data[i].render_finished.Destroy(m_instance, m_device);
				m_frame_data[i].render_finished.Create(m_instance, m_device);
			}
		}

	} // namespace graphics

} // namespace hellengine