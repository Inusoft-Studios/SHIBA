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

}  // namespace
}  // namespace shiba