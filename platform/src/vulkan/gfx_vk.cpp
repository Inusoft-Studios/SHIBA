#include "gfx_backend.hpp"
#include "surface_native.h"
#include "platform/allocator.h"
#include "platform/compiler.h"

// TODO: Other OS-backends
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <cstdio>  // fprintf (validation output)

namespace shiba {
namespace {

constexpr u32 kMaxAdapters = 8;
constexpr u32 kMaxDevices  = 4;
constexpr u32 kMaxBindings = 8;
constexpr u32 kMaxFences   = 256;
constexpr u32 kNameCap     = 256;   // VK_MAX_PHYSICAL_DEVICE_NAME_SIZE

// --- instance-level state, owned by init/shutdown ---
VkInstance               gInstance   = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT gDebug      = VK_NULL_HANDLE;
AllocationCallbacks      gAlloc      = {};
bool                     gValidation = false;

// Adapters are enumerated once at init and stable for the instance lifetime, so
// they need no generation.
VkPhysicalDevice gPhys    [kMaxAdapters] = {};
char             gPhysName[kMaxAdapters][kNameCap] = {};
u32              gPhysCount = 0;

// handle <-> slot
template<class H> H packH(const u32 i, const u16 g) { return H{ (static_cast<u32>(g) << 16) | i }; }
template<class H> u32 idxOf(const H h) { return h.id & 0xFFFF; }
template<class H> u16 genOf(const H h) { return static_cast<u16>(h.id >> 16); }

u8 qk(const GfxQueueKind k) { return static_cast<u8>(k); }

// --- device pool ---
struct DeviceData {
    VkPhysicalDevice phys;
    VkDevice         dev;
    VmaAllocator     vma;
    u32              family[3]; // indexed by GfxQueueKind
    VkQueue          queue [3];
};
DeviceData gDev   [kMaxDevices] = {};
u16        gDevGen[kMaxDevices] = {};

DeviceData* resolveDevice(const GfxDevice d) {
    const u32 i = idxOf(d);
    if (i >= kMaxDevices || gDev[i].dev == VK_NULL_HANDLE || gDevGen[i] != genOf(d)) return nullptr;
    return &gDev[i];
}

// --- binding pool ---
struct BindingData { VkSurfaceKHR surface; };
BindingData gBind   [kMaxBindings] = {};
u16         gBindGen[kMaxBindings] = {};

BindingData* resolveBinding(const GfxBinding b) {
    const u32 i = idxOf(b);
    if (i >= kMaxBindings || gBind[i].surface == VK_NULL_HANDLE || gBindGen[i] != genOf(b)) return nullptr;
    return &gBind[i];
}

// --- fence pool ---
struct FenceData { VkDevice dev; VkFence fence; };
FenceData gFence [kMaxFences] = {};
u16        gFenceGen[kMaxFences] = {};

FenceData* resolveFence(const GfxFence f) {
    const u32 i = idxOf(f);
    if (i >= kMaxFences || gFence[i].fence == VK_NULL_HANDLE || gFenceGen[i] != genOf(f)) return nullptr;
    return &gFence[i];
}

bool vkInit(const bool bValidation, const AllocationCallbacks* alloc) {
    (void)bValidation; (void)alloc;
    return true;
}

void vkShutdown() {

}

u32 vkEnumerateAdapters(GfxAdapter* pOut, u32 cap) {
    (void)pOut; (void)cap;
    return 0;
}

GfxAdapter vkDefaultAdapter() {
    return {};
}

const char* vkAdapterName(GfxAdapter) {
    return "";
}

GfxDevice vkDeviceCreate(GfxAdapter, const AllocationCallbacks*) {
    return {};
}

void vkDeviceDestroy(GfxDevice) {

}

void vkDeviceWait(GfxDevice) {

}

GfxQueue vkDeviceQueue(GfxDevice, GfxQueueKind) {
    return {};
}

GfxBinding vkBindingCreate(GfxDevice, Surface, const AllocationCallbacks*) {
    return {};
}

void vkBindingDestroy(GfxBinding) {

}

GfxFence vkFenceCreate(GfxDevice, bool bSignaled) {
    (void)bSignaled;
    return {};
}

void vkFenceDestroy(GfxFence) {

}

void vkFenceWait(GfxFence) {

}

void vkFenceReset(GfxFence) {

}

}  // namespace

const GfxBackendApi* gfxVkApi() {
    static constexpr GfxBackendApi kApi = {
        vkInit,               vkShutdown,
        vkEnumerateAdapters,  vkDefaultAdapter,   vkAdapterName,
        vkDeviceCreate,       vkDeviceDestroy,    vkDeviceWait,
        vkDeviceQueue,
        vkBindingCreate,      vkBindingDestroy,
        vkFenceCreate,        vkFenceDestroy,     vkFenceWait,   vkFenceReset,
    };
    return &kApi;
}

}  // namespace shiba