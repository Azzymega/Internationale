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

UINTPTR FwClock();
VOID FwInitialize(VOID);
VOID FwSetScheduler(TRAP_HANDLER handler);

UINTPTR FwGetCpuIndex();
struct PROCESSOR_DESCRIPTOR* FwGetCurrentCpuDescriptor();

VOID FwRaiseControlLevel(CONTROL_LEVEL level);
CONTROL_LEVEL FwGetControlLevel();
