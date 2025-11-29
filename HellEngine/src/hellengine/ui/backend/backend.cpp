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
			io.ConfigDragClickToInputText = true;

			SetDarkTheme();

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
		}

		void UIBackend_ImGui::End()
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Render();

			m_backend->SubmitImGui();
		}

		void UIBackend_ImGui::BeginDocking()
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			window_flags |= ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);
			ImGui::PopStyleVar(3);

			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		}

		void UIBackend_ImGui::EndDocking()
		{
			ImGui::End();
		}

		void UIBackend_ImGui::SetDarkTheme()
		{
			// General
			auto* style = &ImGui::GetStyle();
			style->FrameRounding = 2.0f;
			style->WindowPadding = ImVec2(4.0f, 3.0f);
			style->FramePadding = ImVec2(4.0f, 4.0f);
			style->ItemSpacing = ImVec2(4.0f, 3.0f);
			style->IndentSpacing = 12;
			style->ScrollbarSize = 12;
			style->GrabMinSize = 9;

			// Sizes
			style->WindowBorderSize = 0.0f;
			style->ChildBorderSize = 0.0f;
			style->PopupBorderSize = 0.0f;
			style->FrameBorderSize = 0.0f;
			style->TabBorderSize = 0.0f;


			style->WindowRounding = 0.0f;
			style->ChildRounding = 0.0f;
			style->FrameRounding = 0.0f;
			style->PopupRounding = 0.0f;
			style->GrabRounding = 2.0f;
			style->ScrollbarRounding = 12.0f;
			style->TabRounding = 0.0f;

			// Colors
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.22f, 0.24f, 0.25f, 1.00f);
			colors[ImGuiCol_Border] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.15f, 0.16f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
			colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.22f, 0.23f, 0.25f, 1.00f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.84f, 0.34f, 0.17f, 0.14f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.59f, 0.24f, 0.12f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.84f, 0.34f, 0.17f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.28f, 0.14f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.14f, 0.16f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);
			colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.75f, 0.30f, 0.15f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
		}

	} // namespace ui

} // namespace hellengine