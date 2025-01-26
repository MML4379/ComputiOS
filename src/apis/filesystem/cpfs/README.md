# ComputiOS File System (CPFS)
An efficient journaling file system with superblock, based on Ext2.

## Superblock Fields
| Starting Byte | Ending Byte | Size | Description      |
| ------------- | ----------- | ---- | ---------------- |
| 0             | 3           | 4    | Number of inodes 
| 4             | 7           | 4    | Number of blocks 
| 8             | 11          | 4    | Number of blocks reserved for superuser (see offset 80)
| 12            | 15          | 4    | Number of unallocated blocks
| 16            | 19          | 4    | Number of unallocated inodes
| 20            | 23          | 4    | Block number of the Superblock
| 24            | 27          | 4    | log<sub>2</sub>(block_size)-10
| 28            | 31          | 4    | log<sub>2</sub>(fragment_size)-10
| 32            | 35          | 4    | Number of blocks in each group
| 36            | 39          | 4    | Number of fragments in each group
| 40            | 43          | 4    | Number of inodes in each group
| 44            | 47          | 4    | Last mount time (using CPOS_EPOCH)
| 48            | 51          | 4    | Last write time (using CPOS_EPOCH)
| 52            | 53          | 2    | Times volume was mounted since last FSCHK
| 54            | 55          | 2    | Times volume can be mounted before FSCHK is required
| 56            | 57          | 2    | CPFS Signature (0x43504653)
| 58            | 59          | 2    | File system state
| 60            | 61          | 2    | How to handle errors
| 62            | 63          | 2    | Minor Segment of Version (Combines with Major Version and Patch ID to construct full version)
| 64            | 67          | 4    | Last time FSCHK was performed (CPOS_EPOCH)
| 68            | 71          | 4    | Time intervals between required FSCHK cycles
| 72            | 75          | 4    | Operating System ID from which the CPFS volume was created from
| 76            | 79          | 4    | Major Segment of Version (Combines with Minor Version and Patch ID to construct full version)
| 80            | 81          | 2    | User ID that can use reserved blocks
| 82            | 83          | 2    | Group ID that can use reserved blocks
| 84            | 85          | 2    | Patch ID segment of Version (The patch ID will *always* be one character, so suffix it with one byte of 0s)
| 86            | 89          | 4    | Required Features Flags to mount disk for reading and writing
| 90            | 93          | 4    | Optional Features Flags
| 94            | 109         | 16   | File System ID
| 110           | 127         | 18   | Reserved
| 128           | 255         | 128  | Volume Name (128 characters)
| 256           | 256         | 1    | System Disk? (Exclusive to ComputiOS)
| 257           | 257         | 1    | Paging Disk? (Exclusive to ComputiOS)

## File System States
| Value | Details
| ------| -------
| 0     | Ready
| 1     | Error
| 2     | Read only (Set by hardware)
| 3     | Read only (Set by software)

## Error Handling Method
