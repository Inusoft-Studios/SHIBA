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
    const AllocationCallbacks* alloc{};
};

// --- Lifetime ---

template<typename T>
inline void dynamicArrayInit(DynamicArray<T>* arr, const AllocationCallbacks* a) {
    arr->data = nullptr;
    arr->size = 0;
    arr->capacity = 0;
    arr->alloc = a;
}

template<typename T>
inline void dynamicArrayDestroy(DynamicArray<T>* a) {
    if (a->data)
        deallocate(a->alloc, a->data, a->capacity * sizeof(T), alignof(T));
    a->data = nullptr;
    a->size = 0u;
    a->capacity = 0u;
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
