#include "test_pipelines.h"

#if _TEST_BED_ENABLED
TestPipelines::TestPipelines(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}

TestPipelines::~TestPipelines()
{
	NO_OP;
}

void TestPipelines::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_light_move_speed = 25.0f;

	m_width = (f32)m_window->GetWidth() / 3;
	m_height = (f32)m_window->GetHeight();

	m_camera.SetAspect(m_width, m_height);
	m_camera.SetPosition({ 0.0f, 5.0f, 10.5f });
	m_camera.RotateOX(-glm::degrees(std::atan2(5.0f, 10.5f)));

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	CreatePipelines();
	CreateDescriptorSets();

	LoadResources();
	m_model->UploadToGPU<VertexFormatBase>();
}

void TestPipelines::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}

	if (!m_is_camera_active)
	{
		if (Input::IsKeyPressed(KEY_W))
		{
			m_light_data.position.z -= dt * m_light_move_speed;
		}

		if (Input::IsKeyPressed(KEY_S))
		{
			m_light_data.position.z += dt * m_light_move_speed;
		}

		if (Input::IsKeyPressed(KEY_A))
		{
			m_light_data.position.x -= dt * m_light_move_speed;
		}

		if (Input::IsKeyPressed(KEY_D))
		{
			m_light_data.position.x += dt * m_light_move_speed;
		}
	}
}

void TestPipelines::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_light_buffer, &m_light_data, sizeof(LightData));
	m_backend->UpdateUniformBuffer(m_model_buffer, &m_model_data, sizeof(ObjectData));
}

void TestPipelines::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	// Phong pipeline
	{
		m_backend->SetViewport({ { 0.0f, 0.0f, m_width, m_height, 0.0f, 1.0f } });

		m_backend->BindPipeline(m_phong_pipeline);

		m_backend->BindDescriptorSet(m_phong_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_phong_pipeline, m_model_descriptor);
		m_backend->BindDescriptorSet(m_phong_pipeline, m_light_descriptor);

		m_model->Draw(m_phong_pipeline, 0);
	}

	// Toon pipeline
	{
		m_backend->SetViewport({ { m_width, 0.0f, m_width, m_height, 0.0f, 1.0f } });

		m_backend->BindPipeline(m_toon_pipeline);

		m_backend->BindDescriptorSet(m_toon_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_toon_pipeline, m_model_descriptor);
		m_backend->BindDescriptorSet(m_toon_pipeline, m_light_descriptor);

		m_model->Draw(m_toon_pipeline, 0);
	}

	// Wireframe pipeline
	{
		m_backend->SetViewport({ { 2 * m_width, 0.0f, m_width, m_height, 0.0f, 1.0f } });

		m_backend->BindPipeline(m_wireframe_pipeline);

		m_backend->BindDescriptorSet(m_wireframe_pipeline, m_camera_descriptor);
		m_backend->BindDescriptorSet(m_wireframe_pipeline, m_model_descriptor);

		m_model->Draw(m_wireframe_pipeline, 1);
	}

	m_backend->EndDynamicRendering();
}

void TestPipelines::RenderEnd()
{
}

void TestPipelines::UIRender()
{
}

void TestPipelines::Cleanup()
{
	m_backend->DestroyBuffer(m_camera_buffer);
	m_backend->DestroyBuffer(m_model_buffer);
	m_backend->DestroyBuffer(m_light_buffer);

	m_backend->DestroyPipeline(m_phong_pipeline);
	m_backend->DestroyPipeline(m_toon_pipeline);
	m_backend->DestroyPipeline(m_wireframe_pipeline);
}

b8 TestPipelines::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnWindowResize(EventContext& event)
{
	m_width = (f32)event.data.window_resize.width / 3;
	m_height = (f32)event.data.window_resize.height;

	m_camera.SetAspect(m_width, m_height);

	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestPipelines::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestPipelines::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestPipelines::LoadResources()
{
	m_model = AssetManager::LoadModel(FileManager::ReadFile("assets/models/gltf/Treasure/treasure_smooth.gltf"));

	m_model->GenerateDescriptorSets(m_phong_pipeline, 3);
	m_model->GenerateDescriptorSets(m_wireframe_pipeline, 2);
}

void TestPipelines::CreatePipelines()
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

	// Phong pipeline
	{
		pipeline_info.layout = {
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
			},
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
			},
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
			},
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment },
				{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
			}
		};

		pipeline_info.polygon_mode = PipelinePolygonMode_Fill;

		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "pipelines_phong.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "pipelines_phong.frag");

		m_phong_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
	}

	// Toon pipeline
	{
		pipeline_info.layout = {
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
			},
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex },
			},
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
			},
			{
				{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment },
				{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
			}
		};

		pipeline_info.polygon_mode = PipelinePolygonMode_Fill;

		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "pipelines_toon.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "pipelines_toon.frag");

		m_toon_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
	}

	// Wireframe pipeline
	{
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

		shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "pipelines_wireframe.vert");
		shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "pipelines_wireframe.frag");

		m_wireframe_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);
	}

	m_backend->InitImGuiForDynamicRendering(m_wireframe_pipeline->GetRenderingCreateInfo());
}

void TestPipelines::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
		}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_wireframe_pipeline, 0);

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
		m_model_data.model = glm::mat4(1.0f);

		m_model_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));
		m_model_descriptor = m_backend->CreateDescriptorSet(m_wireframe_pipeline, 1);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_model_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_model_descriptor, descriptor_data_objects);
	}

	// Light descriptor
	{
		m_light_data.position = glm::vec3(0.0f, 5.0f, 8.0f);
		m_light_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(LightData));
		m_light_descriptor = m_backend->CreateDescriptorSet(m_phong_pipeline, 2);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = m_light_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = 0;
		descriptor_data.data.buffer.ranges = sizeof(LightData);

		std::vector<DescriptorSetWriteData> descriptor_data_light = { descriptor_data };
		m_backend->WriteDescriptor(&m_light_descriptor, descriptor_data_light);
	}
}
#endif // _TEST_BED_ENABLED