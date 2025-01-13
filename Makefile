# Tools
ASM = nasm

# Directories
SRC_DIR = src
BUILD_DIR = build
STAGE1_DIR = $(SRC_DIR)/boot/stage1

# Output files
BOOT_SECT = $(BUILD_DIR)/bootsect.bin

# Flags
ASMFLAGS = -f bin

# Build rules
all: $(BOOT_SECT)

$(BOOT_SECT): $(STAGE1_DIR)/bootsect.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean