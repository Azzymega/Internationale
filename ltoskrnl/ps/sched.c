#include <pal/pal.h>
#include <hal/hal.h>
#include <mm/mm.h>
#include <ps/proc.h>
#include <ps/ps.h>

#define PS_BASE_STACK_PAGE_SIZE 100

INUGLOBAL UINTPTR PsGlobalThreadId;
INUEXTERN struct LIST_ENTRY PsGlobalSchedulableObjectCollection;

struct THREAD* ThreadAllocate()
{
    struct THREAD* thread = MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED,sizeof(struct THREAD));
    return thread;
}

VOID ThreadInitialize(struct THREAD* self, struct PROCESS* process)
{
    self->header.type = THREAD_TYPE;

    self->owner = process;
    self->type = SCHEDULABLE_OBJECT_THREAD;
    self->state = SCHEDULABLE_OBJECT_WAITING;

    self->isLoaded = FALSE;
    self->id = PsGlobalThreadId++;
    self->stackLength = 0;
    self->isSleep = FALSE;
    self->sleepLength = 0;
    self->sleepStart = 0;
    self->priority = 0;
    self->lockCount = 1;
    self->frame = MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED,HalGetSerializedStateSize());

    ListEntryInitialize(&self->schedulableCollection,self);
    ListEntryInitialize(&self->processThreadCollection,self);
    ListEntryInitialize(&self->externalLock,self);
}

VOID ThreadLoad(struct THREAD *self, struct PROCESS *process, VOID *func, VOID *arg)
{
    INU_ASSERT(self);
    INU_ASSERT(process);
    INU_ASSERT(func);
    INU_ASSERT(process->vasDescriptor);
    INU_ASSERT(process->vasDescriptor->pageTables);

    self->isLoaded = TRUE;

    // TODO: MAKE NORMAL USERMODE VMALLOC ALLOCATOR!
    if (process->mode == PROCESS_KERNEL)
    {
        VOID* stack = MmAllocatePoolMemory(NON_PAGED_HEAP,PS_BASE_STACK_PAGE_SIZE*HalGetPageSize());
        HalModifyFrame(self->frame,process->vasDescriptor->pageTables,stack+PS_BASE_STACK_PAGE_SIZE*HalGetPageSize(),func,arg,process->mode);
    }
    else
    {
        INU_BUGCHECK("Not implemented!");
    }
}

VOID ThreadUnlock(struct THREAD *self)
{
    self->lockCount--;

    INU_ASSERT(self->lockCount >= 0);

    if (self->lockCount == 0)
    {
        if (self->state == SCHEDULABLE_OBJECT_WAITING)
        {
            self->state = SCHEDULABLE_OBJECT_READY;
        }
    }
    else
    {

    }
}

VOID ThreadSleep(struct THREAD* self, UINTPTR count)
{
    self->sleepLength = 500;
    self->sleepStart = PalClock();
    self->isSleep = TRUE;
    ThreadLock(self);
}

VOID ThreadLock(struct THREAD *self)
{
    self->lockCount++;

    if (self->lockCount == 1)
    {
        if (self->state == SCHEDULABLE_OBJECT_READY)
        {
            self->state = SCHEDULABLE_OBJECT_WAITING;
        }
    }
    else
    {

    }

    struct THREAD* current;
    if (INU_SUCCESS(PsGetCurrentThread(&current)))
    {
        if (current == self)
        {
            PalYieldToDispatch();
        }
    }
}
