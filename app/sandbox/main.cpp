#include <platform/allocator.h>
#include <platform/compiler.h>
#include <platform/gfx.h>
#include <platform/surface.h>
#include <platform/timer.h>
#include <platform/types.h>
#include <platform/collections/fixed_array.hpp>

#include <cstdio>   // printf
#include <cstdlib>  // _aligned_malloc / _aligned_free


namespace {

// CRT-backed default allocator. pUser unused here.
// NOTE: _aligned_malloc MUST pair with _aligned_free - plain free() is UB.
void* stdAlloc(void*, const shiba::usize size, const shiba::usize align) {
    return _aligned_malloc(size, align);
}
void stdFree (void*, void* ptr, shiba::usize, shiba::usize) {
    _aligned_free(ptr);
}

void handle(const shiba::SurfaceEvent& e) {
    using E = shiba::SurfaceEventType;
    switch (e.mType) {
        case E::Resize:  printf("[Resize] %ux%u\n", e.extent.width, e.extent.height); break;
        case E::KeyDown: printf("[KeyDown] vk=%d\n", e.keyDown.mKey);                 break;
        case E::MouseButtonDown: printf("[MBDown] btn=%d\n", e.mouseButton.mButton);  break;
        default: break;
    }
}

struct FrameClock {
    shiba::Timer frame;       // resets each frame -> yields delta
    shiba::f64   accum;       // banked real time awaiting fixed steps
    shiba::f64   alpha;       // 0..1 interpolation factor for render
    shiba::f64   fpsElapsed;  // real time banked toward next title update
    shiba::u32   fpsFrames;   // frames counted this interval
};

}  // namespace

int main(int, char**) {
    constexpr shiba::AllocationCallbacks alloc{ stdAlloc, stdFree, nullptr };

    // Temp fixed array test
    shiba::FixedArray<shiba::u32, 8> arr{};
    shiba::faPush(&arr, 1);
    shiba::faPop(&arr);
    shiba::faClear(&arr);

    shiba::SurfaceDesc desc{};
    desc.pTitle     = "SHIBA Sandbox";
    desc.extent     = { 1280, 720 };
    desc.bResizable = true;

    const shiba::Surface window = shiba::surfaceCreate(desc, &alloc);
    if (!shiba::surfaceValid(window)) { printf("surface creation failed\n"); return -1; }

    if (!shiba::gfxInit(shiba::GfxApi::Vulkan, SHIBA_DEBUG, &alloc)) {
        printf("gfx init failed\n");
        shiba::surfaceDestroy(window);
        return -1;
    }

    shiba::GfxAdapter adapters[8];
    const shiba::u32 n = shiba::gfxEnumerateAdapters(adapters, 8);
    printf("[gfx] %u adapter(s):\n", n);
    for (shiba::u32 i = 0; i < n && i < 8; ++i)
        printf("  [%u] %s\n", i, shiba::gfxAdapterName(adapters[i]));
    printf("[gfx] picked adapter: '%s'\n", shiba::gfxAdapterName(shiba::gfxDefaultAdapter()));

    constexpr shiba::f64 kFixedDT = 1.0 / 60.0; // 60Hz sim
    constexpr shiba::f64 kMaxDt   = 0.25;       // clamp: anti "spiral of death"
    const shiba::Clock clock = shibaClockInit();

    FrameClock fc{ shibaTimerStart(), 0.0, 0.0, 0.0, 0 };

    while (!shiba::surfaceShouldClose(window)) {
        shiba::f64 dt = shibaTimerElapsedSec(&fc.frame, &clock);
        shibaTimerReset(&fc.frame);

        // --- fps: count real frames over real time (pre-clamp) ---
        fc.fpsElapsed += dt;
        fc.fpsFrames  += 1;
        if (fc.fpsElapsed >= 1.0) {                    // refresh once per second
            const shiba::f64 fps = fc.fpsFrames / fc.fpsElapsed;
            const shiba::f64 ms  = 1000.0 * fc.fpsElapsed / fc.fpsFrames;
            char title[64];
            std::snprintf(title, sizeof(title), "SHIBA Sandbox - %.2f FPS (%.5f ms)", fps, ms);
            shiba::surfaceSetTitle(window, title);
            fc.fpsElapsed = 0.0;
            fc.fpsFrames  = 0;
        }

        if (dt > kMaxDt) dt = kMaxDt;                   // clamp only affects the sim

        shiba::SurfaceEvent e{};
        while (shiba::surfacePollEvent(window, &e)) handle(e);

        fc.accum += dt;
        while (fc.accum >= kFixedDT) {
            fc.accum -= kFixedDT;
        }
        fc.alpha = fc.accum / kFixedDT;
    }

    shiba::gfxShutdown();
    shiba::surfaceDestroy(window);
    return 0;
}