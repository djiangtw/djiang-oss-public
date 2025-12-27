/**
 * danieRTOS v1.x - Trap Handler (RV64)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v1.x: Dual stack trap handling for M+U mode
 *
 * Key changes from v0:
 *   - Uses mscratch for stack switching between U-mode and M-mode
 *   - Saves user SP to kernel stack, switches to kernel stack for trap handling
 *   - Supports both U-mode tasks (via ecall) and M-mode kernel code
 *
 * mscratch usage:
 *   - When in U-mode: mscratch = kernel_sp (top of kernel stack)
 *   - When in M-mode: mscratch = 0 (indicates already in M-mode)
 *
 * Stack layout after trap from U-mode:
 *   Kernel stack:
 *     [context frame with user registers]
 *     [user_sp saved in REG_SP slot]
 */

#include "config.h"

/*
 * Context frame layout (34 slots * 8 bytes = 272 bytes):
 *   [0]  zero (placeholder, not saved)
 *   [1]  ra
 *   [2]  sp (user SP when trap from U-mode)
 *   [3]  gp
 *   ...
 *   [31] t6
 *   [32] mepc
 *   [33] mstatus
 */
#define CONTEXT_SIZE    (34 * CONFIG_REG_SIZE)

/* Offsets for saved registers */
#define REG_RA      (1  * CONFIG_REG_SIZE)
#define REG_SP      (2  * CONFIG_REG_SIZE)
#define REG_GP      (3  * CONFIG_REG_SIZE)
#define REG_TP      (4  * CONFIG_REG_SIZE)
#define REG_T0      (5  * CONFIG_REG_SIZE)
#define REG_T1      (6  * CONFIG_REG_SIZE)
#define REG_T2      (7  * CONFIG_REG_SIZE)
#define REG_S0      (8  * CONFIG_REG_SIZE)
#define REG_S1      (9  * CONFIG_REG_SIZE)
#define REG_A0      (10 * CONFIG_REG_SIZE)
#define REG_A1      (11 * CONFIG_REG_SIZE)
#define REG_A2      (12 * CONFIG_REG_SIZE)
#define REG_A3      (13 * CONFIG_REG_SIZE)
#define REG_A4      (14 * CONFIG_REG_SIZE)
#define REG_A5      (15 * CONFIG_REG_SIZE)
#define REG_A6      (16 * CONFIG_REG_SIZE)
#define REG_A7      (17 * CONFIG_REG_SIZE)
#define REG_S2      (18 * CONFIG_REG_SIZE)
#define REG_S3      (19 * CONFIG_REG_SIZE)
#define REG_S4      (20 * CONFIG_REG_SIZE)
#define REG_S5      (21 * CONFIG_REG_SIZE)
#define REG_S6      (22 * CONFIG_REG_SIZE)
#define REG_S7      (23 * CONFIG_REG_SIZE)
#define REG_S8      (24 * CONFIG_REG_SIZE)
#define REG_S9      (25 * CONFIG_REG_SIZE)
#define REG_S10     (26 * CONFIG_REG_SIZE)
#define REG_S11     (27 * CONFIG_REG_SIZE)
#define REG_T3      (28 * CONFIG_REG_SIZE)
#define REG_T4      (29 * CONFIG_REG_SIZE)
#define REG_T5      (30 * CONFIG_REG_SIZE)
#define REG_T6      (31 * CONFIG_REG_SIZE)
#define REG_MEPC    (32 * CONFIG_REG_SIZE)
#define REG_MSTATUS (33 * CONFIG_REG_SIZE)

.section .text
.global trap_entry
.type trap_entry, @function
.align 4

trap_entry:
    /* ======================================================================
     * Step 1: Stack switching using mscratch
     *
     * mscratch holds kernel_sp when in U-mode, 0 when in M-mode.
     * We swap sp <-> mscratch atomically using csrrw.
     * ======================================================================*/

#if CONFIG_USER_MODE
    /* Swap sp and mscratch */
    csrrw   sp, mscratch, sp

    /* Check if we came from M-mode (mscratch was 0) */
    beqz    sp, .from_m_mode

    /* -- Trap from U-mode: sp now = kernel_sp, mscratch = user_sp -- */
    /* Allocate context frame on kernel stack */
    addi    sp, sp, -CONTEXT_SIZE

    /* Save t0 first (we need it as scratch) */
    sd      t0, REG_T0(sp)

    /* Get user_sp from mscratch and save it */
    csrr    t0, mscratch
    sd      t0, REG_SP(sp)

    /* Clear mscratch to indicate we're now in M-mode */
    csrw    mscratch, zero

    j       .save_context

.from_m_mode:
    /* -- Trap from M-mode: restore sp from mscratch -- */
    csrr    sp, mscratch
    /* Allocate context frame on current (M-mode) stack */
    addi    sp, sp, -CONTEXT_SIZE

    /* Save t0 first */
    sd      t0, REG_T0(sp)

    /* Save original SP */
    addi    t0, sp, CONTEXT_SIZE
    sd      t0, REG_SP(sp)

.save_context:
#else
    /* v0 compatibility: no U-mode, just use current stack */
    addi    sp, sp, -CONTEXT_SIZE
    sd      t0, REG_T0(sp)
    addi    t0, sp, CONTEXT_SIZE
    sd      t0, REG_SP(sp)
#endif

    /* ======================================================================
     * Step 2: Save all registers
     * ======================================================================*/

    /* Save all general-purpose registers (t0 already saved) */
    sd      ra,  REG_RA(sp)
    sd      gp,  REG_GP(sp)
    sd      tp,  REG_TP(sp)
    /* t0 already saved above */
    sd      t1,  REG_T1(sp)
    sd      t2,  REG_T2(sp)
    sd      s0,  REG_S0(sp)
    sd      s1,  REG_S1(sp)
    sd      a0,  REG_A0(sp)
    sd      a1,  REG_A1(sp)
    sd      a2,  REG_A2(sp)
    sd      a3,  REG_A3(sp)
    sd      a4,  REG_A4(sp)
    sd      a5,  REG_A5(sp)
    sd      a6,  REG_A6(sp)
    sd      a7,  REG_A7(sp)
    sd      s2,  REG_S2(sp)
    sd      s3,  REG_S3(sp)
    sd      s4,  REG_S4(sp)
    sd      s5,  REG_S5(sp)
    sd      s6,  REG_S6(sp)
    sd      s7,  REG_S7(sp)
    sd      s8,  REG_S8(sp)
    sd      s9,  REG_S9(sp)
    sd      s10, REG_S10(sp)
    sd      s11, REG_S11(sp)
    sd      t3,  REG_T3(sp)
    sd      t4,  REG_T4(sp)
    sd      t5,  REG_T5(sp)
    sd      t6,  REG_T6(sp)

    /* Save mepc and mstatus */
    csrr    t0, mepc
    sd      t0, REG_MEPC(sp)
    csrr    t0, mstatus
    sd      t0, REG_MSTATUS(sp)

    /* ======================================================================
     * Step 3: Call C trap handler
     * ======================================================================*/

    /* a0 = sp (pointer to saved context) */
    mv      a0, sp
    call    trap_handler

    /* a0 now contains the new SP (possibly switched task) */
    mv      sp, a0

    /* Fall through to trap_exit */

.global trap_exit
trap_exit:
    /* ======================================================================
     * Step 4: Restore context and return
     * ======================================================================*/

    /* Restore mepc and mstatus first */
    ld      t0, REG_MEPC(sp)
    csrw    mepc, t0
    ld      t0, REG_MSTATUS(sp)
    csrw    mstatus, t0

#if CONFIG_USER_MODE
    /* -- Check if returning to U-mode -- */
    /* Extract MPP field from mstatus (bits 12:11) */
    srli    t1, t0, 11
    andi    t1, t1, 3
    bnez    t1, .restore_m_mode

    /* -- Returning to U-mode: set up mscratch -- */
    /* mscratch = kernel_sp (top of context frame) */
    addi    t1, sp, CONTEXT_SIZE
    csrw    mscratch, t1

.restore_m_mode:
#endif

    /* Restore all general-purpose registers */
    ld      ra,  REG_RA(sp)
    ld      gp,  REG_GP(sp)
    ld      tp,  REG_TP(sp)
    ld      t0,  REG_T0(sp)
    ld      t1,  REG_T1(sp)
    ld      t2,  REG_T2(sp)
    ld      s0,  REG_S0(sp)
    ld      s1,  REG_S1(sp)
    ld      a0,  REG_A0(sp)
    ld      a1,  REG_A1(sp)
    ld      a2,  REG_A2(sp)
    ld      a3,  REG_A3(sp)
    ld      a4,  REG_A4(sp)
    ld      a5,  REG_A5(sp)
    ld      a6,  REG_A6(sp)
    ld      a7,  REG_A7(sp)
    ld      s2,  REG_S2(sp)
    ld      s3,  REG_S3(sp)
    ld      s4,  REG_S4(sp)
    ld      s5,  REG_S5(sp)
    ld      s6,  REG_S6(sp)
    ld      s7,  REG_S7(sp)
    ld      s8,  REG_S8(sp)
    ld      s9,  REG_S9(sp)
    ld      s10, REG_S10(sp)
    ld      s11, REG_S11(sp)
    ld      t3,  REG_T3(sp)
    ld      t4,  REG_T4(sp)
    ld      t5,  REG_T5(sp)
    ld      t6,  REG_T6(sp)

    /* Restore SP (user SP if returning to U-mode) */
    ld      sp, REG_SP(sp)

    /* Return from trap */
    mret

.size trap_entry, . - trap_entry

