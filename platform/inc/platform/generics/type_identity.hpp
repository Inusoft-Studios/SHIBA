// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_GENERICS_TYPE_IDENTITY_HPP_
#define SHIBA_PLATFORM_GENERICS_TYPE_IDENTITY_HPP_

namespace shiba {

template<typename T>
struct TypeIdentity { using type = T; };

template<typename T>
using TypeIdentity_t = typename TypeIdentity<T>::type;

}  // namespace shiba

#endif  // SHIBA_PLATFORM_GENERICS_TYPE_IDENTITY_HPP_
