#include "test_gltf_loading.h"

TestGltfLoading::TestGltfLoading(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestGltfLoading::~TestGltfLoading()
{
	NO_OP;
}

void TestGltfLoading::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera.SetPosition({ 0.0f, 0.5f, 5.0f });

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	m_light_data.position = glm::vec4(5.0f, 5.0f, -5.0f, 1.0f);
	m_light_data.view_position = glm::inverse(m_camera_data.view) * m_light_data.position;

	m_model = AssetManager::LoadModel(FileManager::ReadFile("assets/models/gltf/FlightHelmet/glTF/FlightHelmet.gltf"));

	CreatePipelines();
	CreateDescriptorSets();

	m_model->UploadToGPU<VertexFormatBase>();
	m_model->GenerateDescriptorSets(m_lit_pipeline, 2);
	m_model->GenerateDescriptorSets(m_wireframe_pipeline, 2);
}

void TestGltfLoading::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
	m_light_data.view_position = glm::inverse(m_camera_data.view) * m_light_data.position * m_model_data.model;
}

void TestGltfLoading::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_light_buffer, &m_light_data, sizeof(LightData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
}

void TestGltfLoading::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	if (!m_use_wireframe)
	{
		m_backend->BindPipeline(m_lit_pipeline);
		m_backend->BindDescriptorSet(m_lit_pipeline, m_lit_constants_descriptor);

		m_backend->BindDescriptorSet(m_lit_pipeline, m_lit_model_descriptor);

		m_model->Draw(m_lit_pipeline, 0);
	}
	else
	{
		m_backend->BindPipeline(m_wireframe_pipeline);
		m_backend->BindDescriptorSet(m_wireframe_pipeline, m_wireframe_constants_descriptor);

		m_backend->BindDescriptorSet(m_wireframe_pipeline, m_lit_model_descriptor);

		m_model->Draw(m_wireframe_pipeline, 1);
	}

	m_backend->EndDynamicRendering();
}

void TestGltfLoading::RenderEnd()
{
}

void TestGltfLoading::UIRender()
{
}

void TestGltfLoading::Cleanup()
{
	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_light_buffer);
	m_backend->DestroyBuffer(m_model_buffer);

	m_backend->DestroyPipeline(m_lit_pipeline);
	m_backend->DestroyPipeline(m_wireframe_pipeline);
}

b8 TestGltfLoading::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestGltfLoading::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnKeyPressed(EventContext& event)
{
	if (Input::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		return false;
	}

	if (event.data.key_event.key == KEY_W)
	{
		m_use_wireframe = !m_use_wireframe;
	}

	return false;
}

b8 TestGltfLoading::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestGltfLoading::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestGltfLoading::CreatePipelines()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };

	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();
	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.cull_mode = PipelineCullMode_Back;
	pipeline_info.front_face = PipelineFrontFace_CounterClockwise;

	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_UNORM },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	ShaderStageInfo shader_info = {};

	// Lit pipeline
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
			{ 1, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex}
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "gltf_loading_lit.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "gltf_loading_lit.frag");

	m_lit_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	// Wireframe pipeline
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
	pipeline_info.polygon_mode = PipelinePolygonMode_Line;

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "gltf_loading_unlit.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "gltf_loading_unlit.frag");

	m_wireframe_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_lit_pipeline->GetRenderingCreateInfo());
}

void TestGltfLoading::CreateDescriptorSets()
{
	/*
	* 
	* This is an example of how descriptor sets work between different pipelines.
	* As long as the descriptor set layout is the same, the descriptor set can be used in different pipelines.
	* In this case, the model descriptor set is used in both the lit and wireframe pipelines.
	* 
	*/

	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Objects descriptors
	{
		m_model_data.model = glm::mat4(1.0f);
		m_model_data.model = glm::scale(m_model_data.model, glm::vec3(10.0f));

		m_model_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_lit_model_descriptor = m_backend->CreateDescriptorSet(m_lit_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_model_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_lit_model_descriptor, descriptor_data_objects);
	}

	// Lit constants descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_lit_constants_descriptor = m_backend->CreateDescriptorSet(m_lit_pipeline, 0);

		DescriptorSetWriteData descriptor_data1;
		descriptor_data1.type = DescriptorType_UniformBuffer;
		descriptor_data1.binding = 0;
		descriptor_data1.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data1.data.buffer.offsets = 0;
		descriptor_data1.data.buffer.ranges = sizeof(CameraData);

		m_light_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightData));

		DescriptorSetWriteData descriptor_data2;
		descriptor_data2.type = DescriptorType_UniformBuffer;
		descriptor_data2.binding = 1;
		descriptor_data2.data.buffer.buffers = m_light_buffer->GetHandle();
		descriptor_data2.data.buffer.offsets = 0;
		descriptor_data2.data.buffer.ranges = sizeof(LightData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data1, descriptor_data2 };
		m_backend->WriteDescriptor(&m_lit_constants_descriptor, descriptor_data_camera);
	}

	// Wireframe constants descriptor
	{
		m_wireframe_constants_descriptor = m_backend->CreateDescriptorSet(m_wireframe_pipeline, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_wireframe_constants_descriptor, descriptor_data_camera);
	}
}
