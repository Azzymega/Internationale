#include <hal/hal.h>
#include <mm/mm.h>
#include <mm/vad.h>
#include <ps/proc.h>

#include "mmi.h"

struct VAS_BLOCK_DESCRIPTOR* VasBlockDescriptorAllocate()
{
    return MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED, sizeof(struct VAS_BLOCK_DESCRIPTOR));
}

struct VAS_DESCRIPTOR* VasDescriptorAllocate()
{
    return MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED, sizeof(struct VAS_DESCRIPTOR));
}

VOID VasDescriptorInitialize(struct VAS_DESCRIPTOR* self)
{
    self->header.type = VAS_DESCRIPTOR_TYPE;
    self->header.refCount = 2;
    self->pageTables = HalAllocatePageTable();
}

INUSTATUS VasDescriptorMapMemory(struct VAS_DESCRIPTOR* self, VOID* virtualAddress, VOID* kernelAddress, UINTPTR length,
                            enum VAS_BLOCK_DESCRIPTOR_TYPES attributes)
{
    INU_ASSERT(self != NULL);

    UINTPTR numericVirtualAddress = (UINTPTR)virtualAddress;
    UINTPTR pageSize = HalGetPageSize();
    UINTPTR alignedLength = (length/pageSize)*pageSize;
    if (length % pageSize != 0)
    {
        alignedLength += pageSize;
    }

    struct VAS_BLOCK_DESCRIPTOR* descriptor = self->region;
    while (descriptor != NULL)
    {
        if (descriptor->startVa < numericVirtualAddress && descriptor->endVa > numericVirtualAddress)
        {
            return STATUS_REGION_OVERLAP;
        }
        descriptor = descriptor->next;
    }

    struct VAS_BLOCK_DESCRIPTOR* block = VasBlockDescriptorAllocate();
    VasBlockDescriptorInitialize(block,self,kernelAddress,alignedLength/pageSize,virtualAddress,attributes);

    if (self->region == NULL)
    {
        self->region = block;
    }
    else
    {
        struct VAS_BLOCK_DESCRIPTOR* lastDescriptor = self->region;
        while (lastDescriptor->next != NULL)
        {
            lastDescriptor = lastDescriptor->next;
        }
        lastDescriptor->next = block;
    }

    HalMemoryMap(self,virtualAddress,kernelAddress,alignedLength,block->attributes);

    return STATUS_SUCCESS;
}

VOID VasBlockDescriptorInitialize(struct VAS_BLOCK_DESCRIPTOR* self, struct VAS_DESCRIPTOR* owner, VOID* kmem,
                                  UINTPTR pageLength, VOID* mapTarget, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes)
{
    INU_ASSERT(self);
    INU_ASSERT(owner);
    INU_ASSERT(pageLength > 0);

    self->header.refCount = 2;
    self->header.type = VAS_DESCRIPTOR_TYPE;

    self->startKernelVa = (UINTPTR)kmem;
    self->endKernelVa = (UINTPTR)+pageLength * HalGetPageSize();

    self->startVa = (UINTPTR)mapTarget;
    self->endVa = (UINTPTR)mapTarget + pageLength * HalGetPageSize();

    self->length = pageLength * HalGetPageSize();
    self->attributes = attributes;
    self->owner = owner;
}
