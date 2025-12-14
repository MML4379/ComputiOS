#include "ps2_mouse.hpp"
#include "../../../kernel/kdm.hpp"
#include "../ps2.hpp"
#include "../../../kernel/libk/kprint.hpp"
#include "../../../kernel/vga.hpp"

static bool streq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (*a != *b) return false;
        ++a; ++b;
    }
    return *a == *b;
}

static bool match_ps2_mouse(const kdm::Device* d) {
    return d && d->bus == kdm::BusType::PLATFORM && streq(d->name, "ps2-mouse");
}

static int mouse_open(void*)  { return 0; }
static int mouse_close(void*) { return 0; }

static uint32 g_mouse_x = 0;
static uint32 g_mouse_y = 0;

static void print_mouse_xy(int32 x, int32 y) {
    print_str(5, 0, "MOUSE X=", 0x0A);
    print_dec(5, 8, (uint32)x, 0x0A);
    print_str(5, 20, "Y=", 0x0A);
    print_dec(5, 22, (uint32)y, 0x0A);
}

// Read returns whole 3-byte packets (multiples of 3)
static int mouse_read(void*, void* buf, size_t n) {
    if (!buf || n < 3) return 0;

    uint8* out = (uint8*)buf;
    size_t out_i = 0;

    while (out_i + 3 <= n) {
        uint8 b0 = 0, b1 = 0, b2 = 0;

        // Sync to a valid first byte (bit 3 must be 1)
        while (true) {
            if (!ps2::mouse_try_read(&b0)) return (int)out_i;
            if (b0 & 0x08) break;
        }

        if (!ps2::mouse_try_read(&b1)) return (int)out_i;
        if (!ps2::mouse_try_read(&b2)) return (int)out_i;

        int8 dx = (int8)b1;
        int8 dy = (int8)b2;

        g_mouse_x += dx;
        g_mouse_y -= dy;

        print_mouse_xy(g_mouse_x, g_mouse_y);

        out[out_i++] = b0;
        out[out_i++] = b1;
        out[out_i++] = b2;
    }

    return (int)out_i;
}

static int mouse_write(void*, const void*, size_t) {
    return -1;
}

// Match your FileOps signature: ioctl(ctx, req, arg_u64)
static int mouse_ioctl(void*, uint64, uint64) {
    return -1;
}

static bool mouse_probe(kdm::Device*) {
    ps2::init_controller();

    kdm::DevNode node{};
    node.name = "mouse0";
    node.type = kdm::DevType::CHAR;
    node.ctx  = nullptr;

    node.ops.open  = mouse_open;
    node.ops.close = mouse_close;
    node.ops.read  = mouse_read;
    node.ops.write = mouse_write;
    node.ops.ioctl = mouse_ioctl;

    if (!kdm::kdm_publish_devnode(node)) {
        kprintf("ps2-mouse: failed to publish devnode mouse0\n");
        return false;
    }

    kprintf("ps2-mouse: ready (DEV:/mouse0/)\n");
    return true;
}

static void mouse_remove(kdm::Device*) {}

static kdm::Driver g_ps2mouse;
extern "C" void ps2_mouse_register() {
    g_ps2mouse.name = "ps2-mouse";
    g_ps2mouse.bus = kdm::BusType::PLATFORM;
    g_ps2mouse.match = match_ps2_mouse;
    g_ps2mouse.probe = mouse_probe;
    g_ps2mouse.remove = mouse_remove;

    kdm::kdm_register_driver(g_ps2mouse);
}