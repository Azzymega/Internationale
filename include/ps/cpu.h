#pragma once
#include <ltbase.h>

struct PROCESSOR_DESCRIPTOR
{
    UINTPTR index;
    UINTPTR controlLevel;
    struct SCHEDULABLE_OBJECT* schedulableObject;
    VOID* interruptFrame;
};