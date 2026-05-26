#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/core/uuid/uuid.h>

#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/graphics/mesh/mesh.h>

namespace hellengine
{

	using namespace math;
	namespace graphics
	{

		constexpr u32 MAX_MATERIALS = 10000;
		constexpr u32 MAX_TEXTURES = 100000;
		constexpr u32 MAX_VERTICES = 1000000;
		constexpr u32 MAX_INDICES = MAX_VERTICES * 6;
		constexpr u64 MIN_MEMORY_ALIGNMENT = lcm_array<2>({ sizeof(VertexFormatSimple), sizeof(VertexFormatTangent) });
		constexpr u64 MAX_MEMORY_VERTICES = MAX_VERTICES * MIN_MEMORY_ALIGNMENT;
		constexpr u64 MAX_MEMORY_INDICES = MAX_INDICES * MIN_MEMORY_ALIGNMENT;

		constexpr u32 MESHLET_MAX_VERTICES = 64;
		constexpr u32 MESHLET_MAX_TRIANGLES = 84;

		constexpr u32 MAX_MESHLETS = (MAX_VERTICES / MESHLET_MAX_VERTICES) * 2;

		constexpr u32 SHADER_MATERIALS_BINDING = 1;
		constexpr u32 SHADER_PER_DRAW_DATA_BINDING = 2;
		constexpr u32 SHADER_TEXTURES_BINDING = 3;

		constexpr u32 SHADER_MESHLET_VERTICES_BINDING = 1;  // set 2, binding 1
		constexpr u32 SHADER_MESHLET_INDICES_BINDING = 2;  // set 2, binding 2
		constexpr u32 SHADER_MESHLET_DESC_BINDING = 3;  // set 2, binding 3
		constexpr u32 SHADER_MESHLET_DRAWINFO_BINDING = 4;  // set 2, binding 4

		enum GeometryMode
		{
			GeometryMode_Classic,
			GeometryMode_MeshShader,

			GeometryMode_Count
		};

		struct MeshletData
		{
			u32       vertex_offset;             // Offset into packed vertex SSBO
			u32       index_offset;              // Offset into packed primitive-index SSBO
			u32       vertex_count;              // Vertices in this meshlet (<= MESHLET_MAX_VERTICES)
			u32       triangle_count;            // Triangles  (<= MESHLET_MAX_TRIANGLES)
			glm::vec3 bounding_sphere_center;    // Object-space bounding sphere (task culling)
			f32       bounding_sphere_radius;
		};

		struct MeshTaskIndirectCommand
		{
			u32 group_count_x;  // Always 1
			u32 group_count_y;  // Always 1
			u32 group_count_z;  // Always 1
		};

		struct MeshShaderDrawInfo
		{
			u32 meshlet_count;   // Total meshlets for this draw batch
			u32 instance_count;  // Total instances in this draw batch
		};

		struct BufferPool
		{
			// ---- Classic pipeline ----
			VulkanBuffer* vertex_buffer = nullptr;
			VulkanBuffer* index_buffer = nullptr;
			u32 vertex_count = 0;
			u32 index_count = 0;

			// ---- Mesh shader pipeline ----
			// Packed float vertex data as SSBO (same layout as VertexFormatTangent)
			VulkanStorageBuffer* ms_vertex_buffer = nullptr;
			// Packed u8 primitive indices (local to each meshlet, 0..vertex_count-1)
			VulkanStorageBuffer* ms_index_buffer = nullptr;
			// Array of MeshletData descriptors
			VulkanStorageBuffer* ms_meshlet_buffer = nullptr;
			u32 ms_vertex_count = 0;  // vertices uploaded to ms_vertex_buffer
			u32 ms_index_count = 0;  // primitive-indices uploaded to ms_index_buffer
			u32 ms_meshlet_count = 0;  // meshlets uploaded to ms_meshlet_buffer

			std::vector<u32> mesh_list;
			u32 number_of_instances = 0;
		};

		struct MeshBufferAllocation
		{
			u32 pool_index = 0;

			// Classic pipeline
			u32 index_offset = 0;
			u32 index_count = 0;
			i32 vertex_offset = 0;
			i32 vertex_count = 0;

			// Mesh shader pipeline
			u32 ms_meshlet_offset = 0;   // First meshlet index in the pool's ms_meshlet_buffer
			u32 ms_meshlet_count = 0;   // Number of meshlets for this mesh
			u32 ms_vertex_offset = 0;   // First packed vertex in ms_vertex_buffer
			u32 ms_index_offset = 0;   // First packed primitive-index in ms_index_buffer

			u32 ref_count = 0;
			u32 mesh_index = 0;
		};

		struct MeshInstanceAllocation
		{
			u32 mesh_index = 0;
			UUID entity_id;
			MaterialInfo* material_info = nullptr;
			MeshBufferAllocation* buffer_allocation = nullptr;
		};

		ALIGN_AS(16) struct InstanceData
		{
			glm::mat4 model_matrix;
			u32 material_index;
			i32 entity_id;
		};

		struct DeletionQueue
		{
			std::vector<std::function<void()>> deletors;
			void PushFunction(std::function<void()> function)
			{
				deletors.push_back(function);
			}
			void Flush()
			{
				for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
				{
					(*it)();
				}
				deletors.clear();
			}
		};

		class MeshManager : public Singleton<MeshManager>
		{
		public:
			using MeshHash = u64;

		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			void CreateDescriptors(VulkanPipeline* classic_pipeline, VulkanPipeline* mesh_shader_pipeline);

			Mesh* CreateMesh(std::string name, RawVertexData vertices, std::vector<u32> indices, MaterialInfo* material);
			void CreateMeshInstance(UUID id, Mesh* mesh, MaterialInfo* material = nullptr);
			void RemoveMeshInstance(UUID id);

			template<typename VertexT>
			b8 UploadMeshGeometry(Mesh* mesh);

			void UploadToGpu(TextureType types);

			void CreateDrawCommands();

			void CreatePackedData();
			void UpdatePerDrawData();

			void DrawMeshes(VulkanPipeline* pipeline);

			void CleanUp();

			void SetGeometryMode(GeometryMode mode);
			GeometryMode GetGeometryMode() const { return m_geometry_mode; }

			std::vector<Mesh*>& GetAllMeshes() { return m_meshes; }
			const std::vector<Mesh*>& GetAllMeshes() const { return m_meshes; }
			VulkanDescriptorSet* GetTexturesDescriptor() const { return m_textures_descriptor; }

			void SetMeshInstanceFilter(UUID id, Mesh* mesh);
			Mesh* GetMeshByName(const std::string& mesh_name) const;

		private:
			// ---- Classic pipeline helpers ----
			void CreateClassicDrawCommands();
			void DrawMeshesClassic(VulkanPipeline* pipeline);

			// ---- Mesh shader pipeline helpers ----
			void BuildMeshletsForMesh(Mesh* mesh, MeshBufferAllocation& alloc, BufferPool& pool);
			void CreateMeshShaderDrawCommands();
			void DrawMeshesMeshShader(VulkanPipeline* pipeline);
			void UpdateMeshShaderDescriptors();

			void CreatePool();
			u64 ComputeMeshHash(const RawVertexData& v, const std::vector<u32>& indices);

		private:
			// ---- Mode ----
			GeometryMode m_geometry_mode = GeometryMode_Classic;

			// ---- Mesh data ----
			std::vector<Mesh*> m_meshes;
			std::unordered_map<MeshHash, MeshBufferAllocation> m_meshes_buffer_map;
			std::vector<MeshInstanceAllocation*> m_mesh_instances_allocation;
			std::unordered_map<UUID, u32> m_mesh_instance_allocation_map;
			u32 m_last_mesh_index = 0;

			std::vector<BufferPool> m_pools;

			// ---- Material / texture descriptors (shared between modes) ----
			std::vector<MaterialGPUInfo> m_mesh_gpu_info;
			VulkanDescriptorSet* m_textures_descriptor = nullptr;
			VulkanStorageBuffer* m_materials_buffer = nullptr;
			VulkanDescriptorSet* m_materials_descriptor = nullptr;
			u32 m_last_texture_index = 0;

			// ---- Per-draw InstanceData (shared between modes) ----
			std::vector<InstanceData> m_per_draw_data;
			VulkanStorageBuffer* m_per_draw_data_buffer = nullptr;
			VulkanDescriptorSet* m_per_draw_data_descriptor = nullptr;

			// ---- Classic indirect draw ----
			std::vector<VkDrawIndexedIndirectCommand> m_draw_commands;
			VulkanBuffer* m_draw_commands_buffer = nullptr;

			// ---- Mesh shader indirect draw ----
			// Each entry is one (meshlet × instance) workgroup dispatch (1,1,1).
			std::vector<MeshTaskIndirectCommand> m_mesh_task_commands;
			VulkanDescriptorSet* m_per_draw_data_descriptor_mesh = nullptr;
			VulkanBuffer* m_mesh_task_commands_buffer = nullptr;
			// Parallel array: encodes (meshlet_index, instance_index) for each command.
			// The mesh shader reads this from a storage buffer indexed by gl_DrawID.
			struct MeshShaderDrawArgs
			{
				u32 meshlet_index;
				u32 instance_index;
			};
			std::vector<MeshShaderDrawArgs> m_mesh_shader_draw_args;
			VulkanStorageBuffer* m_mesh_shader_draw_args_buffer = nullptr;

			// ---- Image descriptor arrays ----
			std::vector<VkImageView> m_descriptor_image_views;
			std::vector<VkSampler>   m_descriptor_image_samplers;

			DeletionQueue m_deletion_queue;
			VulkanBackend* m_backend = nullptr;
		};


	} // namespace graphics

} // namespace hellengine
