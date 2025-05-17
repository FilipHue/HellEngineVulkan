#pragma once

// Internal
#include "backend/backend.h"

namespace hellengine
{
	
	namespace ui
	{

		class UI
		{
		public:
			UI() = default;
			~UI() = default;

			void Init(Window* window, VulkanBackend* backend);
			void Shutdown();

			b8 HandleEvents (EventContext& event);

			void Begin() { m_backend->Begin(); }
			void End() { m_backend->End(); }

			HE_API void ShowDemoWindow(b8* p_open = nullptr);

		private:
			IUIBackend* m_backend;
		};

	} // namespace ui

} // namespace hellengine