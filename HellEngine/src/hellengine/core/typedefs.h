#pragma once

#include "assert.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

typedef bool b8;

#define I32MAX INT32_MAX
#define I64MAX INT64_MAX
#define U32MAX UINT32_MAX
#define U64MAX UINT64_MAX

#define I32MIN INT32_MIN
#define I64MIN INT64_MIN
#define U32MIN UINT32_MIN
#define U64MIN UINT64_MIN

HE_STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 (unsigned char) to be 1 byte long");
HE_STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 (unsigned short) to be 2 bytes long");
HE_STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 (unsigned int) to be 4 bytes long");
HE_STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 (unsigned long long) to be 8 bytes long");

HE_STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 (signed char) to be 1 byte long");
HE_STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 (signed short) to be 2 bytes long");
HE_STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 (signed int) to be 4 bytes long");
HE_STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 (signed long long) to be 8 bytes long");

HE_STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 (float) to be 4 bytes long");
HE_STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 (double) to be 8 bytes long");

HE_STATIC_ASSERT(sizeof(b8) == 1, "Expected b8 (bool) to be 1 byte long");