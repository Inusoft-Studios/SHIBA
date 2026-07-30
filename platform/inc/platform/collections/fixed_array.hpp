// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_

#include "platform/cache_line.h"
#include "platform/types.h"

namespace shiba {

template<usize Capacity>
using FixedArraySizeType = std::conditional_t<
    (Capacity <= 0xFFull), u8,
    std::conditional_t<(Capacity <= 0xFFFFull), u16,
    std::conditional_t<(Capacity <= 0xFFFFFFFFull), u32, u64>>>;

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_FIXED_ARRAY_HPP_
