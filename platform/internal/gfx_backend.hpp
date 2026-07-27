#ifndef SHIBA_PLATFORM_GFX_BACKEND_HPP_
#define SHIBA_PLATFORM_GFX_BACKEND_HPP_

#include "platform/gfx.h"

namespace shiba {

struct GfxBackendApi {
    // --- Adapters ---
    decltype(&gfxEnumerateAdapters) enumerateAdapters;
    decltype(&gfxDefaultAdapter)    defaultAdapter;
    decltype(&gfxAdapterName)       adapterName;

    // --- Device ---
    decltype(&gfxDeviceCreate)      deviceCreate;
    decltype(&gfxDeviceDestroy)     deviceDestroy;
    decltype(&gfxDeviceWait)        deviceWait;

    // --- Queues ---
    decltype(&gfxDeviceQueue)       deviceQueue;

    // --- OS drawable binding ---
    decltype(&gfxBindingCreate)     bindingCreate;
    decltype(&gfxBindingDestroy)    bindingDestroy;

    // --- Sync ---
    decltype(&gfxFenceCreate)       fenceCreate;
    decltype(&gfxFenceDestroy)      fenceDestroy;
    decltype(&gfxFenceWait)         fenceWait;
    decltype(&gfxFenceReset)        fenceReset;
};

const GfxBackendApi* gfxVkApi();
const GfxBackendApi* gfxNullApi();

}  // namespace shiba

#endif  // SHIBA_PLATFORM_GFX_BACKEND_HPP_
