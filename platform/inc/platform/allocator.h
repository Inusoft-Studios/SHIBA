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

inline void* alloc(const usize size, const usize align, const AllocationCallbacks* alloc) {
    return alloc->pFnAlloc(alloc->pUser, size, align);
}

inline void free(void* ptr, const usize size, const usize align, const AllocationCallbacks* alloc) {
    alloc->pFnFree(alloc->pUser, ptr, size, align);
}

inline usize alignUp(const usize v, const usize a) { return (v + a - 1) & ~(a - 1); }
inline usize maxAlign(const usize a, const usize b) { return a > b ? a : b; }

}  // namespace shiba

#endif  // SHIBA_PLATFORM_ALLOCATOR_H_
