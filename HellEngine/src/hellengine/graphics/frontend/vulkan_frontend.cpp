#include "hepch.h"
#include "vulkan_frontend.h"

namespace hellengine
{
	namespace graphics
	{

		void VulkanFrontend::Init(VulkanBackend* backend)
		{
			m_backend = backend;

			m_mesh_manager = MeshManager::GetInstance();
			m_mesh_manager->Init(m_backend);

			m_texture_manager = TextureManager::GetInstance();
			m_texture_manager->Init(m_backend);

		}
		void VulkanFrontend::Shutdown()
		{
			m_mesh_manager->Shutdown();
			m_texture_manager->Shutdown();
		}

		b8 VulkanFrontend::CreateMesh(std::string name, std::vector<VertexFormatBase> vertices, std::vector<u32> indices)
		{
			//return m_mesh_manager->CreateMesh(name, vertices, indices);
			return false;
		}

		void VulkanFrontend::DrawMesh(std::string name, u32 instance_count)
		{
			//m_mesh_manager->DrawMesh(name, instance_count);
		}

		VulkanTexture2D* VulkanFrontend::CreateTexture2D(std::string name, const File& file)
		{
			return m_texture_manager->CreateTexture2D(name, file);
		}

		VulkanTexture2D* VulkanFrontend::CreateTexture2D(std::string name, VkFormat format, u32 width, u32 height)
		{
			return m_texture_manager->CreateTexture2D(name, format, width, height);
		}

		VulkanTexture2D* VulkanFrontend::CreateTexture2D(std::string name, VkFormat format, const void* data, u32 width, u32 height)
		{
			return m_texture_manager->CreateTexture2D(name, format, data, width, height);
		}

		VulkanTextureCubemap* VulkanFrontend::CreateTextureCubemap(std::string name, const File& file)
		{
			return m_texture_manager->CreateTextureCubemap(name, file);
		}

		VulkanTextureCubemap* VulkanFrontend::CreateTextureCubemapArray(std::string name, const File& file)
		{
			return m_texture_manager->CreateTextureCubemapArray(name, file);
		}

		VulkanTexture2D* VulkanFrontend::GetTexture2D(std::string name)
		{
			return m_texture_manager->GetTexture2D(name);
		}

		VulkanTextureCubemap* VulkanFrontend::GetTextureCubemap(std::string name)
		{
			return m_texture_manager->GetTextureCubemap(name);
		}

		void VulkanFrontend::DestroyTexture2D(std::string name)
		{
			return m_texture_manager->DestroyTexture2D(name);
		}

	} // namespace graphics

} // namespace hellengine