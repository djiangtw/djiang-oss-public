# danieRTOS v3.x - SMP + User Mode RTOS

A complete RTOS for RISC-V combining SMP multi-core (v2.x) with User Mode protection (v1.x).

## Version History

| Version | Description | Status |
|---------|-------------|--------|
| **v3p0** | Base integration: M-mode kernel tasks running correctly on multi-core | ✅ Complete |
| **v3p1** | Dual-core SMP: Two cores running M-mode + U-mode mixed tasks | ✅ Complete |
| **v3p2** | Full User Mode: syscalls, PMP, Fault Isolation | ✅ Complete |

## Features

| Feature | v3.x |
|---------|------|
| Privilege Mode | M + U mode |
| Cores | SMP (2-8 cores) |
| Memory Protection | PMP (NAPOT) |
| System Call | ✅ |
| Fault Isolation | ✅ |
| Status | ✅ v3p2 Complete |

**Key Features:**
- **Multi-core support**: 2-8 cores (configurable)
- **User Mode (U-mode)**: Tasks run in unprivileged mode
- **Memory Protection (PMP)**: Isolate kernel from user tasks
- **System Calls**: User tasks access kernel via ecall
- **Per-Task Kernel Stack**: Support kernel-mode blocking
- **Fault isolation**: Resource ownership tracking

## Key Design Decisions

### tp/mscratch Swap
- **User Mode**: `tp = User TLS`, `mscratch = &cpu_t`
- **Kernel Mode**: `tp = &cpu_t`, `mscratch = saved User tp`
- Trap entry swaps tp/mscratch to get per-core data immediately

### Hybrid PMP

- Static entries: Kernel code, User code (set at boot)
- Dynamic entry: Current task's stack (updated on context switch)

📖 **PMP Design and Porting Guide**: [docs/PMP_DESIGN.md](docs/PMP_DESIGN.md)

## Quick Start

```bash
# Build with default 2 cores
make clean && make

# Build with 4 cores
make clean && make SMP=4

# Disable User Mode (M-mode only, like v2)
make clean && make USER_MODE=0

# Run with QEMU
make run
```

## Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `SMP` | 2 | Number of CPU cores (2-8) |
| `USER_MODE` | 1 | Enable User Mode (0 = M-mode only) |

## Directory Structure

```
code.v3/
├── include/          # Header files
│   ├── config.h      # Configuration
│   ├── smp.h         # SMP support (with trap scratch area)
│   ├── pmp.h         # Memory protection
│   ├── syscall.h     # System call definitions
│   └── ...
├── src/
│   ├── hal/          # Hardware abstraction
│   │   ├── trap.S    # Trap handler (tp/mscratch swap)
│   │   └── pmp.c     # PMP configuration
│   ├── kernel/       # Kernel implementation
│   ├── user/         # User-mode syscall wrappers
│   └── demos/        # Demo applications
└── Makefile
```

## Requirements

- RISC-V toolchain: `riscv64-unknown-elf-gcc`
- QEMU: `qemu-system-riscv64`
- GDB: `riscv64-unknown-elf-gdb`

## License

MIT License

