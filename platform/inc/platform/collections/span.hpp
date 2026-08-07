// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_COLLECTIONS_SPAN_HPP_
#define SHIBA_PLATFORM_COLLECTIONS_SPAN_HPP_

#include "platform/types.h"

namespace shiba {

template<typename T>
struct Span {
    T*    data;
    usize size;
};

// --- Queries ---

template<typename T>
inline usize spanSize(const Span<T> s) { return s.size; }

template<typename T>
inline bool spanEmpty(const Span<T> s) { return s.size == 0u; }

// --- Access ---

template<typename T>
inline T* spanData(const Span<T> s) { return s.data; }

template<typename T>
inline T* spanAt(const Span<T> s, const usize i) { return &s.data[i]; }

template<typename T>
inline T* spanFront(const Span<T> s) { return &s.data[0]; }

template<typename T>
inline T* spanBack(const Span<T> s) { return &s.data[s.size - 1u]; }

template<typename T>
inline T* spanBegin(const Span<T> s) { return s.data; }

template<typename T>
inline T* spanEnd(const Span<T> s) { return s.data + s.size; }

// --- Subviews ---

template<typename T>
inline Span<T> spanSubspan(const Span<T> s, const usize off, const usize n) {
    return Span<T>{ s.data + off, n };
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_COLLECTIONS_SPAN_HPP_
