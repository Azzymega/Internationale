#include <hal/hal.h>
#include <mm/mm.h>
#include <ps/proc.h>

INUGLOBAL UINTPTR PsGlobalProcessId;

struct PROCESS* ProcessAllocate()
{
    struct PROCESS* process = MmAllocatePoolWithTag(NonPagedPoolZeroed,sizeof(struct PROCESS),0);
    return process;
}

VOID ProcessInitialize(struct PROCESS *self, struct VAS_DESCRIPTOR *vas, enum PROCESS_MODE mode)
{
    INU_ASSERT(self);
    INU_ASSERT(vas);

    self->header.type = PROCESS_TYPE;

    ListEntryInitialize(&self->scheduableObjects,NULL);
    ListEntryInitialize(&self->objects,NULL);

    self->id = PsGlobalProcessId++;
    self->mode = mode;
    vas->owner = self;
    self->vasDescriptor = vas;
}

VOID ProcessRemoveSchedulableObject(struct PROCESS *self, struct THREAD *object)
{
    ListEntryRemove(&object->processThreadCollection);
}

VOID ProcessAddSchedulableObject(struct PROCESS *self, struct THREAD *object)
{
    ListEntryAdd(&self->scheduableObjects,&object->processThreadCollection);
}
