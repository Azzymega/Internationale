#include <hal/hal.h>
#include <mm/mm.h>
#include <ps/proc.h>

INUGLOBAL UINTPTR PsGlobalProcessId;
INUEXTERN struct LIST_ENTRY PsGlobalSchedulableObjectCollection;

struct KERNEL_PROCESS* ProcessAllocate()
{
    struct KERNEL_PROCESS* process = MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED,sizeof(struct KERNEL_PROCESS));
    return process;
}

VOID ProcessInitialize(struct KERNEL_PROCESS *self, struct VAS_DESCRIPTOR *vas, enum PROCESS_MODE mode)
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

VOID ProcessRemoveSchedulableObject(struct KERNEL_PROCESS *self, struct KERNEL_THREAD *object)
{
    ListEntryRemove(&object->processThreadCollection);
    ListEntryRemove(&object->schedulableCollection);
}

VOID ProcessAddSchedulableObject(struct KERNEL_PROCESS *self, struct KERNEL_THREAD *object)
{
    ListEntryAdd(&self->scheduableObjects,&object->processThreadCollection);
    ListEntryAdd(&PsGlobalSchedulableObjectCollection,&object->schedulableCollection);
}
