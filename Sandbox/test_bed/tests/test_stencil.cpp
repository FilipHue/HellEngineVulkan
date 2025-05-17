#include "test_stencil.h"

TestStencil::TestStencil(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}	

TestStencil::~TestStencil()
{
	NO_OP;
}

void TestStencil::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera.SetPosition({ 0.0f, 0.0f, -5.0f });
	m_camera.RotateOX(glm::radians(2.5f));
	m_camera.RotateOY(glm::radians(180.0f));

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	LoadResources();

	CreatePipeline();
	CreateDescriptorSets();

	m_model->UploadToGPU<VertexFormatBase>();
	m_model->GenerateDescriptorSets(m_pipeline_toon, 2);
}

void TestStencil::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestStencil::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_object_buffer_toon, &m_object_data_toon, sizeof(ModelDataToon));
	m_backend->UpdateUniformBuffer(m_object_buffer_outline, &m_object_data_outline, sizeof(ModelDataOutline));
}

void TestStencil::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight() } } });

	m_backend->BindDescriptorSet(m_pipeline_outline, m_camera_descriptor);

	// Toon
	{
		m_backend->BindPipeline(m_pipeline_toon);

		m_backend->BindDescriptorSet(m_pipeline_toon, m_object_descriptor_toon);

		m_model->Draw(m_pipeline_toon, 0);
	}

	// Outline 
	{
		m_backend->BindPipeline(m_pipeline_outline);

		m_backend->BindDescriptorSet(m_pipeline_outline, m_object_descriptor_outline);

		m_model->Draw(m_pipeline_outline, -1);
	}

	m_backend->EndDynamicRendering();
}

void TestStencil::RenderEnd()
{
}

void TestStencil::UIRender()
{
}

void TestStencil::Cleanup()
{
	m_backend->DestroyUniformBuffer(m_camera_buffer);
	m_backend->DestroyUniformBuffer(m_object_buffer_toon);
	m_backend->DestroyUniformBuffer(m_object_buffer_outline);

	m_backend->DestroyPipeline(m_pipeline_toon);
	m_backend->DestroyPipeline(m_pipeline_outline);
}

b8 TestStencil::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestStencil::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestStencil::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestStencil::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestStencil::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestStencil::OnKeyPressed(EventContext& event)
{
	if (event.data.key_event.key == KEY_LEFT)
	{
		m_object_data_outline.outline_width -= 0.01f;
	}

	if (event.data.key_event.key == KEY_RIGHT)
	{
		m_object_data_outline.outline_width += 0.01f;
	}

	return false;
}

b8 TestStencil::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestStencil::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestStencil::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestStencil::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestStencil::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestStencil::LoadResources()
{
	m_model = AssetManager::LoadModel(FileManager::ReadFile(CONCAT_PATHS(MODEL_PATH, "venus.gltf")));
}

void TestStencil::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};

	pipeline_info.topology = PipelinePrimitiveTopology_TriangleList;
	pipeline_info.polygon_mode = PipelinePolygonMode_Fill;
	pipeline_info.cull_mode = PipelineCullMode_None;
	pipeline_info.front_face = PipelineFrontFace_Clockwise;
	pipeline_info.line_width = 1.0f;

	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex}
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Fragment },
			{ 1, DescriptorType_CombinedImageSampler, ShaderStage_Fragment }
		}
	};
	pipeline_info.vertex_binding_description = VertexFormatBase::GetBindingDescription();
	pipeline_info.vertex_attribute_descriptions = VertexFormatBase::GetAttributeDescriptions();

	pipeline_info.depth_stencil_info.stencil_test_enable = true;
	pipeline_info.depth_stencil_info.depth_test_enable = true;
	pipeline_info.depth_stencil_info.depth_write_enable = true;
	pipeline_info.depth_stencil_info.depth_compare_op = PipelineDethStencilCompareOp_LessOrEqual;
	pipeline_info.depth_stencil_info.back = {
		VK_STENCIL_OP_REPLACE,
		VK_STENCIL_OP_REPLACE,
		VK_STENCIL_OP_REPLACE,
		VK_COMPARE_OP_ALWAYS,
		0xff,
		0xff,
		1
	};
	pipeline_info.depth_stencil_info.front = pipeline_info.depth_stencil_info.back;

	pipeline_info.dynamic_rendering_info = {
		{ VK_FORMAT_B8G8R8A8_SRGB },
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};

	ShaderStageInfo shader_info = {};
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "stencil_toon.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "stencil_toon.frag");

	m_pipeline_toon = m_backend->CreatePipeline(pipeline_info, shader_info);

	pipeline_info.layout = {
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		},
		{
			{ 0, DescriptorType_UniformBuffer, ShaderStage_Vertex }
		}
	};

	pipeline_info.depth_stencil_info.depth_test_enable = false;
	pipeline_info.depth_stencil_info.depth_write_enable = false;
	pipeline_info.depth_stencil_info.back = {
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_REPLACE,
		VK_STENCIL_OP_KEEP,
		VK_COMPARE_OP_NOT_EQUAL,
		0xff,
		0xff,
		1
	};
	pipeline_info.depth_stencil_info.front = pipeline_info.depth_stencil_info.back;

	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "stencil_outline.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "stencil_outline.frag");

	m_pipeline_outline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline_outline->GetRenderingCreateInfo());
}

void TestStencil::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPool({
		{ DescriptorType_UniformBuffer, 1 },
		{ DescriptorType_CombinedImageSampler, 1 }
	}, 1);

	// Camera descriptor
	{
		m_camera_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(CameraData));
		m_camera_descriptor = m_backend->CreateDescriptorSet(m_pipeline_toon, 0);

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Object descriptor toon
	{
		auto data = ModelDataToon();
		data.model = glm::mat4(1.0f);
		data.light_position = glm::vec4(0.0f, -2.0f, 1.0f, 0.0f);
		m_object_data_toon = data;

		m_object_descriptor_toon = m_backend->CreateDescriptorSet(m_pipeline_toon, 1);
		m_object_buffer_toon = m_backend->CreateUniformBufferMappedPersistent(sizeof(ModelDataToon));

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_object_buffer_toon->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(ModelDataToon);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_object_descriptor_toon, descriptor_data_objects);
	}

	// Object descriptor outline
	{
		auto data = ModelDataOutline();
		data.model = glm::mat4(1.0f);
		data.outline_width = 0.025f;
		m_object_data_outline = data;

		m_object_descriptor_outline = m_backend->CreateDescriptorSet(m_pipeline_outline, 1);
		m_object_buffer_outline = m_backend->CreateUniformBufferMappedPersistent(sizeof(ModelDataOutline));

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffer = m_object_buffer_outline->GetHandle();
		descriptor_data.data.buffer.offset = 0;
		descriptor_data.data.buffer.range = sizeof(ModelDataOutline);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_object_descriptor_outline, descriptor_data_objects);
	}
}
