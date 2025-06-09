#include "test_triangle.h"

TestTriangle::TestTriangle(ApplicationConfiguration* configuration) : TestBase(configuration)
{
	NO_OP;
}	

TestTriangle::~TestTriangle()
{
	NO_OP;
}

void TestTriangle::Setup()
{
	m_clear_color = glm::vec4(0.0f, 0.0f, 0.2f, 1.0f);

	m_camera.SetPosition({ 0.0f, 0.0f, 5.0f });

	m_camera_data.projection = m_camera.GetProjection();
	m_camera_data.view = m_camera.GetView();

	CreateTriangle();

	CreatePipeline();
	CreateDescriptorSets();
}

void TestTriangle::ProcessUpdate(f32 dt)
{
	if (m_is_camera_active)
	{
		m_camera_data.view = m_camera.GetView();
	}
}

void TestTriangle::RenderBegin()
{
	m_backend->SetExtent({ m_window->GetWidth(), m_window->GetHeight() });
	m_backend->SetClearColor({ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a });

	m_backend->UpdateUniformBuffer(m_camera_buffer, &m_camera_data, sizeof(CameraData));
	m_backend->UpdateUniformBuffer(m_triangle_buffer, &m_triangle_data, sizeof(ObjectData));
}

void TestTriangle::RenderUpdate()
{
	m_backend->BeginDynamicRendering();

	m_backend->SetViewport({ { 0.0f, 0.0f, (f32)m_window->GetWidth(), (f32)m_window->GetHeight(), 0.0f, 1.0f } });
	m_backend->SetScissor({ { { 0, 0 }, { m_window->GetWidth(), m_window->GetHeight()} } });

	m_backend->BindPipeline(m_pipeline);
	m_backend->BindDescriptorSet(m_pipeline, m_camera_descriptor);

	m_backend->BindVertexBuffer(m_vertex_buffer, 0);
	m_backend->BindIndexBuffer(m_index_buffer, 0);

	m_backend->BindDescriptorSet(m_pipeline, m_triangle_descriptor);
	m_backend->DrawIndexed((u32)m_indices.size(), 1, 0, 0, 0);

	m_backend->EndDynamicRendering();
}

void TestTriangle::RenderEnd()
{
}

void TestTriangle::UIRender()
{
}

void TestTriangle::Cleanup()
{
	m_backend->DestroyBuffer(m_index_buffer);
	m_backend->DestroyBuffer(m_vertex_buffer);
	m_backend->DestroyUniformBuffer(m_camera_buffer);
	m_backend->DestroyUniformBuffer(m_triangle_buffer);
	m_backend->DestroyPipeline(m_pipeline);
}

b8 TestTriangle::OnWindowClose(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnWindowResize(EventContext& event)
{
	m_camera_data.projection = m_camera.GetProjection();

	return false;
}

b8 TestTriangle::OnWindowFocus(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnWindowIconified(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnWindowMoved(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnKeyPressed(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnKeyReleased(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnMouseButtonPressed(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnMouseButtonReleased(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnMouseMoved(EventContext& event)
{
	return false;
}

b8 TestTriangle::OnMouseScrolled(EventContext& event)
{
	return false;
}

void TestTriangle::CreateTriangle()
{
	m_vertices = {
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } }
	};

	m_indices = {
		0, 1, 2
	};

	m_vertex_buffer = m_backend->CreateVertexBuffer(m_vertices.data(), (u32)m_vertices.size());
	m_index_buffer = m_backend->CreateIndexBuffer(m_indices.data(), (u32)m_indices.size());
}

void TestTriangle::CreatePipeline()
{
	PipelineCreateInfo pipeline_info = {};
	pipeline_info.dynamic_states = { PipelineDynamicState_Viewport, PipelineDynamicState_Scissor };
	pipeline_info.layout = {
		{
			{ 
				{ 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None }
			},
			DescriptorSetFlags_None
		},
		{
			{ { 0, DescriptorType_UniformBuffer, 1, ShaderStage_Vertex, DescriptorBindingFlags_None } },
			DescriptorSetFlags_None
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
	shader_info.sources[ShaderType_Vertex] = CONCAT_PATHS(SHADER_PATH, "triangle.vert");
	shader_info.sources[ShaderType_Fragment] = CONCAT_PATHS(SHADER_PATH, "triangle.frag");

	m_pipeline = m_backend->CreatePipeline(pipeline_info, shader_info);

	m_backend->InitImGuiForDynamicRendering(m_pipeline->GetRenderingCreateInfo());
}

void TestTriangle::CreateDescriptorSets()
{
	// Init pool
	m_backend->InitDescriptorPoolGrowable({
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
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_camera_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(CameraData);

		std::vector<DescriptorSetWriteData> descriptor_data_camera = { descriptor_data };
		m_backend->WriteDescriptor(&m_camera_descriptor, descriptor_data_camera);
	}

	// Triangle descriptor
	{
		auto data = ObjectData();
		data.model = glm::mat4(1.0f);
		m_triangle_data = data;

		m_triangle_descriptor = m_backend->CreateDescriptorSet(m_pipeline, 1);
		m_triangle_buffer = m_backend->CreateUniformBufferMappedPersistent(sizeof(ObjectData));

		DescriptorSetWriteData descriptor_data;
		descriptor_data.type = DescriptorType_UniformBuffer;
		descriptor_data.binding = 0;
		descriptor_data.data.buffer.buffers = new VkBuffer[1];
		descriptor_data.data.buffer.buffers[0] = m_triangle_buffer->GetHandle();
		descriptor_data.data.buffer.offsets = new VkDeviceSize[1];
		descriptor_data.data.buffer.offsets[0] = 0;
		descriptor_data.data.buffer.ranges = new VkDeviceSize[1];
		descriptor_data.data.buffer.ranges[0] = sizeof(ObjectData);

		std::vector<DescriptorSetWriteData> descriptor_data_objects = { descriptor_data };
		m_backend->WriteDescriptor(&m_triangle_descriptor, descriptor_data_objects);
	}
}
