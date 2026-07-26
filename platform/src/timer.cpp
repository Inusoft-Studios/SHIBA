#include "platform/timer.h"

extern "C" shiba::Clock shibaClockInit(void) {
    return shiba::Clock{shiba::platformFrequency(), shiba::platformTicks()};
}
extern "C" shiba::Timer shibaTimerStart(void) {
    return shiba::Timer{shiba::platformTicks()};
}
extern "C" void shibaTimerReset(shiba::Timer* t) {
    t->start = shiba::platformTicks();
}
extern "C" shiba::u64 shibaTimerElapsedNS (const shiba::Timer* t, const shiba::Clock* c) {
    return shiba::ticksToNS(c->epoch - t->start, c->frequency);
}
extern "C" shiba::f64 shibaTimerElapsedSec(const shiba::Timer* t, const shiba::Clock* c) {
    return static_cast<shiba::f64>(shiba::platformTicks() - t->start) / static_cast<shiba::f64>(c->frequency);
}