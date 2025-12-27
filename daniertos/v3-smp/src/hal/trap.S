/**
 * danieRTOS v3.x - Trap Handler (RV64 SMP + User Mode)
 *
 * Copyright (c) 2025 Danny Jiang <djiang.tw@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 * v3.x Key Design: tp/mscratch Swap for SMP + User Mode
 *
 * In User Mode:
 *   - tp = User TLS (thread-local storage, task-specific)
 *   - mscratch = &cpu_t (kernel per-core data)
 *
 * In Kernel Mode:
 *   - tp = &cpu_t (per-core data, used everywhere in kernel)
 *   - mscratch = saved User tp (will be restored on mret)
 *
 * Trap Entry Flow:
 *   1. Swap tp <-> mscratch (now tp = &cpu_t)
 *   2. Save user sp to cpu_t.user_sp_save
 *   3. Load kernel sp from cpu_t.kernel_trap_sp
 *   4. Save all registers to kernel stack
 *   5. Call C trap handler
 *
 * Trap Exit Flow:
 *   1. Restore all registers from kernel stack
 *   2. If returning to U-mode: swap tp <-> mscratch
 *   3. Restore user sp and mret
 */

#include "config.h"

/* cpu_t offsets (MUST match struct cpu in smp.h!) */
#define CPU_KERNEL_TRAP_SP  0
#define CPU_USER_SP_SAVE    8
#define CPU_USER_TP_SAVE    16

/*
 * Context frame layout (34 slots * 8 bytes = 272 bytes):
 *   [0]  zero (placeholder, not saved)
 *   [1]  ra
 *   [2]  sp (user SP when from U-mode)
 *   [3]  gp
 *   [4]  tp (user TP when from U-mode)
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
     * Step 1: Determine if we came from U-mode or M-mode
     *
     * In U-mode: mscratch = &cpu_t, tp = User TLS
     * In M-mode: tp = &cpu_t, mscratch = saved User tp (or 0 for kernel tasks)
     *
     * We need to:
     *   - If from U-mode: swap tp <-> mscratch to get cpu_t
     *   - If from M-mode: tp is already cpu_t, don't swap
     *
     * We check mstatus.MPP to know where we came from.
     * But mstatus is slow to read. Alternative: check if mscratch looks valid.
     * For simplicity in v3, we use the "TP value test" approach:
     *   - If tp points to g_cpus array, we're from M-mode
     *   - Otherwise, swap to get cpu_t from mscratch
     *
     * Even simpler approach for v3: Always swap and check if tp is valid.
     * If tp becomes 0 or invalid after swap, we need to swap back.
     * ======================================================================*/
#if CONFIG_USER_MODE
    /* Swap tp <-> mscratch unconditionally */
    csrrw   tp, mscratch, tp

    /* If tp is 0, we came from M-mode with kernel task (mscratch was 0) */
    /* Swap back to restore original state */
    beqz    tp, .from_m_mode

    /* tp should now be &cpu_t. Continue with U-mode trap handling. */

    /* Save user sp to cpu_t.user_sp_save */
    sd      sp, CPU_USER_SP_SAVE(tp)

    /* Load kernel sp from cpu_t.kernel_trap_sp */
    ld      sp, CPU_KERNEL_TRAP_SP(tp)

    /* Allocate context frame on kernel stack */
    addi    sp, sp, -CONTEXT_SIZE

    /* Save t0 first (we need it as scratch) */
    sd      t0, REG_T0(sp)

    /* Save user sp (from cpu_t.user_sp_save) */
    ld      t0, CPU_USER_SP_SAVE(tp)
    sd      t0, REG_SP(sp)

    /* Save user tp (from mscratch, which now holds the original tp) */
    csrr    t0, mscratch
    sd      t0, REG_TP(sp)

    /* Also save to cpu_t for later restoration */
    sd      t0, CPU_USER_TP_SAVE(tp)

    j       .save_context

.from_m_mode:
    /* Came from M-mode. Swap back to restore tp and mscratch */
    csrrw   tp, mscratch, tp
    /* Now tp = &cpu_t (original), mscratch = 0 */
    /* Fall through to v2-style M-mode handling */
#endif

    /* v2 compatibility: M-mode trap, just use current stack */
    addi    sp, sp, -CONTEXT_SIZE
    sd      t0, REG_T0(sp)
    addi    t0, sp, CONTEXT_SIZE
    sd      t0, REG_SP(sp)
    sd      tp, REG_TP(sp)

.save_context:
    /* ======================================================================
     * Step 2: Save all registers to kernel stack
     * ======================================================================*/

    /* Save all general-purpose registers (t0, sp, tp already saved) */
    sd      ra,  REG_RA(sp)
    sd      gp,  REG_GP(sp)
    /* tp already saved above */
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
    /* MPP = 0 means U-mode, MPP = 3 means M-mode */
    srli    t1, t0, 11
    andi    t1, t1, 3
    bnez    t1, .restore_m_mode

    /* -- Returning to U-mode: set mscratch = &cpu_t -- */
    /* When U-mode takes a trap, trap_entry will swap tp <-> mscratch */
    /* to get &cpu_t into tp. So we need mscratch = &cpu_t here. */
    csrw    mscratch, tp
    j       .restore_regs_umode

.restore_m_mode:
    /* -- Returning to M-mode: set mscratch = 0 -- */
    /* When M-mode takes a trap, trap_entry will detect mscratch = 0 */
    /* and know not to swap (tp is already &cpu_t). */
    csrw    mscratch, zero
    j       .restore_regs_mmode
#endif

.restore_regs_umode:
    /* Restore all general-purpose registers (U-mode: restore tp) */
    ld      ra,  REG_RA(sp)
    /* gp is not restored - set once at startup */
#if CONFIG_USER_MODE
    ld      tp,  REG_TP(sp)     /* Restore user tp */
#endif
    j       .restore_common

.restore_regs_mmode:
    /* Restore all general-purpose registers (M-mode: keep tp = &cpu_t) */
    ld      ra,  REG_RA(sp)
    /* gp is not restored - set once at startup */
    /* tp is NOT restored - keep as &cpu_t */

.restore_common:
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

