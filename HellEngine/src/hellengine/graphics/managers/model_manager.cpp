#include "hepch.h"
#include "model_manager.h"

namespace hellengine
{

	namespace graphics
	{

		void ModelManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;
		}

		void ModelManager::Shutdown()
		{
			for (auto& model : m_models)
			{
				delete model;
			}
		}

		void ModelManager::AddModel(Model* model)
		{
			m_models.push_back(model);
		}

		ModelManager* ModelManager::GetInstance()
		{
			static ModelManager instance;

			return &instance;
		}

	} // namespace graphics

} // namespace hellengine