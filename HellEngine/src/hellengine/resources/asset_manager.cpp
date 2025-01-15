#include "hepch.h"
#include "asset_manager.h"

// Internal
#include <hellengine/math/core.h>

// External
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace hellengine
{

	using namespace graphics;
	namespace resources
	{

		AssetManager::AssetManager()
		{
		}

		AssetManager::~AssetManager()
		{
		}

		void AssetManager::LoadAsset(File& file)
		{
			Assimp::Importer* importer = new Assimp::Importer();
			const aiScene* scene = importer->ReadFile(file.GetRelativePath(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				HE_CORE_ERROR("Assimp error: {0}", importer->GetErrorString());
				return;
			}
			HE_CORE_DEBUG("Loaded asset: {0}", file.relative_file_path);

			for (unsigned int i = 0; i < scene->mNumMeshes; i++)
			{
				aiMesh* ai_mesh = scene->mMeshes[i];
				
				// Vertices
				std::vector<math::Vertex> vertices;
				for (unsigned int j = 0; j < ai_mesh->mNumVertices; j++)
				{
					math::Vertex vertex;
					vertex.position = glm::vec3(ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z);
					vertex.normal = glm::vec3(ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z);
					if (ai_mesh->mTextureCoords[0])
						vertex.tex_coord = glm::vec2(ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y);
					else
						vertex.tex_coord = glm::vec2(0.0f, 0.0f);
					vertices.push_back(vertex);
				}

				// Indices
				std::vector<unsigned int> indices;
				for (unsigned int j = 0; j < ai_mesh->mNumFaces; j++)
				{
					aiFace face = ai_mesh->mFaces[j];
					for (unsigned int k = 0; k < face.mNumIndices; k++)
						indices.push_back(face.mIndices[k]);
				}
			}
		}

	} // namespace resources

} // namespace hellengine