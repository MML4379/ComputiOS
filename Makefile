NASM = nasm
QEMU = qemu-system-x86_64

CXX = x86_64-elf-g++
OBJCOPY = x86_64-elf-objcopy

CXXFLAGS = -std=gnu++17 \
           -ffreestanding -fno-exceptions -fno-rtti \
           -fno-pie -no-pie \
           -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2 \
           -O2 -Wall -Wextra \
           -Isrc/kernel -Isrc/libk -Isrc/drivers

LDFLAGS = -nostdlib -Wl,-T,linker.ld

BUILD_DIR  = build
BOOT_DIR   = src/boot
KERNEL_DIR = src/kernel
LIBK_DIR   = src/kernel/libk
DRIVER_DIR = src/drivers

KERNEL_OBJS = \
	$(BUILD_DIR)/kernel_entry.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/isr.o \
	$(BUILD_DIR)/pic.o \
	$(BUILD_DIR)/serial.o \
	$(BUILD_DIR)/pci.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/kprint.o \
	$(BUILD_DIR)/memory_manager.o \
	$(BUILD_DIR)/vmm.o \
	$(BUILD_DIR)/heap.o \
	$(BUILD_DIR)/vga.o \
	$(BUILD_DIR)/kdm.o \
	$(BUILD_DIR)/ps2.o \
	$(BUILD_DIR)/ps2_kb.o \
	$(BUILD_DIR)/ps2_mouse.o \
	$(BUILD_DIR)/graphics.o

all: $(BUILD_DIR)/disk.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/stage1.bin: $(BOOT_DIR)/stage1.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(BUILD_DIR)/stage2.bin: $(BOOT_DIR)/stage2.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(BUILD_DIR)/kernel_entry.o: $(KERNEL_DIR)/kernel_entry.asm | $(BUILD_DIR)
	$(NASM) -f elf64 $< -o $@

$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: $(KERNEL_DIR)/idt.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/isr.o: $(KERNEL_DIR)/isr.asm | $(BUILD_DIR)
	$(NASM) -f elf64 $< -o $@

$(BUILD_DIR)/pic.o: $(KERNEL_DIR)/pic.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/serial.o: $(KERNEL_DIR)/serial.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/pci.o: $(KERNEL_DIR)/pci.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: $(LIBK_DIR)/memory.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o: $(LIBK_DIR)/string.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kprint.o: $(LIBK_DIR)/kprint.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/memory_manager.o: $(KERNEL_DIR)/memory_manager.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/vmm.o: $(KERNEL_DIR)/vmm.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/heap.o: $(KERNEL_DIR)/heap.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/vga.o: $(KERNEL_DIR)/vga.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kdm.o: $(KERNEL_DIR)/kdm.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2.o: $(DRIVER_DIR)/ps2/ps2.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2_kb.o: $(DRIVER_DIR)/ps2/kb/ps2_kb.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2_mouse.o: $(DRIVER_DIR)/ps2/mouse/ps2_mouse.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/graphics.o: $(KERNEL_DIR)/graphics.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.elf: $(KERNEL_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(KERNEL_OBJS) -lgcc -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/disk.img: $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$@ bs=1M count=64 status=none
	dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc bs=512 count=1 status=none
	dd if=$(BUILD_DIR)/stage2.bin of=$@ conv=notrunc bs=512 seek=1 status=none
	dd if=$(BUILD_DIR)/kernel.bin of=$@ conv=notrunc bs=512 seek=33 status=none

run: all
	$(QEMU) \
		-drive file=$(BUILD_DIR)/disk.img,format=raw,if=none,id=vd0 \
		-device virtio-blk-pci,drive=vd0 \
		-netdev user,id=net0 \
		-device virtio-net-pci,netdev=net0 \
		-serial stdio \
		-vga std \
		-m 2G

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
