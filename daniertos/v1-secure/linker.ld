/**
 * danieRTOS v1.x - Linker Script for QEMU virt machine (RV64)
 *
 * SPDX-License-Identifier: MIT
 *
 * v1.x: Memory layout for M+U mode with PMP protection
 *
 * Memory map for QEMU virt machine:
 *   0x00001000 - 0x00011FFF : Boot ROM (not used)
 *   0x02000000 - 0x0200FFFF : CLINT (M-mode only)
 *   0x0C000000 - 0x0FFFFFFF : PLIC (M-mode only)
 *   0x10000000 - 0x10000FFF : UART0 (M-mode only)
 *   0x80000000 - ...        : RAM
 *
 * RAM Layout for v1.x:
 *   0x80000000 - 0x8001FFFF : Kernel region (M-mode only) - 128KB
 *   0x80020000 - 0x8007FFFF : User region (U-mode RWX)    - 384KB
 *   0x80080000 - 0x8008FFFF : Shared read-only (U-mode R) - 64KB
 *   0x80090000 - ...        : Heap/stacks
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Memory region sizes */
KERNEL_SIZE = 128K;
USER_SIZE   = 384K;
SHARED_SIZE = 64K;

MEMORY
{
    /* Kernel code and data - M-mode only */
    KERNEL (rwx) : ORIGIN = 0x80000000, LENGTH = 128K

    /* User code and data - U-mode accessible */
    USER   (rwx) : ORIGIN = 0x80020000, LENGTH = 384K

    /* Shared read-only data - U-mode read only */
    SHARED (r)   : ORIGIN = 0x80080000, LENGTH = 64K

    /* Stacks and heap */
    STACKS (rw)  : ORIGIN = 0x80090000, LENGTH = 448K
}

SECTIONS
{
    /* ========================================================================
     * KERNEL REGION (M-mode only)
     * ========================================================================*/
    . = ORIGIN(KERNEL);
    _kernel_start = .;

    /* -- Kernel entry and code -- */
    .text.kernel : {
        _text_start = .;
        *(.text.entry)              /* Entry point first */
        build/startup.o(.text .text.*)
        build/trap.o(.text .text.*)
        build/hal/*.o(.text .text.*)
        build/kernel/*.o(.text .text.*)
        _kernel_text_end = .;
    } > KERNEL

    /* -- Kernel read-only data -- */
    .rodata.kernel : {
        . = ALIGN(8);
        _kernel_rodata_start = .;
        build/startup.o(.rodata .rodata.*)
        build/trap.o(.rodata .rodata.*)
        build/hal/*.o(.rodata .rodata.*)
        build/kernel/*.o(.rodata .rodata.*)
        *(.srodata .srodata.*)
        _kernel_rodata_end = .;
    } > KERNEL

    /* -- Kernel initialized data -- */
    .data.kernel : {
        . = ALIGN(8);
        _kernel_data_start = .;
        build/startup.o(.data .data.* .sdata .sdata.*)
        build/trap.o(.data .data.* .sdata .sdata.*)
        build/hal/*.o(.data .data.* .sdata .sdata.*)
        build/kernel/*.o(.data .data.* .sdata .sdata.*)
        _kernel_data_end = .;
    } > KERNEL

    /* -- Kernel BSS -- */
    .bss.kernel : {
        . = ALIGN(8);
        _kernel_bss_start = .;
        build/startup.o(.bss .bss.* .sbss .sbss.*)
        build/trap.o(.bss .bss.* .sbss .sbss.*)
        build/hal/*.o(.bss .bss.* .sbss .sbss.*)
        build/kernel/*.o(.bss .bss.* .sbss .sbss.*)
        _kernel_bss_end = .;
    } > KERNEL

    _kernel_end = .;

    /* ========================================================================
     * USER REGION (U-mode accessible)
     * ========================================================================*/
    . = ORIGIN(USER);
    _user_start = .;

    /* -- User code (demos, user tasks, user syscall wrappers) -- */
    .text.user : {
        _user_text_start = .;
        build/user/*.o(.text .text.*)   /* User-mode syscall wrappers */
        build/demos/*.o(.text .text.*)  /* Demo code */
        build/main.o(.text .text.*)
        *(.text .text.*)                /* Catch-all for remaining */
        _user_text_end = .;
        _text_end = .;
    } > USER

    /* -- User read-only data -- */
    .rodata.user : {
        . = ALIGN(8);
        _user_rodata_start = .;
        build/user/*.o(.rodata .rodata.*)
        build/demos/*.o(.rodata .rodata.*)
        build/main.o(.rodata .rodata.*)
        *(.rodata .rodata.*)
        _user_rodata_end = .;
        _rodata_end = .;
    } > USER

    /* -- User initialized data (including .sdata for small globals) -- */
    .data.user : {
        . = ALIGN(8);
        _user_data_start = .;
        build/user/*.o(.data .data.* .sdata .sdata.*)
        build/demos/*.o(.data .data.* .sdata .sdata.*)
        build/main.o(.data .data.* .sdata .sdata.*)
        *(.data .data.* .sdata .sdata.*)
        _user_data_end = .;
        _data_end = .;
    } > USER

    /* -- User BSS (including .sbss for small uninitialized globals) -- */
    .bss.user : {
        . = ALIGN(8);
        _user_bss_start = .;
        build/user/*.o(.bss .bss.* .sbss .sbss.*)
        build/demos/*.o(.bss .bss.* .sbss .sbss.*)
        build/main.o(.bss .bss.* .sbss .sbss.*)
        *(.bss .bss.* .sbss .sbss.*)
        *(COMMON)
        _user_bss_end = .;
        _bss_end = .;
    } > USER

    _user_end = .;

    /* ========================================================================
     * SHARED REGION (U-mode read-only)
     * ========================================================================*/
    . = ORIGIN(SHARED);
    _shared_start = .;

    .shared : {
        . = ALIGN(8);
        /* Shared constants, string tables, etc. */
        *(.shared .shared.*)
    } > SHARED

    _shared_end = .;

    /* ========================================================================
     * STACKS REGION
     * ========================================================================*/
    . = ORIGIN(STACKS);

    . = ALIGN(16);
    _heap_start = .;

    /* -- Bootstrap stack (for startup before scheduler) -- */
    . = ORIGIN(STACKS) + LENGTH(STACKS) - 8K;
    _stack_bottom = .;
    . = ORIGIN(STACKS) + LENGTH(STACKS);
    _stack_top = .;

    /* ========================================================================
     * Symbols for startup code and PMP configuration
     * ========================================================================*/
    _memory_start = ORIGIN(KERNEL);
    _memory_end = ORIGIN(STACKS) + LENGTH(STACKS);

    /* PMP region boundaries (for pmp.c to use) */
    _pmp_kernel_start = ORIGIN(KERNEL);
    _pmp_kernel_end   = ORIGIN(KERNEL) + LENGTH(KERNEL);
    _pmp_user_start   = ORIGIN(USER);
    _pmp_user_end     = ORIGIN(USER) + LENGTH(USER);
    _pmp_shared_start = ORIGIN(SHARED);
    _pmp_shared_end   = ORIGIN(SHARED) + LENGTH(SHARED);
    _pmp_stacks_start = ORIGIN(STACKS);
    _pmp_stacks_end   = ORIGIN(STACKS) + LENGTH(STACKS);

    /* BSS symbols for startup (clear all BSS) */
    _bss_start = _kernel_bss_start;
}

