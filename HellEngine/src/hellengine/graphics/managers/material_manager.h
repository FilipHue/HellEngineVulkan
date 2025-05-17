#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>

namespace hellengine
{

	namespace graphics
	{

		class MaterialManager
		{
		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			VulkanBackend* GetBackend() { return m_backend; }

			static MaterialManager* GetInstance();

		private:
			VulkanBackend* m_backend = nullptr;
		};

	} // namespace graphics

} // namespace hellengine