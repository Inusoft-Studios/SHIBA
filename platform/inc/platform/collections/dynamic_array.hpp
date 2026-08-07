// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_

#include <type_traits>
#include <new>  // placement new

#include "platform/allocator.h"
#include "platform/generics/type_identity.hpp"
#include "platform/types.h"
#include "span.hpp"

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
    if (n > a->size) {
        if (!dynamicArrayReserve(a, n)) return false;
        for (usize i = a->size; i < n; ++i)
            new (static_cast<void*>(a->data + i)) T();  // value-init new slots
    } else {
        for (usize i = n; i < a->size; ++i)
            a->data[i].~T();                            // destroy shrunk off tail
    }
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
        if (const usize next = a->capacity ? a->capacity * 2 : 8; !dynamicArrayReserve(a, next))
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
    a->data[a->size].~T();
}

// O(1), does not preserve order. Caller guarantees i < size.
template<typename T>
inline void dynamicArrayRemoveUnordered(DynamicArray<T>* a, const usize i) {
    const usize last = a->size - 1;
    if (i != last)
        a->data[i] = static_cast<T&&>(a->data[last]);
    --a->size;
    a->size = last;
}

// O(n), preserves order. Caller guarantees i < size.
template<typename T>
inline void dynamicArrayRemove(DynamicArray<T>* a, const usize i) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        __builtin_memmove(&a->data[i], &a->data[i + 1], (a->size - i - 1) * sizeof(T));
    } else {
        for (usize j = i; j < a->size; ++j)
            a->data[j] = static_cast<T&&>(a->data[j + 1]);  // shift tail down
        a->data[a->size - 1].~T();                          // destroy stale last
    }
    --a->size;
}

// Inserts v at i, shifting the tail up. Returns the slot, or nullptr on OOM.
// Caller guarantees i <= size.
template<typename T>
inline T* dynamicArrayInsert(DynamicArray<T>*a, const usize i, const TypeIdentity_t<T>& v) {
    if (a->size >= a->capacity) {
        const usize nc = a->capacity ? a->capacity * 2 : 8;
        if (!dynamicArrayReserve(a, nc)) return nullptr;
    }

    if constexpr (std::is_trivially_copyable_v<T>) {
        __builtin_memmove(&a->data[i + 1], &a->data[i], (a->size - i) * sizeof(T));
        a->data[i] = v;
    } else {
        if (i == a->size)
            new (static_cast<void*>(a->data + i)) T(v);         // append into raw slot
        else {
            new (static_cast<void*>(a->data + a->size)) T(static_cast<T&&>(a->data[a->size - 1]));
            for (usize j = a->size - 1; j > i; --j)
                a->data[j] = static_cast<T&&>(a->data[j - 1]);  // shift up
            a->data[i] = v;                                     // overwrite live slot
        }
    }
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

template<typename T>
inline Span<T> dynamicArrayAsSpan(DynamicArray<T>* a) {
    return Span<T>{ a->data, a->size };
}

template<typename T>
inline Span<const T> dynamicArrayAsSpan(const DynamicArray<T>* a) {
    return Span<const T>{ a->data, a->size };
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
