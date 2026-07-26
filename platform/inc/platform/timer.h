// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_TIMER_H_
#define SHIBA_PLATFORM_TIMER_H_

#include "types.h"

namespace shiba {

struct Clock {
    u64 frequency;  // ticks/sec, invariant for the process
    u64 epoch;      // reference tick captured at boot
};

struct Timer {
    u64 start;
};

extern "C" {
void clockInit(void);

Timer timerStart(const Clock* c);
void  timerReset(Timer* t, const Clock* c);

u64   timerElapsedNS (const Timer* t, const Clock* c);
f64   timerElapsedSec(const Timer* t, const Clock* c);
}

u64 platformTicks(void);
u64 platformFrequency(void);

inline u64 ticksToNS(const u64 ticks, const u64 freq) {
    return (ticks / freq) * 1'000'000'000ull
         + (ticks % freq) * 1'000'000'000ull / freq;
}

}  // namespace shiba

#endif  // SHIBA_PLATFORM_TIMER_H_
