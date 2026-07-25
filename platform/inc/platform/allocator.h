// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_ALLOCATOR_H_
#define SHIBA_PLATFORM_ALLOCATOR_H_
#include "types.h"

namespace shiba {
struct AllocationCallbacks {
    void* (*pFnAlloc)(void* user, usize size, usize align);
    void  (*pFnFree)(void* user, void* ptr);
    void* pUser;
};
}  // namespace shiba

#endif  // SHIBA_PLATFORM_ALLOCATOR_H_
