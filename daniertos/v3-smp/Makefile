# danieRTOS v3.x - Makefile (SMP + User Mode Edition)
# SPDX-License-Identifier: MIT

# -----------------------------------------------------------------------------
# Toolchain
# -----------------------------------------------------------------------------
CROSS_COMPILE ?= riscv64-unknown-elf-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE    = $(CROSS_COMPILE)size
GDB     = $(CROSS_COMPILE)gdb

# -----------------------------------------------------------------------------
# Target
# -----------------------------------------------------------------------------
TARGET = daniertos

# -----------------------------------------------------------------------------
# Directories
# -----------------------------------------------------------------------------
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# -----------------------------------------------------------------------------
# Demo Selection (default: smp for v3)
# Usage: make DEMO=smp | make DEMO=basic | make DEMO=ssd_controller
# -----------------------------------------------------------------------------
DEMO ?= smp

# -----------------------------------------------------------------------------
# SMP Core Count (default: 2)
# Usage: make SMP=2 | make SMP=4 | make SMP=8
# -----------------------------------------------------------------------------
SMP ?= 2

# -----------------------------------------------------------------------------
# User Mode (default: enabled)
# Usage: make USER_MODE=1 | make USER_MODE=0
# -----------------------------------------------------------------------------
USER_MODE ?= 1

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
C_SRCS = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/demos/demo_$(DEMO).c \
	$(SRC_DIR)/trap.c \
	$(SRC_DIR)/hal/uart.c \
	$(SRC_DIR)/hal/timer.c \
	$(SRC_DIR)/hal/pmp.c \
	$(SRC_DIR)/kernel/task.c \
	$(SRC_DIR)/kernel/scheduler.c \
	$(SRC_DIR)/kernel/tick.c \
	$(SRC_DIR)/kernel/delay.c \
	$(SRC_DIR)/kernel/critical.c \
	$(SRC_DIR)/kernel/semaphore.c \
	$(SRC_DIR)/kernel/mutex.c \
	$(SRC_DIR)/kernel/queue.c \
	$(SRC_DIR)/kernel/smp.c

# Add user-mode sources if enabled
ifeq ($(USER_MODE),1)
C_SRCS += $(SRC_DIR)/kernel/syscall.c
C_SRCS += $(wildcard $(SRC_DIR)/user/*.c)
endif

ASM_SRCS = \
	$(SRC_DIR)/startup.S \
	$(SRC_DIR)/hal/trap.S

# Object files
C_OBJS   = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
ASM_OBJS = $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.o,$(ASM_SRCS))
OBJS     = $(ASM_OBJS) $(C_OBJS)

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------
# Architecture: RV64IMAC (future: RV64GCV)
# Note: _zicsr is implicit in older GCC (< 12), explicit in newer GCC
ARCH = -march=rv64imac -mabi=lp64

CFLAGS = $(ARCH) \
	-mcmodel=medany \
	-ffreestanding \
	-fno-common \
	-nostdlib \
	-fno-builtin \
	-fno-stack-protector \
	-Wall -Wextra -Werror \
	-O2 -g \
	-I$(INC_DIR) \
	-DCONFIG_NUM_CORES=$(SMP) \
	-DCONFIG_USER_MODE=$(USER_MODE)

ASFLAGS = $(ARCH) \
	-mcmodel=medany \
	-ffreestanding \
	-nostdlib \
	-I$(INC_DIR) \
	-DCONFIG_USER_MODE=$(USER_MODE)

LDFLAGS = $(ARCH) \
	-mcmodel=medany \
	-nostdlib \
	-static \
	-T linker.ld \
	-Wl,--gc-sections \
	-Wl,-Map=$(BUILD_DIR)/$(TARGET).map

# -----------------------------------------------------------------------------
# QEMU (SMP: configurable cores)
# -----------------------------------------------------------------------------
QEMU = qemu-system-riscv64
QEMU_FLAGS = \
	-machine virt \
	-cpu rv64 \
	-smp $(SMP) \
	-m 128M \
	-nographic \
	-bios none \
	-kernel $(BUILD_DIR)/$(TARGET).elf

# -----------------------------------------------------------------------------
# Rules
# -----------------------------------------------------------------------------
.PHONY: all clean run debug disasm check-demo

# Track DEMO and SMP changes - force rebuild if values change
DEMO_FILE = $(BUILD_DIR)/.demo
SMP_FILE = $(BUILD_DIR)/.smp
check-demo:
	@mkdir -p $(BUILD_DIR)
	@if [ ! -f $(DEMO_FILE) ] || [ "$$(cat $(DEMO_FILE))" != "$(DEMO)" ]; then \
		echo "  DEMO    $(DEMO)"; \
		rm -f $(BUILD_DIR)/demos/*.o $(BUILD_DIR)/$(TARGET).elf; \
		echo "$(DEMO)" > $(DEMO_FILE); \
	fi
	@if [ ! -f $(SMP_FILE) ] || [ "$$(cat $(SMP_FILE))" != "$(SMP)" ]; then \
		echo "  SMP     $(SMP) cores"; \
		rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/kernel/*.o $(BUILD_DIR)/$(TARGET).elf; \
		echo "$(SMP)" > $(SMP_FILE); \
	fi

all: check-demo $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin
	@$(SIZE) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@echo "  LD      $@"
	@$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	@echo "  AS      $<"
	@$(AS) $(ASFLAGS) -c -o $@ $<

clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET).elf
	$(QEMU) $(QEMU_FLAGS)

debug: $(BUILD_DIR)/$(TARGET).elf
	$(QEMU) $(QEMU_FLAGS) -S -s &
	@sleep 1
	$(GDB) $(BUILD_DIR)/$(TARGET).elf -ex "target remote :1234"

disasm: $(BUILD_DIR)/$(TARGET).elf
	$(OBJDUMP) -d $< > $(BUILD_DIR)/$(TARGET).asm
	@echo "Disassembly written to $(BUILD_DIR)/$(TARGET).asm"

