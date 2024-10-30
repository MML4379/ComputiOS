#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	    // Port multiplier
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4
#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3
#define	AHCI_BASE	    0x400000	// 4M
#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ  0x08
#define ATA_WRITE_DMA_EXT 0x35     // ATA command for 48-bit LBA write
#define AHCI_CMD_WRITE 0x5         // Command flag for write (bitmask)

typedef enum {
    FIS_TYPE_REG_H2D       = 0x27, // register FIS - host to device
    FIS_TYPE_REG_D2H       = 0x34, // register FIS - device to host
    FIS_TYPE_DMA_ACT       = 0x39, // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP     = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA          = 0x46, // Data FIS - bidirectional
    FIS_TYPE_BIST          = 0x58, // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP     = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS      = 0xA1, // Set device bits FIS - device to host
} FIS_TYPE;

// Host to device register FIS
typedef struct tagFIS_REG_H2D {
    // DWORD 0
    unsigned int fis_type;         // FIS_TYPE_REG_H2D

    unsigned int pmport:4;         // Port multiplier
    unsigned int rsv0:3;           // Reserved
    unsigned int c:1;              // 1: Command, 0: Control

    unsigned int command;          // Command register
    unsigned int featurel;         // Feature register, 7:0

    // DWORD 1
    unsigned int lba0;             // LBA low register, 7:0
    unsigned int lba1;             // LBA mid register, 15:8
    unsigned int lba2;             // LBA high register, 23:16
    unsigned int device;           // Device register

    // DWORD 2
    unsigned int lba3;             // LBA register, 31:24
    unsigned int lba4;             // LBA register, 39:32
    unsigned int lba5;             // LBA register, 47:40
    unsigned int featureh;         // Feature register, 15:8

    // DWORD 3
    unsigned int countl;           // Count register, 7:0
    unsigned int counth;           // Count register, 15:8
    unsigned int icc;              // Isochronous command completion
    unsigned int control;          // Control Register

    // DWORD 4
    unsigned int rsv1[4];          // Reserved
} FIS_REG_H2D;

// Device to host register FIS
typedef struct tagFIS_REG_D2H {
    // DWORD 0
    unsigned int fis_type;         // FIS_TYPE_REG_D2H
    
    unsigned int pmport:4;         // Port multiplier
    unsigned int rsv0:2;           // Reserved
    unsigned int i:1;              // Interrupt bit
    unsigned int rsv1:1;           // Reserved

    unsigned int status;           // Status register
    unsigned int error;            // Error register

    // DWORD 1
    unsigned int lba0;             // LBA low register, 7:0
    unsigned int lba1;             // LBA mid register, 15:8
    unsigned int lba2;             // LBA high register, 23:16
    unsigned int device;           // Device register

    // DWORD 2
    unsigned int lba3;             // LBA register, 31:24
    unsigned int lba4;             // LBA register, 39:32
    unsigned int lba5;             // LBA register, 47:40
    unsigned int rsv2;             // Reserved

    // DWORD 3
    unsigned int countl;           // Count register, 7:0
    unsigned int counth;           // Count register, 15:8
    unsigned int rsv3[2];          // Reserved

    // DWORD 4
    unsigned int rsv4[4];          // Reserved
} FIS_REG_D2H;

// Data FIS, bidirectional
typedef struct tagFIS_DATA {
    // DWORD 0
    unsigned int fis_type;         // FIS_TYPE_DATA

    unsigned int pmport:4;         // Port multiplier
    unsigned int rsv0:4;           // Reserved

    // DWORD 1 ~ N
    unsigned int data[1];          // Payload
} FIS_DATA;

// PIO setup, Device to host
typedef struct tagFIS_PIO_SETUP {
    // DWORD 0
    unsigned int fis_type;         // FIS_TYPE_PIO_SETUP

    unsigned int pmport:4;         // Port multiplier
    unsigned int rsv0:1;           // Reserved
    unsigned int d:1;              // Data transfer direction, 1 - device to host
    unsigned int i:1;              // Interrupt bit
    unsigned int rsv1:1;           // Reserved

    unsigned int status;           // Status register
    unsigned int error;            // Error register

    // DWORD 1
    unsigned int lba0;             // LBA low register, 7:0
    unsigned int lba1;             // LBA mid register, 15:8
    unsigned int lba2;             // LBA high register, 23:16
    unsigned int device;           // Device register

    // DWORD 2
    unsigned int lba3;             // LBA register, 31:24
    unsigned int lba4;             // LBA register, 39:32
    unsigned int lba5;             // LBA register, 47:40
    unsigned int rsv2;             // Reserved
    
    // DWORD 3
    unsigned int countl;            // Count register, 7:0
    unsigned int counth;            // Count register, 15:8
    unsigned int rsv3;              // Reserved
    unsigned int e_status;          // New value of status register

    // DWORD 4
    unsigned int tc;                // Transfer count
    unsigned int rsv4[2];           // Reserved
} FIS_PIO_SETUP;

// DMA Setup - Device to host
typedef struct tagFIS_DMA_SETUP {
    // DWORD 0
    unsigned int fis_type;          // FIS_TYPE_DMA_SETUP

    unsigned int pmport:4;          // Port multiplier
    unsigned int rsv0:1;            // Reserved
    unsigned int d:1;               // Data transfer direction - device to host
    unsigned int i:1;               // Interrupt bit
    unsigned int a:1;               // Auto-activate, specifies if DMA activate FIS is needed.

    unsigned int rsved[2];          // Reserved

    // DWORD 1 and 2
    unsigned int DMAbufferID        // DMA buffer identifier, used to identify DMA buffer in host memory.
                                    // The SATA specification says host specific and not in Spec. Trying AHCI spec might work.

    // DWORD 3
    unsigned int rsvd;              // Yet another reserved register

    // DWORD 4
    unsigned int DMAbufferOffset;   // Byte offset into buffer. First 2 bits must be 0.

    // DWORD 5
    unsigned int TransferCount;     // Number of bytes to transfer. Bit 0 must be 0.

    // DWORD 6
    unsigned int resvd;             // You guessed it, reserved.
} FIS_DMA_SETUP;

/**
 * Example
 * To issue an ATA identify command to the device, the FIS is constructed as follows:
 * 
 * FIS_REG_H2D fis;
 * memset(&fis, 0, sizeof(FIS_REG_H2D));
 * fis.fis_type = FIS_TYPE_REG_H2D;
 * fis.command = ATA_CMD_IDENTIFY; // 0xEC
 * fis.device = 0;                 // Master device
 * fis.c = 1;                      // Write command register
 */

typedef volatile struct tagHBA_MEM {
	// 0x00 - 0x2B, Generic Host Control
	unsigned int cap;		          // 0x00, Host capability
	unsigned int ghc;		          // 0x04, Global host control
	unsigned int is;		          // 0x08, Interrupt status
	unsigned int pi;		          // 0x0C, Port implemented
	unsigned int vs;		          // 0x10, Version
	unsigned int ccc_ctl;	          // 0x14, Command completion coalescing control
	unsigned int ccc_pts;	          // 0x18, Command completion coalescing ports
	unsigned int em_loc;		      // 0x1C, Enclosure management location
	unsigned int em_ctl;		      // 0x20, Enclosure management control
	unsigned int cap2;		          // 0x24, Host capabilities extended
	unsigned int bohc;		          // 0x28, BIOS/OS handoff control and status

	// 0x2C - 0x9F, Reserved
	unsigned int  rsv[0xA0-0x2C];

	// 0xA0 - 0xFF, Vendor specific registers
	unsigned int  vendor[0x100-0xA0];

	// 0x100 - 0x10FF, Port control registers
	HBA_PORT	ports[1];	// 1 ~ 32
} HBA_MEM;

typedef volatile struct tagHBA_PORT {
	unsigned int clb;		          // 0x00, command list base address, 1K-byte aligned
	unsigned int clbu;		          // 0x04, command list base address upper 32 bits
	unsigned int fb;		          // 0x08, FIS base address, 256-byte aligned
	unsigned int fbu;		          // 0x0C, FIS base address upper 32 bits
	unsigned int is;		          // 0x10, interrupt status
	unsigned int ie;		          // 0x14, interrupt enable
	unsigned int cmd;		          // 0x18, command and status
	unsigned int rsv0;		          // 0x1C, Reserved
	unsigned int tfd;		          // 0x20, task file data
	unsigned int sig;		          // 0x24, signature
	unsigned int ssts;		          // 0x28, SATA status (SCR0:SStatus)
	unsigned int sctl;		          // 0x2C, SATA control (SCR2:SControl)
	unsigned int serr;		          // 0x30, SATA error (SCR1:SError)
	unsigned int sact;		          // 0x34, SATA active (SCR3:SActive)
	unsigned int ci;		          // 0x38, command issue
	unsigned int sntf;		          // 0x3C, SATA notification (SCR4:SNotification)
	unsigned int fbs;		          // 0x40, FIS-based switch control
	unsigned int rsv1[11];		      // 0x44 ~ 0x6F, Reserved
	unsigned int vendor[4];			  // 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct tagHBA_FIS {
	// 0x00
	FIS_DMA_SETUP dsfis;		      // DMA Setup FIS
	unsigned int pad0[4];

	// 0x20
	FIS_PIO_SETUP psfis;		     // PIO Setup FIS
	unsigned int pad1[12]; 

	// 0x40
	FIS_REG_D2H	rfis;		         // Register – Device to Host FIS
	unsigned int pad2[4];

	// 0x58
	FIS_DEV_BITS sdbfis;		     // Set Device Bit FIS
	
	// 0x60
	unsigned int ufis[64];

	// 0xA0
	unsigned int rsv[0x100-0xA0];
} HBA_FIS;

typedef struct tagHBA_CMD_HEADER {
	// DW0
	unsigned int  cfl:5;		      // Command FIS length in DWORDS, 2 ~ 16
	unsigned int  a:1;		          // ATAPI
	unsigned int  w:1;		          // Write, 1: H2D, 0: D2H
	unsigned int  p:1;		          // Prefetchable

	unsigned int  r:1;		          // Reset
	unsigned int  b:1;		          // BIST
	unsigned int  c:1;		          // Clear busy upon R_OK
	unsigned int  rsv0:1;		      // Reserved
	unsigned int  pmp:4;		      // Port multiplier port

	uint16_t prdtl;		              // Physical region descriptor table length in entries

	// DW1
	volatile
	unsigned int prdbc;		          // Physical region descriptor byte count transferred

	// DW2, 3
	unsigned int ctba;		          // Command table descriptor base address
	unsigned int ctbau;		          // Command table descriptor base address upper 32 bits

	// DW4 - 7
	unsigned int rsv1[4];		      // Reserved
} HBA_CMD_HEADER;

typedef struct tagHBA_CMD_TBL {
	// 0x00
	unsigned int  cfis[64];	              // Command FIS

	// 0x40
	unsigned int  acmd[16];	              // ATAPI command, 12 or 16 bytes

	// 0x50
	unsigned int  rsv[48];	              // Reserved

	// 0x80
	HBA_PRDT_ENTRY	prdt_entry[1];	      // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

typedef struct tagHBA_PRDT_ENTRY {
	unsigned int dba;		            // Data base address
	unsigned int dbau;		            // Data base address upper 32 bits
	unsigned int rsv0;		            // Reserved

	// DW3
	unsigned int dbc:22;		       // Byte count, 4M max
	unsigned int rsv1:9;		       // Reserved
	unsigned int i:1;		           // Interrupt on completion
} HBA_PRDT_ENTRY;

void probe_port(HBA_MEM *abar);
static int check_type(HBA_PORT *port);
void port_rebase(HBA_PORT *port, int portno);
void start_cmd(HBA_PORT *port);
void stop_cmd(HBA_PORT *port);
bool read(HBA_PORT *port, unsigned int startl, unsigned int starth, unsigned int count, unsigned int *buf);
int find_cmdslot(HBA_PORT *port);