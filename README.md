# ComputiOS
Computer Operating System, something I work on in the free time I have. 
I don't have any idea where the name (more so the "i" in ComputiOS) came from, but it clicks.

## My goals for this OS
I want this operating system to be able to play Crab Rave on YouTube, while rendering a nice looking UI. This goal is definitely more than ambitious, but I believe with enough time I can meet that goal. It will be a 64 bit OS, targeted towards real, modern hardware. 

## What I've built so far:
- A BIOS-only bootloader I wrote myself that sets up long mode, paging, video, and loads the kernel.
- A small kernel with a tick system that runs off the PIC.
- An interrupt descriptor table with an exception handler
- A physical and virtual memory manager.
- A driver model that registers and binds devices to drivers on boot.
- A framebuffer-based software GUI renderer that can render rectangles and text in 32-bit color.

## What I'm planning to add in the future (in no particular order):
- A preemptive scheduler that differentiates processes from threads and vice versa.
- An upgraded tick system using the APIC.
- An AHCI SATA driver to read FAT32 partitions and beyond.
- My own journaling filesystem made for the OS.
- A USB HID driver for keyboards, mice, controllers and such.
- UEFI boot support (written myself, of course).
- Network Interface Device drivers.
- TCP/IP support.
- HTTP/TLS support.
- Audio/video encoding/decoding.
- And more, as I think of things to add or I get requests to add things.

# HOW TO RUN
- You will need a cross compiler+linker, more specifically GCC and LD cross compiled for x86_64-ELF
....- For more info on building a cross compiler, check out [GCC Cross-Compiler on the OSDev Wiki](https://wiki.osdev.org/GCC_Cross-Compiler)
- You will also need to install `nasm` to assemble the assembly files.
- If you want to run it, you'll also need `qemu-system-x86_64`

When everything is setup...
- First run `make clean` to make sure there's no build folder with garbage or old output files.
- Then run `make` to create the build folder and compile everything there.
- To run it in qemu, run `make run`.

# PULL REQUESTS
If there is a problem and you know the solution, please create a pull request to fix it. I can't catch every problem, and seeing what other people catch is really helpful.
