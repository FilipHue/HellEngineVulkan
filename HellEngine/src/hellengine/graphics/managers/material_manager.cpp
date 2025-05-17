#include "hepch.h"
#include "material_manager.h"

namespace hellengine
{

	namespace graphics
	{

		void MaterialManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;
		}

		void MaterialManager::Shutdown()
		{
			NO_OP;
		}

		MaterialManager* MaterialManager::GetInstance()
		{
			static MaterialManager instance;
			return &instance;
		}

	} // namespace graphics

} // namespace hellengine