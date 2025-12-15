NASM     = nasm
QEMU     = qemu-system-x86_64

CXX      = x86_64-elf-g++
OBJCOPY  = x86_64-elf-objcopy

CXXFLAGS = -std=gnu++17 \
           -ffreestanding -fno-exceptions -fno-rtti \
           -fno-pie -no-pie \
           -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2 \
           -O2 -Wall -Wextra \
           -Isrc -Isrc/kernel

LDFLAGS  = -nostdlib -Wl,-T,linker.ld

BUILD_DIR = build
SRC_DIR = src

BOOT_DIR = $(SRC_DIR)/boot
KERNEL_DIR = $(SRC_DIR)/kernel
ARCH_DIR = $(KERNEL_DIR)/arch/x86_64
MM_DIR = $(KERNEL_DIR)/mm
LIBK_DIR = $(KERNEL_DIR)/libk
DRV_DIR = $(KERNEL_DIR)/drivers
INP_DIR = $(DRV_DIR)/input
GFX_DIR = $(KERNEL_DIR)/gfx
IPC_DIR = $(KERNEL_DIR)/ipc

# ============================================================
# Kernel Objects
# ============================================================

KERNEL_OBJS = \
	$(BUILD_DIR)/kernel_entry.o \
	$(BUILD_DIR)/tss.o \
	$(BUILD_DIR)/gdt.o \
	$(BUILD_DIR)/gdt64.o \
	$(BUILD_DIR)/stacks.o \
	$(BUILD_DIR)/isr.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/interrupt.o \
	$(BUILD_DIR)/pic.o \
	$(BUILD_DIR)/serial.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/panic.o \
	$(BUILD_DIR)/syscall_entry.o \
	$(BUILD_DIR)/syscall_init.o \
	$(BUILD_DIR)/syscall.o \
	$(BUILD_DIR)/sys_write.o \
	$(BUILD_DIR)/sys_exit.o \
	$(BUILD_DIR)/memory_manager.o \
	$(BUILD_DIR)/vmm.o \
	$(BUILD_DIR)/heap.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/kprint.o \
	$(BUILD_DIR)/kdm.o \
	$(BUILD_DIR)/pci.o \
	$(BUILD_DIR)/ps2.o \
	$(BUILD_DIR)/ps2_kb.o \
	$(BUILD_DIR)/ps2_mouse.o \
	$(BUILD_DIR)/graphics.o

# ============================================================
# Targets
# ============================================================

all: $(BUILD_DIR)/disk.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ---------------- BOOT ----------------

$(BUILD_DIR)/stage1.bin: $(BOOT_DIR)/stage1.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(BUILD_DIR)/stage2.bin: $(BOOT_DIR)/stage2.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

# ---------------- ARCH ----------------

$(BUILD_DIR)/kernel_entry.o: $(ARCH_DIR)/kernel_entry.asm | $(BUILD_DIR)
	$(NASM) -f elf64 $< -o $@

$(BUILD_DIR)/tss.o: $(ARCH_DIR)/tss.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/gdt.o: $(ARCH_DIR)/gdt.asm | $(BUILD_DIR)
	$(NASM) -f elf64 $< -o $@

$(BUILD_DIR)/isr.o: $(ARCH_DIR)/isr.asm | $(BUILD_DIR)
	$(NASM) -f elf64 $< -o $@

$(BUILD_DIR)/gdt64.o: $(ARCH_DIR)/gdt.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/stacks.o: $(ARCH_DIR)/stacks.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@	

$(BUILD_DIR)/interrupt.o: $(ARCH_DIR)/interrupt.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: $(ARCH_DIR)/idt.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/pic.o: $(ARCH_DIR)/pic.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/serial.o: $(ARCH_DIR)/serial.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/syscall_entry.o: $(ARCH_DIR)/syscall_entry.asm | $(BUILD_DIR)
	$(NASM) -f elf64 $< -o $@

$(BUILD_DIR)/syscall_init.o: $(ARCH_DIR)/syscall_init.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- IPC (SYSCALLS) ----------------

$(BUILD_DIR)/syscall.o: $(IPC_DIR)/syscall.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/sys_write.o: $(IPC_DIR)/sys_write.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/sys_exit.o: $(IPC_DIR)/sys_exit.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- KERNEL ----------------

$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/panic.o: $(KERNEL_DIR)/panic.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- MM ----------------

$(BUILD_DIR)/memory_manager.o: $(MM_DIR)/memory_manager.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/vmm.o: $(MM_DIR)/vmm.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/heap.o: $(MM_DIR)/heap.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- LIBK ----------------

$(BUILD_DIR)/memory.o: $(LIBK_DIR)/memory.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o: $(LIBK_DIR)/string.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kprint.o: $(LIBK_DIR)/kprint.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- DRIVERS ----------------

$(BUILD_DIR)/kdm.o: $(DRV_DIR)/kdm.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/pci.o: $(DRV_DIR)/pci.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2.o: $(INP_DIR)/ps2.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2_kb.o: $(INP_DIR)/ps2_kb.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ps2_mouse.o: $(INP_DIR)/ps2_mouse.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- GFX ----------------

$(BUILD_DIR)/graphics.o: $(GFX_DIR)/graphics.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------- LINK ----------------

$(BUILD_DIR)/kernel.elf: $(KERNEL_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(KERNEL_OBJS) -lgcc -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/disk.img: $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$@ bs=1M count=512 status=none
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
