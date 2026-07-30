// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_

#include <type_traits>

#include "platform/types.h"
#include "platform/generics/type_identity.hpp"

namespace shiba {

template<usize Capacity>
using FixedArraySizeType = std::conditional_t<
    (Capacity <= 0xFFull), u8,
    std::conditional_t<(Capacity <= 0xFFFFull), u16,
    std::conditional_t<(Capacity <= 0xFFFFFFFFull), u32, u64>>>;

template<typename T, usize Capacity>
struct FixedArray {
    static_assert(Capacity > 0, "FixedArray requires a non-zero capacity");
    static_assert(std::is_trivially_copyable_v<T>,
                  "FixedArray stores T inline and copies by assignment; "
                  "use the method-based container for non-trivial T");

    T items[Capacity];
    FixedArraySizeType<Capacity> count;
};

// --- Queries ---

template<typename T, usize Capacity>
constexpr usize fixedArrayCapacity(const FixedArray<T, Capacity>*) { return Capacity; }

template<typename T, usize Capacity>
inline usize fixedArraySize(const FixedArray<T, Capacity>* a) { return a->count; }

template<typename T, usize Capacity>
inline bool fixedArrayEmpty(const FixedArray<T, Capacity>* a) { return a->count == 0u; }

template<typename T, usize Capacity>
inline bool fixedArrayFull(const FixedArray<T, Capacity>* a) { return a->count >= Capacity; }

// --- Access ---

template<typename T, usize Capacity>
inline T* fixedArrayData(FixedArray<T, Capacity>* a) { return a->items; }
template<typename T, usize Capacity>
inline const T* fixedArrayData(const FixedArray<T, Capacity>* a) { return a->items; }

template<typename T, usize Capacity>
inline T* fixedArrayAt(FixedArray<T, Capacity>* a, const usize i) { return &a->items[i]; }
template<typename T, usize Capacity>
inline const T* fixedArrayAt(const FixedArray<T, Capacity>* a, const usize i) { return &a->items[i]; }

template<typename T, usize Capacity>
inline T* fixedArrayFront(FixedArray<T, Capacity>* a) { return &a->items[0]; }
template<typename T, usize Capacity>
inline const T* fixedArrayFront(const FixedArray<T, Capacity>* a) { return &a->items[0]; }

template<typename T, usize Capacity>
inline T* fixedArrayBack(FixedArray<T, Capacity>* a) { return &a->items[a->count - 1u]; }
template<typename T, usize Capacity>
inline const T* fixedArrayBack(const FixedArray<T, Capacity>* a) { return &a->items[a->count - 1u]; }

template<typename T, usize Capacity>
inline T* fixedArrayBegin(FixedArray<T, Capacity>* a) { return a->items; }
template<typename T, usize Capacity>
inline const T* fixedArrayBegin(const FixedArray<T, Capacity>* a) { return a->items; }

template<typename T, usize Capacity>
inline T* fixedArrayEnd(FixedArray<T, Capacity>* a) { return a->items + a->count; }
template<typename T, usize Capacity>
inline const T* fixedArrayEnd(const FixedArray<T, Capacity>* a) { return a->items + a->count; }

// --- Mutation ---

template<typename T, usize Capacity>
inline T* fixedArrayPush(FixedArray<T, Capacity>* a, const TypeIdentity_t<T>& v) {
    if (a->count >= Capacity)
        return nullptr;

    T* slot = &a->items[a->count];
    *slot = v;
    ++a->count;
    return slot;
}

template<typename T, usize Capacity>
inline void fixedArrayPop(FixedArray<T, Capacity>* a) {
    if (fixedArrayEmpty(a))
        return;
    --a->count;
}

template<typename T, usize Capacity>
inline void fixedArrayClear(FixedArray<T, Capacity>* a) { a->count = 0u; }

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_