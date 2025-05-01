#include <pal/pal.h>
#include <hal/hal.h>
#include <mm/mm.h>
#include <ps/crit.h>
#include <ps/ps.h>

struct CRITICAL_SECTION* CriticalSectionAllocate()
{
    struct CRITICAL_SECTION* criticalSection = MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED,sizeof(struct CRITICAL_SECTION));
    return criticalSection;
}

VOID SpinlockInitialize(struct SPINLOCK* spinlock)
{
    spinlock->lock = FALSE;
}

VOID SpinlockEnter(struct SPINLOCK* spinlock)
{
    HalEnterLock(&spinlock->lock);
}

VOID SpinlockExit(struct SPINLOCK* spinlock)
{
    HalExitLock(&spinlock->lock);
}

VOID CriticalSectionInitialize(struct CRITICAL_SECTION* self)
{
    self->header.refCount = 1;
    self->header.type = CRITICAL_SECTION_TYPE;

    SpinlockInitialize(&self->accquireLock);
    ListEntryInitialize(&self->locks,NULL);

    self->owner = NULL;
    self->occupationCounter = 0;
}

VOID CriticalSectionEnter(struct CRITICAL_SECTION* self)
{
    INU_ASSERT(self);

    SpinlockEnter(&self->accquireLock);

    struct KERNEL_THREAD* thread;
    PsGetCurrentThread(&thread);

    if (self->owner == thread)
    {
        self->occupationCounter++;

        SpinlockExit(&self->accquireLock);
    }
    else
    {
        if (self->owner == NULL)
        {
            self->owner = thread;
            self->occupationCounter++;

            SpinlockExit(&self->accquireLock);
        }
        else
        {
            ListEntryAddBack(&self->locks,&thread->externalLock);

            SpinlockExit(&self->accquireLock);
            PsLockThread(thread);

            PalYieldToDispatch();
        }
    }
}

VOID CriticalSectionExit(struct CRITICAL_SECTION* self)
{
    INU_ASSERT(self);

    struct KERNEL_THREAD* thread;
    PsGetCurrentThread(&thread);

    if (self->owner != thread)
    {
        INU_BUGCHECK("Bad critical section exit!");
    }

    SpinlockEnter(&self->accquireLock);

    if (self->owner == thread && self->occupationCounter - 1 > 0)
    {
        self->occupationCounter--;
        SpinlockExit(&self->accquireLock);
        return;
    }

    struct KERNEL_THREAD* selected = ListEntryNext(&self->locks);
    if (selected == NULL)
    {
        self->owner = NULL;
        self->occupationCounter = 0;
        SpinlockExit(&self->accquireLock);
    }
    else
    {
        self->occupationCounter = 1;
        self->owner = selected;
        ListEntryRemove(&selected->externalLock);
        SpinlockExit(&self->accquireLock);
        PsUnlockThread(selected);
    }
}
