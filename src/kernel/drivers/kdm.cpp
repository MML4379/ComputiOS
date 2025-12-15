#include "kdm.hpp"
#include <kernel/libk/types.hpp>
#include <kernel/libk/kprint.hpp>

namespace kdm {
    static constexpr size_t KDM_MAX_DEVICES = 256;
    static constexpr size_t KDM_MAX_DRIVERS = 128;
    static constexpr size_t KDM_MAX_DEVNODES = 256;

    static Device g_devices[KDM_MAX_DEVICES];
    static Driver g_drivers[KDM_MAX_DRIVERS];
    static DevNode g_devnodes[KDM_MAX_DEVNODES];

    static size_t g_device_count = 0;
    static size_t g_driver_count = 0;
    static size_t g_devnode_count = 0;

    static bool streq(const char* a, const char* b) {
        if (!a || !b) return false;
        while (*a && *b) {
            if (*a != *b) return false;
            ++a; ++b;
        }
        return *a == *b;
    }

    void kdm_init() {
        g_device_count  = 0;
        g_driver_count  = 0;
        g_devnode_count = 0;

        // Zero out arrays
        for (size_t i = 0; i < KDM_MAX_DEVICES; ++i) {
            g_devices[i] = Device{};
        }
        for (size_t i = 0; i < KDM_MAX_DRIVERS; ++i) {
            g_drivers[i] = Driver{};
        }
        for (size_t i = 0; i < KDM_MAX_DEVNODES; ++i) {
            g_devnodes[i] = DevNode{};
        }

        kprintf("kdm: initialized\n");
    }

    

    // Device registry

    bool kdm_register_device(const Device& dev) {
        if (g_device_count >= KDM_MAX_DEVICES) {
            kprintf("kdm: device registry full\n");
            return false;
        }

        Device d = dev;
        d.driver_data = nullptr;
        d.bound = false;

        g_devices[g_device_count++] = d;
        return true;
    }

    size_t kdm_device_count() {
        return g_device_count;
    }

    Device* kdm_device_at(size_t i) {
        if (i >= g_device_count) return nullptr;
        return &g_devices[i];
    }



    // Driver registry

    bool kdm_register_driver(const Driver& drv) {
        if (g_driver_count >= KDM_MAX_DRIVERS) {
            kprintf("kdm: driver registry full\n");
            return false;
        }

        if (!drv.probe) {
            kprintf("kdm: refusing to register driver with null probe (%s)\n", drv.name ? drv.name : "(unnamed)");
            return false;
        }

        g_drivers[g_driver_count++] = drv;
        return true;
    }

    size_t kdm_driver_count() {
        return g_driver_count;
    }

    const Driver* kdm_driver_at(size_t i) {
        if (i >= g_driver_count) return nullptr;
        return &g_drivers[i];
    }

    // -------------------------
    // Binding
    // -------------------------

    static bool driver_matches_device(const Driver& drv, const Device& dev) {
        if (drv.bus != dev.bus) return false;
        if (drv.match) return drv.match(&dev);
        return true; // catch-all for that bus
    }

    void kdm_bind_all() {
        for (size_t di = 0; di < g_device_count; ++di) {
            Device* dev = &g_devices[di];
            if (dev->bound) continue;

            for (size_t ri = 0; ri < g_driver_count; ++ri) {
                Driver& drv = g_drivers[ri];

                if (!driver_matches_device(drv, *dev)) continue;

                if (drv.probe(dev)) {
                    dev->bound = true;
                    kprintf("kdm: bound device to driver '%s'\n", drv.name ? drv.name : "(unnamed)");
                    break;
                }
            }

            if (!dev->bound) {
                kprintf("kdm: no driver for device\n");
            }
        }
    }


    
    // dev node registry

    bool kdm_publish_devnode(const DevNode& node) {
        if (!node.name) return false;
        if (g_devnode_count >= KDM_MAX_DEVNODES) {
            kprintf("kdm: devnode registry full\n");
            return false;
        }

        // Prevent duplicates by name
        for (size_t i = 0; i < g_devnode_count; ++i) {
            if (streq(g_devnodes[i].name, node.name)) {
                kprintf("kdm: devnode '%s' already exists\n", node.name);
                return false;
            }
        }

        g_devnodes[g_devnode_count++] = node;
        kprintf("kdm: published devnode '%s'\n", node.name);
        return true;
    }

    size_t kdm_devnode_count() {
        return g_devnode_count;
    }

    DevNode* kdm_find_devnode(const char* name) {
        if (!name) return nullptr;
        for (size_t i = 0; i < g_devnode_count; ++i) {
            if (streq(g_devnodes[i].name, name)) {
                return &g_devnodes[i];
            }
        }
        return nullptr;
    }



    int kdm_open(const char* name) {
        DevNode* n = kdm_find_devnode(name);
        if (!n || !n->ops.open) return -1;
        return n->ops.open(n->ctx);
    }

    int kdm_close(const char* name) {
        DevNode* n = kdm_find_devnode(name);
        if (!n || !n->ops.close) return -1;
        return n->ops.close(n->ctx);
    }

    int kdm_read(const char* name, void* buf, size_t nbytes) {
        DevNode* n = kdm_find_devnode(name);
        if (!n || !n->ops.read) return -1;
        return n->ops.read(n->ctx, buf, nbytes);
    }

    int kdm_write(const char* name, const void* buf, size_t nbytes) {
        DevNode* n = kdm_find_devnode(name);
        if (!n || !n->ops.write) return -1;
        return n->ops.write(n->ctx, buf, nbytes);
    }

    int kdm_ioctl(const char* name, uint64 req, uint64 arg) {
        DevNode* n = kdm_find_devnode(name);
        if (!n || !n->ops.ioctl) return -1;
        return n->ops.ioctl(n->ctx, req, arg);
    }

    // pci matching helpers
    bool pci_match_vid_did(const Device* dev, uint16 vendor, uint16 device) {
        if (!dev || dev->bus != BusType::PCI) return false;
        return dev->u.pci.vendor_id == vendor && dev->u.pci.device_id == device;
    }

    bool pci_match_class(const Device* dev, uint8 class_code, uint8 subclass, uint8 prog_if) {
        if (!dev || dev->bus != BusType::PCI) return false;

        const PciInfo& p = dev->u.pci;
        if (p.class_code != class_code) return false;
        if (p.subclass != subclass) return false;

        // prog_if wildcard allowed
        if (prog_if != 0xFF && p.prog_if != prog_if) return false;

        return true;
    }

} // namespace kdm