// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
#include <type_traits>

namespace shiba {

template<class T>
struct DynamicArray {
    static_assert(std::is_trivially_copyable_v<T>, "shiba: T must be trivially copyable");
};

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_DYNAMIC_ARRAY_HPP_
