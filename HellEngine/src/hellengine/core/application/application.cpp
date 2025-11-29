#include "hepch.h"
#include "application.h"

// Internal
#include <hellengine/core/input/input_codes.h>
#include <hellengine/core/logging/logger.h>

namespace hellengine
{

	using namespace graphics;
	namespace core
	{

		Application::Application(ApplicationConfiguration* configuration)
		{
			m_configuration = configuration;
		}

		Application::~Application()
		{
			NO_OP;
		}

		void Application::SetCursorMode(u32 mode)
		{
			m_window->SetCursorMode(mode);
		}

		void Application::SetWindowTitle(const char* title)
		{
			m_window->SetTitle(title);
		}

		void Application::Setup(ApplicationConfiguration* configuration)
		{
			HE_CORE_TRACE("========== Initializing application core ==========");

			m_configuration = configuration;

			HE_CORE_DEBUG("Creating application in {0}", m_configuration->working_directory);

			m_window = Window::Create({ configuration->width, configuration->height, configuration->title, {100, 100} });

			HE_CORE_TRACE("========== Application core initialized ==========");

			HE_CORE_TRACE("========== Initializing application renderer ==========");

			m_backend = new VulkanBackend();
			m_backend->Init(m_window);
			m_backend->SetRecordCallback([&]() {
				OnRenderBegin();
				OnRenderUpdate();
				OnRenderEnd();

				m_ui->Begin();
				OnUIRender();
				m_ui->End();
			});

			m_frontend = new VulkanFrontend();
			m_frontend->Init(m_backend);

			m_ui = new UI();
			m_ui->Init(m_window, m_backend);

			HE_CORE_TRACE("========== Application renderer initialized ==========");

			Init();
		}

		void Application::Cleanup()
		{
			Shutdown();

			HE_CORE_TRACE("========== Shutting down application ==========");

			m_ui->Shutdown();
			delete m_ui;

			m_frontend->Shutdown();
			delete m_frontend;

			m_backend->Shutdown();
			delete m_backend;

			delete m_window;

			HE_CORE_TRACE("========== Application shut down ==========");
		}

		void Application::OnPollEvents() const
		{
			m_window->PollEvents();
		}

		b8 Application::IsRunning() const
		{
			return m_running;
		}

		b8 Application::IsSuspended() const
		{
			return m_suspended;
		}

		Window* Application::GetWindow() const
		{
			return m_window;
		}

	} // namespace core

} // namespace hellengine