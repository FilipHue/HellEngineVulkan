#pragma once

// Internal
#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/graphics/mesh/model.h>

namespace hellengine
{

	namespace graphics
	{

		class ModelManager
		{
		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			void AddModel(Model* model);

			static ModelManager* GetInstance();

		private:
			std::vector<Model*> m_models;

			VulkanBackend* m_backend;
		};

 	} // namespace graphics

} // namespace hellengine