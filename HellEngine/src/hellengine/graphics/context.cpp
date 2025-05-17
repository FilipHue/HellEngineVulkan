#include "hepch.h"
#include "context.h"

// Internal
#include <hellengine/core/window/window.h>

// External
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

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
			NO_OP;
		}

		void VulkanContext::Init(Window* window)
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

		void VulkanContext::InitImGuiForRenderpass(VulkanRenderPass* renderpass)
		{
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.ApiVersion = VK_API_VERSION_1_3;
			init_info.Instance = m_instance.GetHandle();
			init_info.PhysicalDevice = m_device.GetPhysicalDevice();
			init_info.Device = m_device.GetLogicalDevice();
			init_info.QueueFamily = m_device.GetGraphicsFamilyIndex();
			init_info.Queue = m_device.GetGraphicsQueue();
			init_info.DescriptorPool = m_imgui_descriptor_pool.GetHandle();
			init_info.RenderPass = renderpass->GetHandle();
			init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
			init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&init_info);
		}

		void VulkanContext::InitImGuiForDynamicRendering(VkPipelineRenderingCreateInfoKHR pipeline_create_info)
		{
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.ApiVersion = VK_API_VERSION_1_3;
			init_info.Instance = m_instance.GetHandle();
			init_info.PhysicalDevice = m_device.GetPhysicalDevice();
			init_info.Device = m_device.GetLogicalDevice();
			init_info.QueueFamily = m_device.GetGraphicsFamilyIndex();
			init_info.Queue = m_device.GetGraphicsQueue();
			init_info.DescriptorPool = m_imgui_descriptor_pool.GetHandle();
			init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
			init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.UseDynamicRendering = true;
			init_info.PipelineRenderingCreateInfo = pipeline_create_info;
			ImGui_ImplVulkan_Init(&init_info);

			VulkanCommandBuffer command_buffer = VulkanCommandBuffer();
			command_buffer.Allocate(m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			command_buffer.BeginRecordingOneTime(m_device, m_command_pool);
			ImGui_ImplVulkan_CreateFontsTexture();
			command_buffer.EndRecordingOneTime(m_device, m_command_pool);
		}

		void VulkanContext::SubmitImGui()
		{
			VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index], { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, m_swapchain.GetDepthStencilImage().GetHandle(), { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			VkRenderingAttachmentInfo attachment_info = {};
			attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			attachment_info.pNext = VK_NULL_HANDLE;
			attachment_info.imageView = m_swapchain.GetImageViews()[m_frame_data[m_current_frame].image_index];
			attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_info.clearValue = m_render_data.clear_value;

			VkRenderingAttachmentInfo depth_attachment_info = {};
			depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			depth_attachment_info.pNext = VK_NULL_HANDLE;
			depth_attachment_info.imageView = m_swapchain.GetDepthStencilImage().GetImageView();
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
			rendering_info.viewMask = 0;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments = &attachment_info;
			rendering_info.pDepthAttachment = &depth_attachment_info;
			rendering_info.pStencilAttachment = nullptr;

			vkCmdBeginRendering(m_frame_data[m_current_frame].command_buffer.GetHandle(), &rendering_info);

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_frame_data[m_current_frame].command_buffer.GetHandle());

			EndDynamicRendering();
		}

		void VulkanContext::DrawFrame()
		{
			if (!m_swapchain.AcquireNextImage(m_instance, m_device, m_surface, m_frame_data[m_current_frame]))
			{
				return;
			}

			m_record_callback();

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
			VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index], { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, m_swapchain.GetDepthStencilImage().GetHandle(), { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT , 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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
			depth_attachment_info.imageView = m_swapchain.GetDepthStencilImage().GetImageView();
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
			rendering_info.viewMask = 0;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments = &attachment_info;
			rendering_info.pDepthAttachment = &depth_attachment_info;
			rendering_info.pStencilAttachment = nullptr;

			vkCmdBeginRendering(m_frame_data[m_current_frame].command_buffer.GetHandle(), &rendering_info);
		}

		void VulkanContext::EndDynamicRendering()
		{
			vkCmdEndRendering(m_frame_data[m_current_frame].command_buffer.GetHandle());

			VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, m_swapchain.GetImages()[m_frame_data[m_current_frame].image_index], { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}

		void VulkanContext::BeginDynamicRenderingWithAttachments(const DynamicRenderingInfo& info)
		{
			HE_ASSERT(info.color_attachments.size() > 0, "Dynamic rendering requires at least one color attachment!");

			std::vector<VkRenderingAttachmentInfo> color_attachments;
			for (u32 i = 0; i < info.color_attachments.size(); i++)
			{
				VkRenderingAttachmentInfo attachment_info = {};
				attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				attachment_info.pNext = VK_NULL_HANDLE;
				attachment_info.imageView = info.color_attachments[i].image_view;
				attachment_info.imageLayout = info.color_attachments[i].image_layout;
				attachment_info.loadOp = info.color_attachments[i].load_op;
				attachment_info.storeOp = info.color_attachments[i].store_op;
				attachment_info.clearValue = info.color_attachments[i].clear_value;
				color_attachments.push_back(attachment_info);
			}

			VkRenderingAttachmentInfo depth_attachment_info = {};
			if (info.depth_attachment.has_value())
			{
				depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
				depth_attachment_info.pNext = VK_NULL_HANDLE;
				depth_attachment_info.imageView = info.depth_attachment->image_view;
				depth_attachment_info.imageLayout = info.depth_attachment->image_layout;
				depth_attachment_info.loadOp = info.depth_attachment->load_op;
				depth_attachment_info.storeOp = info.depth_attachment->store_op;
				depth_attachment_info.clearValue = info.depth_attachment->clear_value;
			}
			
			VkRenderingInfo rendering_info = {};
			rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
			rendering_info.pNext = VK_NULL_HANDLE;
			rendering_info.flags = info.flags;
			rendering_info.renderArea = { {0, 0}, m_render_data.extent };
			rendering_info.layerCount = 1;
			rendering_info.viewMask = 0;	
			rendering_info.colorAttachmentCount = (u32)color_attachments.size();
			rendering_info.pColorAttachments = color_attachments.data();
			rendering_info.pDepthAttachment = info.depth_attachment.has_value() ? &depth_attachment_info : VK_NULL_HANDLE;

			for (u32 i = 0; i < info.color_attachments.size(); i++)
			{
				VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, info.color_attachments[i].image, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}

			if (info.depth_attachment.has_value())
			{
				VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, info.depth_attachment.value().image, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			}

			vkCmdBeginRendering(m_frame_data[m_current_frame].command_buffer.GetHandle(), &rendering_info);
		}

		void VulkanContext::EndDynamicRenderingWithAttachments(const DynamicRenderingInfo& info)
		{
			vkCmdEndRendering(m_frame_data[m_current_frame].command_buffer.GetHandle());

			for (u32 i = 0; i < info.color_attachments.size(); i++)
			{
				VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, info.color_attachments[i].image, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			if (info.depth_attachment.has_value())
			{
				VulkanImage::PrepareTransition(m_frame_data[m_current_frame].command_buffer, info.depth_attachment.value().image, { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}

		void VulkanContext::BeginRenderPassWithSwapchain(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const
		{
			VkRenderPassBeginInfo render_pass_info = {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = render_pass->GetHandle();
			render_pass_info.framebuffer = render_pass->GetFramebuffer(m_frame_data[m_current_frame].image_index).GetHandle();
			render_pass_info.renderArea = render_area;
			render_pass_info.clearValueCount = (u32)clear_values.size();
			render_pass_info.pClearValues = &*clear_values.begin();

			vkCmdBeginRenderPass(m_frame_data[m_current_frame].command_buffer.GetHandle(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void VulkanContext::BeginRenderPass(VulkanRenderPass* render_pass, VkRect2D render_area, std::initializer_list<VkClearValue> clear_values) const
		{
			VkRenderPassBeginInfo render_pass_info = {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = render_pass->GetHandle();
			render_pass_info.framebuffer = render_pass->GetFramebuffer(0).GetHandle();
			render_pass_info.renderArea = render_area;
			render_pass_info.clearValueCount = (u32)clear_values.size();
			render_pass_info.pClearValues = &*clear_values.begin();

			vkCmdBeginRenderPass(m_frame_data[m_current_frame].command_buffer.GetHandle(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void VulkanContext::EndRenderPass() const
		{
			vkCmdEndRenderPass(m_frame_data[m_current_frame].command_buffer.GetHandle());
		}

		VulkanRenderPass* VulkanContext::CreateRenderPass(RenderPassInfo& info)
		{
			VulkanRenderPass* render_pass = new VulkanRenderPass();
			render_pass->Create(m_instance, m_device, m_swapchain, info);

			return render_pass;
		}

		void VulkanContext::RecreateRenderPass(VulkanRenderPass* render_pass, RenderPassInfo& info)
		{
			m_device.WaitForIdle();

			render_pass->RecreateFramebuffers(m_instance, m_device, m_swapchain, info);
		}

		void VulkanContext::DestroyRenderPass(VulkanRenderPass* render_pass) const
		{
			m_device.WaitForIdle();

			render_pass->Destroy(m_instance, m_device);
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

		void VulkanContext::BindPushConstants(VulkanPipeline* pipeline, ShaderStage stage, u32 offset, u32 size, const void* data) const
		{
			vkCmdPushConstants(m_frame_data[m_current_frame].command_buffer.GetHandle(), pipeline->GetLayout(), stage, offset, size, data);
		}

		VulkanBuffer* VulkanContext::CreateVertexBuffer(void* data, u32 size)
		{
			std::array<u32, 2> queue_families = { m_device.GetGraphicsFamilyIndex(), m_device.GetTransferFamilyIndex() };

			VulkanBuffer* vertex_buffer = new VulkanBuffer();
			vertex_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, (u32)queue_families.size(), queue_families.data());
			vertex_buffer->SetType(BufferType_Vertex);

			if (data)
			{
				VulkanBuffer stagging_buffer = VulkanBuffer();
				stagging_buffer = CreateStagingBuffer(size, data);

				vertex_buffer->CopyFromBuffer(m_device, m_command_pool, stagging_buffer.GetHandle());
		
				stagging_buffer.Destroy(m_instance, m_device);
			}

			return vertex_buffer;
		}

		VulkanBuffer* VulkanContext::CreateIndexBuffer(void* data, u32 size)
		{
			std::array<u32, 2> queue_families = { m_device.GetGraphicsFamilyIndex(), m_device.GetTransferFamilyIndex() };

			VulkanBuffer* index_buffer = new VulkanBuffer();
			index_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, (u32)queue_families.size(), queue_families.data());
			index_buffer->SetType(BufferType_Index);

			if (data)
			{
				VulkanBuffer stagging_buffer = VulkanBuffer();
				stagging_buffer = CreateStagingBuffer(size, data);

				index_buffer->CopyFromBuffer(m_device, m_command_pool, stagging_buffer.GetHandle());

				stagging_buffer.Destroy(m_instance, m_device);
			}

			return index_buffer;
		}

		VulkanUniformBuffer* VulkanContext::CreateUniformBufferMappedPersistent(u32 size)
		{
			u32 dynamic_alignment = size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);

			VulkanUniformBuffer* uniform_buffer = new VulkanUniformBuffer();
			uniform_buffer->Create(m_instance, m_device, dynamic_alignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			uniform_buffer->SetDynamicAlignment(dynamic_alignment);
			uniform_buffer->SetType(BufferType_Uniform);

			uniform_buffer->Map(m_device, dynamic_alignment, 0, uniform_buffer->GetMappedMemory());

			return uniform_buffer;
		}

		VulkanUniformBuffer* VulkanContext::CreateUniformBufferMappedOnce(void*& data, u32 size, u32 offset)
		{
			u32 dynamic_alignment = size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);

			VulkanUniformBuffer* uniform_buffer = new VulkanUniformBuffer();
			uniform_buffer->Create(m_instance, m_device, dynamic_alignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			uniform_buffer->SetDynamicAlignment(dynamic_alignment);
			uniform_buffer->SetType(BufferType_Uniform);

			uniform_buffer->MapUnmap(m_device, dynamic_alignment - offset, offset, data);
			uniform_buffer->Map(m_device, dynamic_alignment, 0, uniform_buffer->GetMappedMemory());

			return uniform_buffer;
		}

		VulkanUniformBuffer* VulkanContext::CreateDynamicUniformBuffer(void*& data, u32 data_size, u32 data_count)
		{
			u32 dynamic_alignment = data_size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			if (min_ubo_alignment > 0)
			{
				dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);
			}

			u32 size = data_count * dynamic_alignment;
			data = AlignedAllocation(size, dynamic_alignment);

			VulkanUniformBuffer* dynamic_buffer = new VulkanUniformBuffer();
			dynamic_buffer->Create(m_instance, m_device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			dynamic_buffer->SetDynamicAlignment(dynamic_alignment);
			dynamic_buffer->SetType(BufferType_UniformDynamic);

			dynamic_buffer->Map(m_device, size, 0, dynamic_buffer->GetMappedMemory());

			return dynamic_buffer;
		}

		VulkanStorageBuffer* VulkanContext::CreateStorageBuffer(void*& data, u32 data_size, u32 offset)
		{
			u32 dynamic_alignment = data_size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);

			VulkanStorageBuffer* storage_buffer = new VulkanStorageBuffer();
			storage_buffer->Create(m_instance, m_device, dynamic_alignment, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			storage_buffer->SetDynamicAlignment(dynamic_alignment);
			storage_buffer->SetType(BufferType_Uniform);

			storage_buffer->MapUnmap(m_device, dynamic_alignment - offset, offset, data);
			storage_buffer->Map(m_device, dynamic_alignment, 0, storage_buffer->GetMappedMemory());

			return storage_buffer;
		}

		void VulkanContext::UpdateVertexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size)
		{
			if (size && size > buffer->GetSize())
			{
				HE_CORE_ERROR("Buffer size is too small for data");
				return;
			}

			VulkanBuffer stagging_buffer = CreateStagingBuffer(size, data);
			buffer->CopyFromBuffer(m_device, m_command_pool, stagging_buffer.GetHandle(), size, offset);
			stagging_buffer.Destroy(m_instance, m_device);
		}

		void VulkanContext::UpdateIndexBuffer(VulkanBuffer* buffer, u32 offset, void* data, u32 size)
		{
			if (size && size > buffer->GetSize())
			{
				HE_CORE_ERROR("Buffer size is too small for data");
				return;
			}

			VulkanBuffer stagging_buffer = CreateStagingBuffer(size, data);
			buffer->CopyFromBuffer(m_device, m_command_pool, stagging_buffer.GetHandle(), size, offset);
			stagging_buffer.Destroy(m_instance, m_device);
		}

		void VulkanContext::UpdateUniformBuffer(VulkanUniformBuffer* ubo, void* data, u32 size)
		{
			u32 dynamic_alignment = size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);

			memcpy(ubo->GetMappedMemory(), data, dynamic_alignment);

			// FOR MEMORY COHERENCY
			VkMappedMemoryRange memory_range{};
			memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memory_range.memory = ubo->GetDeviceMemory();
			memory_range.offset = 0;
			memory_range.size = dynamic_alignment;

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

		void VulkanContext::UpdateUniformBufferOnce(VulkanUniformBuffer* ubo, void* data, u32 size, u32 offset)
		{
			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			command_buffer.BeginRecordingOneTime(m_device, m_command_pool);

			u32 dynamic_alignment = size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);

			memcpy(ubo->GetMappedMemory(), data, dynamic_alignment);

			// FOR MEMORY COHERENCY
			VkMappedMemoryRange memory_range{};
			memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memory_range.memory = ubo->GetDeviceMemory();
			memory_range.offset = offset;
			memory_range.size = dynamic_alignment;

			VK_CHECK(vkFlushMappedMemoryRanges(m_device.GetLogicalDevice(), 1, &memory_range));

			VkBufferMemoryBarrier bufferBarrier{};
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			bufferBarrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.buffer = ubo->GetHandle();
			bufferBarrier.offset = offset;
			bufferBarrier.size = VK_WHOLE_SIZE;

			// Add the barrier to the command buffer
			vkCmdPipelineBarrier(
				command_buffer.GetHandle(),
				VK_PIPELINE_STAGE_HOST_BIT,          // Source stage: Host writes
				VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, // Destination stage: Vertex shader reads
				0,                                   // Dependency flags
				0, nullptr,                          // No global memory barriers
				1, &bufferBarrier,                   // Buffer memory barrier
				0, nullptr                           // No image memory barriers
			);

			command_buffer.EndRecordingOneTime(m_device, m_command_pool);

			command_buffer.Free(m_device, m_command_pool);
		}

		void VulkanContext::UpdateStorageBufferOnce(VulkanStorageBuffer* buffer, void* data, u32 size, u32 offset)
		{
			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			command_buffer.BeginRecordingOneTime(m_device, m_command_pool);

			u32 dynamic_alignment = size;
			u32 min_ubo_alignment = (u32)m_device.GetProperties().limits.minUniformBufferOffsetAlignment;
			dynamic_alignment = ALIGN(dynamic_alignment, min_ubo_alignment);
			memcpy(buffer->GetMappedMemory(), data, dynamic_alignment);

			// FOR MEMORY COHERENCY
			VkMappedMemoryRange memory_range{};
			memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memory_range.memory = buffer->GetDeviceMemory();
			memory_range.offset = offset;
			memory_range.size = dynamic_alignment;
			VK_CHECK(vkFlushMappedMemoryRanges(m_device.GetLogicalDevice(), 1, &memory_range));

			VkBufferMemoryBarrier bufferBarrier{};
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.buffer = buffer->GetHandle();
			bufferBarrier.offset = offset;
			bufferBarrier.size = VK_WHOLE_SIZE;

			// Add the barrier to the command buffer
			vkCmdPipelineBarrier(
				command_buffer.GetHandle(),
				VK_PIPELINE_STAGE_HOST_BIT,          // Source stage: Host writes
				VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, // Destination stage: Vertex shader reads
				0,                                   // Dependency flags
				0, nullptr,                          // No global memory barriers
				1, &bufferBarrier,                   // Buffer memory barrier
				0, nullptr                           // No image memory barriers
			);

			command_buffer.EndRecordingOneTime(m_device, m_command_pool);

			command_buffer.Free(m_device, m_command_pool);
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

		void VulkanContext::DestroyStorageBuffer(VulkanStorageBuffer* buffer) const
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
			(*descriptor)->Clear();
			(*descriptor)->SetExpectedWrites((u32)data.size());
			for (const auto& write_data : data)
			{
				if (GetVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || GetVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || GetVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				{
					(*descriptor)->WriteBuffer(write_data.data.buffer.buffer, write_data.data.buffer.offset, write_data.data.buffer.range, write_data.binding, GetVulkanDescriptorType(write_data.type));
				}
				else if (GetVulkanDescriptorType(write_data.type) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
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

		VulkanTexture2D* VulkanContext::CreateTexture2D(VkFormat format, i32 width, i32 height)
		{
			VulkanTexture2D* texture = new VulkanTexture2D();
			texture->Create(m_instance, m_device, m_command_pool, format, width, height);

			return texture;
		}

		VulkanTexture2D* VulkanContext::CreateTexture2D(const File& file)
		{
			VulkanTexture2D* texture = new VulkanTexture2D();

			if (file.GetExtension() == ".ktx")
			{
				texture->Create(m_instance, m_device, m_command_pool, VK_FORMAT_R8G8B8A8_UNORM, file.GetAbsolutePath().c_str());
				return texture;
			}

			texture->LoadSTBI(m_instance, m_device, m_command_pool, VK_FORMAT_R8G8B8A8_UNORM, file.GetAbsolutePath().c_str());
			return texture;
		}

		VulkanTexture2D* VulkanContext::CreateTexture2D(VkFormat format, const void* data, i32 width, i32 height)
		{
			VulkanTexture2D* texture = new VulkanTexture2D();
			texture->Create(m_instance, m_device, m_command_pool, format, data, width, height);

			return texture;
		}

		VulkanTexture3D* VulkanContext::CreateTexture3D(VkFormat format, const void* data, i32 width, i32 height, i32 depth)
		{
			VulkanTexture3D* texture = new VulkanTexture3D();
			texture->Create(m_instance, m_device, m_command_pool, format, data, width, height, depth);

			return texture;
		}

		VulkanTextureCubemap* VulkanContext::CreateTextureCubemap(const File& file)
		{
			VulkanTextureCubemap* texture = new VulkanTextureCubemap();
			texture->Create(m_instance, m_device, m_command_pool, VK_FORMAT_R8G8B8A8_UNORM, file.GetAbsolutePath().c_str());

			return texture;
		}

		VulkanTextureCubemap* VulkanContext::CreateTextureCubemapArray(const File& file)
		{
			VulkanTextureCubemap* texture = new VulkanTextureCubemap();
			texture->CreateArray(m_instance, m_device, m_command_pool, VK_FORMAT_R8G8B8A8_UNORM, file.GetAbsolutePath().c_str());

			return texture;
		}

		void VulkanContext::Update(VulkanTexture* texture, const void* data)
		{
			texture->Update(m_instance, m_device, m_command_pool, data);
		}

		void VulkanContext::DestroyTexture(VulkanTexture* texture) const
		{
			m_device.WaitForIdle();

			texture->Destroy(m_instance, m_device);
			delete texture;
		}

		void VulkanContext::Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const
		{
			vkCmdDraw(m_frame_data[m_current_frame].command_buffer.GetHandle(), vertex_count, instance_count, first_vertex, first_instance);
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
				pool_sizes_vk.push_back({ GetVulkanDescriptorType(pool_size.type), pool_size.count});
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

		void VulkanContext::CreateImGuiResources(Window* window)
		{
			// Create descriptor pool for ImGui
			std::vector<VkDescriptorPoolSize> pool_sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			m_imgui_descriptor_pool.Create(m_instance, m_device, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pool_sizes, 1000);
		}

		void VulkanContext::DestroyImGuiResources()
		{
			ImGui_ImplVulkan_Shutdown();

			m_imgui_descriptor_pool.Destroy(m_instance, m_device);
		}

	} // namespace graphics

} // namespace hellengine