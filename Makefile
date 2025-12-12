NASM = nasm
QEMU = qemu-system-x86_64

CXX = x86_64-elf-g++
CXXFLAGS = -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -O2 -Wall -Wextra
LDFLAGS = -nostdlib -Wl,-Ttext=0x00100000 -Wl,--oformat=binary -Wl,-e,kernel_main

BUILD_DIR = build
BOOT_DIR = src/boot
KERNEL_DIR = src/kernel

KERNEL_OBJS = $(BUILD_DIR)/kernel.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/serial.o $(BUILD_DIR)/pci.o $(BUILD_DIR)/memory_manager.o $(BUILD_DIR)/vmm.o $(BUILD_DIR)/kernel_entry.o

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

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(KERNEL_OBJS) -o $@

$(BUILD_DIR)/disk.img: $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$@ bs=1M count=64 status=none
	dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc bs=512 count=1 status=none
	dd if=$(BUILD_DIR)/stage2.bin of=$@ conv=notrunc bs=512 seek=1 status=none
	dd if=$(BUILD_DIR)/kernel.bin of=$@ conv=notrunc bs=512 seek=33 status=none

run: all
	$(QEMU) -drive file=$(BUILD_DIR)/disk.img,format=raw -serial stdio

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
