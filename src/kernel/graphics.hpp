#pragma once
#include "libk/types.hpp"

namespace gfx {
    struct FramebufferInfo {
        uint64 phys_base;
        uint32 width;
        uint32 height;
        uint32 pitch;
        uint32 bpp;
        uint32 format;
    } __attribute__((packed));

    struct Point { int32 x, y; };
    struct Rect { int32 x, y, w, h; };

    struct Color {
        public:
            uint32 value = 0;
            explicit constexpr Color(uint32 rrggbb = 0) : value(rrggbb & 0xFFFFFFu) {}

            static constexpr Color rgb(uint8 r, uint8 g, uint8 b) {
                return Color((uint32(r) << 16) | (uint32(g) << 8) | uint32(b)); 
            }

            static constexpr Color hex(uint32 rrggbb) {
                return Color(rrggbb & 0x00FFFFFFu);
            }

            // Accessor method
            constexpr uint32 get_value() const { return value; }
    };

    bool init(const FramebufferInfo* boot_fb);
    bool enable_double_buffer(bool enable);
    void present();

    // info
    uint32 width();
    uint32 height();
    uint32 pitch();
    bool ready();

    // drawing (draws to current draw target: backbuffer if enabled, otherwise LFB)
    void clear(Color c);
    void put_pixel(int32 x, int32 y, Color c);
    void hline(int32 x, int32 y, int32 w, Color c);
    void vline(int32 x, int32 y, int32 h, Color c);
    void fill_rect(Rect r, Color c);
    void rect(Rect r, Color c);

    // Text (8x16 bitmap font)
    void draw_char(int32 x, int32 y, char ch, Color fg, Color bg, bool use_bg);
    void draw_text(int32 x, int32 y, const char* s, Color fg);
    void draw_text_bg(int32 x, int32 y, const char* s, Color fg, Color bg);

    // UI
    struct Window {
        Rect bounds;
        const char* title;
        Color frame;
        Color title_bg;
        Color title_fg;
        Color body_bg;
        bool visible;
    };

    void ui_begin(); // clears internal window list
    int ui_add_window(const Window& w); // returns handle/index
    void ui_draw(); // draws windows + info bar
    void ui_set_status(const char* left, const char* right);
}