#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/resources/file_manager.h>

// STD
#include <queue>

constexpr auto DEFAULT_WHITE_TEXTURE = "DEFAULT_WHITE_TEXTURE";
constexpr auto DEFAULT_ERROR_TEXTURE = "DEFAULT_ERROR_TEXTURE";

namespace hellengine
{

	using namespace core;
	using namespace resources;
	namespace graphics
	{

		class MeshManager;

		class TextureManager
		{
		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			VulkanTexture2D* CreateTexture2D(std::string name, const File& file);
			VulkanTexture2D* CreateTexture2D(std::string name, VkFormat format, u32 width, u32 height);
			VulkanTexture2D* CreateTexture2D(std::string name, VkFormat format, const void* data, u32 width, u32 height);

			VulkanTextureCubemap* CreateTextureCubemap(std::string name, const File& file);
			VulkanTextureCubemap* CreateTextureCubemapArray(std::string name, const File& file);

			VulkanTexture2D* GetTexture2D(std::string name);
			VulkanTextureCubemap* GetTextureCubemap(std::string name);

			u32 GetTexture2DIndex(std::string name);

			void DestroyTexture2D(std::string name);

			static TextureManager* GetInstance();

		private:
			friend class MeshManager;

			std::vector<VulkanTexture2D*> m_textures_2d_vector;
			std::unordered_map<std::string, u32> m_textures_2d_index_map;
			std::queue<u32> m_textures_2d_free_indices;

			std::unordered_map<std::string, VulkanTextureCubemap*> m_textures_cubemap_map;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine