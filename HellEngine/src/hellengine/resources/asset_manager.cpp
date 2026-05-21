#include "hepch.h"
#include "asset_manager.h"

// Internal
#include <hellengine/math/core.h>
#include <hellengine/graphics/managers/texture_manager.h>
#include <hellengine/ecs/scene/scene_manager.h>

namespace hellengine
{

	using namespace graphics;
	using namespace math;
	using namespace ecs;
	namespace resources
	{
		void AssetManager::LoadModel(const File& file)
		{
			HE_GRAPHICS_INFO("Loading asset: {0}", file.GetRelativePath());

			// Clear previous meshes
			s_loaded_root_meshes.clear();
			s_all_loaded_meshes.clear();

			// Store the filename for later use
			s_last_loaded_filename = file.GetStem();

			Assimp::Importer* importer = new Assimp::Importer();
			u32 import_options = aiProcess_Triangulate | 
								 aiProcess_OptimizeMeshes | 
								 aiProcess_JoinIdenticalVertices | 
								 aiProcess_FlipUVs | 
								 aiProcess_CalcTangentSpace;
			const aiScene* scene = importer->ReadFile(file.GetRelativePath(), import_options);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				HE_CORE_ERROR("\tAssimp error: {0}", importer->GetErrorString());
				return;
			}

			ExtractTextures(scene, file);
			ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f), nullptr, file);

			// Identify root meshes - those with no parent (parent == nullptr)
			for (Mesh* mesh : s_all_loaded_meshes)
			{
				if (mesh && mesh->GetParent() == nullptr)
				{
					s_loaded_root_meshes.push_back(mesh);
					HE_GRAPHICS_INFO("Identified root mesh: {0}", mesh->GetName());
				}
			}

			s_loaded_files.push_back(file);

			HE_GRAPHICS_INFO("Loaded model: Total meshes: {0}, Root meshes: {1}", s_all_loaded_meshes.size(), s_loaded_root_meshes.size());
		}

		Texture2D* AssetManager::LoadTexture2D(const File& file)
		{
			if (TextureManager::GetInstance()->GetTexture2D(file.GetName()) != nullptr)
			{
				return TextureManager::GetInstance()->GetTexture2D(file.GetName());
			}

			return TextureManager::GetInstance()->CreateTexture2D(file.GetName(), file);
		}

		TextureCubemap* AssetManager::LoadTextureCubemap(const File& file)
		{
			if (TextureManager::GetInstance()->GetTextureCubemap(file.GetName()) != nullptr)
			{
				return TextureManager::GetInstance()->GetTextureCubemap(file.GetName());
			}

			return TextureManager::GetInstance()->CreateTextureCubemap(file.GetName(), file);
		}

		Mesh* AssetManager::ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parent_transform, Mesh* parent_mesh, const File& file)
		{
			glm::mat4 local = glm::transpose(glm::make_mat4(&node->mTransformation.a1));
			glm::mat4 world = parent_transform * local;

			Mesh* node_mesh = nullptr;

			// Process all meshes in this node
			for (u32 i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];
				Mesh* mesh = ProcessMesh(ai_mesh, scene, world, parent_mesh, file);

				if (i == 0)
				{
					node_mesh = mesh;
				}
				else if (mesh && node_mesh)
				{
					// If this node has multiple meshes, set them as siblings (same parent)
					mesh->SetParent(parent_mesh);
				}
			}

			// Process child nodes, passing the node's first mesh as their parent
			for (u32 i = 0; i < node->mNumChildren; i++)
			{
				ProcessNode(node->mChildren[i], scene, world, node_mesh ? node_mesh : parent_mesh, file);
			}

			return node_mesh;
		}

		Mesh* AssetManager::ProcessMesh(aiMesh* ai_mesh, const aiScene* scene, const glm::mat4& transform, Mesh* parent_mesh, const File& file)
		{
			// Vertices
			RawVertexData verticies = {};

			for (u32 j = 0; j < ai_mesh->mNumVertices; j++)
			{

				glm::vec4 position(ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z, 1.0f);
				verticies.positions.push_back(glm::vec3(transform * position));

				if (ai_mesh->HasVertexColors(0))
				{
					verticies.colors.push_back(glm::vec4(ai_mesh->mColors[0][j].r, ai_mesh->mColors[0][j].g, ai_mesh->mColors[0][j].b, 1.0f));
				}

				if (ai_mesh->HasTextureCoords(0))
				{
					verticies.tex_coords.push_back(glm::vec2(ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y));
				}

				if (ai_mesh->HasNormals())
				{
					verticies.normals.push_back(glm::vec3(glm::normalize(glm::vec3(ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z))));
				}

				if (ai_mesh->HasTangentsAndBitangents())
				{
					verticies.tangents.push_back(glm::vec3(ai_mesh->mTangents[j].x, ai_mesh->mTangents[j].y, ai_mesh->mTangents[j].z));
					verticies.bitangents.push_back(glm::vec3(ai_mesh->mBitangents[j].x, ai_mesh->mBitangents[j].y, ai_mesh->mBitangents[j].z));
				}
			}

			// Indices
			std::vector<u32> indices;
			for (u32 j = 0; j < ai_mesh->mNumFaces; j++)
			{
				aiFace face = ai_mesh->mFaces[j];
				for (u32 k = 0; k < face.mNumIndices; k++)
				{
					indices.push_back(face.mIndices[k]);
				}
			}

			HE_GRAPHICS_INFO("ProcessMesh: {0} - Vertices: {1}, Indices: {2}", ai_mesh->mName.C_Str(), verticies.positions.size(), indices.size());

			// Material
			aiMaterial* ai_material = scene->mMaterials[ai_mesh->mMaterialIndex];
			MaterialInfo* mesh_mat_info = new MaterialInfo();

			for (int t = aiTextureType_NONE; t <= AI_TEXTURE_TYPE_MAX; ++t) {
				aiTextureType aiType = static_cast<aiTextureType>(t);
				unsigned int texCount = ai_material->GetTextureCount(aiType);
				if (texCount > 0) {
					aiString path;
					if (ai_material->GetTexture(aiType, 0, &path) == AI_SUCCESS) {
						TextureType textureType = GetTextureType(aiType);

						File texture_file = FileManager::ReadFile(file.GetRelativeDirectory() + "/" + std::string(path.C_Str()));
						i32 index = TextureManager::GetInstance()->GetTexture2DIndex(texture_file.GetName());

						mesh_mat_info->Set(textureType, index);
					}
				}
			}

			// BaseColor fallback: Use diffuse texture if base_color missing
			if (!mesh_mat_info->Uses(TextureType_BaseColor) && mesh_mat_info->Uses(TextureType_Diffuse)) {
				mesh_mat_info->Set(TextureType_BaseColor, mesh_mat_info->Get(TextureType_Diffuse));
			}

			// Normal fallback: Use legacy normal if normal_camera missing
			if (!mesh_mat_info->Uses(TextureType_NormalCamera) && mesh_mat_info->Uses(TextureType_Normals)) {
				mesh_mat_info->Set(TextureType_NormalCamera, mesh_mat_info->Get(TextureType_Normals));
			}

			// Emission fallback: Use legacy emissive if emission_color missing
			if (!mesh_mat_info->Uses(TextureType_EmissionColor) && mesh_mat_info->Uses(TextureType_Emissive)) {
				mesh_mat_info->Set(TextureType_EmissionColor, mesh_mat_info->Get(TextureType_Emissive));
			}

			// AO fallback: Use ambient texture if AO missing (approximation)
			if (!mesh_mat_info->Uses(TextureType_AmbientOcclusion) && mesh_mat_info->Uses(TextureType_Ambient)) {
				mesh_mat_info->Set(TextureType_AmbientOcclusion, mesh_mat_info->Get(TextureType_Ambient));
			}

			// Note: Metalness and Roughness have no legacy equivalents, will use shader fallbacks
			// Specular-to-roughness conversion happens in shader (1.0 - specular)

			Mesh* mesh = MeshManager::GetInstance()->CreateMesh(ai_mesh->mName.C_Str(), verticies, indices, mesh_mat_info);
			if (!mesh)
			{
				HE_CORE_ERROR("Failed to create mesh: {0}", ai_mesh->mName.C_Str());
				return nullptr;
			}

			MeshManager::GetInstance()->SetMeshPath(mesh->GetName(), file.GetAbsolutePath());

			// Set parent relationship in the mesh
			mesh->SetParent(parent_mesh);

			// Add this mesh as a child to the parent mesh
			if (parent_mesh)
			{
				parent_mesh->AddChild(mesh);
			}

			// Upload geometry
			b8 uploaded = MeshManager::GetInstance()->UploadMeshGeometry<VertexFormatTangent>(mesh);

			if (!uploaded)
			{
				HE_CORE_WARN("Failed to upload mesh geometry: {0}", mesh->GetName());
			}

			// Track all loaded meshes (even if upload failed)
			s_all_loaded_meshes.push_back(mesh);

			return mesh;
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

		TextureType GetTextureType(aiTextureType type)
		{
			switch (type)
			{
			case aiTextureType_NONE:              return TextureType_None;
			case aiTextureType_DIFFUSE:           return TextureType_Diffuse;
			case aiTextureType_SPECULAR:          return TextureType_Specular;
			case aiTextureType_AMBIENT:           return TextureType_Ambient;
			case aiTextureType_EMISSIVE:          return TextureType_Emissive;
			case aiTextureType_HEIGHT:            return TextureType_Height;
			case aiTextureType_NORMALS:           return TextureType_Normals;
			case aiTextureType_SHININESS:         return TextureType_Shininess;
			case aiTextureType_OPACITY:           return TextureType_Opacity;
			case aiTextureType_DISPLACEMENT:      return TextureType_Displacement;
			case aiTextureType_LIGHTMAP:          return TextureType_Lightmap;
			case aiTextureType_REFLECTION:        return TextureType_Reflection;

			case aiTextureType_BASE_COLOR:        return TextureType_BaseColor;
			case aiTextureType_NORMAL_CAMERA:     return TextureType_NormalCamera;
			case aiTextureType_EMISSION_COLOR:    return TextureType_EmissionColor;
			case aiTextureType_METALNESS:         return TextureType_Metalness;
			case aiTextureType_DIFFUSE_ROUGHNESS: return TextureType_DiffuseRoughness;
			case aiTextureType_AMBIENT_OCCLUSION: return TextureType_AmbientOcclusion;
			case aiTextureType_SHEEN:             return TextureType_Sheen;
			case aiTextureType_CLEARCOAT:         return TextureType_Clearcoat;
			case aiTextureType_TRANSMISSION:      return TextureType_Transmission;

			default:                              return TextureType_Unknown;
			}
		}

		void AssetManager::DebugPrintMeshHierarchy()
		{
			HE_GRAPHICS_INFO("=== Mesh Hierarchy Debug ===");
			HE_GRAPHICS_INFO("Total meshes: {0}", s_all_loaded_meshes.size());
			HE_GRAPHICS_INFO("Root meshes: {0}", s_loaded_root_meshes.size());

			for (size_t i = 0; i < s_all_loaded_meshes.size(); ++i)
			{
				Mesh* mesh = s_all_loaded_meshes[i];
				if (mesh)
				{
					Mesh* parent = mesh->GetParent();
					const char* parent_name = parent ? parent->GetName().c_str() : "NULL";
					HE_GRAPHICS_INFO("  [{0}] {1} - Parent: {2} - Vertices: {3}", i, mesh->GetName(), parent_name, mesh->GetRawData().positions.size());
				}
			}
			HE_GRAPHICS_INFO("=== End Mesh Hierarchy Debug ===");
		}

	} // namespace resources

} // namespace hellengine