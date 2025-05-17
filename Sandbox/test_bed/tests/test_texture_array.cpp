#include "test_texture_array.h"

TestTextureArray::TestTextureArray(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestTextureArray::~TestTextureArray()
{
	NO_OP;
}

void TestTextureArray::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera.SetPosition({ 0.0f, 5.0f, 6.0f });
	m_camera.RotateOX(glm::radians(-45.0f));
	m_camera.SetAspect((f32)m_window->GetWidth(), (f32)m_window->GetHeight());

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	SetMeshData();
	LoadResources();

	CreatePipelines();
	CreateDescriptorSets();
}

void TestTextureArray::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestTextureArray::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_instance_buffer, &m_camera_data, sizeof(CameraData));
}

void TestTextureArray::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });
	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });

	m_backend->BindPipeline(m_pipeline);

	m_backend->BindDescriptorSet(m_pipeline, m_instances_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_texture_descriptor);

	m_frontend->DrawMesh("Cube", m_texture->GetLayerCount());

	m_backend->EndDynamicRendering();
}

void TestTextureArray::RenderEnd()
{
}

void TestTextureArray::UIRender()
{
}

void TestTextureArray::Cleanup()
{
	m_backend->DestroyBuffer(m_instance_buffer);

	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestTextureArray::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestTextureArray::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestTextureArray::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestTextureArray::LoadResources()
{
	m_texture = m_frontend->CreateTexture2D("TextureArray", FileManager::ReadFile(CONCAT_PATHS(TEXTURE_PATH, "texturearray_rgba.ktx")));
	m_frontend->CreateMesh("Cube", m_vertices, m_indices);
}

void TestTextureArray::SetMeshData()
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
}

void TestTextureArray::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_SRGB },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	ShaderStageInfo shader_info = {};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "texture_array.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "texture_array.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestTextureArray::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPool({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Instance descriptor
	{
		f32 offset = -1.5f;
		f32 center = (m_texture->GetLayerCount() * offset) / 2.0f - (offset * 0.5f);
		for (u32 i = 0; i < m_texture->GetLayerCount(); i++)
		{
			InstanceData model_data{};
			model_data.model = glm::translate(glm::mat4(1.0f), glm::vec3(i * offset - center, 0.0f, 0.0f));
			model_data.array_index = (f32)i;
			m_instances_data.push_back(model_data);
		}

		void* data = nullptr;
		data = static_cast<void*>(m_instances_data.data());
		m_instance_buffer = m_backend->CreateUniformBufferMappedOnce(data, sizeof(CameraData) + MAX_OBJECTS * sizeof(InstanceData), sizeof(CameraData));

		m_instances_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_instance_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(CameraData) + MAX_OBJECTS * sizeof(InstanceData);

		std::vector<DescriptorSetWriteData> descriptor_data_model = { descriptor_data };
		m_backend->WriteDescriptor(&m_instances_descriptor, descriptor_data_model);
	}

	// Texture descriptor
	{
		m_texture_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 1);

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_CombinedImageSampler;
		descriptor_data.binding = 0;
		descriptor_data.data.image.image_view = m_texture->GetImageView();
		descriptor_data.data.image.sampler = m_texture->GetSampler();

		std::vector<DescriptorSetWriteData> descriptor_data_texture = { descriptor_data };
		m_backend->WriteDescriptor(&m_texture_descriptor, descriptor_data_texture);
	}
}
