# ComputiOS
Computer Operating System, something I work on in the free time I have. 

## My goals for this OS
I want this operating system to be able to play Crab Rave on YouTube, whilst rendering a nice looking UI. Quite the ambitious goal but I will get there some day. It will be a 64 bit OS, targeted towards real, modern hardware.

## Checklist
BOOTLOADER:
- [x] Basic Functionality
- [x] Protected Mode
- [x] A20 Line
- [x] FAT32 Read functionality
- [x] PAE
- [x] Paging
- [x] Long Mode
- [x] Passing control to the kernel

DRIVERS:
- Disk Driver
    - [x] FAT32 Read/Write
    - [ ] FAT32 Formatting
    - [ ] Custom Journaling File System
    - [ ] Other basic FS implementations
- Video Driver
    - [ ] Implement VGA register utilization
    - [ ] Implement basic GPU interaction, to make everything a bit more **colorful**
