// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COMPILER_H_
#define SHIBA_PLATFORM_COMPILER_H_

// === Compiler detection ===
#if defined (__clang__)
    #define SHIBA_COMPILER_CLANG 1
#elif defined(_MSC_VER)     // ^^^ CLANG / vvv MSVC
    #define SHIBA_COMPILER_MSVC  1
#elif defined(__GNUC__)     // ^^^ MSVC / vvv GCC
    #define SHIBA_COMPILER_GCC   1
#else                       // ^^^ GCC / vvv unsupported
    #error "shiba: unsupported compiler"
#endif                      // ^^^ unsupported

// Define the rest to 0 so `#if SHIBA_COMPILER_X` is always valid
#ifndef SHIBA_COMPILER_CLANG
    #define SHIBA_COMPILER_CLANG 0
#endif
#ifndef SHIBA_COMPILER_MSVC
    #define SHIBA_COMPILER_MSVC 0
#endif
#ifndef SHIBA_COMPILER_GCC
    #define SHIBA_COMPILER_GCC 0
#endif

// Configuration detection
#if NDEBUG
    #define SHIBA_DEBUG 0
#else                       // ^^^ NDEBUG / vvv debug
    #define SHIBA_DEBUG 1
#endif                      // ^^^ debug

// === Inlining ===
#if SHIBA_COMPILER_MSVC
    #define SHIBA_FORCE_INLINE __forceinline
    #define SHIBA_NOINLINE     __declspec(noinline)
#else                       // ^^^ MSVC / vvv non-MSVC
    #define SHIBA_FORCE_INLINE inline __attribute__((always_inline))
    #define SHIBA_NOINLINE     __attribute__((noinline))
#endif                      // ^^^ non-MSVC

// === Branch hints ===
#if SHIBA_COMPILER_MSVC
    // MSVC doesn't know about [[likely]] in C++17
    #define SHIBA_LIKELY(x)   (x)
    #define SHIBA_UNLIKELY(x) (x)
#else                       // ^^^ MSVC / vvv non-MSVC
    #define SHIBA_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define SHIBA_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif                      // ^^^ non-MSVC

// === Control-flow intrinsics ===
#if SHIBA_COMPILER_MSVC
    #define SHIBA_UNREACHABLE   __assume(0)
    #define SHIBA_DEBUGBREAK()  __debugbreak()
    #define SHIBA_FUNCSIG       __FUNCSIG__
#else                       // ^^^ MSVC / vvv non-MSVC
    #define SHIBA_UNREACHABLE   __builtin_unreachable()
    #define SHIBA_DEBUGBREAK()  __builtin_trap()
    #define SHIBA_FUNCSIG       __PRETTY_FUNCTION__
#endif                      // ^^^ non-MSVC

// === Diagnostic push/pop ===
// compiler.h
#if SHIBA_COMPILER_MSVC
    #define SHIBA_PRAGMA(x)   __pragma(x)
    #define SHIBA_WARN_PUSH_DISABLE_ALL()  SHIBA_PRAGMA(warning(push, 0))
    #define SHIBA_WARN_POP()               SHIBA_PRAGMA(warning(pop))
#elif SHIBA_COMPILER_CLANG  // ^^^ MSVC / vvv CLANG
    #define SHIBA_PRAGMA(x)   _Pragma(#x)
    #define SHIBA_WARN_PUSH_DISABLE_ALL()             \
        SHIBA_PRAGMA(clang diagnostic push)           \
        SHIBA_PRAGMA(clang diagnostic ignored "-Weverything")
    #define SHIBA_WARN_POP()  SHIBA_PRAGMA(clang diagnostic pop)
#elif SHIBA_COMPILER_GCC    // ^^^ CLANG / vvv GCC
    #define SHIBA_PRAGMA(x)   _Pragma(#x)
    #define SHIBA_WARN_PUSH_DISABLE_ALL()             \
        SHIBA_PRAGMA(GCC diagnostic push)             \
        SHIBA_PRAGMA(GCC diagnostic ignored "-Wall")  \
        SHIBA_PRAGMA(GCC diagnostic ignored "-Wextra")
    #define SHIBA_WARN_POP()  SHIBA_PRAGMA(GCC diagnostic pop)
#endif                      // ^^^ GCC

#endif // SHIBA_PLATFORM_COMPILER_H_