// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#include "platform/gfx.h"
#include "gfx_backend.hpp"

namespace shiba {
namespace {
const GfxBackendApi* gApi = nullptr;
GfxApi               gActive = GfxApi::Null;

const GfxBackendApi* select(const GfxApi api) {
    switch (api) {
        case GfxApi::Vulkan: return gfxVkApi();
        // case GfxApi::Null:   return gfxNullApi();
        default: return nullptr;
    }
}
}  // namespace

// --- Backend selection / teardown ---
bool gfxInit(const GfxApi api, const bool bValidation, const AllocationCallbacks* alloc) {
    gApi = select(api);
    if (!gApi) return false;
    if (!gApi->init(bValidation, alloc)) { gApi = nullptr; return false; }
    gActive = api;
    return true;
}
void   gfxShutdown()  { if (gApi) { gApi->shutdown(); gApi = nullptr; } gActive = GfxApi::Null; }
GfxApi gfxActiveApi() { return gActive; }

// --- Adapters ---
u32         gfxEnumerateAdapters(GfxAdapter* pOut, const u32 cap) { return gApi->enumerateAdapters(pOut, cap); }
GfxAdapter  gfxDefaultAdapter   ()                                { return gApi->defaultAdapter(); }
const char* gfxAdapterName      (const GfxAdapter a)              { return gApi->adapterName(a); }

// --- Device ---
GfxDevice gfxDeviceCreate (const GfxAdapter a, const AllocationCallbacks* alloc) { return gApi->deviceCreate(a, alloc); }
void      gfxDeviceDestroy(const GfxDevice d)                                    { gApi->deviceDestroy(d); }
void      gfxDeviceWait   (const GfxDevice d)                                    { gApi->deviceWait(d); }

// --- Queues ---
GfxQueue gfxDeviceQueue(const GfxDevice d, const GfxQueueKind k) { return gApi->deviceQueue(d, k); }

// --- OS-drawable binding ---
GfxBinding gfxBindingCreate (const GfxDevice d, const Surface s, const AllocationCallbacks* alloc) { return gApi->bindingCreate(d, s, alloc); }
void       gfxBindingDestroy(const GfxBinding b)                                                   { gApi->bindingDestroy(b); }

// --- Sync ---
GfxFence gfxFenceCreate (const GfxDevice d, const bool bSignaled) { return gApi->fenceCreate(d, bSignaled); }
void     gfxFenceDestroy(const GfxFence f)                        { gApi->fenceDestroy(f); }
void     gfxFenceWait   (const GfxFence f)                        { gApi->fenceWait(f); }
void     gfxFenceReset  (const GfxFence f)                        { gApi->fenceReset(f); }

}  // namespace shiba