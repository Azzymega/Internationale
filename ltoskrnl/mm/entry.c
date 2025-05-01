#include <ltbase.h>
#include <hal/hal.h>
#include <mm/mm.h>
#include <mm/pfndb.h>
#include <mm/pool.h>
#include <pal/pal.h>
#include <rtl/rtl.h>

INUGLOBAL struct PFN_DATABASE* MmPfnGlobalDatabase;

INUGLOBAL struct MEMORY_HEAP MmGlobalNonPagedMemoryHeap;
INUGLOBAL struct MEMORY_HEAP MmGlobalPagedMemoryHeap;

VOID MmInitialize(struct PHYSICAL_MEMORY_BLOCK* blocks, UINTPTR count, VOID* location)
{
    MmPfnGlobalDatabase = location;

    PfnDatabaseInitialize(MmPfnGlobalDatabase, blocks, count);

    MemoryHeapInitialize(&MmGlobalNonPagedMemoryHeap, 0, PalGetNonPagedPoolSize());
    MemoryHeapInitialize(&MmGlobalPagedMemoryHeap, 0,0);
}

VOID* MmAllocateNonPagedPoolMemory(UINTPTR pageCount)
{
    VOID* mem;
    if (INU_SUCCESS(PfnDatabaseAllocatePages(MmPfnGlobalDatabase,pageCount,NULL,(VOID*)0x40000000,&mem)))
    {
        return mem + PalGetNonPagedPoolVirtualAddress();
    }
    else
    {
        INU_BUGCHECK("OUT OF MEMORY!");
        return NULL;
    }
}

VOID MmFreeNonPagedPoolMemory(VOID* address, UINTPTR count)
{
    UINTPTR addressValue = (UINTPTR)address;
    UINTPTR pageIndex = addressValue / HalGetPageSize();

    if (INU_FAIL(PfnDatabaseFreePages(MmPfnGlobalDatabase,pageIndex,count)))
    {
        INU_BUGCHECK("FAIL TO FREE MEMORY!");
    }
}

VOID* MmAllocatePoolMemory(enum HEAP_TYPE type, UINTPTR length)
{
    if (type == NON_PAGED_HEAP_ZEROED)
    {
        VOID* allocated = MemoryHeapAllocate(&MmGlobalNonPagedMemoryHeap,length,type);
        RtlZeroMemory(allocated,length);
        return allocated;
    }
    else if (type == NON_PAGED_HEAP)
    {
        VOID* allocated = MemoryHeapAllocate(&MmGlobalNonPagedMemoryHeap,length,type);
        return allocated;
    }
    else if (type == PAGED_HEAP)
    {
        INU_BUGCHECK("BAD POOL TYPE!");
        VOID* allocated = MemoryHeapAllocate(&MmGlobalPagedMemoryHeap,length,type);
        return allocated;
    }
    else
    {
        INU_BUGCHECK("BAD POOL TYPE!");
        return NULL;
    }
}

VOID MmFreePoolMemory(VOID* mem)
{
    MemoryHeapFree(&MmGlobalNonPagedMemoryHeap, mem);
}

INTPTR MmGetTotalMemory()
{
    return MmPfnGlobalDatabase->frameCount * HalGetPageSize();
}

INTPTR MmGetOccupiedMemory()
{
    return MmPfnGlobalDatabase->allocatedCount * HalGetPageSize();
}

INTPTR MmGetFreeMemory()
{
    return MmGetTotalMemory() - MmGetOccupiedMemory();
}
