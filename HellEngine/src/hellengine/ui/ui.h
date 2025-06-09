#pragma once

// Internal
#include "core.h"

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

			void Begin() { m_backend->Begin(); }
			void End() { m_backend->End(); }

			HE_API void BeginDocking() { m_backend->BeginDocking(); }
			HE_API void EndDocking() { m_backend->EndDocking(); }

			HE_API void ShowDemoWindow(b8* p_open = nullptr);

		private:
			IUIBackend* m_backend;
		};

	} // namespace ui

} // namespace hellengine