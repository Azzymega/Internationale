#include <hal/hal.h>
#include <mm/mm.h>
#include <mm/vad.h>
#include <ps/proc.h>

#include "mmi.h"

struct VAS_BLOCK_DESCRIPTOR* VasBlockDescriptorAllocate()
{
    return MmAllocatePoolWithTag(NonPagedPoolZeroed, sizeof(struct VAS_BLOCK_DESCRIPTOR), 0);
}

struct VAS_DESCRIPTOR* VasDescriptorAllocate()
{
    return MmAllocatePoolWithTag(NonPagedPoolZeroed, sizeof(struct VAS_DESCRIPTOR), 0);
}

VOID VasDescriptorInitialize(struct VAS_DESCRIPTOR* self)
{
    self->header.type = VAS_DESCRIPTOR_TYPE;
    self->header.refCount = 2;
    self->pageTables = HalAllocatePageTable();

    ListEntryInitialize(&self->vasBlockCollection,NULL);
}

VOID VasDescriptorVirtualAlloc(struct VAS_DESCRIPTOR* self, VOID* address, UINTPTR length,
                               enum VAS_BLOCK_DESCRIPTOR_TYPES attributes)
{
    INU_ASSERT(self);

    UINTPTR pageLength = length / HalGetPageSize();
    if (length % HalGetPageSize() != 0)
    {
        pageLength++;
    }

    VOID* kernelVa = MmAllocatePhysical(pageLength);

    struct VAS_BLOCK_DESCRIPTOR* block = VasBlockDescriptorAllocate();
    VasBlockDescriptorInitialize(block, self, kernelVa, pageLength, address, attributes);

    HalUpdateMapping(self);
}

VOID VasDescriptorMapMemory(struct VAS_DESCRIPTOR* self, VOID* virtualAddress, VOID* kernelAddress, UINTPTR length,
                            enum VAS_BLOCK_DESCRIPTOR_TYPES attributes)
{
    INU_ASSERT(self);
    INU_ASSERT(kernelAddress);

    struct LIST_ENTRY* vasBlock = self->vasBlockCollection.next;

    UINTPTR pageLength = length/HalGetPageSize();
    if (pageLength % HalGetPageSize() != 0)
    {
        pageLength++;
    }

    struct VAS_BLOCK_DESCRIPTOR* block = VasBlockDescriptorAllocate();
    VasBlockDescriptorInitialize(block,self,kernelAddress,pageLength,virtualAddress,attributes);

    if (vasBlock->owner == NULL)
    {

    }
    else
    {
        while (vasBlock->owner != NULL)
        {
            struct VAS_BLOCK_DESCRIPTOR* desc = vasBlock->owner;

            if (desc->startVa <= (UINTPTR)virtualAddress && (UINTPTR)virtualAddress <= desc->endVa)
            {
                INU_BUGCHECK("VM region intersect!");
            }

            vasBlock = vasBlock->next;
        }
    }

    HalUpdateMapping(self);
}

VOID VasBlockDescriptorInitialize(struct VAS_BLOCK_DESCRIPTOR* self, struct VAS_DESCRIPTOR* owner, VOID* kmem,
                                  UINTPTR pageLength, VOID* mapTarget, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes)
{
    INU_ASSERT(self);
    INU_ASSERT(owner);
    INU_ASSERT(kmem);
    INU_ASSERT(pageLength > 0);

    self->header.refCount = 2;
    self->header.type = VAS_DESCRIPTOR_TYPE;

    self->startKernelVa = (UINTPTR)kmem;
    self->endKernelVa = (UINTPTR)+pageLength * HalGetPageSize();

    self->startVa = (UINTPTR)mapTarget;
    self->endVa = (UINTPTR)mapTarget + pageLength * HalGetPageSize();

    self->pageLength = pageLength;
    self->attributes = attributes;
    self->owner = owner;

    ListEntryInitialize(&self->vasBlockCollection, self);
    ListEntryAdd(&owner->vasBlockCollection, &self->vasBlockCollection);
}
