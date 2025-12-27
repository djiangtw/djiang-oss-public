/**
 * danieRTOS v2.x - Linker Script for QEMU virt machine (RV64 SMP)
 *
 * SPDX-License-Identifier: MIT
 *
 * Memory map for QEMU virt machine:
 *   0x00001000 - 0x00011FFF : Boot ROM (not used)
 *   0x02000000 - 0x0200FFFF : CLINT
 *   0x0C000000 - 0x0FFFFFFF : PLIC
 *   0x10000000 - 0x10000FFF : UART0
 *   0x80000000 - ...        : RAM (we load here)
 *
 * SMP Stack layout (from _stack_top, growing down):
 *   Core 0: _stack_top - 0 * 64KB to _stack_top - 1 * 64KB
 *   Core 1: _stack_top - 1 * 64KB to _stack_top - 2 * 64KB
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS
{
    . = ORIGIN(RAM);

    /* -- Code -- */
    .text : {
        _text_start = .;
        *(.text.entry)      /* Entry point first */
        *(.text .text.*)
        _text_end = .;
    } > RAM

    /* -- Read-only data -- */
    .rodata : {
        . = ALIGN(8);
        _rodata_start = .;
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
        _rodata_end = .;
    } > RAM

    /* -- Initialized data -- */
    .data : {
        . = ALIGN(8);
        _data_start = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        _data_end = .;
    } > RAM

    /* -- Uninitialized data (BSS) -- */
    .bss : {
        . = ALIGN(8);
        _bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        _bss_end = .;
    } > RAM

    . = ALIGN(16);
    _heap_start = .;

    /* -- Stack (at end of RAM, 64KB per core x 2 cores = 128KB total) -- */
    . = ORIGIN(RAM) + LENGTH(RAM) - 128K;
    _stack_bottom = .;
    . = ORIGIN(RAM) + LENGTH(RAM);
    _stack_top = .;

    /* Symbols for startup code */
    _memory_start = ORIGIN(RAM);
    _memory_end = ORIGIN(RAM) + LENGTH(RAM);
}

