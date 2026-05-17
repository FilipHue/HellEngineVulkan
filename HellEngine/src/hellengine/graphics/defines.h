#pragma once

// External
#if defined(HE_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

constexpr auto MIN_BUFFER_OFFSET_ALIGNMENT = 64U;

/*
* std140 layout rules for uniform buffers:
* - Scalars (float, int, uint, bool) have a base alignment of 4 bytes.
* - vec2 has a base alignment of 8 bytes.
* - vec3 and vec4 have a base alignment of 16 bytes.
* - Arrays of scalars or vectors have a base alignment equal to the base alignment of a single element, rounded up to the next multiple of 16 bytes.
* - Structs have a base alignment equal to the largest base alignment of any of their members, rounded up to the next multiple of 16 bytes.
* - The offset of a member within a struct must be a multiple of the base alignment of that member.
* - The total size of a struct must be a multiple of the base alignment of its largest member.
*/
constexpr auto LAYOUT_STD140_ALIGNMENT = 16U;

/*
* std430 layout rules for storage buffers:
* - Scalars (float, int, uint, bool) have a base alignment of 4 bytes.
* - vec2 has a base alignment of 8 bytes.
* - vec3 and vec4 have a base alignment of 16 bytes.
* - Arrays of scalars or vectors have a base alignment equal to the base alignment of a single element, without rounding up.
* - Structs have a base alignment equal to the largest base alignment of any of their members, without rounding up.
* - The offset of a member within a struct must be a multiple of the base alignment of that member.
* - The total size of a struct must be a multiple of the base alignment of its largest member.
*/
constexpr auto LAYOUT_STD430_ALIGNMENT = 16U;

namespace hellengine
{

	namespace graphics
	{

#define STR_ERROR(e) case VK_ ##e: return #e
#define VK_CHECK(e) do { VkResult res = e; if (res != VK_SUCCESS) { HE_GRAPHICS_ERROR("Vulkan error: {0}", VkResultToString(res)); } } while(0)

		static const char* VkResultToString(VkResult result)
		{
			switch (result)
			{
				STR_ERROR(NOT_READY);
				STR_ERROR(TIMEOUT);
				STR_ERROR(EVENT_SET);
				STR_ERROR(EVENT_RESET);
				STR_ERROR(INCOMPLETE);
				STR_ERROR(ERROR_OUT_OF_HOST_MEMORY);
				STR_ERROR(ERROR_OUT_OF_DEVICE_MEMORY);
				STR_ERROR(ERROR_INITIALIZATION_FAILED);
				STR_ERROR(ERROR_DEVICE_LOST);
				STR_ERROR(ERROR_MEMORY_MAP_FAILED);
				STR_ERROR(ERROR_LAYER_NOT_PRESENT);
				STR_ERROR(ERROR_EXTENSION_NOT_PRESENT);
				STR_ERROR(ERROR_FEATURE_NOT_PRESENT);
				STR_ERROR(ERROR_INCOMPATIBLE_DRIVER);
				STR_ERROR(ERROR_TOO_MANY_OBJECTS);
				STR_ERROR(ERROR_FORMAT_NOT_SUPPORTED);
				STR_ERROR(ERROR_FRAGMENTED_POOL);
				STR_ERROR(ERROR_UNKNOWN);
				STR_ERROR(ERROR_OUT_OF_POOL_MEMORY);
				STR_ERROR(ERROR_INVALID_EXTERNAL_HANDLE);
				STR_ERROR(ERROR_FRAGMENTATION);
				STR_ERROR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
				STR_ERROR(ERROR_SURFACE_LOST_KHR);
				STR_ERROR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STR_ERROR(SUBOPTIMAL_KHR);
				STR_ERROR(ERROR_OUT_OF_DATE_KHR);
				STR_ERROR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STR_ERROR(ERROR_VALIDATION_FAILED_EXT);
				STR_ERROR(ERROR_INVALID_SHADER_NV);
				STR_ERROR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
				STR_ERROR(ERROR_NOT_PERMITTED_EXT);
				STR_ERROR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
				STR_ERROR(THREAD_IDLE_KHR);
				STR_ERROR(THREAD_DONE_KHR);
				STR_ERROR(OPERATION_DEFERRED_KHR);
				STR_ERROR(OPERATION_NOT_DEFERRED_KHR);
				STR_ERROR(PIPELINE_COMPILE_REQUIRED_EXT);
			default: return "UNKNOWN_ERROR";
			}
		}

	} // namespace graphics

} // namespace hellengine