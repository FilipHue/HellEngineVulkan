#pragma once

// Internal
#include "vulkan_debugger.h"

namespace hellengine
{

	namespace core
	{
		class Window;
	}

	using namespace core;
	namespace graphics
	{

		class VulkanInstance
		{
		public:
			VulkanInstance();
			~VulkanInstance();

			void Create(Window* window);
			void Destroy();

			const VkInstance GetHandle() const { return m_handle; }
			const VkAllocationCallbacks* GetAllocator() const { return m_allocator; }
#if defined(HE_DEBUG)
			const VulkanDebugger GetDebugger() const { return m_debugger; }
#endif

		private:
			VkInstance m_handle;
			VkAllocationCallbacks* m_allocator;
#if defined(HE_DEBUG)
			VulkanDebugger m_debugger;
#endif
		};

		static std::vector<const char*> GetVulkanExtensions();
		static b8 ValidateVulkanExtensions(std::vector<const char*> required_extensions);
		static std::vector<const char*> GetVulkanLayers();
		static b8 ValidateVulkanLayers(std::vector<const char*> required_layers);

	} // namespace graphics

} // namespace hellengine