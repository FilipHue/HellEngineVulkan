#include "hepch.h"
#include "asset_manager.h"

// Internal
#include <hellengine/math/core.h>
#include <hellengine/graphics/managers/texture_manager.h>
#include <hellengine/graphics/managers/model_manager.h>

namespace hellengine
{

	using namespace graphics;
	using namespace math;
	namespace resources
	{
		Model* AssetManager::LoadModel(const File& file)
		{
			HE_GRAPHICS_INFO("Loading asset: {0}", file.GetRelativePath());

			Assimp::Importer* importer = new Assimp::Importer();
			u32 import_options = aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;
			const aiScene* scene = importer->ReadFile(file.GetRelativePath(), import_options);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				HE_CORE_ERROR("\tAssimp error: {0}", importer->GetErrorString());
				EXIT(1);
			}

			ExtractTextures(scene, file);

			Model* model = new Model();
			ModelManager::GetInstance()->AddModel(model);

			ProcessNode(scene->mRootNode, scene, model, glm::mat4(1.0f), file);

			HE_GRAPHICS_INFO("\tLoaded");

			return model;
		}

		Texture2D AssetManager::LoadTexture2D(const File& file)
		{
			if (TextureManager::GetInstance()->GetTexture2D(file.GetName()) != nullptr)
			{
				Texture2D texture = TextureManager::GetInstance()->GetTexture2D(file.GetName());
				return texture;
			}

			return TextureManager::GetInstance()->CreateTexture2D(file.GetName(), file);
		}

		TextureCubemap AssetManager::LoadTextureCubemap(const File& file)
		{
			if (TextureManager::GetInstance()->GetTextureCubemap(file.GetName()) != nullptr)
			{
				TextureCubemap texture = TextureManager::GetInstance()->GetTextureCubemap(file.GetName());
				return texture;
			}

			return TextureManager::GetInstance()->CreateTextureCubemap(file.GetName(), file);
		}

		void AssetManager::ProcessNode(aiNode* node, const aiScene* scene, Model* model, const glm::mat4& parent_transform, const File& file)
		{
			glm::mat4 transform = parent_transform * glm::transpose(glm::make_mat4(&node->mTransformation.a1));
			for (u32 i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
				ProcessMesh(ai_mesh, scene, model, transform, file);
			}

			for (u32 i = 0; i < node->mNumChildren; i++)
			{
				ProcessNode(node->mChildren[i], scene, model, transform, file);
			}
		}

		void AssetManager::ProcessMesh(aiMesh* ai_mesh, const aiScene* scene, Model* model, const glm::mat4& transform, const File& file)
		{
			u32 first_vertex = static_cast<u32>(model->GetVertices().size());
			u32 first_index = static_cast<u32>(model->GetIndices().size());

			Mesh* mesh = new Mesh();
			mesh->SetVertexStart(first_vertex);
			mesh->SetFirstIndex(first_index);

			// Vertices
			for (u32 j = 0; j < ai_mesh->mNumVertices; j++)
			{
				RawVertexData vertex;

				glm::vec4 position(ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z, 1.0f);
				vertex.position = glm::vec3(transform * position);

				if (ai_mesh->HasVertexColors(0))
				{
					vertex.color = glm::vec4(ai_mesh->mColors[0][j].r, ai_mesh->mColors[0][j].g, ai_mesh->mColors[0][j].b, 1.0f);
				}
				else
				{
					vertex.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}

				if (ai_mesh->HasTextureCoords(0))
				{
					vertex.tex_coord = glm::vec2(ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y);
				}
				else
				{
					vertex.tex_coord = glm::vec2(0.0f, 0.0f);
				}

				if (ai_mesh->HasNormals())
				{
					vertex.normal = glm::normalize(glm::vec3(ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z));
				}
				else
				{
					vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				if (ai_mesh->HasTangentsAndBitangents())
				{
					vertex.tangent = glm::vec3(ai_mesh->mTangents[j].x, ai_mesh->mTangents[j].y, ai_mesh->mTangents[j].z);
					vertex.bitangent = glm::vec3(ai_mesh->mBitangents[j].x, ai_mesh->mBitangents[j].y, ai_mesh->mBitangents[j].z);
				}
				else
				{
					vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);
					vertex.bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
				}

				model->GetVertices().push_back(vertex);
			}

			// Indices
			for (u32 j = 0; j < ai_mesh->mNumFaces; j++)
			{
				aiFace face = ai_mesh->mFaces[j];
				for (u32 k = 0; k < face.mNumIndices; k++)
				{
					model->GetIndices().push_back(face.mIndices[k]);
				}
			}

			// Material
			aiMaterial* ai_material = scene->mMaterials[ai_mesh->mMaterialIndex];
			aiColor3D color(0.f, 0.f, 0.f);

			LitMaterial* material = new LitMaterial();

			ai_material->Get(AI_MATKEY_BASE_COLOR, color);
			material->color = glm::vec4(color.r, color.g, color.b, 1.0f);

			// Texture
			if (ai_material->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
			{
				aiString path;
				ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
				File texture_path = FileManager::ReadFile((file.GetRelativeDirectory() + "/" + std::string(path.C_Str())).c_str());

				material->texture_base_color = TextureManager::GetInstance()->GetTexture2D(texture_path.GetName());
			}

			mesh->SetIndexCount(static_cast<u32>(model->GetIndices().size()) - first_index);
			mesh->SetMaterial(material);

			mesh->SetModelMatrix(transform);

			model->AddMesh(mesh);
		}

		void AssetManager::ExtractTextures(const aiScene* scene, const File& file)
		{
			for (u32 i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* material = scene->mMaterials[i];
				for (u32 type = aiTextureType_DIFFUSE; type <= AI_TEXTURE_TYPE_MAX; type++)
				{
					ProcessMaterialTexture(material, (aiTextureType)type, file);
				}
			}
		}

		void AssetManager::ProcessMaterialTexture(aiMaterial* material, aiTextureType type, const File& file)
		{
			for (u32 i = 0; i < material->GetTextureCount(type); i++)
			{
				aiString path;
				if (material->GetTexture(type, i, &path) == AI_SUCCESS)
				{
					File texture_path = FileManager::ReadFile(file.GetRelativeDirectory() + "/" + std::string(path.C_Str()));
					TextureManager::GetInstance()->CreateTexture2D(texture_path.GetName(), texture_path);
				}
			}
		}

	} // namespace resources

} // namespace hellengine