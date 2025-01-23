# ComputiOS File System (CPFS)
A new, efficient, and micro-sized journaling file system for ComputiOS, based on Ext2.

## Superblock Fields
| Starting Byte | Ending Byte | Size | Description      |
| ------------- | ----------- | ---- | ---------------- |
| 0             | 3           | 4    | Number of inodes 
| 4             | 7           | 4    | Number of blocks 
| 8             | 11          | 4    | Number of blocks reserved for superuser (see offset 80)
| 12            | 15          | 4    | Number of unallocated blocks
| 16            | 19          | 4    | Number of unallocated inodes
| 20            | 23          | 4    | Block number of the Superblock
| 24            | 27          | 4    | `log<sub>2</sub>(block_size)-10`
| 28            | 31          | 4    | `log<sub>2</sub>(fragment_size)-10`
| 32            | 35          | 4    | Number of blocks in each group
| 36            | 39          | 4    | Number of fragments in each group
| 40            | 43          | 4    | Number of inodes in each group
| 44            | 47          | 4    | Last mount time (using CPOS_EPOCH)
| 48            | 51          | 4    | Last write time (using CPOS_EPOCH)
| 52            | 53          | 2    | Times volume was mounted since last FSCHK
| 54            | 55          | 2    | Times volume can be mounted before FSCHK is required
| 56            | 57          | 2    | CPFS Signature (0x43504653)
| 58            | 59          | 2    | File system state
| 60            | 61          | 2    |