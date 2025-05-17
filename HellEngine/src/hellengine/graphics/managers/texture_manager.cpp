#include "hepch.h"
#include "texture_manager.h"

namespace hellengine
{

	namespace graphics
	{

		void TextureManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;

			const u32 white_texture_data = 0xffffffff;
			CreateTexture2D(DEFAULT_WHITE_TEXTURE, VK_FORMAT_B8G8R8A8_SRGB, &white_texture_data, 1, 1);

			const u32 error_texture_data = 0xffff00ff;
			CreateTexture2D(DEFAULT_ERROR_TEXTURE, VK_FORMAT_B8G8R8A8_SRGB, &error_texture_data, 1, 1);
		}

		void TextureManager::Shutdown()
		{
			for (auto& texture : m_textures_2d)
			{
				m_backend->DestroyTexture(texture.second);
			}

			for (auto& texture : m_textures_cubemap)
			{
				m_backend->DestroyTexture(texture.second);
			}
		}

		VulkanTexture2D* TextureManager::CreateTexture2D(std::string name, const File& file)
		{
			
			if (m_textures_2d.find(name) != m_textures_2d.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return m_textures_2d[DEFAULT_ERROR_TEXTURE];
			}

			VulkanTexture2D* texture = m_backend->CreateTexture2D(file);
			m_textures_2d[name] = texture;

			return texture;
		}

		VulkanTexture2D* TextureManager::CreateTexture2D(std::string name, VkFormat format, const void* data, i32 width, i32 height)
		{

			if (m_textures_2d.find(name) != m_textures_2d.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return m_textures_2d[DEFAULT_ERROR_TEXTURE];
			}

			VulkanTexture2D* texture = m_backend->CreateTexture2D(format, data, width, height);
			m_textures_2d[name] = texture;

			return texture;
		}

		VulkanTextureCubemap* TextureManager::CreateTextureCubemap(std::string name, const File& file)
		{
			if (m_textures_2d.find(name) != m_textures_2d.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return nullptr;
			}

			VulkanTextureCubemap* texture = m_backend->CreateTextureCubemap(file);
			m_textures_cubemap[name] = texture;
			return texture;
		}

		VulkanTextureCubemap* TextureManager::CreateTextureCubemapArray(std::string name, const File& file)
		{
			if (m_textures_2d.find(name) != m_textures_2d.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return nullptr;
			}

			VulkanTextureCubemap* texture = m_backend->CreateTextureCubemapArray(file);
			m_textures_cubemap[name] = texture;
			return texture;
		}

		VulkanTexture2D* TextureManager::GetTexture2D(std::string name)
		{
			return (m_textures_2d.find(name) != m_textures_2d.end()) ? m_textures_2d[name] : nullptr;
		}

		VulkanTextureCubemap* TextureManager::GetTextureCubemap(std::string name)
		{
			return (m_textures_cubemap.find(name) != m_textures_cubemap.end()) ? m_textures_cubemap[name] : nullptr;
		}

		TextureManager* TextureManager::GetInstance()
		{
			static TextureManager instance;

			return &instance;
		}

	} // namespace graphics

} // namespace hellengine