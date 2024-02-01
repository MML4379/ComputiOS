# Roadmap

This is everything I plan for ComputiOS, this may change over time.

## Milestone 0 - Minimum Viable Product

Getting a basic project up and running

- [x] Basic bootloader
- [ ] Basic monolithic kernel
- [ ] Basic system library with asynchonous I/O support
- [ ] Basic IPC library
- [ ] Basic graphics library with subpixel font rendering
- [ ] Basic user interface library
- [ ] Interactive graphical user interface shell

## Milestone 1 - Networking

Adding network support to ComputiOS

- [ ] Simple virtio network driver
- [ ] Simple TCP/IP stack
- [ ] Simple TLS implementation
- [ ] Simple HTTP server & client
- [ ] Simple DNS client
- [ ] Ethernet (RJ-45) networking support

## Milestone 2 - Storage

Adding storage

- [ ] Simple virtio block driver
- [ ] Simple FAT32 filesystem support (read-only)
- [ ] New Journaling File System (Read/Write/Modify)

## Milestone 3 - User Experience

Continue to make a user experience, now with networking and storage down

- [ ] Apps should be able to be started from the shell
- [ ] Terminal
- [ ] Text editor
- [ ] Image viewer
- [ ] Calculator
- [ ] File manager
- [ ] Settings panel
- [ ] Clock
- [ ] Web Browser

## Backlog

Planned for when everything above is tackled.

- [ ] Extending the drivers to work better on specific devices
- [ ] System Monitor
- [ ] Audio support
- [ ] USB support
- [ ] WiFi and Bluetooth support
- [ ] Making an alternative to Wine 
- [ ] Adding update system using new network support
