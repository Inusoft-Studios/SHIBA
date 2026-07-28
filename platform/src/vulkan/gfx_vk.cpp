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

// --- validation ---
VKAPI_ATTR VkBool32 VKAPI_CALL debugCb(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    std::fprintf(stderr, "[vk] %s\n", data->pMessage);
    return VK_FALSE;
}

// --- Lifecycle ---
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

// --- Enumerate ---
u32 vkEnumerateAdapters(GfxAdapter* pOut, const u32 cap) {
    const u32 n = gPhysCount < cap ? gPhysCount : cap;
    for (u32 i = 0; i < n; ++i) pOut[i] = GfxAdapter{ i + 1 };
    return gPhysCount;  // report full count so caller can size a buffer
}

GfxAdapter vkDefaultAdapter() {
    if (gPhysCount == 0) return GfxAdapter{};

    // TODO: properly score GPU
    u32 best = 0;
    for (u32 i = 0; i < gPhysCount; ++i) {
        VkPhysicalDeviceProperties p{};
        vkGetPhysicalDeviceProperties(gPhys[i], &p);
        if (p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            best = i;
            break;
        }
    }
    return GfxAdapter{ best + 1 };
}

const char* vkAdapterName(const GfxAdapter a) {
    const u32 i = a.id - 1;
    return a.id == 0 || i >= gPhysCount ? "" : gPhysName[i];
}

// -- Device ---
GfxDevice vkDeviceCreate(const GfxAdapter adapter, const AllocationCallbacks* alloc) {
    (void)alloc;
    const u32 ai = adapter.id - 1;
    if (adapter.id == 0 || ai >= gPhysCount) return GfxDevice{};
    VkPhysicalDevice phys = gPhys[ai];

    u32 slot = kMaxDevices;
    for (u32 i = 0; i < kMaxDevices; ++i) {
        if (gDev[i].dev == VK_NULL_HANDLE) { slot = i; break; }
    }
    if (slot == kMaxDevices) return GfxDevice{};
    if (gDevGen[slot] == 0) gDevGen[slot] = 1;

    // Queue family selection: graphics, then prefer async compute and a dedicated transfer queue,
    // falling back to the graphics family.
    u32 qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &qCount, nullptr);
    VkQueueFamilyProperties qp[32];
    if (qCount > 32) qCount = 32;
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &qCount, qp);

    u32 gfxF = UINT32_MAX, compF = UINT32_MAX, copyF = UINT32_MAX;
    for (u32 i = 0; i < qCount; ++i)
        if ((qp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && gfxF == UINT32_MAX) gfxF = i;
    for (u32 i = 0; i < qCount; ++i)
        if ((qp[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && !(qp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) { compF = i; break; }
    if (compF == UINT32_MAX)
        for (u32 i = 0; i < qCount; ++i) if (qp[i].queueFlags & VK_QUEUE_COMPUTE_BIT) { compF = i; break; }
    for (u32 i = 0; i < qCount; ++i)
        if ((qp[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(qp[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))) { copyF = i; break; }
    if (copyF == UINT32_MAX)
        for (u32 i = 0; i < qCount; ++i) if (qp[i].queueFlags & VK_QUEUE_TRANSFER_BIT) { copyF = i; break; }

    if (gfxF == UINT32_MAX) return GfxDevice{};     // no graphics-capable family
    if (compF == UINT32_MAX) compF = gfxF;
    if (copyF == UINT32_MAX) copyF = gfxF;

    constexpr float prio = 1.0f;
    const u32 fams[3] = { gfxF, compF, copyF };
    VkDeviceQueueCreateInfo qci[3]{};
    u32 qn = 0;
    for (const u32 fam : fams) {
        bool seen = false;
        for (u32 j = 0; j < qn; ++j) if (qci[j].queueFamilyIndex == fam) seen = true;
        if (seen) continue;
        qci[qn].sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci[qn].queueFamilyIndex   = fam;
        qci[qn].queueCount         = 1;
        qci[qn].pQueuePriorities   = &prio;
        ++qn;
    }

    // Swapchain is a device extension chosen at creation. The GDI above builds swapchains on this
    // device, so the wrapper must enable it here.
    const char* devExts[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo dci{};
    dci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount    = qn;
    dci.pQueueCreateInfos       = qci;
    dci.enabledExtensionCount   = 1;
    dci.ppEnabledExtensionNames = devExts;

    VkDevice dev = VK_NULL_HANDLE;
    if (vkCreateDevice(phys, &dci, nullptr, &dev) != VK_SUCCESS) return GfxDevice{};

    DeviceData& d = gDev[slot];
    d.phys = phys;
    d.dev  = dev;
    d.family[qk(GfxQueueKind::Graphics)] = gfxF;
    d.family[qk(GfxQueueKind::Compute)]  = compF;
    d.family[qk(GfxQueueKind::Copy)]     = copyF;
    for (u32 k = 0; k < 3; ++k) vkGetDeviceQueue(dev, d.family[k], 0, &d.queue[k]);

    VmaAllocatorCreateInfo aci{};
    aci.instance         = gInstance;
    aci.physicalDevice   = phys;
    aci.device           = dev;
    aci.vulkanApiVersion = VK_API_VERSION_1_3;
    vmaCreateAllocator(&aci, &d.vma);

    return packH<GfxDevice>(slot, gDevGen[slot]);
}

void vkDeviceDestroy(const GfxDevice device) {
    DeviceData* d = resolveDevice(device);
    if (!d) return;
    const u32 slot = idxOf(device);
    if (d->vma) vmaDestroyAllocator(d->vma);
    vkDestroyDevice(d->dev, nullptr);
    *d = DeviceData{};  // clears dev to VK_NULL_HANDLE, freeing the slot
    if (++gDevGen[slot] == 0) gDevGen[slot] = 1;
}

void vkDeviceWait(const GfxDevice device) {
    if (const DeviceData* d = resolveDevice(device)) vkDeviceWaitIdle(d->dev);
}

// --- Queues ---
GfxQueue vkDeviceQueue(const GfxDevice device, const GfxQueueKind kind) {
    if (!resolveDevice(device)) return GfxQueue{};
    // pack device slot + generation + kind so a later submit path can resolve back to the stored
    // VkQueue without a separate queue pool.
    return GfxQueue{ (static_cast<u32>(genOf(device)) << 16) |
                     (static_cast<u32>(kind) << 8) |
                     idxOf(device)
    };
}

// --- OS-drawable binding ---
GfxBinding vkBindingCreate(const GfxDevice, const Surface surface, const AllocationCallbacks*) {
    // VkSurfaceKHR is instance-level, device is unused here.
    u32 slot = kMaxBindings;
    for (u32 i = 0; i < kMaxBindings; ++i) if (gBind[i].surface == VK_NULL_HANDLE) { slot = i; break; }
    if (slot == kMaxBindings) return GfxBinding{};
    if (gBindGen[slot] == 0) gBindGen[slot] = 1;

    VkSurfaceKHR vkSurf = VK_NULL_HANDLE;
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    ci.hinstance = GetModuleHandleW(nullptr);
    ci.hwnd = static_cast<HWND>(surfaceGetNativeHandle(surface));
    if (vkCreateWin32SurfaceKHR(gInstance, &ci, nullptr, &vkSurf) != VK_SUCCESS) return GfxBinding{};
#else
    // TODO: Linux KHR surface initialization
    (void)surface;
    return GfxBinding{};
#endif
    gBind[slot].surface = vkSurf;
    return packH<GfxBinding>(slot, gBindGen[slot]);
}

void vkBindingDestroy(const GfxBinding binding) {
    BindingData* b = resolveBinding(binding);
    if (!b) return;
    const u32 slot = idxOf(binding);
    vkDestroySurfaceKHR(gInstance, b->surface, nullptr);
    b->surface = VK_NULL_HANDLE;
    if (++gBindGen[slot] == 0) gBindGen[slot] = 1;
}

// --- Sync ---
GfxFence vkFenceCreate(const GfxDevice device, const bool bSignaled) {
    DeviceData* d = resolveDevice(device);
    if (!d) return GfxFence{};

    u32 slot = kMaxFences;
    for (u32 i = 0; i < kMaxFences; ++i) if (gFence[i].fence == VK_NULL_HANDLE) { slot = i; break; }
    if (slot == kMaxFences) return GfxFence{};
    if (gFenceGen[slot] == 0) gFenceGen[slot] = 1;

    VkFenceCreateInfo fi{};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fi.flags = bSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VkFence f = VK_NULL_HANDLE;
    if (vkCreateFence(d->dev, &fi, nullptr, &f) != VK_SUCCESS) return GfxFence{};

    gFence[slot] = { d->dev, f };
    return packH<GfxFence>(slot, gFenceGen[slot]);
}

void vkFenceDestroy(const GfxFence fence) {
    FenceData* fd = resolveFence(fence);
    if (!fd) return;
    const u32 slot = idxOf(fence);
    vkDestroyFence(fd->dev, fd->fence, nullptr);
    *fd = FenceData{};
    if (++gFenceGen[slot] == 0) gFenceGen[slot] = 1;
}

void vkFenceWait(const GfxFence fence) {
    const FenceData* fd = resolveFence(fence);
    if (fd) vkWaitForFences(fd->dev, 1, &fd->fence, VK_TRUE, UINT64_MAX);
}

void vkFenceReset(const GfxFence fence) {
    const FenceData* fd = resolveFence(fence);
    if (fd) vkResetFences(fd->dev, 1, &fd->fence);
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