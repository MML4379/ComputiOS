#include "ahci.hpp"
#include "ahci_register.hpp"

#include <libk/kprint.hpp>
#include <mm/vmm.hpp>
#include <libk/string.hpp>
#include <drivers/kdm.hpp>

namespace ahci {

    // AHCI hardware definitions
    static constexpr uint32 SATA_SIG_ATA   = 0x00000101;
    static constexpr uint32 SATA_SIG_ATAPI = 0xEB140101;
    static constexpr uint32 SATA_SIG_PM    = 0x96690101;
    static constexpr uint32 SATA_SIG_SEMB  = 0xC33C0101;

    struct HBAPort {
        uint32 clb;
        uint32 clbu;
        uint32 fb;
        uint32 fbu;
        uint32 is;
        uint32 ie;
        uint32 cmd;
        uint32 rsv0;
        uint32 tfd;
        uint32 sig;
        uint32 ssts;
        uint32 sctl;
        uint32 serr;
        uint32 sact;
        uint32 ci;
        uint32 sntf;
        uint32 fbs;
        uint32 rsv1[11];
        uint32 vendor[4];
    };

    struct HBARegs {
        uint32 cap;
        uint32 ghc;
        uint32 is;
        uint32 pi;
        uint32 vs;
        uint32 ccc_ctl;
        uint32 ccc_pts;
        uint32 em_loc;
        uint32 em_ctl;
        uint32 cap2;
        uint32 bohc;
        uint8  rsv[0xA0 - 0x2C];
        uint8  vendor[0x100 - 0xA0];
        HBAPort ports[32];
    };

    struct AhciDisk {
        volatile HBARegs* hba;
        volatile HBAPort* port;
        uint32 port_no;
    };

    // forward declarations
    static bool ahci_probe(kdm::Device* dev);

    // devnode ops (STUBS)
    static int ahci_read_stub(void* ctx, void* buf, size_t bytes) {
        (void)ctx;
        (void)buf;
        (void)bytes;
        kprintf("AHCI: read() stub called\n");
        return -1;
    }

    static int ahci_write_stub(void* ctx, const void* buf, size_t bytes) {
        (void)ctx;
        (void)buf;
        (void)bytes;
        kprintf("AHCI: write() stub called\n");
        return -1;
    }

    // driver registration
    static kdm::Driver ahci_driver = {
        .name  = "ahci",
        .bus   = kdm::BusType::PCI,
        .probe = ahci_probe,
        .remove = nullptr,
        .match = [](const kdm::Device* d) {
            return kdm::pci_match_class(d, 0x01, 0x06, 0x01);
        },
    };

    void register_driver() {
        kdm::kdm_register_driver(ahci_driver);
    }

    // probe implementation
    static bool ahci_probe(kdm::Device* dev) {
        kprintf("AHCI: probing PCI device\n");
        static bool published = false;

        uint64 abar_phys = dev->u.pci.bar[5];
        if (!abar_phys) {
            kprintf("AHCI: BAR5 missing\n");
            return false;
        }

        auto hba = reinterpret_cast<HBARegs*>(
            vmm::map_mmio(abar_phys, 4096)
        );
        if (!hba) {
            kprintf("AHCI: failed to map ABAR\n");
            return false;
        }

        // enable AHCI mode
        hba->ghc |= (1u << 31);

        dev->driver_data = (void*)hba;

        uint32 pi = hba->pi;
        kprintf("AHCI: ports implemented mask = %x\n", pi);

        static int disk_index = 0;

        for (int i = 0; i < 32; i++) {
            if (!(pi & (1u << i)))
                continue;

            HBAPort* port = &hba->ports[i];

            uint32 ssts = port->ssts;
            uint8 det = ssts & 0x0F;
            uint8 ipm = (ssts >> 8) & 0x0F;

            kprintf(
                "AHCI: port %d ssts=%x det=%u ipm=%u sig=%x\n",
                i, ssts, det, ipm, port->sig
            );

            if (det != 3 || ipm != 1) {
                kprintf("AHCI: port %d not active\n", i);
                continue;
            }

            if (!published && port->sig == SATA_SIG_ATA) {
                published = true;
                kprintf("AHCI: SATA (ATA) device on port %d\n", i);

                // allocate disk object
                auto* disk = new AhciDisk {
                    .hba     = hba,
                    .port    = port,
                    .port_no = (uint32)i,
                };

                kdm::DevNode node{};
                node.name = "disk0";
                node.ctx  = disk;
                node.ops.read  = ahci_read_stub;
                node.ops.write = ahci_write_stub;

                kdm::kdm_publish_devnode(node);
            }
            else if (port->sig == SATA_SIG_ATAPI) {
                kprintf("AHCI: ATAPI device on port %d (ignored)\n", i);
            }
            else if (port->sig == SATA_SIG_PM) {
                kprintf("AHCI: Port Multiplier on port %d (ignored)\n", i);
            }
            else if (port->sig == SATA_SIG_SEMB) {
                kprintf("AHCI: SEMB device on port %d (ignored)\n", i);
            }
            else {
                kprintf(
                    "AHCI: unknown device sig=%x on port %d\n",
                    port->sig, i
                );
            }
        }

        return true;
    }

} // namespace ahci