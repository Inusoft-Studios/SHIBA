// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_GFX_BACKEND_HPP_
#define SHIBA_PLATFORM_GFX_BACKEND_HPP_

#include "platform/gfx.h"

namespace shiba {

using PfnEnumerateAdapters = u32        (*)(GfxAdapter* out, u32 cap);
using PfnDefaultAdapter    = GfxAdapter (*)();
using PfnAdapterName       = const char*(*)(GfxAdapter);

using PfnDeviceCreate      = GfxDevice  (*)(GfxAdapter, const AllocationCallbacks*);
using PfnDeviceDestroy     = void       (*)(GfxDevice);
using PfnDeviceWait        = void       (*)(GfxDevice);
using PfnDeviceQueue       = GfxQueue   (*)(GfxDevice, GfxQueueKind);

using PfnBindingCreate     = GfxBinding (*)(GfxDevice, Surface, const AllocationCallbacks*);
using PfnBindingDestroy    = void       (*)(GfxBinding);

using PfnFenceCreate       = GfxFence   (*)(GfxDevice, bool);
using PfnFenceDestroy      = void       (*)(GfxFence);
using PfnFenceWait         = void       (*)(GfxFence);
using PfnFenceReset        = void       (*)(GfxFence);

struct GfxBackendApi {
    // --- Lifetime ---
    bool (*init)(bool bValidation, const AllocationCallbacks*);
    void (*shutdown)();

    // --- Adapters ---
    PfnEnumerateAdapters enumerateAdapters;
    PfnDefaultAdapter    defaultAdapter;
    PfnAdapterName       adapterName;

    // --- Device ---
    PfnDeviceCreate      deviceCreate;
    PfnDeviceDestroy     deviceDestroy;
    PfnDeviceWait        deviceWait;

    // --- Queues ---
    PfnDeviceQueue       deviceQueue;

    // --- OS drawable binding ---
    PfnBindingCreate     bindingCreate;
    PfnBindingDestroy    bindingDestroy;

    // --- Sync ---
    PfnFenceCreate       fenceCreate;
    PfnFenceDestroy      fenceDestroy;
    PfnFenceWait         fenceWait;
    PfnFenceReset        fenceReset;
};

const GfxBackendApi* gfxVkApi();
const GfxBackendApi* gfxNullApi();

}  // namespace shiba

#endif  // SHIBA_PLATFORM_GFX_BACKEND_HPP_
