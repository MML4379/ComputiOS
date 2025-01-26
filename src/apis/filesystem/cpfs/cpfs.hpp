#include "../fs.h"
#include "../../libc/libc.h"

// In same order as table in README
typedef struct Superblock {
    uint32_t inodes;
    uint32_t blocks;
    uint32_t blocks_su;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t block_size;
    uint32_t fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t mounted_since_fschk;
    uint16_t mountable_before_fschk;
    uint16_t signature;
    uint16_t state;
    uint16_t error_handling;
    uint16_t minor_ver;
    uint32_t last_fschk;
    uint32_t time_between_fschk;
    uint32_t os;
    uint32_t major_ver;
    uint16_t userid_access_rsvd_blk;
    
} Superblock;