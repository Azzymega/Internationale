#pragma once
#include <ltbase.h>

struct INUPACKED HaliX86IdtPointer
{
    UINT16 limit;
    UINT32 base;
};

struct INUPACKED HaliX86InterruptDescriptor
{
    UINT16 baseLow;
    UINT16 selector;
    BYTE zero;
    BYTE typeAttributes;
    UINT16 baseHigh;
};

struct INUPACKED HaliX86InterruptFrame
{
    uint32_t dr7;
    uint32_t dr6;
    uint32_t dr3;
    uint32_t dr2;
    uint32_t dr1;
    uint32_t dr0;

    uint32_t cr4;
    uint32_t cr3;
    uint32_t cr2;
    uint32_t cr0;

    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint8_t fpu[108];

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t esp;

    uint32_t int_no;
    uint32_t error_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;

    uint32_t user_esp;
    uint32_t ss;
};

struct INUPACKED Halix86InterruptKernelFrame
{
    uint32_t dr7;
    uint32_t dr6;
    uint32_t dr3;
    uint32_t dr2;
    uint32_t dr1;
    uint32_t dr0;

    uint32_t cr4;
    uint32_t cr3;
    uint32_t cr2;
    uint32_t cr0;

    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint8_t fpu[108];

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t esp;

    uint32_t int_no;
    uint32_t error_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

struct INUPACKED HaliX86PageDirectoryEntry
{
    UINT32 present : 1;
    UINT32 rw : 1;
    UINT32 us : 1;
    UINT32 writeThrough : 1;
    UINT32 cacheDisabled : 1;
    UINT32 accessed : 1;
    UINT32 available : 1;
    UINT32 zero : 1;
    UINT32 available2 : 4;
    UINT32 address : 20;
};

struct INUPACKED HaliX86PageTableEntry
{
    UINT32 present : 1;
    UINT32 rw : 1;
    UINT32 su : 1;
    UINT32 writeThrough : 1;
    UINT32 cacheDisabled : 1;
    UINT32 accessed : 1;
    UINT32 dirty : 1;
    UINT32 attributes : 1;
    UINT32 global : 1;
    UINT32 available2 : 3;
    UINT32 address : 20;
};

struct INUPACKED HaliX86Eflags
{
    UINT32 carry : 1;
    UINT32 reserved1 : 1;
    UINT32 parity : 1;
    UINT32 reserved3 : 1;
    UINT32 aux : 1;
    UINT32 reserved5 : 1;
    UINT32 zero : 1;
    UINT32 sign : 1;
    UINT32 trap : 1;
    UINT32 interruptEnabled : 1;
    UINT32 direction : 1;
    UINT32 overflow : 1;
    UINT32 iopl : 2;
    UINT32 nestedTask : 1;
    UINT32 reserved15 : 1;
    UINT32 resumeFlag : 1;
    UINT32 virtual8086 : 1;
    UINT32 alignmentCheck : 1;
    UINT32 virtualInterrupt : 1;
    UINT32 virtualInterruptPending : 1;
    UINT32 cpuidIsPresent : 1;
    UINT32 unused : 10;
};

struct INUPACKED HaliX86Cr0
{
    UINT32 pe : 1;
    UINT32 monitorCoPorcessor : 1;
    UINT32 x87Emul : 1;
    UINT32 taskSwitched : 1;
    UINT32 extensionType : 1;
    UINT32 numericError : 1;
    UINT32 reserved : 11;
    UINT32 writeProtect : 1;
    UINT32 reserved2 : 1;
    UINT32 alignmentMask : 1;
    UINT32 reserved3 : 9;
    UINT32 wtDisable : 1;
    UINT32 cacheDisable : 1;
    UINT32 paging : 1;
};

struct INUPACKED HaliX86Cr4
{
    UINT32 vme : 1;
    UINT32 pvi : 1;
    UINT32 tsd : 1;
    UINT32 de : 1;
    UINT32 pse : 1;
    UINT32 pae : 1;
    UINT32 mce : 1;
};

struct INUPACKED HaliX86GdtEntry
{
    UINT16 limiteLow;
    UINT16 baseLow;
    BYTE baseMiddle;
    BYTE access;
    BYTE granularity;
    BYTE baseHigh;
};

struct INUPACKED HaliX86GdtPointer
{
    UINT16 limit;
    UINT32 base;
};

struct INUPACKED HaliX86Tss
{
    UINT32 prevTss;
    UINT32 esp0;
    UINT32 ss0;
    UINT32 esp1;
    UINT32 ss1;
    UINT32 esp2;
    UINT32 ss2;
    UINT32 cr3;
    UINT32 eip;
    UINT32 eflags;
    UINT32 eax;
    UINT32 ecx;
    UINT32 edx;
    UINT32 ebx;
    UINT32 esp;
    UINT32 ebp;
    UINT32 esi;
    UINT32 edi;
    UINT32 es;
    UINT32 cs;
    UINT32 ss;
    UINT32 ds;
    UINT32 fs;
    UINT32 gs;
    UINT32 ldt;
    UINT16 trap;
    UINT16 iomap;
};

UINTPTR HaliIsrHandler(struct HaliX86InterruptFrame* descriptor);
UINTPTR HaliIrqHandler(struct HaliX86InterruptFrame* descriptor);

VOID HaliX86InitializeGdt();
VOID HaliX86InitializeInterrupts();
VOID HaliX86InitializeIdt();
VOID HaliX86IntSetIsr(BYTE index, UINT32 base, UINT16 selector, BYTE flags);
VOID HaliX86LoadIdtPointer(struct HaliX86IdtPointer* ptr);
VOID HaliX86SetGdtEntry(const INT32 index, const UINT32 base, const UINT32 limit, const BYTE access,
                        const BYTE granularity);
VOID HaliX86FixupFrame(VOID* frame, enum PROCESS_MODE mode);
