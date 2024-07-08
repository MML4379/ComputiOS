# tools
AS=nasm
CC=x86_64-elf-gcc
LD=x86_64-elf-ld

# locations
BOOTSRC=src/boot
KRNLSRC=src/kernel
MNT=/mnt/e

# files
BOOTBIN=build/boot.bin
BOOTASM=src/boot/boot.asm
BOOTIMG=$(MNT)/boot.img

# flags
ASFLAGS=-f bin -o

all: $(BOOTIMG)

$(BOOTBIN): $(BOOTASM)
	$(AS) $^ $(ASFLAGS) $@

$(BOOTIMG): $(BOOTBIN)
	dd if=$< of=$@ bs=512 count=1

clean:
	rm -f $(BOOT_BIN) $(BOOT_IMG)