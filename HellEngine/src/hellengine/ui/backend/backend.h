#pragma once
// Internal
#include "hellengine/core/window/window.h"
#include "hellengine/core/events/event_types.h"
#include "hellengine/graphics/backend/vulkan_backend.h"

// External
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace hellengine
{

	using namespace core;
	using namespace graphics;

	namespace ui
	{

		class IUIBackend
		{
		public:
			IUIBackend() = default;
			virtual ~IUIBackend() = default;

			virtual void Init(Window* window, VulkanBackend* backend) = 0;
			virtual void Shutdown() = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			virtual b8 HandleEvents(EventContext& event) = 0;
		};

		class UIBackend_ImGui : public IUIBackend
		{
		public:
			UIBackend_ImGui() = default;
			~UIBackend_ImGui() = default;

			void Init(Window* window, VulkanBackend* backend) override;
			void Shutdown() override;

			void Begin() override;
			void End() override;

			b8 HandleEvents(EventContext& event) override;

		private:
			void BeginDocking();
			void EndDocking();

		private:
			VulkanBackend* m_backend = nullptr;
		};

	} // namespace ui

} // namespace hellengine