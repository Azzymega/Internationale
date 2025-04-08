#include <ltbase.h>
#include <rtl/list.h>
#include <stdatomic.h>
#include <hal/hal.h>
#include <ps/cpu.h>

#include "hali.h"

#define X86_HAL_SYSTEM_CALL 0x2E

#define X86_HAL_PAGE_SIZE 4096
#define X86_HAL_INTERRUPT_GATE 0x8E
#define X86_HAL_TRAP_GATE 0x8F
#define X86_HAL_CODE_SEGMENT 0x08
#define X86_HAL_DESCRIPTOR_COUNT 8

#define X86_HAL_DE_INDEX 0
#define X86_HAL_DB_INDEX 1
#define X86_HAL_UD_INDEX 6
#define X86_HAL_DF_INDEX 8
#define X86_HAL_GPF_INDEX 13
#define X86_HAL_PF_INDEX 14

INUGLOBAL struct HaliX86GdtEntry FwGdtEntryTable[X86_HAL_DESCRIPTOR_COUNT];
INUGLOBAL struct HaliX86Tss FwKernelTss;
INUGLOBAL struct HaliX86GdtPointer FwGdtPointer;
INUGLOBAL INUALIGN(0x10) struct HaliX86InterruptDescriptor HalGlobalInterruptDescriptors[256];
INUGLOBAL INUALIGN(0x10) TRAP_HANDLER HalGlobalInterruptsHandlers[256];
INUGLOBAL INUALIGN(0x10) CONTROL_LEVEL HalGlobalInterruptCls[256];
INUGLOBAL struct HaliX86IdtPointer HalGlobalIdtPointer;

INUEXTERN VOID HaliFlushGdt();
INUEXTERN VOID HaliIsr0();
INUEXTERN VOID HaliIsr1();
INUEXTERN VOID HaliIsr2();
INUEXTERN VOID HaliIsr3();
INUEXTERN VOID HaliIsr4();
INUEXTERN VOID HaliIsr5();
INUEXTERN VOID HaliIsr6();
INUEXTERN VOID HaliIsr7();
INUEXTERN VOID HaliIsr8();
INUEXTERN VOID HaliIsr9();
INUEXTERN VOID HaliIsr10();
INUEXTERN VOID HaliIsr11();
INUEXTERN VOID HaliIsr12();
INUEXTERN VOID HaliIsr13();
INUEXTERN VOID HaliIsr14();
INUEXTERN VOID HaliIsr15();
INUEXTERN VOID HaliIsr16();
INUEXTERN VOID HaliIsr17();
INUEXTERN VOID HaliIsr18();
INUEXTERN VOID HaliIsr19();
INUEXTERN VOID HaliIsr20();
INUEXTERN VOID HaliIsr21();
INUEXTERN VOID HaliIsr22();
INUEXTERN VOID HaliIsr23();
INUEXTERN VOID HaliIsr24();
INUEXTERN VOID HaliIsr25();
INUEXTERN VOID HaliIsr26();
INUEXTERN VOID HaliIsr27();
INUEXTERN VOID HaliIsr28();
INUEXTERN VOID HaliIsr29();
INUEXTERN VOID HaliIsr30();
INUEXTERN VOID HaliIsr31();

INUEXTERN VOID HaliIrq0();
INUEXTERN VOID HaliIrq1();
INUEXTERN VOID HaliIrq2();
INUEXTERN VOID HaliIrq3();
INUEXTERN VOID HaliIrq4();
INUEXTERN VOID HaliIrq5();
INUEXTERN VOID HaliIrq6();
INUEXTERN VOID HaliIrq7();
INUEXTERN VOID HaliIrq8();
INUEXTERN VOID HaliIrq9();
INUEXTERN VOID HaliIrq10();
INUEXTERN VOID HaliIrq11();
INUEXTERN VOID HaliIrq12();
INUEXTERN VOID HaliIrq13();
INUEXTERN VOID HaliIrq14();
INUEXTERN VOID HaliIrq15();


VOID HalInitialize()
{
    HaliX86InitializeGdt();
    HaliX86InitializeInterrupts();
}

VOID HaliX86InitializeIdt()
{
    HaliX86LoadIdtPointer(&HalGlobalIdtPointer);
}

VOID HaliX86LoadIdtPointer(struct HaliX86IdtPointer* ptr)
{
    ptr->limit = sizeof(HalGlobalInterruptDescriptors) - 1;
    ptr->base = (UINT32)&HalGlobalInterruptDescriptors;
    asm volatile ("lidt %0" : : "m" (*ptr));
}

VOID HaliX86IntSetIsr(BYTE index, UINT32 base, UINT16 selector, BYTE flags)
{
    HalGlobalInterruptDescriptors[index].baseLow = (base & 0xFFFF);
    HalGlobalInterruptDescriptors[index].baseHigh = (base >> 16) & 0xFFFF;
    HalGlobalInterruptDescriptors[index].selector = selector;
    HalGlobalInterruptDescriptors[index].zero = 0;
    HalGlobalInterruptDescriptors[index].typeAttributes = flags;
}

UINTPTR HalGetPageSize()
{
    return X86_HAL_PAGE_SIZE;
}

VOID HalEnterLock(INUVOLATILE BOOLEAN *monitor)
{
    while (atomic_flag_test_and_set_explicit((atomic_flag*)monitor, memory_order_acquire))
    {
        asm("pause");
    }
}

VOID HalExitLock(INUVOLATILE BOOLEAN *monitor)
{
    atomic_flag_clear_explicit((atomic_flag*)monitor, memory_order_release);
}

BOOLEAN HalTryEnterLock(INUVOLATILE BOOLEAN *monitor)
{
    return !atomic_flag_test_and_set_explicit((atomic_flag*)monitor, memory_order_acquire);
}

VOID HalBugcheck(const CHAR *message)
{
    asm ("cli");
    asm ("hlt");
}

VOID HalCopyMemory(VOID *destination, VOID *source, UINTPTR length)
{
    __asm__ volatile
    (
        "cld\n"
        "rep movsb\n"
        :
        : "S"(destination), "D"(source), "c"(length)
        : "memory"
    );
}

VOID HalSetMemory(VOID *destination, UINTPTR target, UINTPTR length)
{
    __asm__ volatile
    (
        "cld\n"
        "rep stosb\n"
        :
        : "a"(target), "D"(destination), "c"(length)
        : "memory"
    );
}

VOID HalMoveMemory(VOID *destination, VOID *source, UINTPTR length)
{
    __asm__ volatile
    (
        "cmp %%esi, %%edi\n"
        "jb 1f\n"
        "std\n"
        "lea -1(%%esi, %%ecx), %%esi\n"
        "lea -1(%%edi, %%ecx), %%edi\n"
        "rep movsb\n"
        "jmp 2f\n"
        "1:\n"
        "cld\n"
        "rep movsb\n"
        "2:\n"
        "cld\n"
        :
        : "S"(source), "D"(destination), "c"(length)
        : "memory", "cc"
    );
}

VOID HalEnableInterrupts()
{
    asm ("sti");
}

VOID HalDisableInterrupts()
{
    asm ("cli");
}

VOID HalSetInterrupt(TRAP_HANDLER ptr, UINTPTR index, CONTROL_LEVEL cl)
{
    if (index < 256)
    {
        HalGlobalInterruptsHandlers[index] = ptr;
        HalGlobalInterruptCls[index] = cl;
    }
}

CONTROL_LEVEL HalGetInterruptControlLevel(UINTPTR index)
{
    return HalGlobalInterruptCls[index];
}

VOID HalSetZeroDivTrap(TRAP_HANDLER handler)
{
    HalSetInterrupt(handler,X86_HAL_DE_INDEX,CONTROL_LEVEL_FAULT);
}

VOID HalSetDebugTrap(TRAP_HANDLER handler)
{
    HalSetInterrupt(handler,X86_HAL_DB_INDEX,CONTROL_LEVEL_FAULT);
}

VOID HalSetInvalidOpcodeTrap(TRAP_HANDLER handler)
{
    HalSetInterrupt(handler,X86_HAL_UD_INDEX,CONTROL_LEVEL_FAULT);
}

VOID HalSetDoubleFaultTrap(TRAP_HANDLER handler)
{
    HalSetInterrupt(handler,X86_HAL_DF_INDEX,CONTROL_LEVEL_FAULT);
}

VOID HalSetProtectionFaultTrap(TRAP_HANDLER handler)
{
    HalSetInterrupt(handler,X86_HAL_GPF_INDEX,CONTROL_LEVEL_FAULT);
}

VOID HalSetPageFaultTrap(TRAP_HANDLER handler)
{
    HalSetInterrupt(handler,X86_HAL_PF_INDEX,CONTROL_LEVEL_FAULT);
}

UINTPTR HalGetSerializedStateSize()
{
    return sizeof(struct HaliX86InterruptDescriptor);
}

INUSTATIC void outb(UINT32 port, BYTE data) {
    __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC BYTE inb(UINT32 port) {
    BYTE data;
    __asm__ volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

INUSTATIC void outw(UINT32 port, UINT16 data) {
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC UINT16 inw(UINT32 port) {
    UINT16 data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

INUSTATIC void outdw(UINT32 port, UINT32 data) {
    __asm__ volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

INUSTATIC UINT32 indw(UINT32 port) {
    UINT32 data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

void HaliX86SetGdtEntry(const INT32 index, const UINT32 base, const UINT32 limit, const BYTE access, const BYTE granularity) {
    FwGdtEntryTable[index].baseLow = base & 0xFFFF;
    FwGdtEntryTable[index].baseMiddle = (base >> 16) & 0xFF;
    FwGdtEntryTable[index].baseHigh = (base >> 24 & 0xFF);
    FwGdtEntryTable[index].limiteLow = limit & 0xFFFF;
    FwGdtEntryTable[index].granularity = (limit >> 16) & 0x0F;
    FwGdtEntryTable[index].access = access;
    FwGdtEntryTable[index].granularity = FwGdtEntryTable[index].granularity | (granularity & 0xF0);
}

VOID HaliIsrHandler(struct HaliX86InterruptFrame *descriptor)
{
    if (HalGlobalInterruptsHandlers[descriptor->int_no] != NULL)
    {
        CONTROL_LEVEL previousControl = FwGetControlLevel();
        FwRaiseControlLevel(HalGlobalInterruptCls[descriptor->int_no]);

        FwGetCurrentCpuDescriptor()->interruptFrame = descriptor;

        HalGlobalInterruptsHandlers[descriptor->int_no](descriptor);

        FwGetCurrentCpuDescriptor()->interruptFrame = NULL;
        FwRaiseControlLevel(previousControl);
    }
}

VOID HaliIrqHandler(struct HaliX86InterruptFrame *descriptor)
{
    if (HalGlobalInterruptsHandlers[descriptor->int_no] != NULL)
    {
        CONTROL_LEVEL previousControl = FwGetControlLevel();
        FwGetCurrentCpuDescriptor()->interruptFrame = descriptor;

        FwRaiseControlLevel(HalGlobalInterruptCls[descriptor->int_no]);

        HalGlobalInterruptsHandlers[descriptor->int_no](descriptor);

        FwGetCurrentCpuDescriptor()->interruptFrame = NULL;
        FwRaiseControlLevel(previousControl);
    }
}

void HaliX86InitializeGdt(void) {
    FwGdtPointer.limit = sizeof(FwGdtEntryTable) - 1;
    FwGdtPointer.base = (UINT32)FwGdtEntryTable;

    HaliX86SetGdtEntry(0, 0, 0, 0, 0);
    HaliX86SetGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    HaliX86SetGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    HaliX86SetGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    HaliX86SetGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    HaliX86SetGdtEntry(5, (UINT32)&FwKernelTss, (UINT32)((char*)&FwKernelTss)+sizeof(struct HaliX86Tss), 0x9E, 0);

    asm volatile ("lgdt %0" : "=m" (FwGdtPointer));
    HaliFlushGdt(&FwGdtPointer);
}

VOID HaliX86InitializeInterrupts()
{
    HaliX86InitializeIdt();

    HaliX86IntSetIsr(0,(UINT32)HaliIsr0,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(1,(UINT32)HaliIsr1,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(2,(UINT32)HaliIsr2,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(3,(UINT32)HaliIsr3,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(4,(UINT32)HaliIsr4,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(5,(UINT32)HaliIsr5,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(6,(UINT32)HaliIsr6,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(7,(UINT32)HaliIsr7,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(8,(UINT32)HaliIsr8,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(9,(UINT32)HaliIsr9,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(10,(UINT32)HaliIsr10,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(11,(UINT32)HaliIsr11,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(12,(UINT32)HaliIsr12,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(13,(UINT32)HaliIsr13,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(14,(UINT32)HaliIsr14,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(15,(UINT32)HaliIsr15,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(16,(UINT32)HaliIsr16,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(17,(UINT32)HaliIsr17,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(18,(UINT32)HaliIsr18,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(19,(UINT32)HaliIsr19,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(20,(UINT32)HaliIsr20,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(21,(UINT32)HaliIsr21,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(22,(UINT32)HaliIsr22,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(23,(UINT32)HaliIsr23,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(24,(UINT32)HaliIsr24,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(25,(UINT32)HaliIsr25,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(26,(UINT32)HaliIsr26,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(27,(UINT32)HaliIsr27,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(28,(UINT32)HaliIsr28,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(29,(UINT32)HaliIsr29,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(30,(UINT32)HaliIsr30,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(31,(UINT32)HaliIsr31,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);

    HaliX86IntSetIsr(32,(UINT32)HaliIrq0,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(33,(UINT32)HaliIrq1,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(34,(UINT32)HaliIrq2,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(35,(UINT32)HaliIrq3,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(36,(UINT32)HaliIrq4,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(37,(UINT32)HaliIrq5,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(38,(UINT32)HaliIrq6,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(39,(UINT32)HaliIrq7,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(40,(UINT32)HaliIrq8,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(41,(UINT32)HaliIrq9,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(42,(UINT32)HaliIrq10,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(43,(UINT32)HaliIrq11,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(44,(UINT32)HaliIrq12,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(45,(UINT32)HaliIrq13,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
    HaliX86IntSetIsr(46,(UINT32)HaliIrq14,X86_HAL_CODE_SEGMENT,X86_HAL_INTERRUPT_GATE);
}