#include <platform/allocator.h>
#include <platform/surface.h>
#include <platform/types.h>

#include <cstdio>   // printf
#include <cstdlib>  // _aligned_malloc / _aligned_free

namespace {

// CRT-backed default allocator. pUser unused here.
// NOTE: _aligned_malloc MUST pair with _aligned_free - plain free() is UB.
void* stdAlloc(void*, const shiba::usize size, const shiba::usize align) {
    return _aligned_malloc(size, align);
}
void stdFree (void*, void* ptr) {
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

}  // namespace

int main(int, char**) {
    constexpr shiba::AllocationCallbacks alloc{ stdAlloc, stdFree, nullptr };

    shiba::WindowDesc desc{};
    desc.pTitle     = "SHIBA Sandbox";
    desc.extent     = { 1280, 720 };
    desc.bResizable = true;

    const shiba::Surface window = shiba::createSurface(desc, &alloc);
    if (!shiba::valid(window)) { printf("surface creation failed\n"); return -1; }

    while (!shiba::surfaceShouldClose(window)) {
        shiba::SurfaceEvent e{};
        while (shiba::surfacePollEvent(window, &e)) handle(e);
    }

    shiba::destroySurface(window);
    return 0;
}