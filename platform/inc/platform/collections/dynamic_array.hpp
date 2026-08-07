// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_

#include <type_traits>

#include "platform/allocator.h"
#include "platform/generics/type_identity.hpp"
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
    a->allocator = allocation;
}

template<typename T>
inline void dynamicArrayDestroy(DynamicArray<T>* a) {
    // destroy live elements and return the block
    if (a->data) {
        for (usize i = 0; i < a->size; ++i) a->data[i].~T();
        deallocate(a->allocator, a->data, a->capacity * sizeof(T), alignof(T));
    }
    a->data = nullptr;
    a->size = 0;
    a->capacity = 0;
}

// --- Capacity ---

template<typename T>
inline bool dynamicArrayReserve(DynamicArray<T>* a, const usize n) {
    if (n <= a->capacity) return true;

    T* fresh = static_cast<T*>(allocate(a->allocator, n * sizeof(T), alignof(T)));
    if (!fresh) return false;       // recoverable not abort

    if constexpr (std::is_trivially_copyable_v<T>) {
        if (a->size) __builtin_memcpy(fresh, a->data, a->size * sizeof(T));
    } else {
        for (usize i = 0; i < a->size; ++i) {
            new (static_cast<void*>(fresh + i)) T(static_cast<T&&>(a->data[i]));
            a->data[i].~T();
        }
    }

    if (a->data) deallocate(a->allocator, a->data, a->capacity * sizeof(T), alignof(T));
    a->data     = fresh;
    a->capacity = n;
    return true;
}

// Sets the logical size to n; grown slots are left uninitialized.
template<typename T>
inline bool dynamicArrayResize(DynamicArray<T>* a, const usize n) {
    if (!dynamicArrayReserve(a, n)) return false;
    a->size = n;
    return true;
}

// --- Queries ---

template<typename T>
inline usize dynamicArraySize(const DynamicArray<T>* a) { return a->size; }

template<typename T>
inline usize dynamicArrayCapacity(const DynamicArray<T>* a) { return a->capacity; }

template<typename T>
inline bool dynamicArrayEmpty(const DynamicArray<T>* a) { return a->size == 0; }

// --- Access ---

template<typename T>
inline T* dynamicArrayData(DynamicArray<T>* a) { return a->data; }
template<typename T>
inline const T* dynamicArrayData(const DynamicArray<T>* a) { return a->data; }

template<typename T>
inline T* dynamicArrayAt(DynamicArray<T>* a, const usize i) { return &a->data[i]; }
template<typename T>
inline const T* dynamicArrayAt(const DynamicArray<T>* a, const usize i) { return &a->data[i]; }

template<typename T>
inline T* dynamicArrayFront(DynamicArray<T>* a) { return &a->data[0]; }
template<typename T>
inline const T* dynamicArrayFront(const DynamicArray<T>* a) { return &a->data[0]; }

template<typename T>
inline T* dynamicArrayBack(DynamicArray<T>* a) { return &a->data[a->size - 1u]; }
template<typename T>
inline const T* dynamicArrayBack(const DynamicArray<T>* a) { return &a->data[a->size - 1u]; }

template<typename T>
inline T* dynamicArrayBegin(DynamicArray<T>* a) { return a->data; }
template<typename T>
inline const T* dynamicArrayBegin(const DynamicArray<T>* a) { return a->data; }

template<typename T>
inline T* dynamicArrayEnd(DynamicArray<T>* a) { return a->data + a->size; }
template<typename T>
inline const T* dynamicArrayEnd(const DynamicArray<T>* a) { return a->data + a->size; }

// --- Mutation ---

template<typename T, typename... Args>
inline T* dynamicArrayEmplace(DynamicArray<T>* a, Args&&... args) {
    if (a->size >= a->capacity) {
        if (const usize next = a->capacity ? a->capacity * 2 : 1; !dynamicArrayReserve(a, next))
            return nullptr;
    }
    T* slot = a->data + a->size;
    new (static_cast<void*>(slot)) T(static_cast<Args&&>(args)...);
    ++a->size;
    return slot;
}

template<typename T>
inline T* dynamicArrayPush(DynamicArray<T>* a, const TypeIdentity_t<T>& v) {
    return dynamicArrayEmplace(a, v);
}

template<typename T>
inline void dynamicArrayPop(DynamicArray<T>* a) {
    if (a->size == 0)
        return;
    --a->size;
}

// O(1), does not preserve order. Caller guarantees i < size.
template<typename T>
inline void dynamicArrayRemoveUnordered(DynamicArray<T>* a, const usize i) {
    a->data[i] = a->data[a->size - 1];
    --a->size;
}

// O(n), preserves order. Caller guarantees i < size.
template<typename T>
inline void dynamicArrayRemove(DynamicArray<T>* a, const usize i) {
    __builtin_memmove(&a->data[i], &a->data[i + 1], (a->size - i - 1) * sizeof(T));
    --a->size;
}

// Inserts v at i, shifting the tail up. Returns the slot, or nullptr on OOM.
// Caller guarantees i <= size.
template<typename T>
inline T* dynamicArrayInsert(DynamicArray<T>*a, const usize i, const TypeIdentity_t<T>& v) {
    if (a->size >= a->capacity && !dynamicArrayReserve(a, a->size + 1))
        return nullptr;
    __builtin_memmove(&a->data[i + 1], &a->data[i], (a->size - i) * sizeof(T));
    a->data[i] = v;
    ++a->size;
    return &a->data[i];
}

template<typename T>
inline void dynamicArrayClear(DynamicArray<T>* a) {
    T* data = dynamicArrayData(a);
    if (data)
        for (usize i = 0; i < a->size; ++i) data[i].~T();   // delete old data
    a->size = 0;
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
