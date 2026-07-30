// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_

#include <type_traits>

#include "platform/cache_line.h"
#include "platform/types.h"

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

template<typename T, usize Capacity>
constexpr usize faCapacity(const FixedArray<T, Capacity>*) { return Capacity; }

template<typename T, usize Capacity>
inline usize faSize(const FixedArray<T, Capacity>* a) { return a->count; }

template<typename T, usize Capacity>
inline bool faEmpty(const FixedArray<T, Capacity>* a) { return a->count == 0u; }

template<typename T, usize Capacity>
inline bool faFull(const FixedArray<T, Capacity>* a) { return a->count >= Capacity; }

template<typename T, usize Capacity>
inline T* faData(FixedArray<T, Capacity>* a) { return a->items; }

template<typename T, usize Capacity>
inline T* faAt(FixedArray<T, Capacity>* a, const usize i) { return &a->items[i]; }

template<typename T, usize Capacity>
inline T* faBack(FixedArray<T, Capacity>* a) { return &a->items[a->count - 1u]; }

template<typename T, usize Capacity>
inline T* faBegin(FixedArray<T, Capacity>* a) { return a->items; }

template<typename T, usize Capacity>
inline T* faEnd(FixedArray<T, Capacity>* a) { return a->items + a->count; }

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_
