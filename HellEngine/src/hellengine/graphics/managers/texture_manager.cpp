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
			for (auto& texture : m_textures_2d_vector)
			{
				m_backend->DestroyTexture(texture);
			}

			for (auto& texture : m_textures_cubemap_map)
			{
				m_backend->DestroyTexture(texture.second);
			}
		}

		VulkanTexture2D* TextureManager::CreateTexture2D(std::string name, const File& file)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return m_textures_2d_vector[m_textures_2d_index_map[name]];
			}

			VulkanTexture2D* texture = m_backend->CreateTexture2D(file);
			
			u32 index = 0;
			if (!m_textures_2d_free_indices.empty())
			{
				index = m_textures_2d_free_indices.front();
				m_textures_2d_free_indices.pop();
				m_textures_2d_vector[index] = texture;
			}
			else
			{
				index = (u32)m_textures_2d_vector.size();
				m_textures_2d_vector.push_back(texture);
			}

			m_textures_2d_index_map[name] = index;

			return texture;
		}

		VulkanTexture2D* TextureManager::CreateTexture2D(std::string name, VkFormat format, u32 width, u32 height)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return m_textures_2d_vector[m_textures_2d_index_map[name]];
			}

			VulkanTexture2D* texture = m_backend->CreateTexture2D(format, width, height);

			u32 index = 0;
			if (!m_textures_2d_free_indices.empty())
			{
				index = m_textures_2d_free_indices.front();
				m_textures_2d_free_indices.pop();
				m_textures_2d_vector[index] = texture;
			}
			else
			{
				index = (u32)m_textures_2d_vector.size();
				m_textures_2d_vector.push_back(texture);
			}

			m_textures_2d_index_map[name] = index;

			return texture;
		}

		VulkanTexture2D* TextureManager::CreateTexture2D(std::string name, VkFormat format, const void* data, u32 width, u32 height)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return m_textures_2d_vector[m_textures_2d_index_map[name]];
			}

			VulkanTexture2D* texture = m_backend->CreateTexture2D(format, data, width, height);

			u32 index = 0;
			if (!m_textures_2d_free_indices.empty())
			{
				index = m_textures_2d_free_indices.front();
				m_textures_2d_free_indices.pop();
				m_textures_2d_vector[index] = texture;
			}
			else
			{
				index = (u32)m_textures_2d_vector.size();
				m_textures_2d_vector.push_back(texture);
			}

			m_textures_2d_index_map[name] = index;

			return texture;
		}

		VulkanTextureCubemap* TextureManager::CreateTextureCubemap(std::string name, const File& file)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return nullptr;
			}

			VulkanTextureCubemap* texture = m_backend->CreateTextureCubemap(file);
			m_textures_cubemap_map[name] = texture;
			return texture;
		}

		VulkanTextureCubemap* TextureManager::CreateTextureCubemapArray(std::string name, const File& file)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				HE_GRAPHICS_DEBUG("Texture with name {0} already exists", name);

				return nullptr;
			}

			VulkanTextureCubemap* texture = m_backend->CreateTextureCubemapArray(file);
			m_textures_cubemap_map[name] = texture;
			return texture;
		}

		VulkanTexture2D* TextureManager::GetTexture2D(std::string name)
		{
			return (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end()) ? m_textures_2d_vector[m_textures_2d_index_map[name]] : nullptr;
		}

		VulkanTextureCubemap* TextureManager::GetTextureCubemap(std::string name)
		{
			return (m_textures_cubemap_map.find(name) != m_textures_cubemap_map.end()) ? m_textures_cubemap_map[name] : nullptr;
		}

		u32 TextureManager::GetTexture2DIndex(std::string name)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				return m_textures_2d_index_map[name];
			}
			else
			{
				HE_GRAPHICS_ERROR("Texture with name {0} does not exist!", name);
				return 0;
			}
		}

		void TextureManager::DestroyTexture2D(std::string name)
		{
			if (m_textures_2d_index_map.find(name) != m_textures_2d_index_map.end())
			{
				m_backend->DestroyTexture(m_textures_2d_vector[m_textures_2d_index_map[name]]);
				m_textures_2d_free_indices.push(m_textures_2d_index_map[name]);
				m_textures_2d_vector[m_textures_2d_index_map[name]] = nullptr;
				m_textures_2d_index_map.erase(name);
			}
		}

		TextureManager* TextureManager::GetInstance()
		{
			static TextureManager instance;

			return &instance;
		}

	} // namespace graphics

} // namespace hellengine