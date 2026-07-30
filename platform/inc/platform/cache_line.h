// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_CACHE_LINE_H_
#define SHIBA_PLATFORM_CACHE_LINE_H_

#if defined(__APPLE__) && defined(__aarch64__)
    #define SHIBA_CACHE_LINE_SIZE 128   // Apple Silicon
#elif defined(__powerpc64__) || defined(__ppc64__)
    #define SHIBA_CACHE_LINE_SIZE 128
#elif defined(__x86_64__) || defined(_M_X64) || \
      defined(__i386__)   || defined(_M_IX86) || \
      defined(__aarch64__) || defined(_M_ARM64) || \
      defined(__riscv)
            #define SHIBA_CACHE_LINE_SIZE 64
#else
    #define SHIBA_CACHE_LINE_SIZE 64    // safe default
#endif


#endif  // SHIBA_PLATFORM_CACHE_LINE_H_
