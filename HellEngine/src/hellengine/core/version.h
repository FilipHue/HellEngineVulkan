#pragma once

// Internal
#include "typedefs.h"

// Copied from defines.h for cleaner code
#define _TO_STRING_IMPL(x) #x
#define TO_STRING(x) _TO_STRING_IMPL(x)

#define HELL_VERSION_MAJOR 1
#define HELL_VERSION_MINOR 0
#define HELL_VERSION_PATCH 0

#define HELL_MAKE_VERSION(major, minor, patch) \
	((((u32)(major)) << 22U) | (((u32)(minor)) << 12U) | ((u32)(patch)))

#define HELL_VERSION_STRING \
	TO_STRING(HELL_VERSION_MAJOR) "." \
	TO_STRING(HELL_VERSION_MINOR) "." \
	TO_STRING(HELL_VERSION_PATCH)

#define HELL_VERSION_NUMBER \
	HELL_MAKE_VERSION(HELL_VERSION_MAJOR, HELL_VERSION_MINOR, HELL_VERSION_PATCH)

#define HELL_VERSION_NAME "HellEngine"