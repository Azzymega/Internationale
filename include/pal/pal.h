#pragma once
#include <ps/ps.h>
#include <ltbase.h>

enum CONTROL_LEVEL_TYPE
{
    CONTROL_LEVEL_DISPATCH = 0,
    CONTROL_LEVEL_DPC = 1,
    CONTROL_LEVEL_TIMER = 30,
    CONTROL_LEVEL_FAULT = 31
};

UINTPTR PalClock();
VOID PalInitialize(UINTPTR magic, UINTPTR ptr);
VOID PalSetScheduler(TRAP_HANDLER handler);
VOID PalYieldToDispatch();
VOID PalShutdown();
VOID PalSetInterruptFrame(VOID* frame);
VOID PalPrint(const char *format, ...);

UINTPTR PalGetNonPagedPoolVirtualAddress();
UINTPTR PalGetNonPagedPoolPhysicalAddress();
UINTPTR PalGetNonPagedPoolSize();
UINTPTR PalGetPagedPoolSize();

UINTPTR PalGetCpuIndex();
struct PROCESSOR_DESCRIPTOR* PalGetCurrentCpuDescriptor();

VOID PalRaiseControlLevel(CONTROL_LEVEL level);
VOID PalAcknowledgeInterrupt(UINTPTR index);
CONTROL_LEVEL PalGetControlLevel();