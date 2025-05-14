#pragma once
#include <ltbase.h>
#include <mm/mm.h>

#define PIT_DATA_PORT 0x40
#define PIT_CMD_PORT 0x43

#define PIT_INT_TIMER 0x06
#define PIT_BINARY 0
#define PIT_ZERO_COUNTER 0
#define PIT_FREQUENCY 1193182
#define PIT_FULL_RW 0x30

#define SYSTEM_TIMER_RATE 1

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI	0x20
#define PIC_READ_IRR 0x0a
#define PIC_READ_ISR 0x0b

#define FW_TIMER_INTERRUPT_INDEX 0x20
#define FW_SLEEP_PORT 0x80
#define FW_CPU_COUNT 1
#define FW_YIELD_INDEX 0xFE

#define FwX86TextModeAddress ((INT16*)0xb8000)

struct INUPACKED PaliX86Eflags
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


struct INUPACKED PaliX86BiosCallFrame
{
    union
    {
        UINT32 edi;

        struct
        {
            UINT16 di;
            UINT16 upper_edi;
        };

        struct
        {
            BYTE dil;
            BYTE dih;
            BYTE upper_edi_b1;
            BYTE upper_edi_b2;
        };
    };

    union
    {
        UINT32 esi;

        struct
        {
            UINT16 si;
            UINT16 upper_esi;
        };

        struct
        {
            BYTE sil;
            BYTE sih;
            BYTE upper_esi_b1;
            BYTE upper_esi_b2;
        };
    };

    union
    {
        UINT32 ebp;

        struct
        {
            UINT16 bp;
            UINT16 upper_ebp;
        };

        struct
        {
            BYTE bpl;
            BYTE bph;
            BYTE upper_ebp_b1;
            BYTE upper_ebp_b2;
        };
    };

    union
    {
        UINT32 esp;

        struct
        {
            UINT16 sp;
            UINT16 upper_esp;
        };

        struct
        {
            BYTE spl;
            BYTE sph;
            BYTE upper_esp_b1;
            BYTE upper_esp_b2;
        };
    };

    union
    {
        UINT32 ebx;

        struct
        {
            UINT16 bx;
            UINT16 upper_ebx;
        };

        struct
        {
            BYTE bl;
            BYTE bh;
            BYTE upper_ebx_b1;
            BYTE upper_ebx_b2;
        };
    };

    union
    {
        UINT32 edx;

        struct
        {
            UINT16 dx;
            UINT16 upper_edx;
        };

        struct
        {
            BYTE dl;
            BYTE dh;
            BYTE upper_edx_b1;
            BYTE upper_edx_b2;
        };
    };

    union
    {
        UINT32 ecx;

        struct
        {
            UINT16 cx;
            UINT16 upper_ecx;
        };

        struct
        {
            BYTE cl;
            BYTE ch;
            BYTE upper_ecx_b1;
            BYTE upper_ecx_b2;
        };
    };

    union
    {
        UINT32 eax;

        struct
        {
            UINT16 ax;
            UINT16 upper_eax;
        };

        struct
        {
            BYTE al;
            BYTE ah;
            BYTE upper_eax_b1;
            BYTE upper_eax_b2;
        };
    };

    UINT16 gs;
    UINT16 fs;
    UINT16 es;
    UINT16 ds;
    UINT16 ss;

    struct PaliX86Eflags eflags;
};

struct INUPACKED PaliX86TextModeState
{
    UINT16 x;
    UINT16 y;
};

enum PaliX86MemoryMapType
{
    TYPE_USABLE           = 1,
    TYPE_RESERVED         = 2,
    TYPE_ACPI_RECLAIMABLE = 3,
    TYPE_ACPI_NVS         = 4,
    TYPE_BAD_MEMORY       = 5,
    TYPE_UNUSED           = 6
};

struct INUPACKED PaliX86MemoryMap
{
    UINT64 baseAddress;
    UINT64 length;
    enum PaliX86MemoryMapType type;
};

VOID PaliX86BiosCall(BYTE interruptIndex, struct PaliX86BiosCallFrame* frame);
VOID PaliX86BiosCallsInitialize();

VOID PaliX86TranslateMemoryMap(struct PaliX86MemoryMap* map, struct PHYSICAL_MEMORY_BLOCK* block, UINTPTR count);
VOID PaliX86FixMemoryMap(struct PaliX86MemoryMap* map, UINTPTR length);
VOID PaliX86ReadMemoryMap(struct PaliX86MemoryMap* map);
UINTPTR Palix86GetMemoryBlocksCount();
VOID PaliX86ReloadCr3(VOID* address);
VOID PaliPerformInitialMapping();
VOID PaliFramebufferInitialize();
VOID PaliClockHandler(VOID* handler);
VOID PaliPitInitialize(VOID);
VOID PaliPicInitialize(VOID);
BOOLEAN PaliPicCheckInterrupt(UINTPTR irq);
UINT16 PaliPicGetIrr(VOID);
UINT16 PaliPicGetIsr(VOID);
VOID PaliPicMaskIrq(UINTPTR irq);
VOID PaliPicUnmaskIrq(UINTPTR irq);
VOID PaliPicSendEoi(UINTPTR irq);
VOID PaliPicRemap(UINTPTR firstOffset, UINTPTR secondOffset);
VOID PaliPicDisable(VOID);
UINT16 PaliPicGetIrqIndex(VOID);
VOID PaliPrint(const char* message);
VOID* PaliGetBiosCallBuffer();
VOID* PaliGetBiosFrameBuffer();
