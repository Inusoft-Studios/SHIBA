// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_HANDLE_HPP_
#define SHIBA_PLATFORM_HANDLE_HPP_

#include "types.h"

namespace shiba {

// Generic typed handle; id 0 is null handle.
template<typename Tag>
struct Handle { u32 id; };

template<typename Tag> constexpr bool valid(const Handle<Tag>& h) { return h.id != 0; }
template<typename Tag> constexpr bool operator==(Handle<Tag> lhs, Handle<Tag> rhs) { return lhs.id == rhs.id; }
template<typename Tag> constexpr bool operator!=(Handle<Tag> lhs, Handle<Tag> rhs) { return !(lhs == rhs); }

// TODO: create macro for SHIBA_INVALID_HANDLE
#define SHIBA_INVALID_HANDLE(tag) (::shiba::Handle<tag>{0u})

}  // namespace shiba

#endif  // SHIBA_PLATFORM_HANDLE_HPP_
