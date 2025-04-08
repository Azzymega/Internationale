#include <ltbase.h>
#include <stdbool.h>
#include <hal/hal.h>
#include <fw/fw.h>
#include <mm/mm.h>
#include <ps/cpu.h>

#include "fwi.h"

#define FW_TIMER_INTERRUPT_INDEX 0x20
#define FW_SLEEP_PORT 0x80
#define FW_CPU_COUNT 1

INUGLOBAL struct PROCESSOR_DESCRIPTOR FwGlobalProcessorsDescriptors[FW_CPU_COUNT];
INUGLOBAL UINTPTR FwGlobalClock;
INUGLOBAL TRAP_HANDLER FwGlobalSchedulerHandler;

VOID FwInitialize(VOID)
{
    VOID* memStart = (VOID*)4194304;
    UINTPTR memLength = 4194304*4;

    HalInitialize();
    FwiPicInitialize();
    FwiPitInitialize();
    MmInitialize(memStart,memLength);
    HalSetInterrupt(FwiClockHandler,FW_TIMER_INTERRUPT_INDEX,CONTROL_LEVEL_TIMER);

    HalEnableInterrupts();

    while (true)
    {

    }
}

VOID FwSetScheduler(TRAP_HANDLER handler)
{
    FwGlobalSchedulerHandler = handler;
}

UINTPTR FwGetCpuIndex()
{
    return 0;
}

VOID FwRaiseControlLevel(UINTPTR level)
{
    FwGetCurrentCpuDescriptor()->controlLevel = level;

    for (int i = 32; i < 47; ++i)
    {
        CONTROL_LEVEL target = HalGetInterruptControlLevel(i);

        if (target < level)
        {
            FwiPicMaskIrq(i-32);
        }
        else
        {
            FwiPicUnmaskIrq(i-32);
        }
    }
}

CONTROL_LEVEL FwGetControlLevel()
{
    return FwGetCurrentCpuDescriptor()->controlLevel;
}

struct PROCESSOR_DESCRIPTOR* FwGetCurrentCpuDescriptor()
{
    return &FwGlobalProcessorsDescriptors[FwGetCpuIndex()];
}

UINTPTR FwClock()
{
    return FwGlobalClock;
}

VOID FwiClockHandler(VOID* handler)
{
    FwGlobalClock += 10;

    if (FwGlobalSchedulerHandler != NULL)
    {
        FwGlobalSchedulerHandler(handler);
    }
}

INUSTATIC VOID outb(UINT32 port, BYTE data) {
    __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC BYTE inb(UINT32 port) {
    BYTE data;
    __asm__ volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

INUSTATIC VOID outw(UINT32 port, UINT16 data) {
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC UINT16 inw(UINT32 port) {
    UINT16 data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

INUSTATIC VOID outdw(UINT32 port, UINT32 data) {
    __asm__ volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC UINT32 indw(UINT32 port) {
    UINT32 data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

VOID FwiPitInitialize(VOID) {
    const UINT32 hz = 100;
    const UINT32 divisor = PIT_FREQUENCY / hz;
    outb(PIT_CMD_PORT, PIT_BINARY | PIT_INT_TIMER | PIT_FULL_RW | PIT_ZERO_COUNTER);
    outb(PIT_DATA_PORT, divisor);
    outb(PIT_DATA_PORT, divisor >> 8);
}

static UINT16 PicGetIrqInfo(const int ocw3)
{
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

UINT16 FwiPicGetIrr(VOID)
{
    return PicGetIrqInfo(PIC_READ_IRR);
}

UINT16 FwiPicGetIsr(VOID)
{
    return PicGetIrqInfo(PIC_READ_ISR);
}

VOID FwiPicInitialize(VOID) {
    FwiPicRemap(0x20,0x28);
    for (int i = 0; i < 15; ++i)
    {
        FwiPicUnmaskIrq(i);
    }
}

BOOLEAN FwiPicCheckInterrupt(const UINTPTR irq) {
    const UINT16 data = FwiPicGetIsr();
    if (data & (1 << irq)) {
        return TRUE;
    }
    return FALSE;
}

VOID FwiPicMaskIrq(UINTPTR irq) {
    UINT16 port = 0;

    if(irq < 8) {
        port = PIC1_DATA;
    }
    else {
        port = PIC2_DATA;
        irq -= 8;
    }

    const uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}

VOID FwiPicUnmaskIrq(UINTPTR irq) {
    UINT16 port = 0;

    if(irq < 8) {
        port = PIC1_DATA;
    }
    else {
        port = PIC2_DATA;
        irq -= 8;
    }

    const BYTE value = inb(port) & ~(1 << irq);
    outb(port, value);
}

VOID FwiPicSendEoi(const UINTPTR irq) {
    if(irq >= 8) {
        outb(PIC2_COMMAND,PIC_EOI);
    }
    outb(PIC1_COMMAND,PIC_EOI);
}

VOID PicIoWait() {
    for (int i = 0; i < 10; ++i) {
        outb(FW_SLEEP_PORT,0);
    }
}

VOID FwiPicRemap(const UINTPTR firstOffset, const UINTPTR secondOffset) {
    const BYTE firstMask = inb(PIC1_DATA);
    const BYTE secondMask = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    PicIoWait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    PicIoWait();
    outb(PIC1_DATA, firstOffset);
    PicIoWait();
    outb(PIC2_DATA, secondOffset);
    PicIoWait();
    outb(PIC1_DATA, 4);
    PicIoWait();
    outb(PIC2_DATA, 2);
    PicIoWait();
    outb(PIC1_DATA, ICW4_8086);
    PicIoWait();
    outb(PIC2_DATA, ICW4_8086);
    PicIoWait();

    outb(PIC1_DATA, firstMask);
    outb(PIC2_DATA, secondMask);
}

VOID FwiPicDisable() {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

UINT16 FwiPicGetIrqIndex() {
    return FwiPicGetIsr();
}
