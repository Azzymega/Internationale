#pragma once
#include <ltbase.h>

struct PROCESSOR_DESCRIPTOR
{
    UINTPTR index;
    UINTPTR controlLevel;
    struct KERNEL_THREAD* schedulableObject;
    VOID* interruptFrame;
};