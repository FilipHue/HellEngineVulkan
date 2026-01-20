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
			HE_API static Texture2D LoadTexture2D(const File& file);
			HE_API static TextureCubemap LoadTextureCubemap(const File& file);

		private:
			static void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parent_transform, Entity parent_entity, const File& file);
			static void ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform, Entity parent_entity, const File& file);

			static void ExtractTextures(const aiScene* scene, const File& file);
			static void ProcessMaterialTexture(aiMaterial* material, aiTextureType type, const File& file);
		};

		static TextureType GetTextureType(aiTextureType type);

	} // namespace resources

} // namespace hellengine