#pragma once
#include "libk/types.hpp"

// Kernel Driver Model (KDM)
// Registers devices discovered by buses
// Register drivers
// bind drivers to devices
// public device nodes for kernel/user access (future)

namespace kdm {
    enum class BusType : uint8 {
        PCI,
        ISA,
        PLATFORM
    };

    struct PciInfo {
        uint16 vendor_id;
        uint64 device_id;
        uint8 class_code;
        uint8 subclass;
        uint8 prog_if;
        uint8 revision;

        uint8 irq_line;
        uint8 irq_pin;

        uint64 bar[6];
    };

    struct Device {
        BusType bus;

        const char* name;
        void* driver_data;
        bool bound;

        union {
            PciInfo pci;
            // put other bus info structs as needed
        } u;
    };

    using ProbeFn = bool(*)(Device* dev);
    using RemoveFn = void(*)(Device* dev);
    using MatchFn = bool(*)(const Device* dev);

    struct Driver {
        const char* name;
        BusType bus;

        ProbeFn probe;
        RemoveFn remove;
        MatchFn match;
    };

    struct FileOps {
        int (*open)(void* ctx);
        int (*close)(void* ctx);
        int (*read)(void* ctx, void* buf, size_t n);
        int (*write)(void* ctx, const void* buf, size_t n);
        int (*ioctl)(void* ctx, uint64 req, uint64 arg);
    };

    enum class DevType : uint8 {
        CHAR,
        BLOCK,
        NET,
        FB,
        OTHER
    };

    struct DevNode {
        const char* name;
        DevType type;
        FileOps ops;
        void* ctx; // driver-owned object/instance
    };

    void kdm_init();

    // devices (registered by bus enumerators like PCI scan)
    bool kdm_register_device(const Device& dev);
    size_t kdm_device_count();
    Device* kdm_device_at(size_t i);

    // drivers (registered by driver modules)
    bool kdm_register_driver(const Driver& drv);
    size_t kdm_driver_count();
    const Driver* kdm_driver_at(size_t i);

    // bind all unbound devices to drivers
    void kdm_bind_all();

    // dev nodes (published by drivers after node)
    bool kdm_publish_devnode(const DevNode& node);
    size_t kdm_devnode_count();
    const DevNode* kdm_devnode_at(const char* name);

    int kdm_open(const char* name);
    int kdm_close(const char* name);
    int kdm_read(const char* name, void* buf, size_t n);
    int kdm_write(const char* name, const void* buf, size_t n);
    int kdm_ioctl(const char* name, uint64 req, uint64 arg);

    // helpers
    bool pci_match_vid_did(const Device* dev, uint16 vendor, uint16 device);
    bool pci_match_class(const Device* dev, uint8 class_code, uint8 subclass, uint8 prog_if);
} // namespace kdm