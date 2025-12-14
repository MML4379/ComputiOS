#include "ps2.hpp"
#include "../../kernel/libk/kprint.hpp"
#include "../../kernel/libk/types.hpp"
#include "../../kernel/io.hpp"
#include "../../kernel/vga.hpp"
#include "../../kernel/kdm.hpp"
#include "../../kernel/libk/string.hpp"

static bool match_ps2_controller(const kdm::Device* d) {
    return d && d->bus == kdm::BusType::PLATFORM && streq(d->name, "ps2");
}

static bool ps2_probe(kdm::Device*) {
    // Safe to call multiple times
    ps2::init_controller();
    // If you also want mouse enabled early, do it here too:
    // ps2::enable_mouse();
    return true;
}

static void ps2_remove(kdm::Device*) { /* no-op */ }

namespace ps2 {
    static constexpr uint16 DATA_PORT = 0x60;
    static constexpr uint16 STATUS_PORT = 0x64;
    static constexpr uint16 CMD_PORT = 0x64;

    // status bits (read from 0x64)
    static constexpr uint8 ST_OUT_FULL = 1 << 0;
    static constexpr uint8 ST_IN_FULL = 1 << 1;
    static constexpr uint8 ST_AUX_DATA = 1 << 5; // 1 = mouse (aux)

    // Controller commands (write to 0x64)
    static constexpr uint8 CMD_READ_CFG = 0x20;
    static constexpr uint8 CMD_WRITE_CFG = 0x60;
    static constexpr uint8 CMD_DISABLE_KB = 0xAD;
    static constexpr uint8 CMD_ENABLE_KB = 0xAE;
    static constexpr uint8 CMD_DISABLE_MOUSE = 0xA7;
    static constexpr uint8 CMD_ENABLE_MOUSE = 0xA8;
    static constexpr uint8 CMD_WRITE_AUX = 0xD4;

    // Config byte bits (common)
    static constexpr uint8 CFG_KB_IRQ = 1 << 0;
    static constexpr uint8 CFG_AUX_IRQ = 1 << 1;
    static constexpr uint8 CFG_TRANSLATE = 1 << 6;

    // Tiny ring buffers for raw bytes from IRQs
    template<size_t N>
    struct Ring {
        uint8 buf[N];
        volatile size_t head = 0;
        volatile size_t tail = 0;

        bool push(uint8 val) {
            size_t next_head = (head + 1) % N;
            if (next_head == tail) return false; // full
            buf[head] = val;
            head = next_head;
            return true;
        }

        bool pop(uint8* out) {
            if (head == tail) return false; // empty
            *out = buf[tail];
            tail = (tail + 1) % N;
            return true;
        }
    };

    static Ring<256> kb_ring;
    static Ring<256> mouse_ring;

    static inline void io_wait() { outb(0x80, 0); }

    static bool wait_in_empty(uint32 timeout = 100000) {
        while (timeout--) {
            if ((inb(STATUS_PORT) & ST_IN_FULL) == 0) return true;
        }
        return false;
    }

    static bool wait_out_full(uint32 timeout = 100000) {
        while (timeout--) {
            if (inb(STATUS_PORT) & ST_OUT_FULL) return true;
        }
        return false;
    }

    static void drain_output() {
        // drain any pending bytes
        for (int i = 0; i < 32; ++i) {
            uint8 st = inb(STATUS_PORT);
            if (!(st & ST_OUT_FULL)) break;
            (void)inb(DATA_PORT);
        }
    }

    static bool write_cmd(uint8 cmd) {
        if (!wait_in_empty()) return false;
        outb(CMD_PORT, cmd);
        io_wait();
        return true;
    }

    static bool write_data(uint8 data) {
        if (!wait_in_empty()) return false;
        outb(DATA_PORT, data);
        io_wait();
        return true;
    }

    static bool read_data(uint8& out) {
        if (!wait_out_full()) return false;
        out = inb(DATA_PORT);
        return true;
    }

    bool kb_write(uint8 data) {
        return write_data(data);
    }

    bool mouse_write(uint8 data) {
        if (!write_cmd(CMD_WRITE_AUX)) return false;
        return write_data(data);
    }

    static bool expect_ack() {
        // Devices should respond with 0xFA (ACK) (or 0xFE for resend)
        uint8 b = 0;
        for (int tries = 0; tries < 3; ++tries) {
            if (!read_data(b)) return false;
            if (b == 0xFA) return true; // ACK
            if (b == 0xFE) return false; // RESEND (treat as fail for now)
        }
        return false;
    }

    void init_controller() {
        // disable both ports while configuring
        write_cmd(CMD_DISABLE_KB);
        write_cmd(CMD_DISABLE_MOUSE);
        drain_output();

        // Read config
        if (!write_cmd(CMD_READ_CFG)) {
            kputs("PS2: Failed to send READ_CFG command");
            return;
        }

        uint8 cfg = 0;
        if (!read_data(cfg)) {
            kputs("PS2: Failed to read controller config");
            return;
        }

        // Enable IRQs for kb and mouse, disable translation
        cfg |= (CFG_KB_IRQ | CFG_AUX_IRQ);
        cfg &= ~CFG_TRANSLATE;

        // Write config
        if (!write_cmd(CMD_WRITE_CFG)) {
            kputs("PS2: Failed to send WRITE_CFG command");
            return;
        }
        if (!write_data(cfg)) {
            kputs("PS2: Failed to write controller config");
            return;
        }

        // Enable ports
        write_cmd(CMD_ENABLE_KB);
        write_cmd(CMD_ENABLE_MOUSE);

        // Enable mouse device streaming
        // 0xF6 = set defaults, 0xF4 = enable data reporting
        (void)mouse_write(0xF6); (void)expect_ack();
        (void)mouse_write(0xF4); (void)expect_ack();
    
        kputs("PS2: Controller initialized");
    }

    void on_irq(uint8 irq) {
        // read status first so byte routing is correct
        uint8 st = inb(STATUS_PORT);
        if (!(st & ST_OUT_FULL)) return; // spurious

        uint8 data = inb(DATA_PORT);

        // Route based on AUX bit primarily
        if (st & ST_AUX_DATA) {
            // mouse
            mouse_ring.push(data);
        } else {
            // keyboard
            kb_ring.push(data);
        }

        (void)irq;
    }

    bool kb_try_read(uint8* out) {
        return kb_ring.pop(out);
    }

    bool mouse_try_read(uint8* out) {
        return mouse_ring.pop(out);
    }

    static kdm::Driver g_ps2;
    extern "C" void ps2_register() {
        g_ps2.name = "ps2";
        g_ps2.bus = kdm::BusType::PLATFORM;
        g_ps2.match = match_ps2_controller;
        g_ps2.probe = ps2_probe;
        g_ps2.remove = ps2_remove;
    }
}