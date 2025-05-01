#pragma once
#include <ltbase.h>

enum HEAP_TYPE {
    NON_PAGED_HEAP,
    NON_PAGED_HEAP_ZEROED,
    PAGED_HEAP,
};

enum PHYSICAL_MEMORY_BLOCK_TYPE
{
    BLOCK_TYPE_UNUSED,
    BLOCK_TYPE_USABLE,
    BLOCK_TYPE_RESERVED
};

struct PHYSICAL_MEMORY_BLOCK
{
    UINTPTR start;
    UINTPTR length;
    UINTPTR end;
    enum PHYSICAL_MEMORY_BLOCK_TYPE type;
};

VOID MmInitialize(struct PHYSICAL_MEMORY_BLOCK* blocks, UINTPTR count, VOID* location);

VOID* MmAllocatePoolMemory(enum HEAP_TYPE type, UINTPTR length);
VOID MmFreePoolMemory(VOID* mem);

INTPTR MmGetTotalMemory();
INTPTR MmGetOccupiedMemory();
INTPTR MmGetFreeMemory();