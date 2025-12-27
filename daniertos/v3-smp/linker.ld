/**
 * danieRTOS v3.x - Linker Script for QEMU virt machine (RV64 SMP + User Mode)
 *
 * SPDX-License-Identifier: MIT
 *
 * v3.x = v2.x (SMP) + v1.x (User Mode)
 *
 * Memory map for QEMU virt machine:
 *   0x00001000 - 0x00011FFF : Boot ROM (not used)
 *   0x02000000 - 0x0200FFFF : CLINT (M-mode only)
 *   0x0C000000 - 0x0FFFFFFF : PLIC (M-mode only)
 *   0x10000000 - 0x10000FFF : UART0 (M-mode only)
 *   0x80000000 - ...        : RAM
 *
 * Note: For v3.x, we use a simple flat memory layout (like v2.x) for now.
 * PMP protection is configured at runtime based on the symbols defined here.
 * This avoids ELF loading issues with multiple LOAD segments.
 *
 * SMP Stack layout (from _stack_top, growing down):
 *   Core 0: _stack_top - 0 * 64KB to _stack_top - 1 * 64KB
 *   Core 1: _stack_top - 1 * 64KB to _stack_top - 2 * 64KB
 *   ...
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

    /* -- Read-only data (Shared region for U-mode read access) --
     *
     * Aligned to 4KB for NAPOT PMP configuration.
     * This allows U-mode to read string constants and global data.
     */
    . = ALIGN(4096);
    .rodata ALIGN(4096) : {
        _shared_start = .;
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
        . = ALIGN(4096);
        _shared_end = .;
    } > RAM

    /* -- Uninitialized data (BSS) -- */
    .bss (NOLOAD) : {
        . = ALIGN(8);
        _bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        _bss_end = .;
    } > RAM

    /* -- User Mode Section (v3p2) --
     *
     * NAPOT requires base aligned to size (power of 2).
     * We align to 128KB to support up to 128KB user section.
     * If user section grows beyond 128KB, increase alignment to 256KB.
     */
    . = ALIGN(128K);  /* 128KB align for NAPOT */
    _user_section_start = .;
    .user_text ALIGN(128K) : {
        _user_text_start = .;
        *(.user_text .user_text.*)
        _user_text_end = .;
    } > RAM
    .user_data : {
        _user_data_start = .;
        *(.user_data .user_data.*)
        _user_data_end = .;
    } > RAM
    . = ALIGN(4096);
    _user_section_end = .;

    . = ALIGN(16);
    _heap_start = .;

    /* -- Stack (at end of RAM, 64KB per core x 8 cores max = 512KB) -- */
    . = ORIGIN(RAM) + LENGTH(RAM) - 512K;
    _stack_bottom = .;
    . = ORIGIN(RAM) + LENGTH(RAM);
    _stack_top = .;

    /* ========================================================================
     * Symbols for startup code and PMP configuration
     * ========================================================================*/
    _memory_start = ORIGIN(RAM);
    _memory_end = ORIGIN(RAM) + LENGTH(RAM);

    /* ========================================================================
     * PMP region boundaries (for pmp.c to use)
     *
     * v3p2 Memory Protection Layout:
     *   - Kernel region: 0x80000000 - _user_section_start (M-mode only)
     *   - User region:   _user_section_start - _user_section_end (U-mode RWX)
     *   - Stack region:  _stack_bottom - _stack_top (U-mode RW)
     *
     * Note: U-mode access is DENIED by default for addresses outside PMP regions.
     * ========================================================================*/
    _pmp_kernel_start = ORIGIN(RAM);
    _pmp_kernel_end   = _user_section_start;
    _pmp_user_start   = _user_section_start;
    _pmp_user_end     = _user_section_end;
    _pmp_shared_start = _shared_start;
    _pmp_shared_end   = _shared_end;
    _pmp_stacks_start = _stack_bottom;
    _pmp_stacks_end   = _stack_top;
}

