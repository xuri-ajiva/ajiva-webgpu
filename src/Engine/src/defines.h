//
// Created by XuriAjiva on 31.07.2023.
//
#pragma once

#include <cstdint>

// based on https://github.com/travisvroman/kohi/blob/8cf14746f61799184d21f96a4885eef20dede3f9/engine/src/defines.h

// Unsigned int types.

/** @brief Unsigned 8-bit integer */
typedef uint8_t u8;

/** @brief Unsigned 16-bit integer */
typedef uint16_t u16;

/** @brief Unsigned 32-bit integer */
typedef uint32_t u32;

/** @brief Unsigned 64-bit integer */
typedef uint64_t u64;

// Signed int types.

/** @brief Signed 8-bit integer */
typedef int8_t i8;

/** @brief Signed 16-bit integer */
typedef int16_t i16;

/** @brief Signed 32-bit integer */
typedef int32_t i32;

/** @brief Signed 64-bit integer */
typedef int64_t i64;

// Floating point types

/** @brief 32-bit floating point number */
typedef float f32;

/** @brief 64-bit floating point number */
typedef double f64;

// Boolean types

/** @brief 32-bit boolean type, used for APIs which require it */
typedef int b32;

/** @brief 8-bit boolean type */
typedef bool b8;

/** @brief A range, typically of memory */
typedef struct range {
    /** @brief The offset in bytes. */
    u64 offset;
    /** @brief The size in bytes. */
    u64 size;
} range;

// Properly define static assertions.

// Ensure all types are of the correct size.

/** @brief Assert u8 to be 1 byte.*/
static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");

/** @brief Assert u16 to be 2 bytes.*/
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");

/** @brief Assert u32 to be 4 bytes.*/
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");

/** @brief Assert u64 to be 8 bytes.*/
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

/** @brief Assert i8 to be 1 byte.*/
static_assert(sizeof(i8) == 1, "Expected i8 to be 1 byte.");

/** @brief Assert i16 to be 2 bytes.*/
static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");

/** @brief Assert i32 to be 4 bytes.*/
static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");

/** @brief Assert i64 to be 8 bytes.*/
static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

/** @brief Assert f32 to be 4 bytes.*/
static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");

/** @brief Assert f64 to be 8 bytes.*/
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

/**
 * @brief Any id set to this should be considered invalid,
 * and not actually pointing to a real object.
 */
#define INVALID_ID_U64 18446744073709551615UL
#define INVALID_ID 4294967295U
#define INVALID_ID_U16 65535U
#define INVALID_ID_U8 255U

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define AJ_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define AJ_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define AJ_PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define AJ_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define AJ_PLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
#define AJ_PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define AJ_PLATFORM_IOS 1
#define AJ_PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define AJ_PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif

#ifdef AJ_EXPORT
// Exports
#ifdef _MSC_VER
#define AJ_API __declspec(dllexport)
#else
#define AJ_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
/** @brief Import/export qualifier */
//#define AJ_API __declspec(dllimport)
#define AJ_API
#else
/** @brief Import/export qualifier */
#define AJ_API
#endif
#endif

// Inlining
#if defined(__clang__) || defined(__gcc__)
/** @brief Inline qualifier */
#define AJ_INLINE __attribute__((always_inline)) inline

/** @brief No-inline qualifier */
#define AJ_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)

/** @brief Inline qualifier */
#define AJ_INLINE __forceinline

/** @brief No-inline qualifier */
#define AJ_NOINLINE __declspec(noinline)
#else

/** @brief Inline qualifier */
#define AJ_INLINE static inline

/** @brief No-inline qualifier */
#define AJ_NOINLINE
#endif

/** @brief Gets the number of bytes from amount of gibibytes (GiB) (1024*1024*1024) */
#define GIBIBYTES(amount) ((amount) * 1024ULL * 1024ULL * 1024ULL)
/** @brief Gets the number of bytes from amount of mebibytes (MiB) (1024*1024) */
#define MEBIBYTES(amount) ((amount) * 1024ULL * 1024ULL)
/** @brief Gets the number of bytes from amount of kibibytes (KiB) (1024) */
#define KIBIBYTES(amount) ((amount) * 1024ULL)

/** @brief Gets the number of bytes from amount of gigabytes (GB) (1000*1000*1000) */
#define GIGABYTES(amount) ((amount) * 1000ULL * 1000ULL * 1000ULL)
/** @brief Gets the number of bytes from amount of megabytes (MB) (1000*1000) */
#define MEGABYTES(amount) ((amount) * 1000ULL * 1000ULL)
/** @brief Gets the number of bytes from amount of kilobytes (KB) (1000) */
#define KILOBYTES(amount) ((amount) * 1000ULL)

AJ_INLINE u64 get_aligned(u64 operand, u64 granularity) {
    return ((operand + (granularity - 1)) & ~(granularity - 1));
}

AJ_INLINE range get_aligned_range(u64 offset, u64 size, u64 granularity) {
    return static_cast<range>(range{get_aligned(offset, granularity), get_aligned(size, granularity)});
}

#define AJ_MIN(x, y) (x < y ? x : y)
#define AJ_MAX(x, y) (x > y ? x : y)