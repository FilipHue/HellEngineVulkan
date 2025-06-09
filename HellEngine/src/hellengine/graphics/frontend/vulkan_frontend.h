#pragma once

// Internal
#include <hellengine/graphics/managers/material_manager.h>
#include <hellengine/graphics/managers/mesh_manager.h>
#include <hellengine/graphics/managers/model_manager.h>
#include <hellengine/graphics/managers/texture_manager.h>

namespace hellengine
{

	namespace graphics
	{

		class VulkanFrontend
		{
		public:
			VulkanFrontend() = default;
			~VulkanFrontend() = default;

			HE_API void Init(VulkanBackend* backend);
			HE_API void Shutdown();

			// Mesh
			HE_API b8 CreateMesh(std::string name, std::vector<VertexFormatBase> vertices, std::vector<u32> indices);
			HE_API void DrawMesh(std::string name, u32 instance_count = 1);

			// Texture
			HE_API VulkanTexture2D* CreateTexture2D(std::string name, const File& file);
			HE_API VulkanTexture2D* CreateTexture2D(std::string name, VkFormat format, u32 width, u32 height);
			HE_API VulkanTexture2D* CreateTexture2D(std::string name, VkFormat format, const void* data, u32 width, u32 height);

			HE_API VulkanTextureCubemap* CreateTextureCubemap(std::string name, const File& file);
			HE_API VulkanTextureCubemap* CreateTextureCubemapArray(std::string name, const File& file);

			HE_API VulkanTexture2D* GetTexture2D(std::string name);
			HE_API VulkanTextureCubemap* GetTextureCubemap(std::string name);

			HE_API void DestroyTexture2D(std::string name);

		private:
			VulkanBackend* m_backend;

			MaterialManager* m_material_manager;
			MeshManager* m_mesh_manager;
			ModelManager* m_model_manager;
			TextureManager* m_texture_manager;
		};

	} // namespace graphics

} // namespace hellengine