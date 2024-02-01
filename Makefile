SRC_DIR = src
BOOT_DIR = $(SRC_DIR)/boot
CORE_DIR = $(SRC_DIR)/core
INCLUDE_DIR = $(CORE_DIR)/kernel/include

KERNEL_ENTRY_POINT = 0x1000
KERNEL_OUTPUT = bin/kernel.bin

CXX = g++
CXXFLAGS = -m32 -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -Wall -Wextra -Werror -I$(INCLUDE_DIR)

KERNEL_SRCS = $(CORE_DIR)/kernel/kernel_main.cpp \
              $(CORE_DIR)/kernel/include/screen.h \
              $(CORE_DIR)/kernel/include/memory.h

KERNEL_OBJS = $(KERNEL_SRCS:%.cpp=%.o)

.PHONY: all clean

all: $(KERNEL_OUTPUT)

$(KERNEL_OUTPUT): $(KERNEL_OBJS) $(BOOT_DIR)/kernel_entry.o
	$(CXX) -T $(CORE_DIR)/link.ld -o $@ $^

$(BOOT_DIR)/kernel_entry.o: $(BOOT_DIR)/kernel_entry.asm
	nasm $< -f elf32 -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(KERNEL_OBJS) $(KERNEL_OUTPUT)