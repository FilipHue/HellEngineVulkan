#pragma once

/************************************************/
/*               System Detection               */
/************************************************/

/* Windows */
#if defined(_WIN32)
    /* Windows x64 */
    #if defined(_WIN64)
        #define HE_WINDOWS_X64
    #else
        /* Windows x86 is not supported */
        #error "HellEngine doesn't support 32-bit Windows (x86) builds! Please switch to a 64-bit environment."
    #endif

/* Apple platforms */
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #if TARGET_OS_IPHONE
            #error "HellEngine doesn't support iOS!"
        #elif TARGET_IPHONE_SIMULATOR
            #error "HellEngine doesn't support iOS Simulator!"
        #elif TARGET_OS_MACCATALYST
            #error "HellEngine doesn't support Mac Catalyst!"
        #else
            #error "HellEngine doesn't support macOS!"
        #endif
    #else
        #error "Unknown Apple platform!"
    #endif

/* Android */
#elif defined(__ANDROID__)
    #error "HellEngine doesn't currently support Android! Please use a different platform."

/* Linux */
#elif defined(__linux__) and not defined(__ANDROID__)
    #error "HellEngine doesn't currently support Linux! Please use a different platform."

/* FreeBSD */
#elif defined(__FreeBSD__)
    #error "HellEngine doesn't support FreeBSD!"

/* Web Assembly */
#elif defined(__EMSCRIPTEN__)
    #error "HellEngine doesn't support WebAssembly (Emscripten)!"

/* Platform not detected or unknown */
#else
    #error "Unknown platform! HellEngine doesn't support this platform yet."
#endif


/************************************************/
/*              Compiler Detection              */
/************************************************/

/* Microsoft Visual Studio */
#if defined(_MSC_VER)
    #define HE_COMPILER_MSVC
#if _MSC_VER < 1920
    #error "HellEngine requires at least Visual Studio 2019 (MSVC 16.0 or higher)!"
#endif

/* Clang */
#elif defined(__clang__)
    #define HE_COMPILER_CLANG
#if __clang_major__ < 10
    #error "HellEngine requires at least Clang 10.0!"
#endif

/* GCC (GNU Compiler Collection) */
#elif defined(__GNUC__) || defined(__GNUG__)
    #define HE_COMPILER_GCC
#if __GNUC__ < 9
    #error "HellEngine requires at least GCC 9.0!"
#endif

#else
    #error "Unknown compiler! HellEngine only supports MSVC, Clang, and GCC."
#endif

/************************************************/
/*              Standard Detection              */
/************************************************/

// C++17 or higher required
// This macro is set to 199711L by default for compatibility reasons.
// See https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170 for more information.
// You need to enable /Zc:__cplusplus in Properties->C/C++->Command Line->Additional Options to get the correct value.
#if __cplusplus < 201703L
    #error "HellEngine requires at least C++17 support! Please enable C++17 or higher."
#endif