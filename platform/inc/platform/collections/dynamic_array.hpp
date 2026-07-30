// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_

#include <type_traits>

#include "platform/allocator.h"
#include "platform/types.h"

namespace shiba {

template<typename T>
struct DynamicArray {
    static_assert(std::is_trivially_copyable_v<T>, "shiba: T must be trivially copyable");

    T*                         data{};
    usize                      size{};
    usize                      capacity{};
    const AllocationCallbacks* allocator{};
};

// --- Lifetime ---

template<typename T>
inline void dynamicArrayInit(DynamicArray<T>* a, const AllocationCallbacks* allocation) {
    a->data = nullptr;
    a->size = 0;
    a->capacity = 0;
    a->alloc = allocation;
}

template<typename T>
inline void dynamicArrayDestroy(DynamicArray<T>* a) {
    if (a->data)
        deallocate(a->alloc, a->data, a->capacity * sizeof(T), alignof(T));
    a->data = nullptr;
    a->size = 0;
    a->capacity = 0;
}

// --- Capacity ---

template<typename T>
inline bool dynamicArrayReserve(DynamicArray<T>* a, const usize n) {
    if (n <= a->capacity)
        return true;

    usize nc = a->capacity ? a->capacity : 8;
    while (nc < n)
        nc += nc / 2 + 1;  // ~1.5x growth

    T* fresh = static_cast<T*>(allocate(a->alloc, nc * sizeof(T), alignof(T)));
    if (!fresh) return false;

    if (a->data) {
        __builtin_memcpy(fresh, a->data, a->size * sizeof(T));
        deallocate(a->alloc, a->data, a->capacity * sizeof(T), alignof(T));
    }
    a->data = fresh;
    a->capacity = nc;
    return true;
}

// Sets the logical size to n; grown slots are left uninitialized.
template<typename T>
inline bool dynamicArrayResize(DynamicArray<T>* a, const usize n) {
    if (!dynamicArrayReserve(a, n)) return false;
    a->size = n;
    return true;
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
