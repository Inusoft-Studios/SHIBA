// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#ifndef SHIBA_PLATFORM_TIMER_H_
#define SHIBA_PLATFORM_TIMER_H_

#include "types.h"

namespace shiba {

struct Clock { u64 frequency; u64 epoch; };
struct Timer { u64 start; };

inline u64 ticksToNS(const u64 ticks, const u64 freq) {
    return (ticks / freq) * 1'000'000'000ull
         + (ticks % freq) * 1'000'000'000ull / freq;
}
u64 platformTicks(void);
u64 platformFrequency(void);
}  // namespace shiba

extern "C" {
shiba::Clock shibaClockInit(void);
shiba::Timer shibaTimerStart(const shiba::Clock* c);
void         shibaTimerReset(shiba::Timer* t, const shiba::Clock* c);
shiba::u64   shibaTimerElapsedNS (const shiba::Timer* t, const shiba::Clock* c);
shiba::f64   shibaTimerElapsedSec(const shiba::Timer* t, const shiba::Clock* c);
}

#endif  // SHIBA_PLATFORM_TIMER_H_
