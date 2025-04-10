#pragma once
#include <ltbase.h>

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

#define FwX86TextModeAddress ((INT16*)0xb8000)

struct INUPACKED FwX86TextModeState
{
    UINT16 x;
    UINT16 y;
};

VOID FwiInitializeDisplay();
VOID FwiClockHandler(VOID* handler);
VOID FwiPitInitialize(VOID);
VOID FwiPicInitialize(VOID);
BOOLEAN FwiPicCheckInterrupt(UINTPTR irq);
UINT16 FwiPicGetIrr(VOID);
UINT16 FwiPicGetIsr(VOID);
VOID FwiPicMaskIrq(UINTPTR irq);
VOID FwiPicUnmaskIrq(UINTPTR irq);
VOID FwiPicSendEoi(UINTPTR irq);
VOID FwiPicRemap(UINTPTR firstOffset, UINTPTR secondOffset);
VOID FwiPicDisable(VOID);
UINT16 FwiPicGetIrqIndex(VOID);