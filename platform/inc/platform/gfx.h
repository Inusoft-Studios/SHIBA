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

enum class Backend : u8 { Null, Vulkan, OpenGL };  // TODO: fill more in along the way

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

// --- Descriptors ---

struct alignas(8) GfxInstanceDesc {
    const char* pAppName;
    Backend     backend;
    bool        bValidation;
    u8          _pad[2];
    u32         appVersion;
};
static_assert(sizeof(GfxInstanceDesc) == 16 && "shiba: GfxInstanceDesc must be 16 bytes in size.");

struct alignas(4) GfxDeviceDesc {
    GfxInstance instance;
    Surface     surface;         // device is chosen for presentability to this
    u8          framesInFlight;  // 2-3
    u8          _pad[7];
};
static_assert(sizeof(GfxDeviceDesc) == 16 && "shiba: GfxDeviceDesc must be 16 bytes in size.");

struct alignas(4) GfxSwapchainDesc {
    GfxDevice   device;
    Surface     surface;
    Extent2D    extent;
    u32         imageCount;
    Format      format;
    PresentMode presentMode;
    u8          _pad[2];
};
static_assert(sizeof(GfxSwapchainDesc) == 24 && "shiba: GfxSwapchainDesc must be 24 bytes in size.");

struct alignas(8) GfxSwapchainCaps {
    u32         minImages, maxImages;
    Extent2D    minExtent, maxExtent, currentExtent;
    Format      formats[8];
    u32         formatCount;
    PresentMode modes[4];
    u32         modeCount;
    u32         _pad[3];
};
static_assert(sizeof(GfxSwapchainCaps) == 64 && "shiba: GfxSwapchainCaps must be 64 bytes in size.");

// --- Instance lifetime ---
GfxInstance gfxInstanceCreate (const GfxInstanceDesc&, const AllocationCallbacks*);
void        gfxInstanceDestroy(GfxInstance);

// --- Device lifetime ---
GfxDevice gfxDeviceCreate   (const GfxDeviceDesc&, const AllocationCallbacks*);
void      gfxDeviceDestroy  (GfxDevice);
void      gfxDeviceWaitIdle (GfxDevice);      // block until GPU is drained

// --- Swapchain ---
void         gfxSwapchainQueryCaps(GfxDevice, Surface, GfxSwapchainCaps* out);
GfxSwapchain gfxSwapchainCreate   (const GfxSwapchainDesc&, const AllocationCallbacks*);
void         gfxSwapchainDestroy  (GfxSwapchain);
void         gfxSwapchainResize   (GfxSwapchain, Extent2D);    // recreate in place

Extent2D     gfxSwapchainExtent(GfxSwapchain);
Format       gfxSwapchainFormat(GfxSwapchain);
u32          gfxSwapchainImageCount(GfxSwapchain);

// Acquire a back buffer image from the swapchain.
GfxResult gfxAcquire(GfxSwapchain, GfxBackBuffer* out);
// Present a rendered back buffer image to the screen using the swapchain.
GfxResult gfxPresent(GfxSwapchain, GfxBackBuffer);

// --- Diagnostics ---
Backend     gfxBackend   (GfxInstance);
const char* gfxDeviceName(GfxDevice);   // e.g. "NVIDIA RTX 30x/40x/50x"

}  // namespace shiba

#endif  // SHIBA_PLATFORM_GFX_H_
