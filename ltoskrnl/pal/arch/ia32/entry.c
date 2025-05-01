#include <limits.h>
#include <ltbase.h>
#include <stdarg.h>
#include <hal/hal.h>
#include <pal/pal.h>
#include <mm/mm.h>
#include <ps/cpu.h>
#include <ps/crit.h>
#include <rtl/rtl.h>

#include "apm.h"
#include "pali.h"
#include "mb.h"
#include "vesa.h"
#include "../../../hal/arch/ia32/hali.h"

INUIMPORT UINTPTR PalKernelImageStart;
INUIMPORT UINTPTR PalKernelImageEnd;

INUGLOBAL struct PROCESSOR_DESCRIPTOR PalGlobalProcessorsDescriptors[FW_CPU_COUNT];
INUGLOBAL UINTPTR PalGlobalClock;
INUGLOBAL TRAP_HANDLER PalGlobalSchedulerHandler;
INUGLOBAL struct PaliX86TextModeState PalGlobalTextModeState;
INUGLOBAL struct CRITICAL_SECTION PalGlobalPrintLock;

INUGLOBAL INUALIGN(0x1000) struct HaliX86PageDirectoryEntry HaliX86InitialMapping[1024];
INUGLOBAL INUALIGN(0x1000) struct HaliX86PageTableEntry HaliX86BaseLowMemoryMapping[1024];
INUGLOBAL INUALIGN(0x1000) struct HaliX86PageTableEntry HaliX86BaseHighMemoryMapping[256][1024];



VOID PalInitialize(UINTPTR magic, UINTPTR ptr)
{
    HalInitialize();
    PaliPerformInitialMapping();
    PaliX86BiosCallsInitialize();
    PaliFramebufferInitialize();

    UINTPTR memoryMapLength = Palix86GetMemoryBlocksCount();
    struct PaliX86MemoryMap memoryMap[memoryMapLength+1];
    struct PHYSICAL_MEMORY_BLOCK blocks[memoryMapLength+1];

    RtlZeroMemory(memoryMap,sizeof memoryMap);
    RtlZeroMemory(blocks,sizeof blocks);
    PaliX86ReadMemoryMap(memoryMap);
    PaliX86TranslateMemoryMap(memoryMap,blocks,memoryMapLength);

    MmInitialize(blocks,memoryMapLength+1,(VOID*)&PalKernelImageEnd);

    if (PaliApmIsPresent(0x101))
    {
        PaliApmEnable();
    }
    else
    {
        INU_BUGCHECK("Not implemented!");
    }

    PaliPicInitialize();
    PaliPitInitialize();
    HalSetInterrupt(PaliClockHandler,FW_TIMER_INTERRUPT_INDEX, CONTROL_LEVEL_TIMER);
    PsInitialize();

    while (TRUE)
    {
    }
}



VOID PaliPerformInitialMapping()
{
    UINTPTR address = 0;
    for (int i = 0; i < 1024; ++i)
    {
        HaliX86BaseLowMemoryMapping[i].su = TRUE;
        HaliX86BaseLowMemoryMapping[i].rw = TRUE;
        HaliX86BaseLowMemoryMapping[i].address = address >> 12;
        HaliX86BaseLowMemoryMapping[i].present = TRUE;
        address += 4096;
    }

    HaliX86InitialMapping[0].present = TRUE;
    HaliX86InitialMapping[0].rw = TRUE;
    HaliX86InitialMapping[0].us = FALSE;
    HaliX86InitialMapping[0].address = ((UINTPTR)HaliX86BaseLowMemoryMapping-PalGetNonPagedPoolVirtualAddress()) >> 12;

    address = 0;
    UINTPTR offset = 0;

    for (int i = 512; i < 768; ++i) {
        if (offset >= 256) break;

        struct HaliX86PageTableEntry* pte = &HaliX86BaseHighMemoryMapping[offset][0];
        offset++;

        HaliX86InitialMapping[i].present = TRUE;
        HaliX86InitialMapping[i].rw = TRUE;
        HaliX86InitialMapping[i].us = FALSE;
        HaliX86InitialMapping[i].address = ((UINTPTR)pte-PalGetNonPagedPoolVirtualAddress()) >> 12;

        for (int j = 0; j < 1024; ++j) {
            pte[j].address = address >> 12;
            pte[j].present = TRUE;
            pte[j].rw = TRUE;
            pte[j].su = TRUE;
            address += 4096;
        }
    }

    UINTPTR cr3Physical = ((UINTPTR)HaliX86InitialMapping-PalGetNonPagedPoolVirtualAddress());
    PaliX86ReloadCr3((VOID*)cr3Physical);
}

VOID PaliX86ReloadCr3(VOID* address)
{
    asm volatile (
        "mov %0, %%cr3"
        :
        : "r" (address)
        : "memory"
    );
}

VOID PaliX86ReadMemoryMap(struct PaliX86MemoryMap* map)
{
    UINTPTR index = 0;
    UINTPTR offset = 0;
    struct PaliX86BiosCallFrame frame = {0};

    while (TRUE)
    {
        frame.eax = 0xE820;
        frame.edx = 0x534D4150;
        frame.ebx = offset;
        frame.es = 0;
        frame.di = (UINTPTR)PaliGetBiosCallBuffer();
        frame.ecx = sizeof(struct PaliX86MemoryMap);

        PaliX86BiosCall(0x15,&frame);
        RtlCopyMemory(&map[index++],PaliGetBiosCallBuffer(),sizeof(struct PaliX86MemoryMap));

        if (frame.ebx == 0 || frame.eflags.carry == TRUE)
        {
            break;
        }
        else
        {
            offset += frame.ebx;
        }
    }
}

UINTPTR Palix86GetMemoryBlocksCount()
{
    UINTPTR counter = 0;
    UINTPTR offset = 0;
    struct PaliX86BiosCallFrame frame = {0};

    while (TRUE)
    {
        frame.eax = 0xE820;
        frame.edx = 0x534D4150;
        frame.ebx = offset;
        frame.es = 0;
        frame.di = (UINTPTR)PaliGetBiosCallBuffer();
        frame.ecx = sizeof(struct PaliX86MemoryMap);

        PaliX86BiosCall(0x15,&frame);
        if (frame.ebx == 0 || frame.eflags.carry == TRUE)
        {
            break;
        }
        else
        {
            offset += frame.ebx;
            counter++;
        }
    }
    return counter;
}

VOID PalSetScheduler(TRAP_HANDLER handler)
{
    PalGlobalSchedulerHandler = handler;
    HalSetInterrupt(handler,FW_YIELD_INDEX, CONTROL_LEVEL_DISPATCH);
}


VOID PaliX86TranslateMemoryMap(struct PaliX86MemoryMap* map, struct PHYSICAL_MEMORY_BLOCK* block, UINTPTR count)
{
    for (int i = 0; i < count; ++i)
    {
        block[i].start = map[i].baseAddress;
        block[i].length = map[i].length;
        block[i].end = block[i].start+block[i].length;

        if (block[i].start < 1024*1024)
        {
            block[i].type = BLOCK_TYPE_RESERVED;
            continue;
        }

        switch (map[i].type)
        {
        case TYPE_USABLE:
            block[i].type = BLOCK_TYPE_USABLE;
            break;
        case TYPE_RESERVED:
            block[i].type = BLOCK_TYPE_RESERVED;
            break;
        case TYPE_ACPI_RECLAIMABLE:
            block[i].type = BLOCK_TYPE_RESERVED;
            break;
        case TYPE_ACPI_NVS:
            block[i].type = BLOCK_TYPE_RESERVED;
            break;
        case TYPE_BAD_MEMORY:
            block[i].type = BLOCK_TYPE_RESERVED;
            break;
        case TYPE_UNUSED:
            block[i].type = BLOCK_TYPE_UNUSED;
            break;
        }
    }

    block[count].start = (UINTPTR)&PalKernelImageStart-PalGetNonPagedPoolVirtualAddress();
    block[count].end = (UINTPTR)&PalKernelImageEnd-PalGetNonPagedPoolVirtualAddress();
    block[count].type = BLOCK_TYPE_RESERVED;
}

VOID PaliX86FixMemoryMap(struct PaliX86MemoryMap* map, UINTPTR length)
{
    for (int i = 0; i < length; ++i)
    {
        if (map[i].baseAddress < 1000000 && map[i].type == TYPE_USABLE)
        {
            map[i].type = TYPE_RESERVED;
        }
    }
}

VOID PalYieldToDispatch()
{
    if (PalGlobalSchedulerHandler != NULL)
    {
        __asm__("int %0\n" : : "N"((FW_YIELD_INDEX)) : "cc", "memory");
    }
}

VOID PalShutdown()
{
    PaliApmShutdown();
}

VOID PalSetInterruptFrame(VOID* frame)
{
    PalGetCurrentCpuDescriptor()->interruptFrame = frame;
}

UINTPTR PalGetCpuIndex()
{
    return 0;
}

VOID PalRaiseControlLevel(UINTPTR level)
{
    CONTROL_LEVEL currentControl = PalGetCurrentCpuDescriptor()->controlLevel;

    if (level == currentControl)
    {
        return;
    }
    else
    {
        PalGetCurrentCpuDescriptor()->controlLevel = level;
    }

    for (int i = 32; i < 47; ++i)
    {
        CONTROL_LEVEL target = HalGetInterruptControlLevel(i);

        if (target < level)
        {
            PaliPicMaskIrq(i - 32);
        }
        else
        {
            PaliPicUnmaskIrq(i - 32);
        }
    }
}

VOID PalAcknowledgeInterrupt(UINTPTR index)
{
    PaliPicSendEoi(index-32);
}

CONTROL_LEVEL PalGetControlLevel()
{
    return PalGetCurrentCpuDescriptor()->controlLevel;
}

struct PROCESSOR_DESCRIPTOR* PalGetCurrentCpuDescriptor()
{
    return &PalGlobalProcessorsDescriptors[PalGetCpuIndex()];
}

UINTPTR PalClock()
{
    return PalGlobalClock;
}


VOID PaliClockHandler(VOID* handler)
{
    PalGlobalClock += 10;

    if (PalGlobalSchedulerHandler != NULL)
    {
        PalGlobalSchedulerHandler(handler);
    }
}

INUSTATIC VOID outb(UINT32 port, BYTE data)
{
    __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC BYTE inb(UINT32 port)
{
    BYTE data;
    __asm__ volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

INUSTATIC VOID outw(UINT32 port, UINT16 data)
{
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC UINT16 inw(UINT32 port)
{
    UINT16 data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

INUSTATIC VOID outdw(UINT32 port, UINT32 data)
{
    __asm__ volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC UINT32 indw(UINT32 port)
{
    UINT32 data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

VOID PaliPitInitialize(VOID)
{
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

UINT16 PaliPicGetIrr(VOID)
{
    return PicGetIrqInfo(PIC_READ_IRR);
}

UINT16 PaliPicGetIsr(VOID)
{
    return PicGetIrqInfo(PIC_READ_ISR);
}

VOID PaliPicInitialize(VOID)
{
    PaliPicRemap(0x20, 0x28);
    for (int i = 0; i < 15; ++i)
    {
        PaliPicUnmaskIrq(i);
    }
}

BOOLEAN PaliPicCheckInterrupt(const UINTPTR irq)
{
    const UINT16 data = PaliPicGetIsr();
    if (data & (1 << irq))
    {
        return TRUE;
    }
    return FALSE;
}

VOID PaliPicMaskIrq(UINTPTR irq)
{
    UINT16 port = 0;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }

    const uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}

VOID PaliPicUnmaskIrq(UINTPTR irq)
{
    UINT16 port = 0;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }

    const BYTE value = inb(port) & ~(1 << irq);
    outb(port, value);
}

VOID PaliPicSendEoi(const UINTPTR irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND,PIC_EOI);
    }
    outb(PIC1_COMMAND,PIC_EOI);
}

VOID PicIoWait()
{
    for (int i = 0; i < 10; ++i)
    {
        outb(FW_SLEEP_PORT, 0);
    }
}

VOID PaliPicRemap(const UINTPTR firstOffset, const UINTPTR secondOffset)
{
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

VOID PaliPicDisable()
{
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

UINT16 PaliPicGetIrqIndex()
{
    return PaliPicGetIsr();
}

VOID FwpScrollTextModeScreen()
{
    PalGlobalTextModeState.y += 1;
    PalGlobalTextModeState.x = 0;

    if (PalGlobalTextModeState.y >= 25)
    {
        PalGlobalTextModeState.y = 24;
        PalGlobalTextModeState.x = 0;
        RtlMoveMemory(FwX86TextModeAddress,FwX86TextModeAddress + 80, 160 * 24);
        RtlSetMemory(&FwX86TextModeAddress[0 + (24 * 80)], 0, 160);
    }
}

INT32 FwpPrintCharacterTextModeScreen(char value)
{
Start:
    if (PalGlobalTextModeState.x >= 80)
    {
        FwpScrollTextModeScreen();
        goto Start;
    }

    if (value == '\r')
    {
        PalGlobalTextModeState.x = 0;
    }
    else if (value == '\n')
    {
        FwpScrollTextModeScreen();
    }
    else
    {
        uint16_t attrib = (0 << 4) | (7 & 0x0F);
        FwX86TextModeAddress[((PalGlobalTextModeState.x++) + (PalGlobalTextModeState.y * 80))] = value | attrib << 8;
    }

    return value;
}

static BOOLEAN FwpPrint(const char* data, UINTPTR length)
{
    const unsigned char* bytes = (const unsigned char*)data;
    for (UINTPTR i = 0; i < length; i++)
        if (FwpPrintCharacterTextModeScreen(bytes[i]) == -1)
            return FALSE;
    return TRUE;
}

char* FwpUlongToString(UINT64 value, char* string, int radix)
{
    char* baseStr = string;
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

VOID PalPrint(const char* format, ...)
{
    CriticalSectionEnter(&PalGlobalPrintLock);

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

        const char* format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(parameters, int);
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
            const char* str = va_arg(parameters, const char*);
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
            char* str = itoa(value, buff, 10);
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
            char* str = FwpUlongToString(value, buff, 16);
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

    CriticalSectionExit(&PalGlobalPrintLock);

    return;
}

UINTPTR PalGetNonPagedPoolVirtualAddress()
{
    return 0x80000000;
}

UINTPTR PalGetNonPagedPoolPhysicalAddress()
{
    return 0;
}

UINTPTR PalGetNonPagedPoolSize()
{
    return 0x40000000;
}

UINTPTR PalGetPagedPoolSize()
{
    return 0x40000000;
}

VOID FwpElbow(const char* string)
{
    for (UINTPTR i = 0; i < 25; ++i)
    {
        for (UINTPTR q = 0; q < 80; ++q)
        {
            FwpPrintCharacterTextModeScreen(' ');
        }
    }

    PalGlobalTextModeState.x = 0;
    PalGlobalTextModeState.y = 0;

    const UINTPTR stringLength = strlen(string);
    for (int i = 0; i < stringLength; ++i)
    {
        FwpPrintCharacterTextModeScreen(string[i]);
    }
}

VOID PaliPrint(const char* string)
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

VOID PaliFramebufferInitialize()
{
    CriticalSectionInitialize(&PalGlobalPrintLock);

    //PaliEnableVesa();

    for (UINTPTR i = 0; i < 25; ++i)
    {
        for (UINTPTR q = 0; q < 80; ++q)
        {
            FwpPrintCharacterTextModeScreen(' ');
        }
    }
}
