#pragma once

#if defined(HE_DLL)
	#if defined(HE_PLATFORM_WINDOWS)
		#if defined(HE_BUILD_DLL)
			#define HE_API __declspec(dllexport)
		#else
			#define HE_API __declspec(dllimport)
		#endif // HE_BUILD_DLL
	#else
		#error HellEngine only supports Windows!
	#endif // HE_PLATFORM_WINDOWS
#else
	#define HE_API
#endif // HE_DLL