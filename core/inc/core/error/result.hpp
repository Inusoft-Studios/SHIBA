// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_CORE_ERROR_RESULT_HPP_
#define SHIBA_CORE_ERROR_RESULT_HPP_

#include <type_traits>
#include <cassert>  // replace with assert handler

namespace shiba {

template<typename T> struct Ok  { T value; };
template<typename E> struct Err { E error; };

template<typename T, typename E>
struct Result {
    union {
        T value;
        E error;
    };
    bool bOk;

    static_assert(std::is_trivially_copyable_v<T>, "Result<T, E>: T must be trivially copyable");
    static_assert(std::is_trivially_copyable_v<E>, "Result<T, E>: E must be trivially copyable");

    Result(Ok<T> ok)   : value(ok.value), bOk(true) {}
    Result(Err<E> err) : error(err.error), bOk(false) {}
};

// --- Construction ---

template<typename T> Ok<T>  makeOk(T v)  { return {v}; }
template<typename E> Err<E> makeErr(E e) { return {e}; }

// --- Queries ---

template<typename T, typename E> inline bool isOk (const Result<T, E>& r) { return r.bOk; }
template<typename T, typename E> inline bool isErr(const Result<T, E>& r) { return !r.bOk; }

template<typename T, typename E> inline const T& getValue(const Result<T, E>& r) { assert(r.bOk);  return r.value; }
template<typename T, typename E> inline const E& getError(const Result<T, E>& r) { assert(!r.bOk); return r.error; }

template<typename U, typename T, typename E>
inline const U& get(const Result<T, E>& r) {
    static_assert(std::is_same_v<U, T> || std::is_same_v<U, E>, "get<U>: U must be either T or E");
    static_assert(!std::is_same_v<T, E>, "get<U> ambiguous when T == E");
    if constexpr (std::is_same_v<U, T>) return r.value;
    else                                return r.error;
}

}  // namespace shiba

#endif  // SHIBA_CORE_ERROR_RESULT_HPP_
