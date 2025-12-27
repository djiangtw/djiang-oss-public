# danieRTOS v3p2 - PMP 記憶體保護設計文件

## 概述

本文件記錄 danieRTOS v3p2 版本中 Physical Memory Protection (PMP) 的設計決策、
開發過程中的經驗教訓、目前的限制，以及移植到其他配置時的注意事項。

---

## 1. 記憶體布局 (Memory Map)

### 1.1 目前配置 (QEMU virt, 128MB RAM)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    RAM: 0x80000000 - 0x88000000 (128MB)             │
├─────────────────────────────────────────────────────────────────────┤
│ 0x80000000 ┌─────────────────────────────────────────┐              │
│            │  .text (Kernel Code)                    │ ~9KB         │
│            │  M-mode 專用，U-mode 無法執行           │              │
│ 0x800023FC └─────────────────────────────────────────┘              │
│            ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐              │
│ 0x80003000 │  Gap (~3KB, 4KB 對齊填充)              │              │
│            └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘              │
│ 0x80003000 ┌─────────────────────────────────────────┐              │
│            │  .rodata + .data (Shared Region)       │ 4KB NAPOT    │
│            │  U-mode 可讀取（字串常量）              │              │
│ 0x80004000 └─────────────────────────────────────────┘              │
│ 0x80004000 ┌─────────────────────────────────────────┐              │
│            │  .bss (Kernel Globals)                 │ ~84KB        │
│            │  M-mode 專用                            │              │
│ 0x80018B30 └─────────────────────────────────────────┘              │
│            ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐              │
│ 0x80020000 │  Gap (~30KB, 128KB 對齊填充)           │              │
│            └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘              │
│ 0x80020000 ┌─────────────────────────────────────────┐              │
│            │  .user_text (User Code)                │ ~1KB         │
│            │  U-mode 可執行                          │              │
│ 0x80020404 ├─────────────────────────────────────────┤              │
│            │  .user_data (User Stacks)              │ 64KB         │
│            │  8 tasks × 8KB stack                   │              │
│ 0x80030410 └─────────────────────────────────────────┘              │
│            │  Padding to 4KB boundary               │              │
│ 0x80031000 └─────────────────────────────────────────┘ 128KB NAPOT  │
│                                                                      │
│            ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐              │
│            │  Heap (未使用)                         │ ~127MB       │
│            └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘              │
│                                                                      │
│ 0x87F80000 ┌─────────────────────────────────────────┐              │
│            │  Kernel Stacks                         │ 512KB NAPOT  │
│            │  8 cores × 64KB per core               │              │
│            │  U-mode 可讀寫（任務執行時使用）        │              │
│ 0x88000000 └─────────────────────────────────────────┘              │
└─────────────────────────────────────────────────────────────────────┘
```

### 1.2 PMP 配置表

| Entry | 區域 | 地址範圍 | NAPOT Size | 權限 | 用途 |
|-------|------|----------|------------|------|------|
| 0 | User Region | 0x80020000 - 0x80040000 | 128KB | RWX | User 程式碼與資料 |
| 1 | Shared Region | 0x80003000 - 0x80004000 | 4KB | R | 字串常量（rodata） |
| 2 | Stack Region | 0x87F80000 - 0x88000000 | 512KB | RW | Kernel/User 共用堆疊 |
| 3-7 | 保留 | - | - | OFF | 未使用 |

---

## 2. 開發經驗教訓 (Lessons Learned)

### 2.1 NAPOT 對齊要求 ⚠️ 最重要

**問題**：NAPOT (Naturally Aligned Power-Of-Two) 模式要求：
- `size` 必須是 2 的冪次方
- `base` 必須對齊到 `size`

**踩過的坑**：
```
原始配置：
  user_base = 0x80018000 (未對齊)
  user_size = 0x11000 (68KB, 非 2 的冪次方)
  
NAPOT 會 round up size 到 128KB (0x20000)
但 0x80018000 % 0x20000 ≠ 0

結果：NAPOT 實際覆蓋 0x80000000 - 0x80020000
這包含了 kernel code！導致 U-mode 可以存取 kernel！
```

**解決方案**：
```c
// linker.ld 中必須使用足夠大的對齊
. = ALIGN(128K);  // 確保 base 對齊到可能的 NAPOT size
_user_section_start = .;
.user_text ALIGN(128K) : { ... }
```

### 2.2 字串常量的位置

**問題**：User task 中的字串常量（如 `"Hello"`）會被放在 `.rodata` section，
但 `.rodata` 預設在 kernel 區域，U-mode 無法讀取。

**症狀**：
```
mcause=5 (Load access fault)
mtval=0x80003xxx (rodata 區域)
```

**解決方案**：建立 Shared Region，允許 U-mode 讀取 `.rodata`：
```c
// pmp.c
cfg |= (PMP_A_NAPOT | PMP_R) << (1 * 8);  // Entry 1: Read-only
```

### 2.3 User Stack 的放置

**問題**：User task 的 stack 必須在 U-mode 可存取的區域。

**原始錯誤**：User stack 在 kernel `.bss` 中：
```c
static uint8_t g_user_ustacks[MAX_USER_TASKS][USER_USTACK_SIZE];
// 被放在 .bss，U-mode 無法存取
```

**解決方案**：使用 section attribute：
```c
static uint8_t g_user_ustacks[MAX_USER_TASKS][USER_USTACK_SIZE]
    __attribute__((section(".user_data")));
```

### 2.4 USER_FUNC Macro 的必要性

**問題**：User task 函數本體也必須在 U-mode 可執行的區域。

**解決方案**：
```c
// user_syscall.h
#define USER_FUNC __attribute__((section(".user_text")))
#define USER_DATA __attribute__((section(".user_data")))

// demo.c
USER_FUNC static void my_user_task(void *arg) {
    // 這個函數會被放在 .user_text
}
```

### 2.5 Linker Script 中的符號定義順序

**問題**：在 section 定義外部設定的符號，其值可能不如預期。

**錯誤範例**：
```
. = ALIGN(4096);
_shared_start = .;      // 這個符號可能不在預期位置
.rodata : { ... }
```

**正確做法**：在 section 內部定義符號：
```
.rodata ALIGN(4096) : {
    _shared_start = .;  // 確保在 section 開始處
    *(.rodata .rodata.*)
    _shared_end = .;    // 若需要在 section 結束處
}
```

---

## 3. 目前限制 (Limitations)

### 3.1 PMP Entry 數量限制

- RISC-V 標準定義最多 **16 個 PMP entries**
- QEMU virt 實作 **8 個 entries**
- 目前使用 **3 個 entries**（User, Shared, Stack）
- 剩餘 5 個 entries 可用於：
  - 每任務獨立 stack 隔離
  - 周邊設備存取控制
  - 動態記憶體區域

### 3.2 NAPOT 粒度限制

- 最小 NAPOT region = **8 bytes**（RISC-V 規範）
- 實際上 QEMU 最小有效單位約 **4KB**
- 非 2 的冪次方大小會浪費記憶體：
  ```
  實際需要: 68KB user region
  NAPOT 需要: 128KB (下一個 2^n)
  浪費: 60KB
  ```

### 3.3 共享記憶體安全性

- 目前 Shared Region 包含所有 `.rodata` 和 `.data`
- U-mode 可讀取任何字串常量，包括可能的敏感資訊
- 更安全的做法：
  - 分離 kernel rodata 和 user rodata
  - 但會增加 PMP entry 使用

### 3.4 Stack 未隔離

- 目前所有 user tasks 共享同一個 stack region
- Task A 理論上可以讀寫 Task B 的 stack
- 完整隔離需要每個 task 獨立 PMP entry

### 3.5 固定記憶體布局

- 目前配置針對 QEMU virt 128MB RAM
- User region 固定為 128KB NAPOT
- Stack region 固定為 512KB NAPOT（8 cores × 64KB）

---

## 4. 移植指南 (Porting Guide)

### 4.1 變更 RAM 大小

修改 `linker.ld`：

```ld
MEMORY {
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 64M  /* 改為你的大小 */
}
```

並調整 stack 區域位置：

```ld
/* Stack 區域必須在 RAM 範圍內 */
. = ORIGIN(RAM) + LENGTH(RAM) - (CONFIG_SMP_MAX_CORES * 64K);
_stack_bottom = .;
```

### 4.2 變更核心數

1. 修改 `include/config.h`：
```c
#define CONFIG_SMP_MAX_CORES    4  /* 改為你的核心數 */
```

2. 調整 stack 區域大小（需為 2 的冪次方以符合 NAPOT）：
```
4 cores × 64KB = 256KB → NAPOT size = 256KB ✓
5 cores × 64KB = 320KB → NAPOT size = 512KB（浪費 192KB）
```

### 4.3 增加 User 程式碼大小

如果 user code 超過 128KB：

1. 計算新的 NAPOT size：
```python
def next_power_of_2(n):
    return 1 << (n - 1).bit_length()

user_size = 200 * 1024  # 200KB
napot_size = next_power_of_2(user_size)  # = 256KB
```

2. 修改 linker.ld 中的對齊：
```ld
. = ALIGN(256K);  /* 改為新的 NAPOT size */
_user_section_start = .;
```

### 4.4 增加 User Tasks 數量

1. 修改 `include/config.h`：
```c
#define MAX_USER_TASKS    16  /* 預設是 8 */
```

2. 確認 `.user_data` 夠大：
```
16 tasks × 8KB stack = 128KB
→ 如果 user code + 128KB > 128KB NAPOT，需要擴大 user region
```

### 4.5 不同平台的 PMP 實作差異

| 平台 | PMP Entries | 注意事項 |
|------|-------------|----------|
| QEMU virt | 8 | 標準實作 |
| SiFive U54 | 8 | 同 QEMU |
| Kendryte K210 | 8 | 有些版本有 bug |
| 自定義 | 0-16 | 檢查 CSR 是否存在 |

**檢測 PMP 支援**：
```c
// 嘗試寫入 pmpcfg0，若觸發 exception 則不支援
uint64_t test;
asm volatile("csrr %0, pmpcfg0" : "=r"(test));
```

### 4.6 除錯 PMP 問題的步驟

1. **確認 fault 類型**：
   - mcause=1: Instruction access fault (X 權限問題)
   - mcause=5: Load access fault (R 權限問題)
   - mcause=7: Store access fault (W 權限問題)

2. **檢查 mtval**：fault 發生的地址

3. **dump PMP 狀態**：
```c
pmp_dump();  // 印出所有 PMP entries
```

4. **驗證 NAPOT 對齊**：
```python
# 驗證 base 對齊到 size
def check_napot(base, size):
    napot_size = 1 << (size - 1).bit_length()
    if base % napot_size != 0:
        print(f"ERROR: 0x{base:x} not aligned to 0x{napot_size:x}")
        # 計算實際覆蓋範圍
        actual_base = base & ~(napot_size - 1)
        print(f"Actual coverage: 0x{actual_base:x} - 0x{actual_base + napot_size:x}")
```

5. **使用 GDB 單步追蹤**：
```bash
make debug  # 啟動 QEMU + GDB
# 在 trap_handler 設斷點觀察 fault
```

---

## 5. 未來改進方向

1. **Per-task stack isolation**: 每個 task 獨立 PMP entry
2. **MPU-style protection**: 使用 TOR mode 減少對齊需求
3. **Separate user rodata**: 分離 kernel/user 字串常量
4. **Dynamic PMP**: 根據 task 切換調整 PMP 配置
5. **sPMP (Supervisor PMP)**: 支援 S-mode 的系統

---

## 6. 參考資料

- [RISC-V Privileged Specification v1.12](https://riscv.org/specifications/privileged-isa/)
- Chapter 3.7: Physical Memory Protection
- [QEMU RISC-V virt machine](https://www.qemu.org/docs/master/system/riscv/virt.html)

