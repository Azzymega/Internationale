#pragma once
#include <ltbase.h>
#include <ob/object.h>
#include <rtl/list.h>


struct VAS_BLOCK_DESCRIPTOR;
struct VAS_DESCRIPTOR;


enum VAS_BLOCK_DESCRIPTOR_TYPES
{
    VAS_DESCRIPTOR_RW = 2,
    VAS_DESCRIPTOR_SU = 4
};

struct VAS_DESCRIPTOR
{
    struct KERNEL_OBJECT_HEADER header;

    struct PROCESS* owner;
    VOID* pageTables;

    struct VAS_BLOCK_DESCRIPTOR* region;
};

struct VAS_BLOCK_DESCRIPTOR
{
    struct KERNEL_OBJECT_HEADER header;

    struct VAS_DESCRIPTOR* owner;

    enum VAS_BLOCK_DESCRIPTOR_TYPES attributes;
    UINTPTR startVa;
    UINTPTR endVa;

    UINTPTR startKernelVa;
    UINTPTR endKernelVa;
    UINTPTR length;

    struct VAS_BLOCK_DESCRIPTOR* next;
};

struct VAS_DESCRIPTOR* VasDescriptorAllocate();
VOID VasDescriptorInitialize(struct VAS_DESCRIPTOR* self);

INUSTATUS VasDescriptorMapMemory(struct VAS_DESCRIPTOR* self, VOID* virtualAddress, VOID* kernelAddress, UINTPTR length, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes);

struct VAS_BLOCK_DESCRIPTOR* VasBlockDescriptorAllocate();
VOID VasBlockDescriptorInitialize(struct VAS_BLOCK_DESCRIPTOR* self, struct VAS_DESCRIPTOR* owner, VOID* kmem, UINTPTR pageLength, VOID* mapTarget, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes);