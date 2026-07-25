// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_API_H
#define SHIBA_API_H

#include "compiler.h"

// === Symbol visibility ===
#if SHIBA_COMPILER_MSVC
    #define SHIBA_EXPORT __declspec(dllexport)
    #define SHIBA_IMPORT __declspec(dllimport)
#else
    #define SHIBA_EXPORT __attribute__((visibility("default")))
    #define SHIBA_IMPORT __attribute__((visibility("default")))
#endif

#endif  // SHIBA_API_H
