#include "pci.hpp"
#include <kernel/libk/types.hpp>
#include <kernel/arch/x86_64/serial.hpp>
#include <kernel/arch/x86_64/io.hpp>
#include <kernel/drivers/kdm.hpp>

// config addr for bus/dev/func/offset
static uint32 pci_config_address(uint8 bus, uint8 device, uint8 function, uint8 offset) {
    return (uint32)(
        (1u << 31) |
        ((uint32)bus << 16) |
        ((uint32)device << 11) |
        ((uint32)function << 8) |
        (offset & 0xFC)
    );
}

static uint32 pci_config_read_32(uint8 bus, uint8 device, uint8 function, uint8 offset) {
    uint32 addr = pci_config_address(bus, device, function, offset);
    outl(0xCF8, addr);
    return inl(0xCFC);
}

static uint16 pci_read16(uint8 bus, uint8 dev, uint8 func, uint8 off) {
    uint32 v = pci_config_read_32(bus, dev, func, off);
    return (uint16)((v >> ((off & 2) * 8)) & 0xFFFF);
}

static uint8 pci_read8(uint8 bus, uint8 dev, uint8 func, uint8 off) {
    uint32 v = pci_config_read_32(bus, dev, func, off);
    return (uint8)((v >> ((off & 3) * 8)) & 0xFF);
}

void pci_init() {
    serial_write_str("PCI: Scanning buses...\n");

    int row = 6;

    for (uint8 bus = 0; bus < 256; ++bus) {
        for (uint8 dev = 0; dev < 32; ++dev) {
            for (uint8 func = 0; func < 8; ++func) {

                uint32 vendor_device = pci_config_read_32(bus, dev, func, 0x00);
                uint16 vendor = (uint16)(vendor_device & 0xFFFF);

                if (vendor == 0xFFFF) {
                    if (func == 0) break; else continue;
                }

                uint16 device_id = (uint16)((vendor_device >> 16) & 0xFFFF);

                // Read class info
                uint32 class_reg = pci_config_read_32(bus, dev, func, 0x08);
                uint8 revision   = (uint8)(class_reg & 0xFF);
                uint8 prog_if    = (uint8)((class_reg >> 8) & 0xFF);
                uint8 subclass   = (uint8)((class_reg >> 16) & 0xFF);
                uint8 class_code = (uint8)((class_reg >> 24) & 0xFF);

                // Read IRQ line/pin (legacy)
                uint8 irq_line = pci_read8(bus, dev, func, 0x3C);
                uint8 irq_pin  = pci_read8(bus, dev, func, 0x3D);

                // Read BARs (raw)
                uint64 bars[6];
                for (int i = 0; i < 6; ++i) {
                    bars[i] = (uint64)pci_config_read_32(bus, dev, func, (uint8)(0x10 + i * 4));
                }

                // KDM device registration
                kdm::Device kd{};
                kd.bus = kdm::BusType::PCI;
                kd.name = "pci-func";
                kd.driver_data = nullptr;
                kd.bound = false;

                kd.u.pci.vendor_id  = vendor;
                kd.u.pci.device_id  = device_id;
                kd.u.pci.class_code = class_code;
                kd.u.pci.subclass   = subclass;
                kd.u.pci.prog_if    = prog_if;
                kd.u.pci.revision   = revision;

                kd.u.pci.irq_line = irq_line;
                kd.u.pci.irq_pin  = irq_pin;

                for (int i = 0; i < 6; ++i) kd.u.pci.bar[i] = bars[i];

                kdm::kdm_register_device(kd);

                // serial logging
                serial_write_str("PCI dev: bus=0x");
                serial_write_hex8(bus);
                serial_write_str(" dev=0x");
                serial_write_hex8(dev);
                serial_write_str(" func=0x");
                serial_write_hex8(func);
                serial_write_str(" vendor=0x");
                serial_write_hex16(vendor);
                serial_write_str(" device=0x");
                serial_write_hex16(device_id);
                serial_write_str(" class=");
                serial_write_hex8(class_code);
                serial_write_str(":");
                serial_write_hex8(subclass);
                serial_write_str(":");
                serial_write_hex8(prog_if);
                serial_write_str("\n");

                ++row;
                if (row >= 24) return;
            }
        }
    }

    serial_write_str("PCI: Scan done.\n");
}
