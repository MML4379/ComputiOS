#include "ahci.h"

void probe_port(HBA_MEM *abar) {
    // Search disk in implemented ports
    unsigned int pi = abar->pi;
    int i = 0;
    while (i<32) {
        if (pi & 1) {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
                trace_ahci("SATA drive found at port %d\n", i);
            }else if (dt == AHCI_DEV_SATAPI) {
                trace_ahci("SATAPI drive found at port %d\n", i);
            }else if (dt == AHCI_DEV_SEMB) {
                trace_ahci("SEMB drive foud at port %d\n", i);
            }else if (dt == AHCI_DEV_PM) {
                trace_ahci("PM drive found at port %d\n", i);
            }else {
                trace_ahci("No drive found at port %d\n", i);
            }
        }

        pi >>= 1;
        i++;
    }
}

static int check_type(HBA_PORT *port) {
    unsigned int ssts = port->ssts;

    unsigned int ipm = (ssts >> 8) & 0x0F;
    unsigned int det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) {
        return AHCI_DEV_NULL;
    }
    if (ipm != HBA_PORT_IPM_ACTIVE) {
        return AHCI_DEV_NULL;
    }

    switch (port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_SEMB;
        default:
            return AHCI_DEV_SATA;
    }
}

void port_rebase(HBA_PORT *port, int portno) {
    stop_cmd(port); // stop command engine

    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry max. count = 32
    // Command list max. size = 32*32 = 1024 (1K) per port
    port->clb = AHCI_BASE + (portno<<10);
    port->clbu = 0;
    memset((void*)(port->clb), 0, 1024);

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32<<10) + (portno<<8);
    port->fbu = 0;
    memset((void*)(port->fb), 0, 256);

    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(port->clb);
    for (int i = 0; i < 32; i++) {
        cmdheader[i].prdtl = 8;     // 8 prdt entries per command table
                                    // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
        cmdheader[i].ctbau = 0;
        memset((void*)cmdheader[i].ctba, 0, 256);
    }

    start_cmd(port); // start command engine
}

void start_cmd(HBA_PORT *port) {
    // wait until CR (bit 15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // SET FRE (bit 4) and ST (bit 0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

void stop_cmd(HBA_PORT *port) {
    // Clear ST (bit 0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Clear FRE (bit 4)
    port->cmd &= ~HBA_PxCMD_FRE;

    // Wait until FR (bit 14) and CR (bit 15) are cleared
    while (1) {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }
}

bool read(HBA_PORT *port, unsigned int startl, unsigned int starth, unsigned int count, unsigned int *buf) {
    port->is = (unsigned int) -1;     // clear pending interrupt bits
    int spin = 0;                     // spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1) {
        return false;
    }

    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(unsigned int); // command FIS size
    cmdheader->w = 0; // read from device
    cmdheader->prdtl = (unsigned int)((count-1)>>4) + 1; // PRDT entries count

    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

    // 8K bytes (16 sectors) per PRDT
    for (int i = 0; i < cmdheader->prdtl-1; i++) {
        cmdtbl->prdt_entry[i].dba = (unsigned int) buf;
        cmdtbl->prdt_entry[i].dbc = 8*1024-1; // 8K bytes (this value should always be set to 1 less than the actual value)
        cmdtbl->prdt_entry[i].i = 1;

        // Setup command
        FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

        cmdfis->fis_type = FIS_TYPE_REG_H2D;
        cmdfis->c = 1;
        cmdfis->command = ATA_CMD_READ_DMA_EX;

        cmdfis->lba0 = (unsigned int)startl;
        cmdfis->lba1 = (unsigned int)(startl>>8);
        cmdfis->lba2 = (unsigned int)(startl>>16);
        cmdfis->device = 1<<6; // LBA mode

        cmdfis->lba3 = (unsigned int)(startl>>24);
        cmdfis->lba4 = (unsigned int)starth;
        cmdfis->lba5 = (unsigned int)(starth>>8);

        cmdfis->countl = count & 0xFF;
        cmdfis->counth = (count >> 8) & 0xFF;
        
        // This loop waits until the port is no longer busy before issuing a new command.
        while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
            spin++;
        }
        if (spin == 1000000) {
            trace_ahci("Port is hung\n");
            return false;
        }

        port->ci = 1<<slot; // Issue command

        // Wait for completion
        while(1) {
            // In some longer duration reads, it may be helpful o spin on the DPS bit in the PxIS port field as well (1<<5)
            if ((port->ci & (1<<slot)) == 0) {
                break;
            }
            if (port->is & HBA_PxIS_TFES) { // Task file error
                trace_ahci("Disk read error!\n");
                return false;
            }
        }

        // Check again
        if (port->is & HBA_PxIS_TFES) {
            trace_ahci("Disk read error\n");
            return false;
        }

        return true;
    }
}

bool write(unsigned int sector, unsigned int count, void* buffer, unsigned int port) {
    // Find available command slot
    port->is = (unsigned int) -1; // clear pending interrupt bits
    int spin = 0;                 // spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1) {
        return false;
    }

    // Get the port structure and command list base address
    volatile hba_port_t *port_struct = get_port(port);
    hba_cmd_header_t *cmd_header = (hba_cmd_header_t *)port_struct->clb;
    cmd_header += slot; // Point to selected command slot header

    // Set up the command header for writing
    cmd_header->cfl = sizeof(fis_reg_h2d_t) / sizeof(unsigned int); // Command FIS size
    cmd_header->w = 1; // write operation
    cmd_header->prdtl = (unsigned int)((count-1) >> 4) + 1; // PRDT entries needed

    // Set up command table
    hba_cmd_tbl_t *cmd_tbl = (hba_cmd_tbl_t*)(cmd_header->ctba);
    memset(cmd_tbl, 0, sizeof(hba_cmd_tbl_t) + (cmd_header->prdtl - 1) * sizeof(hba_prdt_entry_t));

    // Copy the buffer to PRDT entries in the command table
    unsigned int i;
    for (i = 0; i < cmd_header->prdtl - 1; i++) {
        cmd_tbl->prdt_entry[i].dba = (unsigned int) buffer + (i << 12); // Physical address of data block
        cmd_tbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K Bytes per entry
        cmd_tbl->prdt_entry[i].i = 1; // Interrupt on completion
    }
    // Last entry
    cmd_tbl->prdt_entry[i].dba = (unsigned int) buffer + (i << 12);
    cmd_tbl->prdt_entry[i].dbc = ((count << 9) - 1) & 0x3FFFFF; // Last entry block count
    cmd_tbl->prdt_entry[i].i = 1;

    // Set up command FIS
    fis_reg_h2d_t *cmd_fis = (fis_reg_h2d_t *)(&cmd_tbl->cfis);

    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1; // Command (write)
    cmd_fis->command = ATA_WRITE_CMA_EXT;

    // Set up LBA 48-bit addressing
    cmd_fis->lba0 = (unsigned int)sector;
    cmd_fis->lba1 = (unsigned int)(sector >> 8);
    cmd_fis->lba2 = (unsigned int)(sector >> 16);
    cmd_fis->device = 1 << 6; // LBA mode

    cmd_fis->lba3 = (unsigned int)(sector >> 24);
    cmd_fis->lba4 = (unsigned int)(sector >> 32);
    cmd_fis->lba5 = (unsigned int)(sector >> 40);

    // Issue the command
    port_struct->ci = 1 << slot; // Start command

    // Wait for completion
    while(1) {
        // In some longer duration writes, it may be helpful o spin on the DPS bit in the PxIS port field as well (1<<5)
        if ((port->ci & (1<<slot)) == 0) {
            break;
        }
        if (port->is & HBA_PxIS_TFES) { // Task file error
            trace_ahci("Disk write error!\n");
                return false;
            }
        }

        // Check again
        if (port->is & HBA_PxIS_TFES) {
            trace_ahci("Disk write error!!\n");
            return false;
        }

        return true;
}

// Find free command list slot
int find_cmdslot(HBA_PORT *port) {
    // If not set in SACT and CI, the slot is free
    unsigned int slots = (port->sact | port->ci);
    for (int i = 0; i < cmdslots; i++) {
        if ((slots&1) == 0) {
            return i;
        }
        slots >>= 1;
    }
    trace_ahci("Cannot find free command list entry\n");
    return -1;
}