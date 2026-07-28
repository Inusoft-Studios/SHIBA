// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_VERSION_H_
#define SHIBA_PLATFORM_VERSION_H_

#include "types.h"

#ifndef SHIBA_VERSION_MAJOR
    #define SHIBA_VERSION_MAJOR 0
#endif
#ifndef SHIBA_VERSION_MINOR
    #define SHIBA_VERSION_MINOR 0
#endif
#ifndef SHIBA_VERSION_PATCH
    #define SHIBA_VERSION_PATCH 0
#endif

namespace shiba {
const char* versionStr();

inline u32 versionMajor() { return SHIBA_VERSION_MAJOR; }
inline u32 versionMinor() { return SHIBA_VERSION_MINOR; }
inline u32 versionPatch() { return SHIBA_VERSION_PATCH; }
}  // namespace shiba

#endif  // SHIBA_PLATFORM_VERSION_H_
