# danieRTOS v0.x - Source Code

**danieRTOS** (Daniel + RTOS) - A minimal RTOS for RISC-V

## Version: v0.x (M-mode, Single Core)

This is the source code for danieRTOS v0.x, an educational RTOS project.

| Feature | v0.x |
|---------|------|
| Privilege Mode | M-mode only |
| Cores | Single core |
| Features | Task, Scheduler, Semaphore, Mutex, Queue |
| Status | ✅ Complete |

## Directory Structure

```
code.v0/
├── README.md          # This file
├── Makefile           # Build script (TBD)
├── src/               # Source files
│   ├── main.c         # Entry point
│   ├── task.c         # Task management
│   ├── scheduler.c    # Scheduler
│   ├── port.c         # RISC-V port (C)
│   └── portasm.S      # RISC-V port (Assembly)
├── include/           # Header files
│   ├── daniertos.h    # Main header
│   ├── task.h         # Task API
│   ├── scheduler.h    # Scheduler API
│   └── port.h         # Port definitions
└── demo/              # Demo applications (TBD)
```

## Building

```bash
# Build
make

# Clean
make clean

# Run on QEMU
make run

# Debug with GDB
make debug
```

## Requirements

- RISC-V toolchain: `riscv64-unknown-elf-gcc`
- QEMU: `qemu-system-riscv32` or `qemu-system-riscv64`
- GDB: `riscv64-unknown-elf-gdb`

## License

MIT License

