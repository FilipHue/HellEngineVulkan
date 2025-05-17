#include "hepch.h"
#include "backend.h"

namespace hellengine
{

	namespace ui
	{

		void UIBackend_ImGui::Init(Window* window, VulkanBackend* backend)
		{
			m_backend = backend;

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)window->GetHandle(), false);

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigDockingWithShift = true;

			m_backend->CreateImGuiGraphicContext(window);
		}

		void UIBackend_ImGui::Shutdown()
		{
			m_backend->DestroyImGuiGraphicContext();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		void UIBackend_ImGui::Begin()
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			BeginDocking();
		}

		void UIBackend_ImGui::End()
		{
			EndDocking();

			ImGuiIO& io = ImGui::GetIO();
			ImGui::Render();

			m_backend->SubmitImGui();
		}

		b8 UIBackend_ImGui::HandleEvents(EventContext& event)
		{
			ImGuiIO& io = ImGui::GetIO();

			return false;
		}

		void UIBackend_ImGui::BeginDocking()
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		}

		void UIBackend_ImGui::EndDocking()
		{
		}

	} // namespace ui

} // namespace hellengine