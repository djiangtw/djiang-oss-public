# danieRTOS

A minimal RISC-V RTOS for educational purposes.

> **Note:** Current code has passed basic validation (build + demo runs). Stress testing and full regression testing are not yet complete.

## Overview

danieRTOS is a real-time operating system built from scratch for RISC-V 64-bit architecture. It was developed as part of a 40-article tutorial series "Building danieRTOS" that covers RTOS internals step by step.

## Versions

| Directory | Version | Codename | Description |
|-----------|---------|----------|-------------|
| `v0-nano/` | v0.x | Nano | Basic RTOS core |
| `v1-secure/` | v1.x | Secure | User mode + security |
| `v2-msmp/` | v2.x | MSMP | M-mode SMP |
| `v3-smp/` | v3.x | SMP | Full integration |

### v0.x Nano - Basic RTOS Core

- Task management with TCB (Task Control Block)
- Cooperative and preemptive scheduling
- Timer tick and delay
- Critical sections
- Semaphore, Mutex, Queue

### v1.x Secure - User Mode + Security

- RISC-V privilege modes (M-mode, U-mode)
- PMP (Physical Memory Protection)
- System calls (ecall)
- Fault handling

### v2.x MSMP - M-mode SMP

- Multi-core boot sequence
- Per-core data structures
- Spinlock with atomic operations
- IPI (Inter-Processor Interrupt)
- SMP-aware scheduler

### v3.x SMP - Full Integration

- Combines v2.x SMP + v1.x User Mode
- Per-task kernel stacks
- SMP PMP configuration
- Fault isolation across cores

## Prerequisites

- RISC-V GCC toolchain (`riscv64-unknown-elf-gcc`)
- QEMU with RISC-V support (`qemu-system-riscv64`)

## Building and Running

Each version has its own Makefile with demo selection:

```bash
make clean && make DEMO=<demo_name> [SMP=<cores>] all && make DEMO=<demo_name> [SMP=<cores>] run
```

**Exit QEMU:** Press `Ctrl+A` then `X`

### v0-nano (Basic RTOS)

```bash
cd v0-nano
make clean && make DEMO=basic all && make DEMO=basic run
make clean && make DEMO=ble_sensor all && make DEMO=ble_sensor run
make clean && make DEMO=producer_consumer all && make DEMO=producer_consumer run
```

### v1-secure (User Mode + PMP)

```bash
cd v1-secure
make clean && make DEMO=basic all && make DEMO=basic run
make clean && make DEMO=ble_sensor all && make DEMO=ble_sensor run
make clean && make DEMO=producer_consumer all && make DEMO=producer_consumer run
```

### v2-msmp (M-mode SMP)

```bash
cd v2-msmp
make clean && make DEMO=smp SMP=2 all && make DEMO=smp SMP=2 run
make clean && make DEMO=smp SMP=4 all && make DEMO=smp SMP=4 run
make clean && make DEMO=ssd_controller SMP=4 all && make DEMO=ssd_controller SMP=4 run
make clean && make DEMO=basic all && make DEMO=basic run
make clean && make DEMO=producer_consumer all && make DEMO=producer_consumer run
```

### v3-smp (Full: SMP + User Mode)

```bash
cd v3-smp
make clean && make DEMO=smp SMP=2 all && make DEMO=smp SMP=2 run
make clean && make DEMO=smp SMP=4 all && make DEMO=smp SMP=4 run
make clean && make DEMO=smp8 SMP=8 all && make DEMO=smp8 SMP=8 run
make clean && make DEMO=usermode SMP=2 all && make DEMO=usermode SMP=2 run
make clean && make DEMO=fault SMP=2 all && make DEMO=fault SMP=2 run
make clean && make DEMO=ssd_controller SMP=4 all && make DEMO=ssd_controller SMP=4 run
make clean && make DEMO=v3p2 SMP=2 all && make DEMO=v3p2 SMP=2 run
```

## Debugging

```bash
make debug   # Run QEMU with GDB server on port 1234
```

In another terminal:
```bash
riscv64-unknown-elf-gdb build/daniertos.elf
(gdb) target remote :1234
(gdb) continue
```

## Tutorial

This RTOS accompanies the "Building danieRTOS" tutorial series (40 articles in Traditional Chinese).

## License

MIT License - see [../LICENSE](../LICENSE) for details.

