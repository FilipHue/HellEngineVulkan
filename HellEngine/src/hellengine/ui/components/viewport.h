#pragma once

// Internal
#include "hellengine/ui/components/panel.h"
#include "hellengine/ui/shared.h"

namespace hellengine
{

	namespace ui
	{

		class Viewport : public Panel
		{
		public:
			Viewport(const std::string& name);
			virtual ~Viewport() = default;

			void* GetHandle() const { return m_handle; }
			void SetHandle(void* handle) { m_handle = handle; }

			virtual b8 Begin() override;
			virtual void Draw() override;
			virtual void End() override;

		protected:
			void* m_handle;
		};

	} // namespace ui

} // namespace hellengine