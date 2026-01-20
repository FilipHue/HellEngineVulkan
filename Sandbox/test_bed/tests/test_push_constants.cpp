#include "test_push_constants.h"

#if _TEST_BED_ENABLED
TestPushConstants::TestPushConstants(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestPushConstants::~TestPushConstants()
{
	NO_OP;
}

void TestPushConstants::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera.SetPosition({ 0.0f, 0.0f, 5.0f });

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_model_data.model = glm::mat4(1.0f);
	m_model_data.model = glm::scale(m_model_data.model, glm::vec3(0.5f));

	CreatePipelines();
	CreateDescriptorSets();

	LoadResources();
	SetupSpheres();
	m_model->UploadToGPU<VertexFormatBase>();
}

void TestPushConstants::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestPushConstants::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
}

void TestPushConstants::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });
	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });

	m_backend->BindPipeline(m_pipeline);

	m_backend->BindDescriptorSet(m_pipeline, m_camera_descriptor);
	m_backend->BindDescriptorSet(m_pipeline, m_model_descriptor);

	for (u32 i = 0; i < m_model_count; i++)
	{
		m_backend->BindPushConstants(m_pipeline, ShaderStage_Vertex, 0, sizeof(SphereData), &m_spheres[i]);

		m_model->Draw(m_pipeline, 0);
	}

	m_backend->EndDynamicRendering();
}

void TestPushConstants::RenderEnd()
{
}

void TestPushConstants::UIRender()
{
}

void TestPushConstants::Cleanup()
{
	m_backend->DestroyUniformBuffer(m_camera_buffer);
	m_backend->DestroyUniformBuffer(m_model_buffer);

	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestPushConstants::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestPushConstants::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestPushConstants::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestPushConstants::LoadResources()
{
	m_model = AssetManager::LoadModel(FileManager::ReadFile("assets/models/gltf/Sphere/sphere.gltf"));

	m_model->GenerateDescriptorSets(m_pipeline, 2);
}

void TestPushConstants::SetupSpheres()
{
	for (u32 i = 0; i < m_model_count; i++)
	{
		m_spheres[i].color = glm::vec4(Random::GetInRange<f32>(0.1f, 1.0f), math::Random::GetInRange(0.1f, 1.0f), math::Random::GetInRange(0.1f, 1.0f), 1.0f);
		const f32 rad = glm::radians(i * 360.0f / m_model_count);
		m_spheres[i].position = glm::vec4(glm::vec3(sin(rad), cos(rad), 0.0f) * 3.5f, 1.0f);
	}
}

void TestPushConstants::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_Back;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	pipeline_info.push_constant_ranges = {
		{ ShaderStage_Vertex, 0, sizeof(SphereData) }
	};

	ShaderStageInfo shader_info = {};

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "push_constants.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "push_constants.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestPushConstants::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Camera descriptor
	{
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 0);
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Model descriptor
	{
		m_model_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 1);
		m_model_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));

		DescriptorSetWriteData descriptor_data{};
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_model_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_model = { descriptor_data };
		m_backend->WriteDescriptor(&m_model_descriptor, descriptor_data_model);
	}
}
#endif //_TEST_BED_ENABLED