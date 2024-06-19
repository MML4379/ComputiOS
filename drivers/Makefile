# drivers/Makefile

GCC = gcc

DRIVER_OBJS = gpu.o keyboard.o mouse.o disk.o usb.o cpu.o

all: drivers.bin

drivers.bin: $(DRIVER_OBJS)
	$(LD) -T linker.ld -o drivers.bin $(DRIVER_OBJS)

%.o: %.c
	$(GCC) -m32 -c $< -o $@

clean:
	rm -f *.o drivers.bin