// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_ALLOCATOR_H_
#define SHIBA_PLATFORM_ALLOCATOR_H_
#include "types.h"

namespace shiba {
struct AllocationCallbacks {
    void* (*pFnAlloc)(void* user, usize size, usize align);
    void  (*pFnFree)(void* user, void* ptr, usize size, usize align);
    void* pUser;
};

inline void* allocate(const AllocationCallbacks* a, const usize size, const usize align) {
    return a->pFnAlloc(a->pUser, size, align);
}

inline void deallocate(const AllocationCallbacks* a, void* ptr, const usize size, const usize align) {
    a->pFnFree(a->pUser, ptr, size, align);
}

// Rounds value up to a power-of-two alignment.
inline usize alignUp(const usize value, const usize align) {
    return (value + align - 1u) & ~(align - 1u);
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_ALLOCATOR_H_
