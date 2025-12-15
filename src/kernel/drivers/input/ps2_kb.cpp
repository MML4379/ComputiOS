#include "ps2_kb.hpp"
#include "ps2.hpp"
#include <kernel/libk/types.hpp>
#include <kernel/libk/kprint.hpp>
#include <kernel/libk/string.hpp>
#include <kernel/drivers/kdm.hpp>

static bool match_ps2_kb(const kdm::Device* d) {
    return d && d->bus == kdm::BusType::PLATFORM && streq(d->name, "ps2-kb");
}

static int kb_open(void*)  { return 0; }
static int kb_close(void*) { return 0; }

static int kb_read(void*, void* buf, size_t n) {
    if (!buf || n == 0) return 0;

    uint8* out = (uint8*)buf;
    size_t got = 0;

    while (got < n) {
        uint8 b = 0;
        if (!ps2::kb_try_read(&b)) break; // pointer-based
        out[got++] = b;
    }

    return (int)got;
}

static int kb_write(void*, const void*, size_t) { return -1; }
static int kb_ioctl(void*, uint64, uint64) { return -1; }

static bool kb_probe(kdm::Device*) {
    // Safe to call multiple times
    ps2::init_controller();

    kdm::DevNode node{};
    node.name = "kb0";
    node.type = kdm::DevType::CHAR;
    node.ctx = nullptr;

    node.ops.open = kb_open;
    node.ops.close = kb_close;
    node.ops.read = kb_read;
    node.ops.write = kb_write;
    node.ops.ioctl = kb_ioctl;

    if (!kdm::kdm_publish_devnode(node)) {
        kprintf("ps2-kb: failed to publish devnode kb0\n");
        return false;
    }

    kprintf("ps2-kb: ready (DEV:/kb0/)\n");
    return true;
}

static void kb_remove(kdm::Device*) {} // no-op for now

static kdm::Driver g_ps2kb;
extern "C" void ps2_kb_register() {
    g_ps2kb.name = "ps2-kb";
    g_ps2kb.bus = kdm::BusType::PLATFORM;
    g_ps2kb.match = match_ps2_kb;
    g_ps2kb.probe = kb_probe;
    g_ps2kb.remove = kb_remove;

    kdm::kdm_register_driver(g_ps2kb);
}