#pragma once
#include <ltbase.h>
#include <rtl/list.h>

enum SCHEDULABLE_OBJECT_TYPE
{
    SCHEDULABLE_OBJECT_THREAD,
    SCHEDULABLE_OBJECT_DPC
};

enum SCHEDULABLE_OBJECT_STATE
{
    SCHEDULABLE_OBJECT_WAITING,
    SCHEDULABLE_OBJECT_SLEEP,
    SCHEDULABLE_OBJECT_READY,
    SCHEDULABLE_OBJECT_EXECUTING,
};


struct SCHEDULABLE_OBJECT
{
    struct OBJECT_HEADER header;

    enum SCHEDULABLE_OBJECT_TYPE type;
    enum SCHEDULABLE_OBJECT_STATE state;

    struct LIST_ENTRY resources;
    struct LIST_ENTRY externalLock;
    struct LIST_ENTRY schedulableCollection;

    UINTPTR cpuIndex;

    BOOLEAN isSleep;
    UINTPTR sleepStart;
    UINTPTR sleepLength;

    UINTPTR index;
    UINTPTR priority;
    UINTPTR lockCount;
    VOID* frame;
};