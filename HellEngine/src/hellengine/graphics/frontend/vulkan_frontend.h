#pragma once

// Internal
#include <hellengine/graphics/backend/vulkan_backend.h>

namespace hellengine
{

	namespace graphics
	{

		class VulkanFrontend
		{
		public:
			VulkanFrontend() = default;
			~VulkanFrontend() = default;

			HE_API void Init(core::Window* window);
			HE_API void Shutdown();

			HE_API const VulkanBackend* GetBackend() const { return m_backend; }
			HE_API void SetBackend(VulkanBackend* backend);

		private:
			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine