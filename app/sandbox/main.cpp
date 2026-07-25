#include <platform/error/result.hpp>
#include <platform/types.h>

#include <cassert>

namespace {
enum class AppError : shiba::u8 {
    Success,
    Fail
};

using R = shiba::Result<shiba::u8, AppError>;

R foo() { return shiba::makeOk(shiba::u8{1}); }
R bar() { return shiba::makeErr(AppError::Fail); }

void smokeTest() {
    // --- Ok path ---
    {
        const R r = foo();
        assert(shiba::isOk(r));
        assert(!shiba::isErr(r));
        assert(shiba::getValue(r) == 1);
        assert(shiba::get<shiba::u8>(r) == 1);
    }

    // --- Err path ---
    {
        const R r = bar();
        assert(!shiba::isOk(r));
        assert(shiba::isErr(r));
        assert(shiba::getError(r) == AppError::Fail);
        assert(shiba::getError(r) != AppError::Success);
        assert(shiba::get<AppError>(r) == AppError::Fail);
    }

    // --- get() returns a reference into this object, not a copy ---
    {
        const R r = foo();
        static_assert(std::is_same_v<decltype(shiba::getValue(r)), const shiba::u8&>);
        static_assert(std::is_same_v<decltype(shiba::get<shiba::u8>(r)), const shiba::u8&>);
        assert(&shiba::getValue(r) == &r.value);
    }

    // --- layout invariants the codebase relies on ---
    static_assert(std::is_trivially_copyable_v<R>, "Result must stay memcpy-able");
    static_assert(!std::is_default_constructible_v<R>, "Result should require Ok/Err");
}

}  // namespace

int main(int, char**) {
    smokeTest();
    return 0;
}