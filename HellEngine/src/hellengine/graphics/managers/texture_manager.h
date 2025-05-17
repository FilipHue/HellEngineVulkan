#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/resources/file_manager.h>

constexpr auto DEFAULT_WHITE_TEXTURE = "DEFAULT_WHITE_TEXTURE";
constexpr auto DEFAULT_ERROR_TEXTURE = "DEFAULT_ERROR_TEXTURE";

namespace hellengine
{

	using namespace core;
	using namespace resources;
	namespace graphics
	{

		class TextureManager
		{
		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			VulkanTexture2D* CreateTexture2D(std::string name, const File& file);
			VulkanTexture2D* CreateTexture2D(std::string name, VkFormat format, const void* data, i32 width, i32 height);

			VulkanTextureCubemap* CreateTextureCubemap(std::string name, const File& file);
			VulkanTextureCubemap* CreateTextureCubemapArray(std::string name, const File& file);

			VulkanTexture2D* GetTexture2D(std::string name);
			VulkanTextureCubemap* GetTextureCubemap(std::string name);

			static TextureManager* GetInstance();

		private:
			std::unordered_map<std::string, VulkanTexture2D*> m_textures_2d;
			std::unordered_map<std::string, VulkanTextureCubemap*> m_textures_cubemap;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine