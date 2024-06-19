# bootloader/Makefile

NASM = nasm
GCC = gcc
LD = ld

BOOT_OBJS = boot.o loader.o

all: bootloader.bin

bootloader.bin: $(BOOT_OBJS)
	$(LD) -T linker.ld -o bootloader.bin $(BOOT_OBJS)

boot.o: boot.asm
	$(NASM) -f elf32 boot.asm -o boot.o

loader.o: loader.c
	$(GCC) -m32 -c loader.c -o loader.o

clean:
	rm -f *.o bootloader.bin