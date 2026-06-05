#pragma once

#include "hellengine/hellengine.h"

// STL
#include <vector>
#include <random>
#include <algorithm>

using namespace hellengine;
using namespace tools;
using namespace core;
using namespace math;
using namespace graphics;
using namespace resources;

constexpr const char* SHADER_PATH   = "assets/shaders";
constexpr const char* TEXTURE_PATH  = "assets/textures";
constexpr const char* MODEL_PATH    = "assets/models";

constexpr const char* GRAPHICS_PIPELINE_NAME = "graphics_pipeline";
constexpr const char* WIREFRAME_PIPELINE_NAME = "wireframe_pipeline";
constexpr const char* DEBUG_LINES_PIPELINE_NAME = "debug_lines_pipeline";
constexpr const char* PHYSICS_COMPUTE_PIPELINE_NAME = "physics_compute_pipeline";

constexpr const char* SHADER_UNLIT[2] = { "unlit_instanced.vert", "unlit_instanced.frag" };
constexpr const char* SHADER_WIREFRAME[2] = { "wireframe.vert", "wireframe.frag" };
constexpr const char* SHADER_DEBUG_LINES[2] = { "debug_lines.vert", "debug_lines.frag" };
constexpr const char* SHADER_PHYSICS_COMPUTE = "physics_collide.comp";

static glm::vec4 RandomColor()
{
	return glm::vec4(
		Random::GetInRange(0.2f, 1.0f),
		Random::GetInRange(0.2f, 1.0f),
		Random::GetInRange(0.2f, 1.0f),
		1.0f
	);
}

static glm::vec3 RandVec3(const glm::vec3& minv, const glm::vec3& maxv)
{
	return glm::vec3(
		Random::GetInRange(minv.x, maxv.x),
		Random::GetInRange(minv.y, maxv.y),
		Random::GetInRange(minv.z, maxv.z)
	);
}

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB() : min(0.0f), max(0.0f) {}
	AABB(const glm::vec3& mi, const glm::vec3& ma) : min(mi), max(ma) {}

	glm::vec3 GetCenter() const { return 0.5f * (min + max); }
	glm::vec3 GetExtents() const { return 0.5f * (max - min); }

	static AABB Union(const AABB& a, const AABB& b) { return AABB(glm::min(a.min, b.min), glm::max(a.max, b.max)); }
};

struct WireframeCubeData
{
	std::vector<VertexFormatSimple> vertices;
	std::vector<u32> indices;

	Buffer* vertex_buffer = nullptr;
	Buffer* index_buffer = nullptr;

	Pipeline* pipeline = nullptr;

	UniformBuffer* transform_buffer = nullptr;
	DescriptorSet* transform_descriptor = nullptr;

	glm::mat4 transform{ 1.0f };
	AABB aabb;

	WireframeCubeData()
	{
		vertices = {
			{ { -0.5f, -0.5f, -0.5f }, { 1,0,0,1 }, {0,0}, {0,0,0} },
			{ {  0.5f, -0.5f, -0.5f }, { 0,1,0,1 }, {1,0}, {0,0,0} },
			{ {  0.5f, -0.5f,  0.5f }, { 0,0,1,1 }, {1,1}, {0,0,0} },
			{ { -0.5f, -0.5f,  0.5f }, { 1,1,1,1 }, {0,1}, {0,0,0} },
			{ { -0.5f,  0.5f, -0.5f }, { 1,1,1,1 }, {0,1}, {0,0,0} },
			{ {  0.5f,  0.5f, -0.5f }, { 1,1,1,1 }, {1,1}, {0,0,0} },
			{ {  0.5f,  0.5f,  0.5f }, { 1,1,1,1 }, {1,1}, {0,0,0} },
			{ { -0.5f,  0.5f,  0.5f }, { 1,1,1,1 }, {0,1}, {0,0,0} },
		};

		indices = {
			0,1, 1,2, 2,3, 3,0,
			4,5, 5,6, 6,7, 7,4,
			0,4, 1,5, 2,6, 3,7
		};

		aabb = AABB(glm::vec3(-0.5f), glm::vec3(0.5f));
		pipeline = PipelineManager::GetInstance()->GetPipeline(WIREFRAME_PIPELINE_NAME);
	}

	void CreateBuffers(VulkanBackend* backend)
	{
		vertex_buffer = backend->CreateVertexBuffer(vertices.data(), (u32)vertices.size());
		index_buffer = backend->CreateIndexBuffer(indices.data(), (u32)indices.size());

		transform_buffer = backend->CreateUniformBufferMappedPersistent(sizeof(transform), 1);
		transform_descriptor = backend->CreateDescriptorSet(pipeline, 1);

		DescriptorSetWriteData w{};
		w.type = DescriptorType_UniformBuffer;
		w.binding = 0;
		w.data.buffer.buffers = new VkBuffer[1]{ transform_buffer->GetHandle() };
		w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
		w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(transform) };

		std::vector<DescriptorSetWriteData> writes = { w };
		backend->WriteDescriptor(&transform_descriptor, writes);
	}

	void DestroyBuffers(VulkanBackend* backend)
	{
		if (vertex_buffer) backend->DestroyBuffer(vertex_buffer);
		if (index_buffer)  backend->DestroyBuffer(index_buffer);
		if (transform_buffer) backend->DestroyBuffer(transform_buffer);

		vertex_buffer = nullptr;
		index_buffer = nullptr;
		transform_buffer = nullptr;
		transform_descriptor = nullptr;
	}

	void UpdateBuffers(VulkanBackend* backend)
	{
		backend->UpdateUniformBuffer(transform_buffer, &transform, sizeof(transform));
	}

	void Draw(VulkanBackend* backend)
	{
		backend->BindDescriptorSet(pipeline, transform_descriptor, 0);
		backend->BindVertexBuffer(vertex_buffer, 0);
		backend->BindIndexBuffer(index_buffer, 0);
		backend->DrawIndexed((u32)indices.size(), 1, 0, 0, 0);
	}

	void SetScale(const glm::vec3& scale)
	{
		transform = glm::scale(glm::mat4(1.0f), scale);

		glm::vec3 localCenter(0.0f);
		glm::vec3 localExtents(0.5f);

		glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(localCenter, 1.0f));

		glm::mat3 A = glm::mat3(transform);
		glm::mat3 absA(
			glm::abs(A[0]),
			glm::abs(A[1]),
			glm::abs(A[2])
		);

		glm::vec3 worldExtents = absA * localExtents;

		aabb.min = worldCenter - worldExtents;
		aabb.max = worldCenter + worldExtents;
	}
};

struct ShapeManager
{
	struct ShapeData
	{
		std::vector<VertexFormatSimple> vertices;
		std::vector<u32> indices;
		Buffer* vertex_buffer = nullptr;
		Buffer* index_buffer = nullptr;
	};

	struct ShapeInstanceData
	{
		glm::vec4 tint_color;
		glm::mat4 transform;
	};

	ShapeManager(u32 instance_count)
		: pipeline(nullptr), instance_count(instance_count)
	{
		pipeline = PipelineManager::GetInstance()->GetPipeline(GRAPHICS_PIPELINE_NAME);
	}

	void CreateBuffers(VulkanBackend* backend)
	{
		for (auto& shape : shapes)
		{
			shape.vertex_buffer = backend->CreateVertexBuffer(shape.vertices.data(), (u32)shape.vertices.size());
			shape.index_buffer = backend->CreateIndexBuffer(shape.indices.data(), (u32)shape.indices.size());
		}

		const u32 maxInstances = instance_count * (u32)shapes.size();
		transforms_buffer = backend->CreateStorageBufferMappedPersistent(sizeof(ShapeInstanceData), maxInstances);

		transforms_descriptor = backend->CreateDescriptorSet(pipeline, 1);

		DescriptorSetWriteData w{};
		w.type = DescriptorType_StorageBuffer;
		w.binding = 0;
		w.data.buffer.buffers = new VkBuffer[1]{ transforms_buffer->GetHandle() };
		w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
		w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(ShapeInstanceData) * maxInstances };

		std::vector<DescriptorSetWriteData> writes = { w };
		backend->WriteDescriptor(&transforms_descriptor, writes);
	}

	void DestroyBuffers(VulkanBackend* backend)
	{
		for (auto& shape : shapes)
		{
			if (shape.vertex_buffer) backend->DestroyBuffer(shape.vertex_buffer);
			if (shape.index_buffer)  backend->DestroyBuffer(shape.index_buffer);
			shape.vertex_buffer = nullptr;
			shape.index_buffer = nullptr;
		}

		if (transforms_buffer) backend->DestroyBuffer(transforms_buffer);
		transforms_buffer = nullptr;
		transforms_descriptor = nullptr;
	}

	void UpdateBuffers(VulkanBackend* backend)
	{
		if (!instance_data.empty())
		{
			backend->UpdateStorageBuffer(transforms_buffer,
				instance_data.data(),
				sizeof(ShapeInstanceData) * (u32)instance_data.size());
		}
	}

	void Draw(VulkanBackend* backend)
	{
		backend->BindDescriptorSet(pipeline, transforms_descriptor, 0);

		u32 first_instance = 0;
		for (auto& shape : shapes)
		{
			backend->BindVertexBuffer(shape.vertex_buffer, 0);
			backend->BindIndexBuffer(shape.index_buffer, 0);
			backend->DrawIndexed((u32)shape.indices.size(), instance_count, 0, 0, first_instance);
			first_instance += instance_count;
		}
	}

	void CreateCube(f32 size);
	void CreateUVSphere(f32 radius, u32 sector_count, u32 stack_count);
	void CreateCone(f32 radius, f32 height, u32 sector_count);

	void BuildInitialInstances()
	{
		instance_data.clear();
		instance_data.reserve(instance_count * (u32)shapes.size());

		for (u32 s = 0; s < (u32)shapes.size(); ++s)
		{
			for (u32 i = 0; i < instance_count; ++i)
			{
				ShapeInstanceData inst{};
				inst.tint_color = RandomColor();
				inst.transform = glm::mat4(1.0f);
				instance_data.push_back(inst);
			}
		}
	}

	Pipeline* pipeline = nullptr;

	u32 instance_count = 0;
	std::vector<ShapeInstanceData> instance_data;

	StorageBuffer* transforms_buffer = nullptr;
	DescriptorSet* transforms_descriptor = nullptr;

	std::vector<ShapeData> shapes;
};

enum ColliderType : u32
{
	Collider_Sphere = 0,
	Collider_AABB = 1,
	Collider_Capsule = 2,
	Collider_Cone = 3
};

struct Body
{
	glm::vec3 position{ 0.0f };
	f32 inv_mass = 1.0f;

	glm::vec3 velocity{ 0.0f };
	u32 type = Collider_Sphere;

	glm::vec3 half_extents{ 0.5f };
	f32 radius = 0.5f;

	glm::vec3 capsule_tip{ 0.0f, 0.5f, 0.0f };  // Local offset to capsule tip from position
	f32 capsule_radius = 0.25f;                  // Capsule radius

	glm::vec3 cone_direction{ 0.0f, 1.0f, 0.0f }; // Cone axis direction (normalized)
	f32 cone_height = 1.0f;                        // Cone height
	f32 cone_base_radius = 0.5f;                   // Cone base radius
	f32 _pad0 = 0.0f;
};

static inline AABB ComputeBodyAABB(const Body& b)
{
	if (b.type == Collider_AABB)
	{
		return AABB(b.position - b.half_extents, b.position + b.half_extents);
	}
	else if (b.type == Collider_Capsule)
	{
		glm::vec3 base = b.position;
		glm::vec3 tip = b.position + b.capsule_tip;
		glm::vec3 minPt = glm::min(base, tip) - glm::vec3(b.capsule_radius);
		glm::vec3 maxPt = glm::max(base, tip) + glm::vec3(b.capsule_radius);
		return AABB(minPt, maxPt);
	}
	else if (b.type == Collider_Cone)
	{
		glm::vec3 apex = b.position + b.cone_direction * b.cone_height;
		glm::vec3 base_center = b.position;

		// AABB encompassing cone apex and base circle
		glm::vec3 r(b.cone_base_radius);
		glm::vec3 minPt = glm::min(apex, base_center - r);
		glm::vec3 maxPt = glm::max(apex, base_center + r);
		return AABB(minPt, maxPt);
	}

	glm::vec3 r(b.radius);
	return AABB(b.position - r, b.position + r);
}

struct BVHNode
{
	AABB bounds;
	i32 left = -1;
	i32 right = -1;
	u32 first = 0;
	u32 count = 0;
	std::vector<u32> leaf_indices;
};

struct BVHNodeGPU
{
	glm::vec4 bounds_min_left;   // xyz = min, w = left (as float)
	glm::vec4 bounds_max_right;  // xyz = max, w = right (as float)
	glm::vec4 first_count;       // x = first index into flat_leaf_indices, y = count, zw unused
};

struct BodyGPU
{
	glm::vec4 pos_invMass;        // xyz pos, w invMass
	glm::vec4 vel_type;           // xyz vel, w type (float)
	glm::vec4 halfExt_radius;     // xyz halfExt, w radius
	glm::vec4 capsule_tip_radius; // xyz capsule_tip offset, w capsule_radius
	glm::vec4 cone_dir_height;    // xyz cone direction, w cone height
	glm::vec4 cone_base_rad_pad;  // x cone base radius, yzw padding
};

struct PhysicsParams
{
	glm::vec3 world_min;
	f32 dt;
	glm::vec3 world_max; 
	u32 body_count;
	f32 separation_slop;
	f32 push_strength;
	f32 damping;
	u32 bvh_node_count;
};

struct PhysicsWorld
{
	PhysicsWorld()
	{
		debug_pipeline = PipelineManager::GetInstance()->GetPipeline(DEBUG_LINES_PIPELINE_NAME);
		compute_pipeline = PipelineManager::GetInstance()->GetPipeline(PHYSICS_COMPUTE_PIPELINE_NAME);

		m_params.separation_slop = 0.001f;
		m_params.push_strength = 0.85f;
		m_params.damping = 0.985f;
	}

	void CreateBuffers(VulkanBackend* backend, ShapeManager* sm)
	{
		const u32 max_bodies = sm->instance_count * (u32)sm->shapes.size();

		m_capacity_bodies = max_bodies;

		bodies_buffer = backend->CreateStorageBufferMappedPersistent(sizeof(BodyGPU), max_bodies);
		params_buffer = backend->CreateUniformBufferMappedPersistent(sizeof(PhysicsParams), 1);

		// BVH buffer: worst case is 2*N-1 nodes for N bodies
		const u32 max_bvh_nodes = (max_bodies > 0) ? (2 * max_bodies - 1) : 0;
		bvh_buffer = backend->CreateStorageBufferMappedPersistent(sizeof(BVHNodeGPU), glm::max(max_bvh_nodes, 1u));

		// BVH leaf indices buffer: worst case is all bodies in leaves (with max 4 per leaf, so ~N indices total)
		bvh_indices_buffer = backend->CreateStorageBufferMappedPersistent(sizeof(u32), glm::max(max_bodies, 1u));

		// set = 0 -> bodies
		bodies_set = backend->CreateDescriptorSet(compute_pipeline, 0);
		{
			DescriptorSetWriteData w{};
			w.type = DescriptorType_StorageBuffer;
			w.binding = 0;
			w.data.buffer.buffers = new VkBuffer[1]{ bodies_buffer->GetHandle() };
			w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(BodyGPU) * max_bodies };
			std::vector<DescriptorSetWriteData> writes = { w };
			backend->WriteDescriptor(&bodies_set, writes);
		}

		// set = 1 -> instances (same as graphics)
		instances_set = backend->CreateDescriptorSet(compute_pipeline, 1);
		{
			DescriptorSetWriteData w{};
			w.type = DescriptorType_StorageBuffer;
			w.binding = 0;
			w.data.buffer.buffers = new VkBuffer[1]{ sm->transforms_buffer->GetHandle() };
			w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(ShapeManager::ShapeInstanceData) * max_bodies };
			std::vector<DescriptorSetWriteData> writes = { w };
			backend->WriteDescriptor(&instances_set, writes);
		}

		// set = 2 -> params
		params_set = backend->CreateDescriptorSet(compute_pipeline, 2);
		{
			DescriptorSetWriteData w{};
			w.type = DescriptorType_UniformBuffer;
			w.binding = 0;
			w.data.buffer.buffers = new VkBuffer[1]{ params_buffer->GetHandle() };
			w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(PhysicsParams) };
			std::vector<DescriptorSetWriteData> writes = { w };
			backend->WriteDescriptor(&params_set, writes);
		}

		// set = 3 -> BVH nodes and indices
		bvh_set = backend->CreateDescriptorSet(compute_pipeline, 3);
		{
			const u32 max_bvh_nodes = (max_bodies > 0) ? (2 * max_bodies - 1) : 0;

			DescriptorSetWriteData w0{};
			w0.type = DescriptorType_StorageBuffer;
			w0.binding = 0;
			w0.data.buffer.buffers = new VkBuffer[1]{ bvh_buffer->GetHandle() };
			w0.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w0.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(BVHNodeGPU) * glm::max(max_bvh_nodes, 1u) };

			DescriptorSetWriteData w1{};
			w1.type = DescriptorType_StorageBuffer;
			w1.binding = 1;
			w1.data.buffer.buffers = new VkBuffer[1]{ bvh_indices_buffer->GetHandle() };
			w1.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w1.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(u32) * glm::max(max_bodies, 1u) };

			std::vector<DescriptorSetWriteData> writes = { w0, w1 };
			backend->WriteDescriptor(&bvh_set, writes);
		}

		// reserve CPU staging for capacity to avoid realloc when changing active count
		bodies_gpu.reserve(m_capacity_bodies);
		bodies.reserve(m_capacity_bodies);
		prim_bounds.reserve(m_capacity_bodies);
		prim_indices.reserve(m_capacity_bodies);
	}

	void DestroyBuffers(VulkanBackend* backend)
	{
		// debug
		if (debug_vb) backend->DestroyBuffer(debug_vb);
		if (debug_ib) backend->DestroyBuffer(debug_ib);
		debug_vb = nullptr;
		debug_ib = nullptr;

		// compute
		if (bodies_buffer) backend->DestroyBuffer(bodies_buffer);
		if (params_buffer) backend->DestroyBuffer(params_buffer);
		if (bvh_buffer) backend->DestroyBuffer(bvh_buffer);
		if (bvh_indices_buffer) backend->DestroyBuffer(bvh_indices_buffer);
		bodies_buffer = nullptr;
		params_buffer = nullptr;
		bvh_buffer = nullptr;
		bvh_indices_buffer = nullptr;

		bodies_set = nullptr;
		instances_set = nullptr;
		params_set = nullptr;
		bvh_set = nullptr;

		m_capacity_bodies = 0;
	}

	void SetSpawnBox(const glm::vec3& min_corner, const glm::vec3& max_corner)
	{
		spawn_min = glm::min(min_corner, max_corner);
		spawn_max = glm::max(min_corner, max_corner);
	}

	void SetShapePrototypes(const std::vector<Body>& per_shape)
	{
		shape_prototypes = per_shape;
	}

	void Spawn(u32 instances_per_shape)
	{
		const u32 per_shape_clamped =
			(m_capacity_bodies == 0)
			? instances_per_shape
			: glm::min(instances_per_shape, m_capacity_bodies / (u32)glm::max<size_t>(1, shape_prototypes.size()));

		active_instances_per_shape = per_shape_clamped;

		bodies.clear();
		bodies.reserve(active_instances_per_shape * (u32)shape_prototypes.size());

		for (const Body& proto : shape_prototypes)
		{
			for (u32 i = 0; i < active_instances_per_shape; ++i)
			{
				Body b = proto;
				b.position = RandVec3(spawn_min, spawn_max);
				bodies.push_back(b);
			}
		}

		// build GPU upload array on CPU (size = ACTIVE bodies)
		bodies_gpu.resize(bodies.size());
		for (u32 i = 0; i < (u32)bodies.size(); ++i)
		{
			const Body& s = bodies[i];
			BodyGPU g{};
			g.pos_invMass = glm::vec4(s.position, s.inv_mass);
			g.vel_type = glm::vec4(s.velocity, (f32)s.type);
			g.halfExt_radius = glm::vec4(s.half_extents, s.radius);
			g.capsule_tip_radius = glm::vec4(s.capsule_tip, s.capsule_radius);
			g.cone_dir_height = glm::vec4(s.cone_direction, s.cone_height);
			g.cone_base_rad_pad = glm::vec4(s.cone_base_radius, 0.0f, 0.0f, 0.0f);

			bodies_gpu[i] = g;
		}

		needs_gpu_upload = true;
	}

	void UploadSpawnDataIfNeeded(VulkanBackend* backend, ShapeManager* sm)
	{
		if (!needs_gpu_upload) return;
		if (!backend || !sm) return;

		// Upload ACTIVE bodies only
		if (!bodies_gpu.empty())
		{
			backend->UpdateStorageBuffer(
				bodies_buffer,
				bodies_gpu.data(),
				sizeof(BodyGPU) * (u32)bodies_gpu.size());
		}

		sm->UpdateBuffers(backend);

		needs_gpu_upload = false;
	}

	void RunCompute(VulkanBackend* backend, const AABB& world_bounds, f32 dt)
	{
		if (!backend) return;

		PhysicsParams p{};
		p.world_min = world_bounds.min;
		p.world_max = world_bounds.max;
		p.dt = dt;
		p.body_count = (u32)bodies.size();
		p.separation_slop = m_params.separation_slop;
		p.push_strength = m_params.push_strength;
		p.damping = m_params.damping;
		p.bvh_node_count = (u32)nodes.size();

		backend->UpdateUniformBuffer(params_buffer, &p, sizeof(PhysicsParams));

		backend->BarrierGraphicsToCompute();

		backend->BindPipeline(compute_pipeline);
		backend->BindDescriptorSet(compute_pipeline, bodies_set, 0);
		backend->BindDescriptorSet(compute_pipeline, instances_set, 0);
		backend->BindDescriptorSet(compute_pipeline, params_set, 0);
		backend->BindDescriptorSet(compute_pipeline, bvh_set, 0);

		const u32 N = (u32)bodies.size();
		if (N > 0)
		{
			const u32 groups = (N + 63u) / 64u;
			backend->Dispatch(groups, 1, 1);
		}

		backend->BarrierComputeToGraphics();
	}

	void SyncBodiesFromMappedGPU()
	{
		const u32 N = (u32)bodies.size();
		if (N == 0) return;
		if (!bodies_buffer) return;

		const BodyGPU* gpu = (const BodyGPU*)bodies_buffer->GetMappedMemory();

		for (u32 i = 0; i < N; ++i)
		{
			bodies[i].position = glm::vec3(gpu[i].pos_invMass);
			bodies[i].inv_mass = gpu[i].pos_invMass.w;
			bodies[i].velocity = glm::vec3(gpu[i].vel_type);
			bodies[i].type = (u32)gpu[i].vel_type.w;
			bodies[i].half_extents = glm::vec3(gpu[i].halfExt_radius);
			bodies[i].radius = gpu[i].halfExt_radius.w;
			bodies[i].capsule_tip = glm::vec3(gpu[i].capsule_tip_radius);
			bodies[i].capsule_radius = gpu[i].capsule_tip_radius.w;
			bodies[i].cone_direction = glm::vec3(gpu[i].cone_dir_height);
			bodies[i].cone_height = gpu[i].cone_dir_height.w;
			bodies[i].cone_base_radius = gpu[i].cone_base_rad_pad.x;
		}
	}

	void UploadBVHToGPU(VulkanBackend* backend)
	{
		if (!backend || !bvh_buffer || !bvh_indices_buffer) return;
		if (nodes.empty()) return;

		// Flatten leaf indices into a single array
		flat_leaf_indices.clear();

		// Convert CPU BVH nodes to GPU format
		bvh_nodes_gpu.resize(nodes.size());
		for (u32 i = 0; i < (u32)nodes.size(); ++i)
		{
			const BVHNode& n = nodes[i];
			BVHNodeGPU& g = bvh_nodes_gpu[i];

			g.bounds_min_left = glm::vec4(n.bounds.min, (f32)n.left);
			g.bounds_max_right = glm::vec4(n.bounds.max, (f32)n.right);

			// If leaf node, store offset into flat_leaf_indices
			if (n.left == -1 && !n.leaf_indices.empty())
			{
				u32 offset = (u32)flat_leaf_indices.size();
				g.first_count = glm::vec4((f32)offset, (f32)n.count, 0.0f, 0.0f);

				// Add indices to flat array
				for (u32 idx : n.leaf_indices)
				{
					flat_leaf_indices.push_back(idx);
				}
			}
			else
			{
				g.first_count = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}

		// Upload nodes to GPU
		backend->UpdateStorageBuffer(
			bvh_buffer,
			bvh_nodes_gpu.data(),
			sizeof(BVHNodeGPU) * (u32)bvh_nodes_gpu.size());

		// Upload flattened indices to GPU
		if (!flat_leaf_indices.empty())
		{
			backend->UpdateStorageBuffer(
				bvh_indices_buffer,
				flat_leaf_indices.data(),
				sizeof(u32) * (u32)flat_leaf_indices.size());
		}
	}

	void RebuildBVHAndUploadDebug(VulkanBackend* backend, bool draw_internal_nodes = true)
	{
		BuildPrimBounds();
		BuildBVH();
		UploadBVHToGPU(backend);
		BuildDebugMesh(draw_internal_nodes);
		UploadDebugMesh(backend);
	}

	void DrawDebug(VulkanBackend* backend, DescriptorSet* camera_set0)
	{
		if (!debug_vb || !debug_ib) return;

		backend->BindDescriptorSet(debug_pipeline, camera_set0, 0);
		backend->BindVertexBuffer(debug_vb, 0);
		backend->BindIndexBuffer(debug_ib, 0);
		backend->DrawIndexed((u32)debug_indices.size(), 1, 0, 0, 0);
	}

	glm::vec3 spawn_min = glm::vec3(-10.0f);
	glm::vec3 spawn_max = glm::vec3(10.0f);

	std::vector<Body> bodies;
	std::vector<Body> shape_prototypes;
	PhysicsParams m_params;

	// CPU BVH
	std::vector<AABB> prim_bounds;
	std::vector<u32> prim_indices;
	std::vector<BVHNode> nodes;

	// debug mesh CPU
	std::vector<VertexFormatSimple> debug_vertices;
	std::vector<u32> debug_indices;

	// debug mesh GPU
	Buffer* debug_vb = nullptr;
	Buffer* debug_ib = nullptr;
	Pipeline* debug_pipeline = nullptr;

	// compute
	Pipeline* compute_pipeline = nullptr;
	StorageBuffer* bodies_buffer = nullptr;
	UniformBuffer* params_buffer = nullptr;
	StorageBuffer* bvh_buffer = nullptr;
	StorageBuffer* bvh_indices_buffer = nullptr;

	DescriptorSet* bodies_set = nullptr;
	DescriptorSet* instances_set = nullptr;
	DescriptorSet* params_set = nullptr;
	DescriptorSet* bvh_set = nullptr;

	std::vector<BodyGPU> bodies_gpu;
	std::vector<BVHNodeGPU> bvh_nodes_gpu;
	std::vector<u32> flat_leaf_indices;
	b8 needs_gpu_upload = false;

	u32 m_capacity_bodies = 0;
	u32 active_instances_per_shape = 0;

private:
	void BuildPrimBounds()
	{
		const u32 N = (u32)bodies.size();
		prim_bounds.resize(N);

		for (u32 i = 0; i < N; ++i)
		{
			prim_bounds[i] = ComputeBodyAABB(bodies[i]);
		}

		prim_indices.resize(N);
		for (u32 i = 0; i < N; ++i)
		{
			prim_indices[i] = i;
		}
	}

	void BuildBVH()
	{
		nodes.clear();
		const u32 N = (u32)bodies.size();
		if (N == 0) return;
		BuildBVHRecursive(0, N);
	}

	u32 BuildBVHRecursive(u32 first, u32 count)
	{
		const u32 node_index = (u32)nodes.size();
		nodes.emplace_back();

		AABB b = prim_bounds[prim_indices[first]];
		for (u32 i = 1; i < count; ++i)
		{
			b = AABB::Union(b, prim_bounds[prim_indices[first + i]]);
		}

		BVHNode& node = nodes[node_index];
		node.bounds = b;

		const u32 LEAF_MAX = 4;
		if (count <= LEAF_MAX)
		{
			// Store actual body indices in leaf
			node.leaf_indices.clear();
			for (u32 i = 0; i < count; ++i)
			{
				node.leaf_indices.push_back(prim_indices[first + i]);
			}
			node.first = 0; // Will be set during flattening
			node.count = count;
			node.left = -1;
			node.right = -1;
			return node_index;
		}

		glm::vec3 e = b.GetExtents();
		int axis = 0;
		if (e.y > e.x) axis = 1;
		if (e.z > (axis == 0 ? e.x : e.y)) axis = 2;

		const u32 mid = first + count / 2;

		std::nth_element(
			prim_indices.begin() + first,
			prim_indices.begin() + mid,
			prim_indices.begin() + first + count,
			[&](u32 a, u32 c)
			{
				return prim_bounds[a].GetCenter()[axis] < prim_bounds[c].GetCenter()[axis];
			}
		);

		u32 left = BuildBVHRecursive(first, count / 2);
		u32 right = BuildBVHRecursive(mid, count - count / 2);

		node.left = (i32)left;
		node.right = (i32)right;
		node.first = 0;
		node.count = 0;

		return node_index;
	}

	void BuildDebugMesh(bool draw_internal_nodes)
	{
		debug_vertices.clear();
		debug_indices.clear();
		if (nodes.empty()) return;
		EmitNodeDebug(0, 0, draw_internal_nodes);
	}

	void EmitNodeDebug(u32 node_index, u32 depth, bool draw_internal_nodes)
	{
		const BVHNode& n = nodes[node_index];
		const bool is_leaf = (n.left == -1);

		if (draw_internal_nodes || is_leaf)
		{
			float t = 1.0f / (1.0f + (float)depth * 0.35f);
			glm::vec4 color = is_leaf ? glm::vec4(1.0f, t, t, 1.0f) : glm::vec4(t, t, 1.0f, 1.0f);
			AddAABBWire(n.bounds, color);
		}

		if (!is_leaf)
		{
			EmitNodeDebug((u32)n.left, depth + 1, draw_internal_nodes);
			EmitNodeDebug((u32)n.right, depth + 1, draw_internal_nodes);
		}
	}

	void AddAABBWire(const AABB& b, const glm::vec4& color)
	{
		glm::vec3 c[8] = {
			{b.min.x,b.min.y,b.min.z},
			{b.max.x,b.min.y,b.min.z},
			{b.max.x,b.max.y,b.min.z},
			{b.min.x,b.max.y,b.min.z},
			{b.min.x,b.min.y,b.max.z},
			{b.max.x,b.min.y,b.max.z},
			{b.max.x,b.max.y,b.max.z},
			{b.min.x,b.max.y,b.max.z}
		};

		u32 base = (u32)debug_vertices.size();
		for (int i = 0; i < 8; ++i)
			debug_vertices.emplace_back(c[i], color, glm::vec2(0), glm::vec3(0));

		static const u32 E[24] = {
			0,1, 1,2, 2,3, 3,0,
			4,5, 5,6, 6,7, 7,4,
			0,4, 1,5, 2,6, 3,7
		};

		for (int i = 0; i < 24; ++i)
			debug_indices.push_back(base + E[i]);
	}

	void UploadDebugMesh(VulkanBackend* backend)
	{
		if (!backend) return;

		if (debug_vb) backend->DestroyBuffer(debug_vb);
		if (debug_ib) backend->DestroyBuffer(debug_ib);
		debug_vb = nullptr;
		debug_ib = nullptr;

		if (debug_vertices.empty() || debug_indices.empty())
		{
			return;
		}

		debug_vb = backend->CreateVertexBuffer(debug_vertices.data(), (u32)debug_vertices.size());
		debug_ib = backend->CreateIndexBuffer(debug_indices.data(), (u32)debug_indices.size());
	}
};

class SandboxApplication : public Application
{
public:
	SandboxApplication(ApplicationConfiguration* configuration);
	~SandboxApplication();

	void Init() override;
	void Shutdown() override;

	void OnProcessUpdate(f32 delta_time) override;

	void OnRenderBegin() override;
	void OnRenderUpdate() override;
	void OnRenderEnd() override;

	void OnUIRender() override;

	b8 OnWindowClose(EventContext& event);
	b8 OnWindowResize(EventContext& event);
	b8 OnWindowMinimised(EventContext& event);
	b8 OnKeyPressed(EventContext& event);

	void RestartSimulation();

private:
	void CreateResources();
	void CreatePipelines();
	void CreateDescriptorSets();

private:
	u32 m_cursor_mode = GLFW_CURSOR_NORMAL;
	b8 m_show_debug = false;
	b8 m_simulation_paused = true;
	b8 m_use_step_simulation = true;
	b8 m_first_frame = true;
	u32 m_instances_per_shape_min = 1;
	u32 m_instances_per_shape_max = 1;
	u32 m_instances_per_shape_ui  = 1;
	u32 m_instances_per_shape_active = 1;
	b8  m_request_apply_instance_count = false;

	f32 m_dt = 0.016f;

	CameraData m_camera_data;
	PerspectiveCamera m_camera;
	PerspectiveController m_controller;

	WireframeCubeData* m_wireframe_cube = nullptr;
	ShapeManager* m_shape_manager = nullptr;
	PhysicsWorld* m_physics_world = nullptr;

	UniformBuffer* m_camera_buffer = nullptr;

	DescriptorSet* m_camera_descriptor_graphics = nullptr;
	DescriptorSet* m_camera_descriptor_wireframe = nullptr;
	DescriptorSet* m_camera_descriptor_debug_lines = nullptr;

	std::vector<Body> m_shape_prototypes;
};
