#include <limits.h>
#include <ltbase.h>
#include <stdarg.h>
#include <hal/hal.h>
#include <fw/fw.h>
#include <mm/mm.h>
#include <ps/cpu.h>
#include <ps/crit.h>
#include <rtl/rtl.h>

#include "fwi.h"
#include "mb.h"

#define FW_TIMER_INTERRUPT_INDEX 0x20
#define FW_SLEEP_PORT 0x80
#define FW_CPU_COUNT 1
#define FW_YIELD_INDEX 0xFE

INUGLOBAL struct PROCESSOR_DESCRIPTOR FwGlobalProcessorsDescriptors[FW_CPU_COUNT];
INUGLOBAL UINTPTR FwGlobalClock;
INUGLOBAL TRAP_HANDLER FwGlobalSchedulerHandler;
INUGLOBAL struct FwX86TextModeState FwGlobalTextModeState;
INUGLOBAL struct CRITICAL_SECTION FwGlobalPrintLock;

VOID FwInitialize(UINTPTR magic, UINTPTR ptr)
{
    VOID* memStart = (VOID*)4194304;
    UINTPTR memLength = 4194304*4;

    HalInitialize();
    MmInitialize(memStart,memLength);
    FwiPicInitialize();
    FwiPitInitialize();
    FwiInitializeDisplay();
    HalSetInterrupt(FwiClockHandler,FW_TIMER_INTERRUPT_INDEX,CONTROL_LEVEL_TIMER);
    PsInitialize();

    while (TRUE)
    {

    }
}

VOID FwSetScheduler(TRAP_HANDLER handler)
{
    FwGlobalSchedulerHandler = handler;
    HalSetInterrupt(handler,FW_YIELD_INDEX,CONTROL_LEVEL_DISPATCH);
}

VOID FwYieldToDispatch()
{
    if (FwGlobalSchedulerHandler != NULL)
    {
        __asm__("int %0\n" : : "N"((FW_YIELD_INDEX)) : "cc", "memory");
    }
}

VOID FwSetInterruptFrame(VOID* frame)
{
    FwGetCurrentCpuDescriptor()->interruptFrame = frame;
}

UINTPTR FwGetCpuIndex()
{
    return 0;
}

VOID FwRaiseControlLevel(UINTPTR level)
{
    CONTROL_LEVEL currentControl = FwGetCurrentCpuDescriptor()->controlLevel;

    if (level == currentControl)
    {
        return;
    }
    else
    {
        FwGetCurrentCpuDescriptor()->controlLevel = level;
    }

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

VOID FwSignalEoi(UINTPTR irqIndex)
{
    FwiPicSendEoi(irqIndex);
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

VOID FwpScrollTextModeScreen()
{
    FwGlobalTextModeState.y += 1;
    FwGlobalTextModeState.x = 0;

    if (FwGlobalTextModeState.y >= 25)
    {
        FwGlobalTextModeState.y = 24;
        FwGlobalTextModeState.x = 0;
        RtlMoveMemory(FwX86TextModeAddress,FwX86TextModeAddress + 80, 160 * 24);
        RtlSetMemory(&FwX86TextModeAddress[0 + (24 * 80)], 0, 160);
    }
}

INT32 FwpPrintCharacterTextModeScreen(char value)
{
    Start:
        if (FwGlobalTextModeState.x >= 80)
        {
            FwpScrollTextModeScreen();
            goto Start;
        }

    if (value == '\r')
    {
        FwGlobalTextModeState.x = 0;
    }
    else if (value == '\n')
    {
        FwpScrollTextModeScreen();
    }
    else
    {
        uint16_t attrib = (0 << 4) | (7 & 0x0F);
        FwX86TextModeAddress[((FwGlobalTextModeState.x++) + (FwGlobalTextModeState.y * 80))] = value | attrib << 8;
    }

    return value;
}

static BOOLEAN FwpPrint(const char *data, UINTPTR length)
{
    const unsigned char *bytes = (const unsigned char *) data;
    for (UINTPTR i = 0; i < length; i++)
        if (FwpPrintCharacterTextModeScreen(bytes[i]) == -1)
            return FALSE;
    return TRUE;
}

char *FwpUlongToString(UINT64 value, char *string, int radix)
{
    char *baseStr = string;
    unsigned char index;
    char buffer[32];

    index = 32;

    do
    {
        buffer[--index] = '0' + (value % radix);
        if (buffer[index] > '9')
            buffer[index] += 'A' - ':';
        value /= radix;
    }
    while (value != 0);

    do
    {
        *string++ = buffer[index++];
    }
    while (index < 32);

    *string = 0;
    return baseStr;
}

VOID FwDebugPrint(const char *format, ...)
{
    CriticalSectionEnter(&FwGlobalPrintLock);

    FwiPrint("[DEBUG] ");
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0')
    {
        UINTPTR maxrem = INT_MAX - written;

        if (format[0] != '%' || format[1] == '%')
        {
            if (format[0] == '%')
                format++;
            UINTPTR amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                return;
            }
            if (!FwpPrint(format, amount))
                return;
            format += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char) va_arg(parameters, int);
            if (!maxrem)
            {
                return;
            }
            if (!FwpPrint(&c, sizeof(c)))
                return;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(parameters, const char*);
            UINTPTR len = strlen(str);
            if (maxrem < len)
            {
                return;
            }
            if (!FwpPrint(str, len))
                return;
            written += len;
        }
        else if (*format == 'i')
        {
            format++;
            int value = va_arg(parameters, int);
            char buff[100];
            char *str = itoa(value, buff, 10);
            UINTPTR len = strlen(str);
            if (maxrem < len)
            {
                return;
            }
            if (!FwpPrint(str, len))
                return;
            written += len;
        }
        else if (*format == 'x')
        {
            format++;
            int value = va_arg(parameters, int);
            char buff[100];
            char *str = FwpUlongToString(value, buff, 16);
            UINTPTR len = strlen(str);
            if (maxrem < len)
            {
                return;
            }
            if (!FwpPrint(str, len))
                return;
            written += len;
        }
        else
        {
            format = format_begun_at;
            UINTPTR len = strlen(format);
            if (maxrem < len)
            {
                return;
            }
            if (!FwpPrint(format, len))
                return;
            written += len;
            format += len;
        }
    }

    va_end(parameters);

    CriticalSectionExit(&FwGlobalPrintLock);

    return;
}

VOID FwpElbow(const char *string)
{
    for (UINTPTR i = 0; i < 25; ++i)
    {
        for (UINTPTR q = 0; q < 80; ++q)
        {
            FwpPrintCharacterTextModeScreen(' ');
        }
    }

    FwGlobalTextModeState.x = 0;
    FwGlobalTextModeState.y = 0;

    const UINTPTR stringLength = strlen(string);
    for (int i = 0; i < stringLength; ++i)
    {
        FwpPrintCharacterTextModeScreen(string[i]);
    }
}

VOID FwiPrint(const char *string)
{
    const UINTPTR stringLength = strlen(string);
    for (int i = 0; i < stringLength; ++i)
    {
        FwpPrintCharacterTextModeScreen(string[i]);
    }
}

VOID FwPutCharacter(CHAR wchar)
{
    FwpPrintCharacterTextModeScreen(wchar);
}

VOID FwiInitializeDisplay()
{
    CriticalSectionInitialize(&FwGlobalPrintLock);

    for (UINTPTR i = 0; i < 25; ++i)
    {
        for (UINTPTR q = 0; q < 80; ++q)
        {
            FwpPrintCharacterTextModeScreen(' ');
        }
    }
}