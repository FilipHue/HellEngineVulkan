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

#define SHADER_PATH   "assets/shaders"
#define TEXTURE_PATH  "assets/textures"
#define MODEL_PATH    "assets/models"

constexpr const char* GRAPHICS_PIPELINE_NAME = "graphics_pipeline";
constexpr const char* WIREFRAME_PIPELINE_NAME = "wireframe_pipeline";
constexpr const char* DEBUG_LINES_PIPELINE_NAME = "debug_lines_pipeline";
constexpr const char* PHYSICS_COMPUTE_PIPELINE_NAME = "physics_compute_pipeline";

constexpr const char* SHADER_UNLIT[2] = { "unlit_instanced.vert", "unlit_instanced.frag" };
constexpr const char* SHADER_WIREFRAME[2] = { "wireframe.vert", "wireframe.frag" };
constexpr const char* SHADER_DEBUG_LINES[2] = { "debug_lines.vert", "debug_lines.frag" };
constexpr const char* SHADER_PHYSICS_COMPUTE = "physics_collide.comp";

// ------------------------------------------------------------
// Random helpers
// ------------------------------------------------------------
static glm::vec4 RandomColor()
{
	static std::mt19937 rng{ std::random_device{}() };
	static std::uniform_real_distribution<float> dist(0.2f, 1.0f);
	return glm::vec4(dist(rng), dist(rng), dist(rng), 1.0f);
}

static std::mt19937& GlobalRng()
{
	static std::mt19937 rng{ std::random_device{}() };
	return rng;
}

static float RandRange(float a, float b)
{
	std::uniform_real_distribution<float> dist(a, b);
	return dist(GlobalRng());
}

static glm::vec3 RandVec3(const glm::vec3& minv, const glm::vec3& maxv)
{
	return glm::vec3(
		RandRange(minv.x, maxv.x),
		RandRange(minv.y, maxv.y),
		RandRange(minv.z, maxv.z)
	);
}

// ------------------------------------------------------------
// AABB utilities (CPU)
// ------------------------------------------------------------
struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB() : min(0.0f), max(0.0f) {}
	AABB(const glm::vec3& mi, const glm::vec3& ma) : min(mi), max(ma) {}
};

static inline AABB AABBUnion(const AABB& a, const AABB& b)
{
	return AABB(glm::min(a.min, b.min), glm::max(a.max, b.max));
}

static inline glm::vec3 AABBCenter(const AABB& b)
{
	return 0.5f * (b.min + b.max);
}

static inline glm::vec3 AABBExtents(const AABB& b)
{
	return 0.5f * (b.max - b.min);
}

// ------------------------------------------------------------
// Wireframe cube (spawn volume visualization)
// ------------------------------------------------------------
struct WireframeCubeData
{
	std::vector<VertexFormatBase> vertices;
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

	// MUST be called only in OnRenderBegin/OnRenderEnd (command recording region)
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
		aabb = AABB(-scale * 0.5f, scale * 0.5f);
	}
};

// ------------------------------------------------------------
// ShapeManager (rendering only)
// ------------------------------------------------------------
struct ShapeManager
{
	struct ShapeData
	{
		std::vector<VertexFormatBase> vertices;
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

	// Used ONLY for initial tint upload (or respawn tint changes). Must be called in OnRenderBegin/End.
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
				inst.transform = glm::mat4(1.0f); // compute overwrites
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

// ------------------------------------------------------------
// PhysicsWorld (CPU BVH + debug draw + GPU compute collisions)
// ------------------------------------------------------------
enum ColliderType : u32
{
	Collider_Sphere = 0,
	Collider_AABB = 1,
	Collider_ConeSphereApprox = 2
};

struct Body
{
	glm::vec3 position{ 0.0f };
	f32 inv_mass = 1.0f;

	glm::vec3 velocity{ 0.0f };
	u32 type = Collider_Sphere;

	glm::vec3 half_extents{ 0.5f };
	f32 radius = 0.5f;
};

static inline AABB ComputeBodyAABB(const Body& b)
{
	if (b.type == Collider_AABB)
		return AABB(b.position - b.half_extents, b.position + b.half_extents);

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
};

struct BodyGPU
{
	glm::vec4 pos_invMass;     // xyz pos, w invMass
	glm::vec4 vel_type;        // xyz vel, w type (float)
	glm::vec4 halfExt_radius;  // xyz halfExt, w radius
};

struct PhysicsParams
{
	glm::vec3 world_min; f32 dt;
	glm::vec3 world_max; u32 body_count;
	f32 separation_slop;
	f32 push_strength;
	f32 damping;
	f32 _pad0;
};

struct PhysicsWorld
{
	PhysicsWorld()
	{
		debug_pipeline = PipelineManager::GetInstance()->GetPipeline(DEBUG_LINES_PIPELINE_NAME);
		compute_pipeline = PipelineManager::GetInstance()->GetPipeline(PHYSICS_COMPUTE_PIPELINE_NAME);
	}

	// compute resources need ShapeManager instance buffer handle
	void CreateBuffers(VulkanBackend* backend, ShapeManager* sm)
	{
		HE_ASSERT(backend != nullptr, "PhysicsWorld::CreateBuffers: backend null!");
		HE_ASSERT(sm != nullptr, "PhysicsWorld::CreateBuffers: sm null!");

		const u32 maxBodies = sm->instance_count * (u32)sm->shapes.size();

		bodies_buffer = backend->CreateStorageBufferMappedPersistent(sizeof(BodyGPU), maxBodies);
		params_buffer = backend->CreateUniformBufferMappedPersistent(sizeof(PhysicsParams), 1);

		// set=0 bodies
		bodies_set = backend->CreateDescriptorSet(compute_pipeline, 0);
		{
			DescriptorSetWriteData w{};
			w.type = DescriptorType_StorageBuffer;
			w.binding = 0;
			w.data.buffer.buffers = new VkBuffer[1]{ bodies_buffer->GetHandle() };
			w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(BodyGPU) * maxBodies };
			std::vector<DescriptorSetWriteData> writes = { w };
			backend->WriteDescriptor(&bodies_set, writes);
		}

		// set=1 instances (same as graphics)
		instances_set = backend->CreateDescriptorSet(compute_pipeline, 1);
		{
			DescriptorSetWriteData w{};
			w.type = DescriptorType_StorageBuffer;
			w.binding = 0;
			w.data.buffer.buffers = new VkBuffer[1]{ sm->transforms_buffer->GetHandle() };
			w.data.buffer.offsets = new VkDeviceSize[1]{ 0 };
			w.data.buffer.ranges = new VkDeviceSize[1]{ sizeof(ShapeManager::ShapeInstanceData) * maxBodies };
			std::vector<DescriptorSetWriteData> writes = { w };
			backend->WriteDescriptor(&instances_set, writes);
		}

		// set=2 params
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
		bodies_buffer = nullptr;
		params_buffer = nullptr;

		bodies_set = nullptr;
		instances_set = nullptr;
		params_set = nullptr;
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
		bodies.clear();
		bodies.reserve(instances_per_shape * (u32)shape_prototypes.size());

		for (const Body& proto : shape_prototypes)
		{
			for (u32 i = 0; i < instances_per_shape; ++i)
			{
				Body b = proto;
				b.position = RandVec3(spawn_min, spawn_max);
				bodies.push_back(b);
			}
		}

		// build GPU upload array on CPU
		bodies_gpu.resize(bodies.size());
		for (u32 i = 0; i < (u32)bodies.size(); ++i)
		{
			const Body& s = bodies[i];
			BodyGPU g{};
			g.pos_invMass = glm::vec4(s.position, s.inv_mass);
			g.vel_type = glm::vec4(s.velocity, (f32)s.type);
			g.halfExt_radius = glm::vec4(s.half_extents, s.radius);
			bodies_gpu[i] = g;
		}

		needs_gpu_upload = true;
	}

	// -----------------------------------------------------------------
	// MUST be called only in OnRenderBegin/OnRenderEnd (NO dynamic draw)
	// -----------------------------------------------------------------
	void UploadSpawnDataIfNeeded(VulkanBackend* backend, ShapeManager* sm)
	{
		if (!needs_gpu_upload) return;

		backend->UpdateStorageBuffer(bodies_buffer,
			bodies_gpu.data(),
			sizeof(BodyGPU) * (u32)bodies_gpu.size());

		// tint upload (compute will write transforms)
		sm->UpdateBuffers(backend);

		needs_gpu_upload = false;
	}

	// -----------------------------------------------------------------
	// MUST be called only in OnRenderBegin/OnRenderEnd (NO dynamic draw)
	// Includes barriers + dispatch.
	// -----------------------------------------------------------------
	void RunCompute(VulkanBackend* backend, const AABB& world_bounds, f32 dt)
	{
		PhysicsParams p{};
		p.world_min = world_bounds.min;
		p.world_max = world_bounds.max;
		p.dt = dt;
		p.body_count = (u32)bodies.size();
		p.separation_slop = 0.001f;
		p.push_strength = 0.85f;
		p.damping = 0.985f;

		backend->UpdateUniformBuffer(params_buffer, &p, sizeof(PhysicsParams));

		backend->BarrierGraphicsToCompute();

		backend->BindPipeline(compute_pipeline);
		backend->BindDescriptorSet(compute_pipeline, bodies_set, 0);
		backend->BindDescriptorSet(compute_pipeline, instances_set, 0);
		backend->BindDescriptorSet(compute_pipeline, params_set, 0);

		const u32 N = (u32)bodies.size();
		if (N > 0)
		{
			const u32 groups = (N + 63u) / 64u;
			backend->Dispatch(groups, 1, 1);
		}

		backend->BarrierComputeToGraphics();
	}

	// -----------------------------------------------------------------
	// CPU BVH build + GPU debug mesh upload (also must be outside dynamic)
	// -----------------------------------------------------------------
	void RebuildBVHAndUploadDebug(VulkanBackend* backend, bool draw_internal_nodes = true)
	{
		BuildPrimBounds();
		BuildBVH();
		BuildDebugMesh(draw_internal_nodes);
		UploadDebugMesh(backend);
	}

	// draw-only (allowed inside dynamic rendering)
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

	// CPU BVH
	std::vector<AABB> prim_bounds;
	std::vector<u32> prim_indices;
	std::vector<BVHNode> nodes;

	// debug mesh CPU
	std::vector<VertexFormatBase> debug_vertices;
	std::vector<u32> debug_indices;

	// debug mesh GPU
	Buffer* debug_vb = nullptr;
	Buffer* debug_ib = nullptr;
	Pipeline* debug_pipeline = nullptr;

	// compute
	Pipeline* compute_pipeline = nullptr;
	StorageBuffer* bodies_buffer = nullptr;
	UniformBuffer* params_buffer = nullptr;

	DescriptorSet* bodies_set = nullptr;
	DescriptorSet* instances_set = nullptr;
	DescriptorSet* params_set = nullptr;

	std::vector<BodyGPU> bodies_gpu;
	b8 needs_gpu_upload = false;

private:
	void BuildPrimBounds()
	{
		const u32 N = (u32)bodies.size();
		prim_bounds.resize(N);
		for (u32 i = 0; i < N; ++i)
			prim_bounds[i] = ComputeBodyAABB(bodies[i]);

		prim_indices.resize(N);
		for (u32 i = 0; i < N; ++i)
			prim_indices[i] = i;
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
			b = AABBUnion(b, prim_bounds[prim_indices[first + i]]);

		BVHNode& node = nodes[node_index];
		node.bounds = b;

		const u32 LEAF_MAX = 4;
		if (count <= LEAF_MAX)
		{
			node.first = first;
			node.count = count;
			node.left = -1;
			node.right = -1;
			return node_index;
		}

		glm::vec3 e = AABBExtents(b);
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
				return AABBCenter(prim_bounds[a])[axis] < AABBCenter(prim_bounds[c])[axis];
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
		if (debug_vb) backend->DestroyBuffer(debug_vb);
		if (debug_ib) backend->DestroyBuffer(debug_ib);
		debug_vb = nullptr;
		debug_ib = nullptr;

		if (debug_vertices.empty() || debug_indices.empty())
			return;

		debug_vb = backend->CreateVertexBuffer(debug_vertices.data(), (u32)debug_vertices.size());
		debug_ib = backend->CreateIndexBuffer(debug_indices.data(), (u32)debug_indices.size());
	}
};

// ------------------------------------------------------------
// Application
// ------------------------------------------------------------
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

	// Stored prototypes for restart
	std::vector<Body> m_shape_prototypes;
};
