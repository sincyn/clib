/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;


#define true 1
#define false 0
#define null ((void *)0)

// Compiler detection
#if defined(_MSC_VER) && !defined(__clang__)
#define CL_COMPILER_MSVC
#elif defined(__clang__)
#define CL_COMPILER_CLANG
#elif defined(__GNUC__)
#define CL_COMPILER_GCC
#elif defined(__ZIG__)
#define CL_COMPILER_ZIG
#else
#error "Unsupported compiler"
#endif

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
#define CL_PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#define CL_PLATFORM_APPLE
#elif defined(__linux__)
#define CL_PLATFORM_LINUX
#else
#error "Unsupported platform"
#endif
