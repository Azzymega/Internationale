#pragma once
#include <ltbase.h>

#define HALI_GDT_KERNEL_MODE 0
#define HALI_GDT_USER_MODE 3

#define HALI_GDT_READ_ONLY 0
#define HALI_GDT_READ_WRITE 1

#define HALI_GDT_CODE_SEGMENT 1
#define HALI_GDT_DATA_SEGMENT 0

#define HALI_GDT_TSS 0
#define HALI_GDT_SEGMENT 1

#define HALI_GDT_PAGE_GRANULARITY 1
#define HALI_GDT_BYTE_GRANULARITY 0

#define HALI_32BIT_SEGMENT 1
#define HALI_16BIT_SEGMENT 0

#define HALI_TSS_16TSS_AVL 0x1
#define HALI_TSS_16TSS_BUSY 0x3
#define HALI_TSS_LDT 0x02
#define HALI_TSS_32TSS_AVL 0x9
#define HALI_TSS_32TSS_BUSY 0xB

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
    UINT32 dr7;
    UINT32 dr6;
    UINT32 dr3;
    UINT32 dr2;
    UINT32 dr1;
    UINT32 dr0;

    UINT32 cr4;
    UINT32 cr3;
    UINT32 cr2;
    UINT32 cr0;

    UINT32 gs;
    UINT32 fs;
    UINT32 es;
    UINT32 ds;

    uint8_t fpu[108];

    UINT32 edi;
    UINT32 esi;
    UINT32 ebp;
    UINT32 ebx;
    UINT32 edx;
    UINT32 ecx;
    UINT32 eax;
    UINT32 esp;

    UINT32 int_no;
    UINT32 error_code;

    UINT32 eip;
    UINT32 cs;
    UINT32 eflags;

    UINT32 user_esp;
    UINT32 ss;
};

struct INUPACKED Halix86InterruptKernelFrame
{
    UINT32 dr7;
    UINT32 dr6;
    UINT32 dr3;
    UINT32 dr2;
    UINT32 dr1;
    UINT32 dr0;

    UINT32 cr4;
    UINT32 cr3;
    UINT32 cr2;
    UINT32 cr0;

    UINT32 gs;
    UINT32 fs;
    UINT32 es;
    UINT32 ds;

    uint8_t fpu[108];

    UINT32 edi;
    UINT32 esi;
    UINT32 ebp;
    UINT32 ebx;
    UINT32 edx;
    UINT32 ecx;
    UINT32 eax;
    UINT32 esp;

    UINT32 int_no;
    UINT32 error_code;

    UINT32 eip;
    UINT32 cs;
    UINT32 eflags;
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
    UINT16 lowLimit;
    UINT16 lowBase;
    BYTE midBase;

    BYTE accessed : 1;
    BYTE rw : 1;
    BYTE direction : 1;
    BYTE executable : 1;
    BYTE isSegment : 1;
    BYTE privilegeLevel : 2;
    BYTE present : 1;


    BYTE highLimit : 4;
    BYTE avl : 1;
    BYTE reserved : 1;
    BYTE size : 1;
    BYTE pageGranularity : 1;

    BYTE highBase;
};

struct INUPACKED HaliX86TssEntry
{
    UINT16 lowLimit;
    UINT16 lowBase;
    BYTE midBase;

    BYTE type : 4;
    BYTE isSegment : 1;
    BYTE privilegeLevel : 2;
    BYTE present : 1;

    BYTE highLimit : 4;

    BYTE avl : 1;
    BYTE reserved : 1;
    BYTE size : 1;
    BYTE pageGranularity : 1;

    BYTE highBase;
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
VOID HaliX86LoadGdtEntry(UINT32 index, UINT32 base, UINT32 limit, BYTE rw, BYTE executable, BYTE isSegment,
                         BYTE privilegeLevel, BYTE granularity, BYTE sizeType);
VOID HaliX86LoadTssEntry(UINT32 index, UINT32 base, UINT32 limit, BYTE tssType, BYTE privilegeLevel);
VOID HaliX86FixupFrame(VOID* frame, enum PROCESS_MODE mode);
