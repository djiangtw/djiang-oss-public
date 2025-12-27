# danieRTOS v2.x - M-mode SMP RTOS

**danieRTOS** (Danny + RTOS) - A minimal RTOS for RISC-V 64-bit

## Version: v2.x (M-mode, Multi-Core SMP)

This is the source code for danieRTOS v2.x, an educational RTOS demonstrating
symmetric multi-processing (SMP) with M-mode kernel running across multiple cores.

| Feature | v0.x | v1.x | v2.x |
|---------|------|------|------|
| Privilege Mode | M-mode only | M + U mode | M-mode only |
| Memory Protection | None | PMP | None |
| Cores | Single | Single | **SMP (2-8 cores)** |
| Synchronization | Semaphore, Mutex | Same | **+ Spinlock, IPI** |
| Features | Task, Scheduler, Queue | + Syscalls, PMP | + Multi-core Scheduler |
| Status | ✅ Complete | ✅ Complete | ✅ Complete |

## Key Features

- **Multi-core SMP**: Support 2-8 cores (configurable)
- **Spinlock**: Hardware-supported atomic locks for SMP synchronization
- **IPI (Inter-Processor Interrupt)**: Cross-core communication via CLINT MSIP
- **Per-core Idle Task**: Each core has its own idle task
- **Core Affinity**: Tasks can be bound to specific cores or migrate freely
- **Multi-core Scheduler**: Load balancing across cores

## Configuration

Key configuration options in `include/config.h`:

| Option | Default | Description |
|--------|---------|-------------|
| `CONFIG_NUM_CORES` | 2 | Number of CPU cores (2-8) |
| `CONFIG_MAX_TASKS` | 16 | Maximum tasks including idle tasks |
| `CONFIG_MAX_PRIORITY` | 4 | Priority levels (0=lowest, 3=highest) |
| `CONFIG_TICK_RATE_HZ` | 1000 | System tick frequency (1 tick = 1ms) |
| `CONFIG_MINIMAL_STACK_SIZE` | 1024 | Stack size per task (bytes) |
| `CONFIG_CORE_STACK_SIZE` | 64KB | Stack size per core |

## Directory Structure

```
v2-msmp/
├── README.md              # This file
├── Makefile               # Build script
├── linker.ld              # Linker script
├── include/
│   ├── config.h           # Configuration options
│   ├── daniertos.h        # Main header
│   ├── smp.h              # SMP support (per-core data)
│   ├── spinlock.h         # Spinlock API
│   ├── task.h             # Task API
│   ├── scheduler.h        # Scheduler API
│   └── ...                # Semaphore, Mutex, Queue
├── src/
│   ├── main.c             # Entry point
│   ├── startup.S          # Multi-core boot
│   ├── trap.c             # Trap handler
│   ├── hal/               # Hardware abstraction
│   ├── kernel/            # Kernel (task, scheduler, sync)
│   └── demos/             # Demo applications
└── build/                 # Build output
```

## Building

```bash
# Build with default 2 cores
make clean && make

# Build with 4 cores
make clean && make SMP=4

# Run on QEMU
make run

# Debug with GDB
make debug

# Generate disassembly
make disasm
```

## Requirements

- RISC-V toolchain: `riscv64-unknown-elf-gcc`
- QEMU: `qemu-system-riscv64`
- GDB: `riscv64-unknown-elf-gdb`

## License

MIT License
