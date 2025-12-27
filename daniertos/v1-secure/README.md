# danieRTOS v1.x - Source Code

**danieRTOS** (Danny + RTOS) - A minimal RTOS for RISC-V 64-bit

## Version: v1.x (M+U mode, Single Core)

This is the source code for danieRTOS v1.x, an educational RTOS project demonstrating
privilege separation with M-mode kernel and U-mode user tasks.

| Feature | v0.x | v1.x |
|---------|------|------|
| Privilege Mode | M-mode only | M-mode kernel + U-mode tasks |
| Memory Protection | None | PMP (Physical Memory Protection) |
| System Calls | N/A | ecall-based syscall interface |
| Cores | Single | Single |
| Features | Task, Scheduler, Semaphore, Mutex, Queue | Same + Syscalls, PMP |
| Status | ✅ Complete | ✅ Complete |

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                  APPLICATION LAYER (U-mode)                     │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────────┐    │
│  │ demo_basic.c │  │demo_ble_*.c  │  │demo_producer_*.c   │    │
│  └──────────────┘  └──────────────┘  └────────────────────┘    │
│                              │                                  │
│                              ▼                                  │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │            user_syscall.c  (syscall wrappers)             │ │
│  │   sys_delay(), sys_printf(), sys_log(), sys_exit() ...    │ │
│  └───────────────────────────────────────────────────────────┘ │
│                              │                                  │
│                          ecall                                  │
├──────────────────────────────┼──────────────────────────────────┤
│                              ▼                                  │
│                     KERNEL LAYER (M-mode)                       │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │         syscall.c  (syscall handler/dispatcher)           │ │
│  └───────────────────────────────────────────────────────────┘ │
│                              │                                  │
│       ┌──────────────────────┼──────────────────────┐          │
│       ▼                      ▼                      ▼          │
│  ┌─────────┐  ┌───────────────────┐  ┌───────────────────────┐ │
│  │ task.c  │  │   scheduler.c     │  │ semaphore/mutex/queue │ │
│  └─────────┘  └───────────────────┘  └───────────────────────┘ │
│                              │                                  │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │                  HAL (Hardware Abstraction)                │ │
│  │           trap.S, uart.c, timer.c, pmp.c                   │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

## Configuration

Key configuration options in `include/config.h`:

| Option | Default | Description |
|--------|---------|-------------|
| `CONFIG_MAX_TASKS` | 8 | Maximum tasks including idle task (7 user tasks) |
| `CONFIG_MAX_PRIORITY` | 4 | Priority levels (0=lowest, 3=highest) |
| `CONFIG_TICK_RATE_HZ` | 1000 | System tick frequency (1 tick = 1ms) |
| `CONFIG_MINIMAL_STACK_SIZE` | 1024 | User stack size per task (bytes) |
| `CONFIG_KERNEL_STACK_SIZE` | 512 | Kernel stack size per U-mode task (bytes) |
| `CONFIG_USER_MODE` | 1 | Enable M+U mode (0 = M-mode only like v0) |

### Memory Usage Per Task

- **User stack**: 1024 bytes (configurable)
- **Kernel stack**: 512 bytes (for U-mode trap handling)
- **TCB**: ~100 bytes

### Memory Layout

```
0x80000000 - 0x8001FFFF : KERNEL (128KB) - M-mode only
0x80020000 - 0x8007FFFF : USER   (384KB) - U-mode RWX
0x80080000 - 0x8008FFFF : SHARED (64KB)  - U-mode read-only
0x80090000 - 0x800FFFFF : STACKS (448KB) - Task stacks
```

## Directory Structure

```
code.v1/
├── README.md              # This file
├── Makefile               # Build script
├── linker.ld              # Linker script with memory regions
├── include/
│   ├── config.h           # Configuration options
│   ├── daniertos.h        # Main header
│   ├── syscall.h          # Syscall numbers
│   ├── user_syscall.h     # User-mode syscall API
│   ├── pmp.h              # PMP configuration
│   └── ...                # Other headers
├── src/
│   ├── main.c             # Entry point
│   ├── startup.S          # Boot code
│   ├── trap.c             # Trap handler (C)
│   ├── kernel/
│   │   ├── task.c         # Task management
│   │   ├── scheduler.c    # Scheduler
│   │   ├── syscall.c      # Syscall handler
│   │   └── ...            # Semaphore, Mutex, Queue
│   ├── hal/
│   │   ├── trap.S         # Trap handler (ASM)
│   │   ├── pmp.c          # PMP setup
│   │   ├── uart.c         # UART driver
│   │   └── timer.c        # Timer driver
│   ├── user/
│   │   └── user_syscall.c # User-mode syscall wrappers
│   └── demos/
│       ├── demo_basic.c   # Basic 3-task demo
│       └── ...            # Other demos
└── build/                 # Build output
```

## User-mode Syscall API

User tasks must use syscalls to interact with the kernel:

```c
#include "user_syscall.h"

void my_task(void *arg) {
    sys_log("Task started\n");        // Print with timestamp
    sys_delay_ms(500);                // Delay 500ms
    sys_printf("Hello %d\n", 42);     // Print without timestamp
    sys_yield();                      // Yield CPU
    sys_exit();                       // Exit task
}
```

| Syscall | Description |
|---------|-------------|
| `sys_delay(ticks)` | Delay for N ticks |
| `sys_delay_ms(ms)` | Delay for N milliseconds |
| `sys_yield()` | Yield CPU to other tasks |
| `sys_exit()` | Exit current task |
| `sys_get_tick()` | Get current tick count |
| `sys_putchar(c)` | Output a character |
| `sys_puts(s)` | Output a string |
| `sys_printf(fmt, ...)` | Formatted output |
| `sys_log(fmt, ...)` | Formatted output with timestamp |
| `sys_get_task_id()` | Get current task ID |

## Building

```bash
# Build and run (default: basic demo)
make run

# Build and run specific demo
make DEMO=basic run
make DEMO=ble_sensor run
make DEMO=producer_consumer run

# Clean build
make clean && make DEMO=ble_sensor run

# Build only (without running)
make DEMO=ble_sensor

# Debug with GDB
make DEMO=basic debug

# Generate disassembly
make disasm
```

**Important**: Always pass `DEMO=xxx` in the same make command. The DEMO selection
is tracked, and running `make run` separately will use the default (basic) demo.

## Requirements

- RISC-V toolchain: `riscv64-unknown-elf-gcc`
- QEMU: `qemu-system-riscv64`
- GDB: `riscv64-unknown-elf-gdb` (optional, for debugging)

## Example Output

```
========================================
  danieRTOS v0.1 - RISC-V 64-bit
========================================

Basic Demo - 3 tasks (U-mode)

[0000.000] Task C: 1/3
[0000.000] Task B: 1/4
[0000.000] Task A: 1/5
[0000.500] Task A: 2/5
[0000.700] Task B: 2/4
[0001.000] Task C: 2/3
...

========================================
  All tasks completed - Demo End!
========================================
```

## License

MIT License
