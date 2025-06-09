#include "test_dynamic_uniform_buffer.h"

TestDynamicUniformBuffer::TestDynamicUniformBuffer(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestDynamicUniformBuffer::~TestDynamicUniformBuffer()
{
	NO_OP;
}

void TestDynamicUniformBuffer::Setup()
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

void TestDynamicUniformBuffer::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}

	UpdateCubes(dt);
}

void TestDynamicUniformBuffer::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_objects_buffer, m_objects_data, m_objects_buffer->GetDynamicAlignment() * m_objects);
}

void TestDynamicUniformBuffer::RenderUpdate()
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
		u32 offset = i * m_objects_buffer->GetDynamicAlignment();

		m_backend->BindDescriptorSet(m_pipeline, m_objects_descriptor, 1, &offset);
		m_backend->DrawIndexed((u32)m_indices.size(), 1, 0, 0, 0);
	}

	m_backend->EndDynamicRendering();
}

void TestDynamicUniformBuffer::RenderEnd()
{
}

void TestDynamicUniformBuffer::UIRender()
{
}

void TestDynamicUniformBuffer::Cleanup()
{
	m_backend->DestroyBuffer(m_index_buffer);
	m_backend->DestroyBuffer(m_vertex_buffer);
	m_backend->DestroyUniformBuffer(m_camera_buffer);
	m_backend->DestroyUniformBuffer(m_objects_buffer);
	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestDynamicUniformBuffer::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestDynamicUniformBuffer::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestDynamicUniformBuffer::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestDynamicUniformBuffer::CreateCube()
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
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 8
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 9
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 10
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 11

		// Bottom face
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 12
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 13
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 14
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 15
	};


	m_indices = {
		// Front face
		0, 1, 2, 0, 2, 3,

		// Back face
		4, 5, 6, 4, 6, 7,

		// Left face
		4, 0, 3, 4, 3, 7,

		// Right face
		1, 5, 6, 1, 6, 2,

		// Top face
		8, 10, 9, 8, 10, 11,

		// Bottom face
		12, 13, 14, 12, 14, 15
	};


	m_vertex_buffer = m_backend->CreateVertexBuffer(m_vertices.data(), (u32)m_vertices.size());
	m_index_buffer = m_backend->CreateIndexBuffer(m_indices.data(), (u32)m_indices.size());
}

void TestDynamicUniformBuffer::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBufferDynamic, ShaderStage_Vertex}
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
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "dynamic_uniform_buffer.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "dynamic_uniform_buffer.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestDynamicUniformBuffer::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_UniformBufferDynamic, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
	}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Objects descriptors
	{
		m_objects_buffer = m_backend->CreateDynamicUniformBuffer((void*&)m_objects_data, sizeof(ObjectData), m_objects);
		m_objects_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 1);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_UniformBufferDynamic;
		descriptor_data1.binding = 0;
		descriptor_data1.data.buffer.buffers = m_objects_buffer->GetHandle();
		descriptor_data1.data.buffer.offsets = 0;
		descriptor_data1.data.buffer.ranges = m_objects_buffer->GetDynamicAlignment();

		std::vector<DescriptorSetWriteData> descriptor_data_dynamic = { descriptor_data1 };
		m_backend->WriteDescriptor(&m_objects_descriptor, descriptor_data_dynamic);

		for (u32 i = 0; i < m_objects; i++)
		{
			m_objects_data[i].model = glm::mat4(1.0f);
		}
	}

	// Textures descriptor
	{
		m_texture_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 2);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_CombinedImageSampler;
		descriptor_data1.binding = 0;
		descriptor_data1.data.image.image_views = m_texture1->GetImageView();
		descriptor_data1.data.image.samplers = m_texture1->GetSampler();

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_CombinedImageSampler;
		descriptor_data2.binding = 1;
		descriptor_data2.data.image.image_views = m_texture2->GetImageView();
		descriptor_data2.data.image.samplers = m_texture2->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_texture = { descriptor_data1, descriptor_data2 };
		m_backend->WriteDescriptor(&m_texture_descriptor, descriptor_data_texture);
	}
}

void TestDynamicUniformBuffer::SetupCubesData()
{
	m_rotations.resize(m_objects);
	m_rotation_speeds.resize(m_objects);

	std::default_random_engine rndEngine((u32)time(nullptr));
	std::normal_distribution<float> rndDist(-1.0f, 1.0f);
	for (u32 i = 0; i < m_objects; i++) {
		m_rotations[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine)) * 2.0f * glm::pi<f32>();
		m_rotation_speeds[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
	}
}

void TestDynamicUniformBuffer::UpdateCubes(f32 dt)
{
	u32 dim = static_cast<u32>(pow(m_objects, (1.0f / 3.0f)));
	glm::vec3 offset(2.5f);

	for (u32 x = 0; x < dim; x++)
	{
		for (u32 y = 0; y < dim; y++)
		{
			for (u32 z = 0; z < dim; z++)
			{
				u32 index = x * dim * dim + y * dim + z;

				glm::mat4& model = m_objects_data[index].model;

				m_rotations[index] += dt * m_rotation_speeds[index];

				glm::vec3 pos = glm::vec3(-((dim * offset.x) / 2.0f) + offset.x / 2.0f + x * offset.x, -((dim * offset.y) / 2.0f) + offset.y / 2.0f + y * offset.y, -((dim * offset.z) / 2.0f) + offset.z / 2.0f + z * offset.z);
				model = glm::translate(glm::mat4(1.0f), pos);
				model = glm::rotate(model, m_rotations[index].x, glm::vec3(1.0f, 1.0f, 0.0f));
				model = glm::rotate(model, m_rotations[index].y, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, m_rotations[index].z, glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}
	}
}