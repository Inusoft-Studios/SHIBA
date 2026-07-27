// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_SURFACE_NATIVE_H_
#define SHIBA_PLATFORM_SURFACE_NATIVE_H_

#include "platform/surface.h"
#include "platform/types.h"

namespace shiba {

using NativeHandle = void*;   // HWND / ANativeWindow* / CAMetalLayer*, back-end specific

NativeHandle surfaceGetNativeHandle(Surface surface);

}  // namespace shiba

#endif  // SHIBA_PLATFORM_SURFACE_NATIVE_H_