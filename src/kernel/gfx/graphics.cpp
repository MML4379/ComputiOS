#include "graphics.hpp"
#include "font.hpp"
#include <kernel/libk/kprint.hpp>
#include <kernel/libk/memory.hpp>
#include <kernel/mm/heap.hpp>
#include <kernel/mm/vmm.hpp>

namespace gfx {
    // internal framebuffer state
    static FramebufferInfo g_fb{};
    static bool g_ready = false;

    static uint8* g_lfb = nullptr; // mapped linear framebuffer (virtual)
    static uint8* g_draw = nullptr; // current draw target (lfb or backbuffer)
    static uint8* g_back = nullptr; // backbuffer (heap)
    static bool g_double = false;

    static uint64 align_down_4k(uint64 x) { return x & ~0xFFFull; }
    static uint64 align_up_4k(uint64 x)   { return (x + 0xFFF) & ~0xFFFull; }

    static constexpr uint64 FB_VIRT_BASE = 0xFFFF'8000'1000'0000ull;

    // mapping framebuffer
    static void* map_lfb(const FramebufferInfo& fb) {
        const uint64 bytes = (uint64)fb.pitch * (uint64)fb.height;

        const uint64 phys_base = fb.phys_base;
        const uint64 phys0 = align_down_4k(phys_base);
        const uint64 end  = align_up_4k(phys_base + bytes);
        const uint64 pages = (end - phys0) / 0x1000;
        const uint64 offset = phys_base - phys0;

        const uint64 flags =
            vmm::PAGE_PRESENT |
            vmm::PAGE_RW |
            vmm::PAGE_PCD |
            vmm::PAGE_PWT;

        for (uint64 i = 0; i < pages; i++) { if (!vmm::map_page(FB_VIRT_BASE + i * 0x1000, phys0 + i * 0x1000, flags)) return nullptr; }

        return (void*)(FB_VIRT_BASE + offset);
    }

    // public API
    bool init(const FramebufferInfo* boot_fb) {
        kputs("CPOS_GFX: setting up...");
        if (!boot_fb) return false;

        g_fb = *boot_fb;
        if (g_fb.phys_base == 0 || g_fb.width == 0 || g_fb.height == 0) return false;
        if (g_fb.bpp != 32) {
            kprintf("CPOS_GFX: unsupported bpp=%u (expected 32)\n", g_fb.bpp);
            return false;
        }

        g_lfb = (uint8*)map_lfb(g_fb);
        if (!g_lfb) {
            kprintf("CPOS_GFX: failed to map LFB\n");
            return false;
        }

        g_draw = g_lfb;
        g_ready = true;

        kprintf("CPOS_GFX: LFB mapped virt=%x phys=%x %ux%u pitch=%u\n",
            (uint64)g_lfb, g_fb.phys_base, g_fb.width, g_fb.height, g_fb.pitch);

        return true;
    }

    bool enable_double_buffer(bool enable) {
    if (!g_ready) return false;

    if (!enable) {
        if (g_back) {
            // adjust if your heap uses different names
            kfree(g_back);
            g_back = nullptr;
        }
        g_double = false;
        g_draw = g_lfb;
        return true;
    }

    if (g_back) {
        g_double = true;
        g_draw = g_back;
        return true;
    }

    const uint64 bytes = (uint64)g_fb.pitch * (uint64)g_fb.height;
    g_back = (uint8*)kmalloc(bytes);
    if (!g_back) return false;

    // clear backbuffer
    for (uint64 i = 0; i < bytes; i++) g_back[i] = 0;

    g_double = true;
    g_draw = g_back;
    return true;
    }

    void present() {
        if (!g_ready || !g_double || !g_back) return;

        const uint64 bytes = (uint64)g_fb.pitch * (uint64)g_fb.height;
        volatile uint8* dst = (volatile uint8*)g_lfb;
        const uint8* src = (const uint8*)g_back;

        for (uint64 i = 0; i < bytes; i++) dst[i] = src[i];
    }

    uint32 width()  { return g_fb.width; }
    uint32 height() { return g_fb.height; }
    uint32 pitch()  { return g_fb.pitch; }
    bool ready()    { return g_ready; }

    // Primitives (32bpp)
    static inline void put32(uint8* base, uint32 x, uint32 y, uint32 pitch, uint32 c) {
        *(uint32*)(base + (uint64)y * pitch + (uint64)x * 4) = c;
    }

    void clear(Color c) {
        if (!g_ready) return;
        for (uint32 y = 0; y < g_fb.height; y++) {
            uint8* row = g_draw + (uint64)y * g_fb.pitch;
            uint32* px = (uint32*)row;
            for (uint32 x = 0; x < g_fb.width; x++) {
                px[x] = c.value;
            }
        }
    }

    void put_pixel(int32 x, int32 y, Color c) {
        if (!g_ready) return;
        if ((uint32)x >= g_fb.width || (uint32)y >= g_fb.height) return;
        put32(g_draw, (uint32)x, (uint32)y, g_fb.pitch, c.value);
    }

    void hline(int32 x, int32 y, int32 w, Color c) {
        if (!g_ready || w <= 0) return;
        if ((uint32)y >= g_fb.height) return;

        int32 x0 = x;
        int32 x1 = x + w - 1;
        if (x0 < 0) x0 = 0;
        if (x1 >= (int32)g_fb.width) x1 = (int32)g_fb.width - 1;

        uint8* row = g_draw + (uint64)(uint32)y * g_fb.pitch;
        uint32* px = (uint32*)row;
        for (int32 xi = x0; xi <= x1; xi++) px[xi] = c.value;
    }

    void vline(int32 x, int32 y, int32 h, Color c) {
        if (!g_ready || h <= 0) return;
        if ((uint32)x >= g_fb.width) return;

        int32 y0 = y;
        int32 y1 = y + h - 1;
        if (y0 < 0) y0 = 0;
        if (y1 >= (int32)g_fb.height) y1 = (int32)g_fb.height - 1;

        for (int32 yi = y0; yi <= y1; yi++) {
            put32(g_draw, (uint32)x, (uint32)yi, g_fb.pitch, c.value);
        }
    }

    void fill_rect(Rect r, Color c) {
        if (!g_ready) return;
        if (r.w <= 0 || r.h <= 0) return;

        int32 x0 = r.x, y0 = r.y;
        int32 x1 = r.x + r.w - 1;
        int32 y1 = r.y + r.h - 1;

        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x1 >= (int32)g_fb.width)  x1 = (int32)g_fb.width - 1;
        if (y1 >= (int32)g_fb.height) y1 = (int32)g_fb.height - 1;

        for (int32 y = y0; y <= y1; y++) {
            uint8* row = g_draw + (uint64)(uint32)y * g_fb.pitch;
            uint32* px = (uint32*)row;
            for (int32 x = x0; x <= x1; x++) px[x] = c.value;
        }
    }

    void rect(Rect r, Color c) {
        hline(r.x, r.y, r.w, c);
        hline(r.x, r.y + r.h - 1, r.w, c);
        vline(r.x, r.y, r.h, c);
        vline(r.x + r.w - 1, r.y, r.h, c);
    }

    // Text
    void draw_char(int32 x, int32 y, char ch, Color fg, Color bg, bool use_bg) {
        if (!g_ready) return;

        uint8 uc = (uint8)ch;
        if (uc >= 128) uc = '?';

        const unsigned char* glyph = font8x16[uc];

        for (int row = 0; row < 16; row++) {
            uint8 bits = (uint8)glyph[row];

            // If the font renders mirrored, swap to (bits >> col) & 1
            for (int col = 0; col < 8; col++) {
                bool on = (bits >> (7 - col)) & 1;
                if (on) put_pixel(x + col, y + row, fg);
                else if (use_bg) put_pixel(x + col, y + row, bg);
            }
        }
    }

    void draw_text(int32 x, int32 y, const char* s, Color fg) {
        if (!s) return;
        int32 cx = x;
        while (*s) {
            if (*s == '\n') { cx = x; y += 16; s++; continue; }
            draw_char(cx, y, *s, fg, Color::hex(0), false);
            cx += 16;
            s++;
        }
    }

    void draw_text_bg(int32 x, int32 y, const char* s, Color fg, Color bg) {
        if (!s) return;
        int32 cx = x;
        while (*s) {
            if (*s == '\n') { cx = x; y += 16; s++; continue; }
            draw_char(cx, y, *s, fg, bg, true);
            cx += 16;
            s++;
        }
    }

    // UI
    static Window g_windows[16];
    static int g_win_count = 0;

    static const char* g_status_left = "ComputiOS";
    static const char* g_status_right = "1280x720";

    void ui_begin() {
        g_win_count = 0;
    }

    int ui_add_window(const Window& w) {
        if (g_win_count >= 16) return -1;
        g_windows[g_win_count] = w;
        return g_win_count++;
    }

    void ui_set_status(const char* left, const char* right) {
        if (left)  g_status_left = left;
        if (right) g_status_right = right;
    }

    static void draw_window(const Window& w) {
        if (!w.visible) return;

        // Outer frame
        fill_rect(w.bounds, w.frame);

        // Titlebar (top 18 px)
        Rect title{ w.bounds.x + 2, w.bounds.y + 2, w.bounds.w - 4, 18 };
        fill_rect(title, w.title_bg);

        // Body
        Rect body{ w.bounds.x + 2, w.bounds.y + 2 + 18, w.bounds.w - 4, w.bounds.h - 4 - 18 };
        fill_rect(body, w.body_bg);

        // Title text (8px font, vertically centered-ish)
        draw_text(title.x + 6, title.y + 5, w.title ? w.title : "Window", w.title_fg);

        // Border highlight
        rect(w.bounds, Color::rgb(255,255,255));
    }

    void ui_draw() {
        if (!g_ready) return;

        // Desktop background
        clear(Color::rgb(18, 18, 22));

        // Windows (in insertion order)
        for (int i = 0; i < g_win_count; i++) draw_window(g_windows[i]);

        // Bottom status bar
        const int bar_h = 24;
        Rect bar{ 0, (int32)g_fb.height - bar_h, (int32)g_fb.width, bar_h };
        fill_rect(bar, Color::rgb(20, 20, 28));
        hline(0, bar.y, (int32)g_fb.width, Color::hex(0x0a0a0a));

        draw_text(bar.x + 6, bar.y + 4, g_status_left, Color::rgb(220,220,230));

        // Right text: crude right-align by strlen*8 (no libc; do manual)
        int len = 0;
        for (const char* p = g_status_right; p && *p; ++p) len++;
        int32 rx = (int32)g_fb.width - 6 - len * 16;
        if (rx < 0) rx = 0;
        draw_text(rx, bar.y + 4, g_status_right, Color::rgb(220,220,230));
    }
}