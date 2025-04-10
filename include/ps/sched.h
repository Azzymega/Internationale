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
    SCHEDULABLE_OBJECT_UNINITIALIZED,
    SCHEDULABLE_OBJECT_WAITING,
    SCHEDULABLE_OBJECT_SLEEP,
    SCHEDULABLE_OBJECT_READY,
    SCHEDULABLE_OBJECT_EXECUTING,
    SCHEDULABLE_OBJECT_EXITED
};

struct THREAD
{
    struct OBJECT_HEADER header;

    enum SCHEDULABLE_OBJECT_TYPE type;
    enum SCHEDULABLE_OBJECT_STATE state;

    struct PROCESS* owner;

    struct LIST_ENTRY resources;
    struct LIST_ENTRY externalLock;
    struct LIST_ENTRY schedulableCollection;
    struct LIST_ENTRY processThreadCollection;

    UINTPTR stackLength;
    UINTPTR cpuIndex;
    BOOLEAN isLoaded;
    BOOLEAN isSleep;
    UINTPTR sleepStart;
    UINTPTR sleepLength;

    INTPTR returnCode;
    UINTPTR id;
    UINTPTR priority;
    INTPTR lockCount;
    VOID* frame;
};

struct THREAD* ThreadAllocate();
VOID ThreadInitialize(struct THREAD* self, struct PROCESS* process);
VOID ThreadLoad(struct THREAD* self, struct PROCESS* process, VOID* func, VOID* arg);
VOID ThreadLock(struct THREAD* self);
VOID ThreadUnlock(struct THREAD* self);