#pragma once
#include <ltbase.h>

enum POOL_TYPE {
    UnusedPool,
    NonPagedPool,
    NonPagedPoolExecute,
    NonPagedPoolZeroed,
    NonPagedPoolZeroedExecute,
    NonPagedPoolWriteCombining,
    MaximumPool
};

VOID MmInitialize(VOID* blockStart, UINTPTR blockSize);

VOID* MmAllocatePhysical(UINTPTR pageCount);
VOID MmFreePhysical(VOID* address, UINTPTR count);

VOID* MmAllocatePoolWithTag(enum POOL_TYPE type, UINTPTR length, UINT32 tag);
VOID MmFreePoolWithTag(VOID* pool, UINT32 tag);

INTPTR MmGetTotalMemory();
INTPTR MmGetOccupiedMemory();
INTPTR MmGetFreeMemory();