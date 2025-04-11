#pragma once
#include <ob/object.h>
#include <rtl/list.h>

struct SPINLOCK
{
    BOOLEAN lock;
};

struct CRITICAL_SECTION
{
    struct OBJECT_HEADER header;
    struct SPINLOCK accquireLock;
    struct THREAD* owner;
    struct LIST_ENTRY locks;
    UINTPTR occupationCounter;
};

VOID SpinlockInitialize(struct SPINLOCK* spinlock);
VOID SpinlockEnter(struct SPINLOCK* spinlock);
VOID SpinlockExit(struct SPINLOCK* spinlock);

struct CRITICAL_SECTION* CriticalSectionAllocate();
VOID CriticalSectionInitialize(struct CRITICAL_SECTION* self);
VOID CriticalSectionEnter(struct CRITICAL_SECTION* self);
VOID CriticalSectionExit(struct CRITICAL_SECTION* self);

