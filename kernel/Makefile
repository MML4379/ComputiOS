# kernel/Makefile

GCC = gcc

KERNEL_OBJS = kernel.o

all: kernel.bin

kernel.bin: $(KERNEL_OBJS)
	$(LD) -T linker.ld -o kernel.bin $(KERNEL_OBJS)

kernel.o: kernel.c
	$(GCC) -m32 -c kernel.c -o kernel.o

clean:
	rm -f *.o kernel.bin