#pragma once
#include <ltbase.h>

struct PROCESSOR_DESCRIPTOR
{
    UINTPTR index;
    UINTPTR controlLevel;
    struct THREAD* schedulableObject;
    VOID* interruptFrame;
};