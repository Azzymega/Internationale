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

struct KERNEL_THREAD
{
    struct KERNEL_OBJECT_HEADER header;

    enum SCHEDULABLE_OBJECT_TYPE type;
    enum SCHEDULABLE_OBJECT_STATE state;

    struct KERNEL_PROCESS* owner;

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

struct KERNEL_THREAD* ThreadAllocate();
VOID ThreadInitialize(struct KERNEL_THREAD* self, struct KERNEL_PROCESS* process);
VOID ThreadLoad(struct KERNEL_THREAD* self, struct KERNEL_PROCESS* process, VOID* func, VOID* arg);
VOID ThreadLock(struct KERNEL_THREAD* self);
VOID ThreadUnlock(struct KERNEL_THREAD* self);
VOID ThreadSleep(struct KERNEL_THREAD* self, UINTPTR count);