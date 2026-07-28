#include "gfx_backend.hpp"
#include "surface_native.h"
#include "platform/allocator.h"
#include "platform/compiler.h"
#include "platform/version.h"

// TODO: Other OS-backends
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

SHIBA_WARN_PUSH_DISABLE_ALL()
#include "vk_mem_alloc.h"
SHIBA_WARN_POP()

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
template<class H> [[maybe_unused]] H packH(const u32 i, const u16 g) { return H{ (static_cast<u32>(g) << 16) | i }; }
template<class H> [[maybe_unused]] u32 idxOf(const H h) { return h.id & 0xFFFF; }
template<class H> [[maybe_unused]] u16 genOf(const H h) { return static_cast<u16>(h.id >> 16); }

[[maybe_unused]] u8 qk(const GfxQueueKind k) { return static_cast<u8>(k); }

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

[[maybe_unused]] DeviceData* resolveDevice(const GfxDevice d) {
    const u32 i = idxOf(d);
    if (i >= kMaxDevices || gDev[i].dev == VK_NULL_HANDLE || gDevGen[i] != genOf(d)) return nullptr;
    return &gDev[i];
}

// --- binding pool ---
struct BindingData { VkSurfaceKHR surface; };
BindingData gBind   [kMaxBindings] = {};
u16         gBindGen[kMaxBindings] = {};

[[maybe_unused]] BindingData* resolveBinding(const GfxBinding b) {
    const u32 i = idxOf(b);
    if (i >= kMaxBindings || gBind[i].surface == VK_NULL_HANDLE || gBindGen[i] != genOf(b)) return nullptr;
    return &gBind[i];
}

// --- fence pool ---
struct FenceData { VkDevice dev; VkFence fence; };
FenceData gFence [kMaxFences] = {};
u16        gFenceGen[kMaxFences] = {};

[[maybe_unused]] FenceData* resolveFence(const GfxFence f) {
    const u32 i = idxOf(f);
    if (i >= kMaxFences || gFence[i].fence == VK_NULL_HANDLE || gFenceGen[i] != genOf(f)) return nullptr;
    return &gFence[i];
}

// --- validation ---
VKAPI_ATTR VkBool32 VKAPI_CALL debugCb(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    std::fprintf(stderr, "[vk] %s\n", data->pMessage);
    return VK_FALSE;
}

bool vkInit(const bool bValidation, const AllocationCallbacks* alloc) {
    if (gInstance != VK_NULL_HANDLE) return true;
    gAlloc      = alloc ? *alloc : AllocationCallbacks{};
    gValidation = bValidation;

    VkApplicationInfo app{};
    app.sType         = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pEngineName   = "SHIBA";
    app.engineVersion = VK_MAKE_VERSION(versionMajor(), versionMinor(), versionPatch());
    app.apiVersion    = VK_API_VERSION_1_3;

    const char* exts[8]; u32 extCount = 0;
    exts[extCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
#ifdef _WIN32
    exts[extCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif
    const char* layers[1]; u32 layerCount = 0;
    if (bValidation) {
        exts[extCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        layers[layerCount++] = "VK_LAYER_KHRONOS_validation";
    }

    VkInstanceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo        = &app;
    ci.enabledExtensionCount   = extCount;
    ci.ppEnabledExtensionNames = exts;
    ci.enabledLayerCount       = layerCount;
    ci.ppEnabledLayerNames     = layers;
    if (vkCreateInstance(&ci, nullptr, &gInstance) != VK_SUCCESS) return false;

    if (bValidation) {
        const auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(gInstance, "vkCreateDebugUtilsMessengerEXT"));
        if (create) {
            VkDebugUtilsMessengerCreateInfoEXT dci{};
            dci.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            dci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            dci.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            dci.pfnUserCallback = debugCb;
            create(gInstance, &dci, nullptr, &gDebug);
        }
    }

    // Enumerate and cache adapters up front.
    u32 count = 0;
    vkEnumeratePhysicalDevices(gInstance, &count, nullptr);
    if (count > kMaxAdapters) count = kMaxAdapters;
    vkEnumeratePhysicalDevices(gInstance, &count, gPhys);
    gPhysCount = count;
    for (u32 i = 0; i < count; ++i) {
        VkPhysicalDeviceProperties p{};
        vkGetPhysicalDeviceProperties(gPhys[i], &p);
        std::snprintf(gPhysName[i], kNameCap, "%s", p.deviceName);
    }
    return true;
}

void vkShutdown() {
    if (gInstance == VK_NULL_HANDLE) return;
    if (gDebug) {
        const auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(gInstance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroy) destroy(gInstance, gDebug, nullptr);
        gDebug = VK_NULL_HANDLE;
    }
    vkDestroyInstance(gInstance, nullptr);
    gInstance = VK_NULL_HANDLE;
    gPhysCount = 0;
    gValidation = false;
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