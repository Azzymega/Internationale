#include <ltbase.h>
#include <hal/hal.h>
#include <mm/mm.h>
#include <mm/pfndb.h>
#include <mm/pool.h>
#include <rtl/rtl.h>

INUGLOBAL struct PFN_DATABASE MmPfnGlobalDatabase;
INUGLOBAL struct MEMORY_HEAP MmGlobalMemoryHeap;

VOID MmInitialize(VOID* blockStart, UINTPTR blockSize)
{
    PfnDatabaseInitialize(&MmPfnGlobalDatabase,blockStart,blockSize);
    MemoryHeapInitialize(&MmGlobalMemoryHeap);
}

VOID* MmAllocatePhysical(UINTPTR pageCount)
{
    return PfnDatabaseAllocatePages(&MmPfnGlobalDatabase,pageCount);
}

VOID MmFreePhysical(VOID *address, UINTPTR count)
{
    UINTPTR addressValue = (UINTPTR)address;
    UINTPTR pageIndex = addressValue/HalGetPageSize();

    PfnDatabaseFreePages(&MmPfnGlobalDatabase,pageIndex,count);
}

VOID * MmAllocatePoolWithTag(enum POOL_TYPE type, UINTPTR length, UINT32 tag)
{
    if (type == NonPagedPoolZeroed)
    {
        VOID* allocated = MemoryHeapAllocate(&MmGlobalMemoryHeap,length);
        RtlSetMemory(allocated,0,length);
        return allocated;
    }
    else
    {
        return MemoryHeapAllocate(&MmGlobalMemoryHeap,length);
    }
}

VOID MmFreePoolWithTag(VOID *pool, UINT32 tag)
{

}
