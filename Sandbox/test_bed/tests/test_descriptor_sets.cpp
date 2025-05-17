#include "test_descriptor_sets.h"

TestDescriptorSet::TestDescriptorSet(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestDescriptorSet::~TestDescriptorSet()
{
	NO_OP;
}

void TestDescriptorSet::Setup()
{
	m_clear_color = glm::vec4(0.2f, 0.5f, 1.0f, 1.0f);

	m_camera.SetPosition({ 0.0f, 0.0f, 5 * glm::pow(m_objects, 1.0f / 3) });

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_texture1 = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "statue.jpg")));
	m_texture2 = AssetManager::LoadTexture2D(FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "wall.jpg")));

	CreateCube();

	CreatePipeline();
	CreateDescriptorSets();

	SetupCubesData();
}

void TestDescriptorSet::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}

	UpdateCubes(dt);
}

void TestDescriptorSet::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));

	for (u32 i = 0; i < m_objects; i++)
	{
		m_backend->UpdateUniformBuffer(m_object_buffers[i], &m_object_data[i], sizeof(ObjectData));
	}
}

void TestDescriptorSet::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->BindPipeline(m_pipeline);

	m_backend->BindDescriptorSet(m_pipeline, m_camera_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_texture_descriptor);

	m_backend->BindVertexBuffer(m_vertex_buffer, 0);
	m_backend->BindIndexBuffer(m_index_buffer, 0);
	for (u32 i = 0; i < m_objects; i++)
	{
		m_backend->BindDescriptorSet(m_pipeline, m_object_descriptors[i]);
		m_backend->DrawIndexed((u32)m_indices.size(), 1, 0, 0, 0);
	}

	m_backend->EndDynamicRendering();
}

void TestDescriptorSet::RenderEnd()
{
}

void TestDescriptorSet::UIRender()
{
}

void TestDescriptorSet::Cleanup()
{
	m_backend->DestroyBuffer(m_index_buffer);
	m_backend->DestroyBuffer(m_vertex_buffer);

	m_backend->DestroyUniformBuffer(m_camera_buffer);
	for (u32 i = 0; i < m_objects; i++)
	{
		m_backend->DestroyUniformBuffer(m_object_buffers[i]);
	}

	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestDescriptorSet::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestDescriptorSet::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

    return false;
}

b8 TestDescriptorSet::OnWindowFocus(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnWindowIconified(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnWindowMoved(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnKeyPressed(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnKeyReleased(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnMouseButtonPressed(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnMouseButtonReleased(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnMouseMoved(EventContext& event)
{
    return false;
}

b8 TestDescriptorSet::OnMouseScrolled(EventContext& event)
{
    return false;
}

void TestDescriptorSet::CreateCube()
{
	m_vertices = {
		// Front face
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 0
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 1
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 2
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 3

		// Back face
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 4
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 5
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 6
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 7

		// Top face
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 7
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 6
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 2
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 3

		// Bottom face
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 4
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 5
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 1
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 0
	};


	m_indices = {
		// Front face
		0, 2, 1, 0, 3, 2,

		// Back face
		4, 5, 6, 4, 6, 7,

		// Left face
		4, 0, 3, 4, 3, 7,

		// Right face
		1, 6, 5, 1, 2, 6,

		// Top face
		8, 10, 9, 8, 11, 10,

		// Bottom face
		12, 13, 14, 12, 14, 15
	};


	m_vertex_buffer = m_backend->CreateVertexBuffer(m_vertices.data(), (u32)m_vertices.size());
	m_index_buffer = m_backend->CreateIndexBuffer(m_indices.data(), (u32)m_indices.size());
}

void TestDescriptorSet::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex}
		},
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};
	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "descriptor_set.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "descriptor_set.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestDescriptorSet::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPool({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
	}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Objects descriptors
	{
		for (u32 i = 0; i < m_objects; i++)
		{
			auto data = ObjectData();
			data.model = glm::mat4(1.0f);
			data.model = glm::translate(data.model, glm::vec3(0.0f, 0.0f, i * -2.0f));
			m_object_data.push_back(data);

			m_object_descriptors.push_back(m_backend->CreateDescriptorSet(m_pipeline, 1));
			m_object_buffers.push_back(m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData)));

			DescriptorSetWriteData descriptor_data;
			descriptor_data.type = DescriptorType_UniformBuffer;
			descriptor_data.binding = 0;
			descriptor_data.data.buffer.buffer = m_object_buffers[i]->GetHandle();
			descriptor_data.data.buffer.offset = 0;
			descriptor_data.data.buffer.range = sizeof(ObjectData);

			std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
			m_backend->WriteDescriptor(&m_object_descriptors[i], descriptor_data_objects);
		}
	}

	// Textures descriptor
	{
		m_texture_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 2);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_view = m_texture1->GetImageView();
		descriptor_data1.data.image.sampler = m_texture1->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_view = m_texture2->GetImageView();
		descriptor_data2.data.image.sampler = m_texture2->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_texture = { descriptor_data1, descriptor_data2 };
		m_backend->WriteDescriptor(&m_texture_descriptor, descriptor_data_texture);
	}
}

void TestDescriptorSet::SetupCubesData()
{
	m_rotations.resize(m_objects);
	m_rotation_speeds.resize(m_objects);

	std::default_random_engine rndEngine((u32)time(nullptr));
	std::normal_distribution<float> rndDist(-1.0f, 1.0f);
	for (uint32_t i = 0; i < m_objects; i++) {
		m_rotations[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine)) * 2.0f * glm::pi<f32>();
		m_rotation_speeds[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
	}
}

void TestDescriptorSet::UpdateCubes(f32 dt)
{
	uint32_t dim = static_cast<uint32_t>(pow(m_objects, (1.0f / 3.0f)));
	glm::vec3 offset(2.5f);

	for (uint32_t x = 0; x < dim; x++)
	{
		for (uint32_t y = 0; y < dim; y++)
		{
			for (uint32_t z = 0; z < dim; z++)
			{
				uint32_t index = x * dim * dim + y * dim + z;

				glm::mat4 model = glm::mat4(1.0f);

				// Update rotations
				m_rotations[index] += dt * m_rotation_speeds[index];

				// Update matrices
				glm::vec3 pos = glm::vec3(-((dim * offset.x) / 2.0f) + offset.x / 2.0f + x * offset.x, -((dim * offset.y) / 2.0f) + offset.y / 2.0f + y * offset.y, -((dim * offset.z) / 2.0f) + offset.z / 2.0f + z * offset.z);
				model = glm::translate(glm::mat4(1.0f), pos);
				model = glm::rotate(model, m_rotations[index].x, glm::vec3(1.0f, 1.0f, 0.0f));
				model = glm::rotate(model, m_rotations[index].y, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, m_rotations[index].z, glm::vec3(0.0f, 0.0f, 1.0f));

				m_object_data[index].model = model;
			}
		}
	}
}
