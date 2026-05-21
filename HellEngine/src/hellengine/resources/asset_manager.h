#pragma once

// Internal
#include <hellengine/graphics/graphic_types.h> 
#include <hellengine/resources/file_manager.h>
#include <hellengine/graphics/managers/mesh_manager.h>
#include <hellengine/ecs/entity/entity.h>

// External
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace hellengine
{
	using namespace graphics;
	using namespace ecs;
	namespace resources
	{

		class AssetManager
		{
		public:
			HE_API static void LoadModel(const File& file);
			HE_API static Texture2D* LoadTexture2D(const File& file);
			HE_API static TextureCubemap* LoadTextureCubemap(const File& file);

			HE_API static std::vector<Mesh*> GetLoadedRootMeshes() { return s_loaded_root_meshes; }
			HE_API static std::vector<Mesh*> GetAllLoadedMeshes() { return s_all_loaded_meshes; }
			HE_API static std::string GetLastLoadedFilename() { return s_last_loaded_filename; }
			HE_API static void DebugPrintMeshHierarchy();

		private:
			static void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parent_transform, Mesh* parent_mesh, const File& file);
			static void ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform, Mesh* parent_mesh, const File& file);

			static void ExtractTextures(const aiScene* scene, const File& file);
			static void ProcessMaterialTexture(aiMaterial* material, aiTextureType type, const File& file);

			static std::vector<Mesh*> s_all_loaded_meshes;
			static std::vector<Mesh*> s_loaded_root_meshes;
			static std::string s_last_loaded_filename;
		};

		static TextureType GetTextureType(aiTextureType type);

	} // namespace resources

} // namespace hellengine