#include "hepch.h"
#include "vulkan_surface.h"

// Internal
#include <hellengine/core/window/window.h>
#include <hellengine/platform/windows/windows_types.h>
#include <hellengine/graphics/objects/vulkan_instance.h>

namespace hellengine
{

	namespace graphics
	{

		VulkanSurface::VulkanSurface()
		{
			m_handle = VK_NULL_HANDLE;
		}

		VulkanSurface::~VulkanSurface()
		{
			NO_OP;
		}

		void VulkanSurface::Create(const VulkanInstance& instance, const core::Window* window)
		{
			HE_GRAPHICS_DEBUG("Creating vulkan surface...");
#if defined(HE_PLATFORM_WINDOWS)
			VkWin32SurfaceCreateInfoKHR create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.pNext = nullptr;
			create_info.flags = 0;
			create_info.hinstance = (((platform::WindowsInternalState*)(window->GetState().data))->instance_handle);
			create_info.hwnd = (((platform::WindowsInternalState*)(window->GetState().data))->window_handle);

			VK_CHECK(vkCreateWin32SurfaceKHR(instance.GetHandle(), &create_info, instance.GetAllocator(), &m_handle));
#endif
			HE_GRAPHICS_DEBUG("Vulkan surface created successfully");
		}

		void VulkanSurface::Destroy(const VulkanInstance& instance)
		{
			HE_GRAPHICS_DEBUG("Destroying vulkan surface...");
			vkDestroySurfaceKHR(instance.GetHandle(), m_handle, instance.GetAllocator());

			m_handle = VK_NULL_HANDLE;
			HE_GRAPHICS_DEBUG("Vulkan surface destroyed successfully");
		}

	} // namespace graphics

} // namespace hellengine