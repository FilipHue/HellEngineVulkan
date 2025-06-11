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
			HE_API Viewport(const std::string& name);
			HE_API virtual ~Viewport() = default;

			HE_API void* GetHandle() const { return m_handle; }
			HE_API void SetHandle(void* handle) { m_handle = handle; }

			HE_API void Draw() override;

		private:
			void* m_handle;
		};

	} // namespace ui

} // namespace hellengine