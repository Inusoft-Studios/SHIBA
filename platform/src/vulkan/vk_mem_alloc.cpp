#include <vulkan/vulkan.h>
#include "platform/compiler.h"

#define VMA_IMPLEMENTATION

// VMA is third-party and won't pass our warnings-as-errors. Silence for this TU only.
SHIBA_WARN_PUSH_DISABLE_ALL()
#include "vk_mem_alloc.h"
SHIBA_WARN_POP()