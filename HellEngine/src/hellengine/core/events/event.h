#pragma once

// Internal
#include "event_types.h"

// Standard
#include <array>
#include <vector>

namespace hellengine
{

	namespace core
	{

		class EventDispatcher
		{
		public:
			EventDispatcher() = delete;

			HE_API static void AddListener(EventType type, EventCallback callback);
			HE_API static void RemoveListener(EventType type, EventCallback callback);

			static void Dispatch(EventContext& event);

		private:
			INLINE static std::array<std::vector<EventCallback>, EventType_Count> m_listeners = {};
		};

	} // namespace core

} // namespace hellengine