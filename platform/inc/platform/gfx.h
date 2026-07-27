// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_GFX_H_
#define SHIBA_PLATFORM_GFX_H_

#include "allocator.h"
#include "surface.h"
#include "types.h"

namespace shiba {
struct AllocationCallbacks;

struct GfxAdapter { u32 id; };  // a physical device the OS enumerates
struct GfxDevice  { u32 id; };  // a logical device created on an adapter
struct GfxQueue   { u32 id; };  // a submission queue on a device
struct GfxFence   { u32 id; };  // CPU-visible sync primitive
struct GfxBinding { u32 id; };  // device <-> OS drawable link (VkSurfaceKHR / IDXGISwapchain-target)

enum class GfxApi       : u8 { Null, Vulkan, D3D12, OpenGL };
enum class GfxQueueKind : u8 { Graphics, Compute, Copy };

// --- Backend selection / teardown ---

bool   gfxInit(GfxApi api, bool bValidation, const AllocationCallbacks*);
void   gfxShutdown();
GfxApi gfxActiveApi();

// --- Adapters ---
u32         gfxEnumerateAdapters(GfxAdapter* pOut, u32 cap);    // fills out, returns count
GfxAdapter  gfxDefaultAdapter   ();                             // picks the OS-preferred adapter
const char* gfxAdapterName      (GfxAdapter);

// --- Device ---
GfxDevice gfxDeviceCreate (GfxAdapter, const AllocationCallbacks*);
void      gfxDeviceDestroy(GfxDevice);
void      gfxDeviceWait   (GfxDevice);                          // block until the device is idle

// --- Queues ---
GfxQueue gfxDeviceQueue(GfxDevice, GfxQueueKind);               // retrieve a queue of the given kind

// --- OS-drawable binding ---
// Links a device to the window's native drawable.

GfxBinding gfxBindingCreate (GfxDevice, Surface, const AllocationCallbacks*);
void       gfxBindingDestroy(GfxBinding);

// --- Sync ---
GfxFence gfxFenceCreate  (GfxDevice, bool bSignaled);
void     gfxFenceDestroy (GfxFence);
void     gfxFenceWait    (GfxFence);                            // block until signaled
void     gfxFenceReset   (GfxFence);

}  // namespace shiba

#endif  // SHIBA_PLATFORM_GFX_H_