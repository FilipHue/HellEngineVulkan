#pragma once

// Internal
#include "logging/logger.h"
	
namespace hellengine
{

	namespace core
	{
	
#if defined(HE_DEBUG)
	#define HE_ASSERT(x, ...) { if(!(x)) { HE_CORE_ERROR("Assertion Failed: {0} | File: {1} | Line: {2}", __VA_ARGS__, __FILE__, __LINE__); DEBUG_BREAK; } }
#else
	#define HE_ASSERT(x, ...)
#endif // HE_DEBUG

#define HE_STATIC_ASSERT static_assert

	} // namespace core

} // namespace hellengine