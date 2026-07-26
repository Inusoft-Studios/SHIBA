// =============================================================================
//                        Copyright Inusoft Studios
// =============================================================================

#include "platform/surface.h"
#include "platform/allocator.h"  // AllocationCallbacks definition

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>

#include <new>                  // placement new

namespace shiba {

namespace {
constexpr u32 kMaxSurfaces  = 8;
constexpr u32 kEventCap     = 128;  // per surface event ring
constexpr i32 kCharCap      = 256;

struct SurfaceData {
    HWND                hwnd;
    Extent2D            extent;
    AllocationCallbacks alloc;
    SurfaceEvent        events[kEventCap];
    u32                 head, tail;
    bool                bShouldClose;
    bool                bTrackingMouse;
};

SurfaceData* gTable[kMaxSurfaces] = {};
u16          gGen  [kMaxSurfaces] = {};

// --- handle <-> slot ---
Surface pack(const u32 i, const u16 g) { return Surface{ (static_cast<u32>(g) << 16) | (i)}; }
u32     idxOf(const Surface h)   { return h.id & 0xFFFF; }
u16     genOf(const Surface h)   { return static_cast<u16>(h.id >> 16); }

SurfaceData* resolve(const Surface h) {
    const u32 i = idxOf(h);
    if (i >= kMaxSurfaces || !gTable[i] || gGen[i] != genOf(h)) return nullptr;
    return gTable[i];
}

// --- event ring ---
void push(SurfaceData* s, const SurfaceEvent& e) {
    const u32 next = (s->tail + 1) % kEventCap;
    if (next == s->head) return;    // full: drop newest
    s->events[s->tail] = e;
    s->tail = next;
}
bool pop(SurfaceData* s, SurfaceEvent* out) {
    if (s->head == s->tail) return false;
    *out = s->events[s->head];
    s->head = (s->head + 1) % kEventCap;
    return true;
}
SurfaceEvent ev(const SurfaceEventType t) { SurfaceEvent e{}; e.mType = t; return e; }

// --- window class ---
const wchar_t* kClassName = L"ShibaSurfaceClass";
bool gClassRegistered = false;

int toWide(const char* utf8, wchar_t* out) {
    const int n = MultiByteToWideChar(CP_UTF8, 0, utf8 ? utf8 : "", -1, out, kCharCap);
    if (n == 0) out[0] = L'\0';
    return n;
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    auto* s = reinterpret_cast<SurfaceData*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (!s) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
        case WM_CLOSE: {
            s->bShouldClose = true;
            push(s, ev(SurfaceEventType::Close));
            return 0;
        }

        case WM_SIZE: {
            s->extent = { static_cast<u32>(LOWORD(lp)), static_cast<u32>(HIWORD(lp)) };
            if (s->extent.width && s->extent.height) {
                SurfaceEvent e = ev(SurfaceEventType::Resize);
                e.extent = s->extent;
                push(s, e);
            }
            return 0;
        }

        case WM_KEYDOWN: case WM_SYSKEYDOWN: {
            SurfaceEvent e = ev(SurfaceEventType::KeyDown);
            e.keyDown.mKey = static_cast<i32>(wp);
            push(s, e);
            return 0;
        }
        case WM_KEYUP: case WM_SYSKEYUP: {
            SurfaceEvent e = ev(SurfaceEventType::KeyUp);
            e.keyUp.mKey = static_cast<i32>(wp);
            push(s, e);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (!s->bTrackingMouse) {
                TRACKMOUSEEVENT tme{ sizeof(tme), TME_LEAVE, hwnd, 0 };
                TrackMouseEvent(&tme);
                s->bTrackingMouse = true;
                SurfaceEvent e = ev(SurfaceEventType::MouseEnter);
                e.mouseEnter = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
                push(s, e);
            }
            SurfaceEvent e = ev(SurfaceEventType::MouseMove);
            e.mouseMove = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
            push(s, e);
            return 0;
        }
        case WM_MOUSELEAVE: {
            s->bTrackingMouse = false;
            push(s, ev(SurfaceEventType::MouseLeave));
            return 0;
        }

        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: {
            SurfaceEvent e = ev(SurfaceEventType::MouseButtonDown);
            e.mouseButton.mButton = (msg == WM_LBUTTONDOWN) ? 0 : (msg == WM_RBUTTONDOWN) ? 1 : 2;
            push(s, e);
            return 0;
        }
        case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: {
            SurfaceEvent e = ev(SurfaceEventType::MouseButtonUp);
            e.mouseButton.mButton = (msg==WM_LBUTTONUP)?0:(msg==WM_RBUTTONUP)?1:2;
            push(s, e); return 0;
        }
        case WM_MOUSEWHEEL: {
            SurfaceEvent e = ev(SurfaceEventType::MouseWheel);
            e.mouseWheel = { 0, GET_WHEEL_DELTA_WPARAM(wp) };
            push(s, e); return 0;
        }
        case WM_MOUSEHWHEEL: {
            SurfaceEvent e = ev(SurfaceEventType::MouseScroll);
            e.scroll = { GET_WHEEL_DELTA_WPARAM(wp), 0 };
            push(s, e); return 0;
        }
        case WM_SETFOCUS: case WM_KILLFOCUS: {
            SurfaceEvent e = ev(SurfaceEventType::Focus);
            e.mFocus.bGained = (msg == WM_SETFOCUS);
            push(s, e); return 0;
        }

        default: return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

void ensureClass() {
    if (gClassRegistered) return;
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = wndProc;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    wc.hbrBackground = reinterpret_cast<HBRUSH>((COLOR_WINDOW + 1));  // TODO: replace this with RDI
    RegisterClassExW(&wc);
    gClassRegistered = true;
}
}  // namespace

Surface createSurface(const WindowDesc& desc, const AllocationCallbacks* alloc) {
    ensureClass();

    u32 slot = kMaxSurfaces;
    for (u32 i = 0; i < kMaxSurfaces; ++i) if (!gTable[i]) { slot = i; break; }
    if (slot == kMaxSurfaces) return SHIBA_INVALID_HANDLE(SurfaceTag);
    if (gGen[slot] == 0) gGen[slot] = 1;

    void* mem = alloc->pFnAlloc(alloc->pUser, sizeof(SurfaceData), alignof(SurfaceData));
    if (!mem) return SHIBA_INVALID_HANDLE(SurfaceTag);
    auto* s   = new (mem) SurfaceData{};
    s->alloc  = *alloc;
    s->extent = desc.extent;
    gTable[slot] = s;       // register before create

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!desc.bResizable) style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

    RECT r { 0, 0, static_cast<i32>(desc.extent.width), static_cast<i32>(desc.extent.height) };
    AdjustWindowRect(&r, style, FALSE);

    wchar_t wtitle[kCharCap];
    toWide(desc.pTitle, wtitle);

    s->hwnd = CreateWindowExW(0, kClassName, wtitle, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        r.right - r.left, r.bottom - r.top,
        nullptr, nullptr, GetModuleHandleW(nullptr), s);
    if (!s->hwnd) {
        gTable[slot] = nullptr;
        s->~SurfaceData();
        alloc->pFnFree(alloc->pUser, mem);
        return SHIBA_INVALID_HANDLE(SurfaceTag);
    }
    ShowWindow(s->hwnd, SW_SHOW);
    return pack(slot, gGen[slot]);
}

void destroySurface(Surface surface) {
    SurfaceData* s = resolve(surface);
    if (!s) return;
    const u32 slot = idxOf(surface);

    if (s->hwnd) DestroyWindow(s->hwnd);
    const AllocationCallbacks a = s->alloc;
    s->~SurfaceData();
    a.pFnFree(a.pUser, s);
    gTable[slot] = nullptr;
    if (++gGen[slot] == 0) gGen[slot] = 1;
}

bool surfacePollEvent(Surface surface, SurfaceEvent* out) {
    SurfaceData* s = resolve(surface);
    if (!s) return false;
    MSG msg;
    while (s->head == s->tail &&        // pump only until we have one
           PeekMessageW(&msg, s->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
           }
    return pop(s, out);
}

bool surfaceShouldClose(Surface surface) {
    const auto* s = resolve(surface);
    return s ? s->bShouldClose : true;
}
Extent2D surfaceGetExtent (Surface surface) {
    const auto* s = resolve(surface);
    return s ? s->extent : Extent2D{0,0};
}
NativeHandle surfaceGetNativeHandle(Surface surface) {
    const auto* s = resolve(surface);
    return s ? s->hwnd : nullptr;
}

void surfaceSetTitle(Surface surface, const char* title) {
    const SurfaceData* s = resolve(surface);
    if (!s) return;
    wchar_t w[256];
    toWide(title, w);
    SetWindowTextW(s->hwnd, w);
}

}  // namespace shiba