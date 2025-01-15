#include "hepch.h"
#include "vulkan_frontend.h"

namespace hellengine
{
	namespace graphics
	{

		void VulkanFrontend::Init(core::Window* window)
		{
			m_backend = new VulkanBackend();
			m_backend->Init(window);
		}
		void VulkanFrontend::Shutdown()
		{
			m_backend->Shutdown();
			delete m_backend;
		}

		void VulkanFrontend::SetBackend(VulkanBackend* backend)
		{
			if (m_backend)
			{
				m_backend->Shutdown();
				delete m_backend;
			}

			m_backend = backend;
		}

	} // namespace graphics

} // namespace hellengine