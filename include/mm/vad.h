#pragma once
#include <ltbase.h>
#include <ob/object.h>
#include <rtl/list.h>

enum VAS_BLOCK_DESCRIPTOR_TYPES
{
    VAS_DESCRIPTOR_RW = 2,
    VAS_DESCRIPTOR_SU = 4
};

struct VAS_DESCRIPTOR
{
    struct OBJECT_HEADER header;

    struct PROCESS* owner;
    VOID* pageTables;
    struct LIST_ENTRY vasBlockCollection;
};

struct VAS_BLOCK_DESCRIPTOR
{
    struct OBJECT_HEADER header;

    struct VAS_DESCRIPTOR* owner;
    struct LIST_ENTRY vasBlockCollection;

    enum VAS_BLOCK_DESCRIPTOR_TYPES attributes;
    UINTPTR startVa;
    UINTPTR endVa;

    UINTPTR startKernelVa;
    UINTPTR endKernelVa;
    UINTPTR pageLength;

};

struct VAS_DESCRIPTOR* VasDescriptorAllocate();
VOID VasDescriptorInitialize(struct VAS_DESCRIPTOR* self);

VOID VasDescriptorVirtualAlloc(struct VAS_DESCRIPTOR* self, VOID* address, UINTPTR length, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes);
VOID VasDescriptorMapMemory(struct VAS_DESCRIPTOR* self, VOID* virtualAddress, VOID* kernelAddress, UINTPTR length, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes);

struct VAS_BLOCK_DESCRIPTOR* VasBlockDescriptorAllocate();
VOID VasBlockDescriptorInitialize(struct VAS_BLOCK_DESCRIPTOR* self, struct VAS_DESCRIPTOR* owner, VOID* kmem, UINTPTR pageLength, VOID* mapTarget, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes);