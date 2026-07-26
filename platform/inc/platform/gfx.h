// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_GFX_H_
#define SHIBA_PLATFORM_GFX_H_

#include "handle.hpp"
#include "surface.h"
#include "types.h"

namespace shiba {
struct AllocationCallbacks;

using GfxInstance  = Handle<struct GfxInstanceTag>;
using GfxDevice    = Handle<struct GfxDeviceTag>;
using GfxSwapchain = Handle<struct GfxSwapchainTag>;

// A back buffer image acquired for the current frame.
using GfxBackBuffer = Handle<struct GfxBackBufferTag>;

// --- API Enums ---

enum class Backend : u8 { OpenGL, Vulkan };  // TODO: fill more in along the way

enum class GfxResult : u8 {
    Ok,
    Suboptimal,     // acquired, but swapchain needs a rebuild
    OutOfDate,      // must recreate swapchain before rendering
    DeviceLost,
    Error,
};

enum class PresentMode : u8 { Immediate, Mailbox, Fifo };

enum class Format : u8 {
    Unknown,
    RGBA8, BGRA8,
    RGBA16F,
    D32F, D24S8
};

}  // namespace shiba

#endif  // SHIBA_PLATFORM_GFX_H_
