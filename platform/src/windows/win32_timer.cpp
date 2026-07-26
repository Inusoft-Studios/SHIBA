#include "platform/timer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace shiba {
u64 platformTicks() {
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return static_cast<u64>(c.QuadPart);
}
u64 platformFrequency() {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return static_cast<u64>(f.QuadPart);
}
}  // namespace shiba