#pragma once

// Internal
#include "configuration.h"
#include <hellengine/core/events/event.h>
#include <hellengine/core/window/window.h>
#include <hellengine/core/api.h>

#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/graphics/frontend/vulkan_frontend.h>

namespace hellengine
{

	namespace core
	{

		class Application
		{
		public:
			HE_API Application(ApplicationConfiguration* configuration);
			HE_API virtual ~Application();

			HE_API void SetCursorMode(u32 mode);
			HE_API void SetWindowTitle(const char* title);

			virtual void Init() = 0;
			virtual void OnProcessUpdate(f32 delta_time) = 0;
			virtual void OnRenderBegin() = 0;
			virtual void OnRenderUpdate() = 0;
			virtual void OnRenderEnd() = 0;
			virtual void Shutdown() = 0;

			INLINE void OnPollEvents() const { m_window->PollEvents(); }

			INLINE b8 IsRunning() const { return m_running; }
			INLINE b8 IsSuspended() const { return m_suspended; }

		private:
			void Setup(ApplicationConfiguration* configuration);
			void Cleanup();

			friend class Engine;

		protected:
			ApplicationConfiguration* m_configuration = nullptr;
			Window* m_window = nullptr;
			graphics::VulkanBackend* m_backend = nullptr;
			graphics::VulkanFrontend* m_frontend = nullptr;

			b8 m_running = true;
			b8 m_suspended = false;
		};

	} // namespace core

} // namespace hellengine