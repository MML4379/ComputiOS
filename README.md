# ComputiOS

A modern 64-bit operating system built from scratch for x86-64 hardware.

## Vision

ComputiOS aims to be a fully functional operating system capable of running on real, modern hardware with features you'd expect from contemporary OSes - including web browsing, multimedia support, and a polished graphical interface.

This is a complete rewrite of the original project, designed with cleaner architecture and UEFI support from the ground up.
(okay maybe not UEFI but the architecture will definitely be different - I won't be doing a *nix philosophy anymore)

## Current Status

🚧 **Active Development - Early Stage**

ComputiOS is currently in the planning and initial implementation phase. Check back for updates as development progresses!

### Completed
- Architecture planning and design documentation
- Repository structure
- Legacy bootloader

### In Progress
- UEFI bootloader
- Memory management subsystem

### Planned Features
- **Boot**: UEFI bootloader with modern hardware support
- **Memory**: Physical and virtual memory managers with proper page table management
- **Processes**: Preemptive multitasking scheduler with process/thread separation
- **Drivers**: Modular driver system for hardware devices
- **Storage**: Custom journaling filesystem optimized for ComputiOS
- **Input**: USB HID support for keyboards, mice, and controllers
- **Graphics**: Hardware-accelerated GUI with compositor
- **Networking**: Full TCP/IP stack with NIC drivers
- **Multimedia**: Audio and video encoding/decoding
- **Applications**: Web browser and core system utilities

## Design Principles

- **Modern First with Compatibility**: Targeting current hardware with UEFI and Legacy BIOS
- **Clean Architecture**: Well-defined subsystem boundaries and interfaces
- **Real Hardware Ready**: Built to run on physical machines, not just emulators
- **Documented**: Every major component has design documentation

## Building ComputiOS

### Prerequisites

You'll need:
- `x86_64-elf` cross-compiler (GCC and binutils)
  - See the [OSDev Wiki GCC Cross-Compiler guide](https://wiki.osdev.org/GCC_Cross-Compiler)
- `nasm` assembler
- `qemu-system-x86_64` for testing
- GNU `make`

### Build Instructions

```bash
# Clean any previous builds
make clean

# Build the OS
make

# Run in QEMU
make run
```

## Documentation

Design documents and architecture specs are located in the `/docs` directory.

## Project History

ComputiOS is a complete rewrite. The original version can be found at [ComputiOS-legacy](https://github.com/MML4379/ComputiOS-legacy), but all active development happens here.

## Contributing

This is primarily a personal learning project, but if you spot issues or have suggestions, feel free to open an issue! Pull requests are welcome for bug fixes.

## License

ComputiOS Usage License - See [LICENSE](LICENSE) for details.

## Resources

Helpful resources for OS development:
- [OSDev Wiki](https://wiki.osdev.org/)
- [UEFI Specification](https://uefi.org/specifications)
- [Intel Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

---

**Note**: This OS is a work in progress. Expect frequent breaking changes as the architecture evolves.
