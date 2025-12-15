#pragma once
#include <kernel/libk/types.hpp>

namespace ps2 {
    void init_controller(); // init controller + enable irqs for kb/mouse

    // IRQ demus called from IRQ handler
    // - irq == 1 -> keyboard
    // - irq == 12 -> mouse
    void on_irq(uint8 irq);

    // non-blocking reads from internal byte queues
    bool kb_try_read(uint8* out);
    bool mouse_try_read(uint8* out);

    // Send commands to keyboard/mouse
    bool kb_write(uint8* data);
    bool mouse_write(uint8* data);
}