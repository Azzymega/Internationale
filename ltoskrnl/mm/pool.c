#include <hal/hal.h>
#include <mm/mm.h>
#include <mm/pfndb.h>
#include <mm/pool.h>
#include <pal/pal.h>

INUIMPORT struct PFN_DATABASE* MmPfnGlobalDatabase;

VOID MemoryPoolInitialize(struct MEMORY_POOL* self, UINTPTR blockSize, UINTPTR pageCount)
{
    self->blockSize = blockSize;
    MemoryPoolFillBuffer(self,MM_POOL_BASE_SIZE);
}

VOID* MemoryPoolAlloc(struct MEMORY_POOL* self)
{
    HalEnterLock(&self->lock);

Start:
    if (self->list == NULL)
    {
        MemoryPoolFillBuffer(self,MM_POOL_BASE_SIZE);
        goto Start;
    }
    else
    {
        struct MEMORY_POOL_BLOCK* block = self->list;
        self->list = block->next;
        VOID* mem = block;

        HalExitLock(&self->lock);

        return mem + sizeof(struct MEMORY_POOL_BLOCK);
    }
}

VOID MemoryPoolFree(struct MEMORY_POOL* self, VOID* ptr)
{
    HalEnterLock(&self->lock);

    struct MEMORY_POOL_BLOCK* block = ptr - sizeof(struct MEMORY_POOL_BLOCK);
    block->next = self->list;
    self->list = block;

    HalExitLock(&self->lock);
}

VOID MemoryPoolFillBuffer(struct MEMORY_POOL* self, UINTPTR pageCount)
{
    VOID* address = MmAllocatePoolMemory(NON_PAGED_HEAP, pageCount * HalGetPageSize());
    UINTPTR iterationCount = pageCount * HalGetPageSize() / (self->blockSize + sizeof(struct MEMORY_POOL_BLOCK));

    for (int i = 0; i < iterationCount; ++i)
    {
        struct MEMORY_POOL_BLOCK* block = address;
        block->size = self->blockSize;
        block->type = POOL_BLOCK_SMALL;

        block->next = self->list;
        self->list = block;

        address += sizeof(struct MEMORY_POOL_BLOCK) + self->blockSize;
    }
}

VOID MemoryHeapInitialize(struct MEMORY_HEAP* self, UINTPTR startBoundary, UINTPTR endBoundary)
{
    self->startBoundary = startBoundary;
    self->endBoundary = endBoundary;

    MemoryPoolInitialize(&self->pools[0], 32,MM_POOL_BASE_SIZE);
    MemoryPoolInitialize(&self->pools[1], 64,MM_POOL_BASE_SIZE);
    MemoryPoolInitialize(&self->pools[2], 128,MM_POOL_BASE_SIZE);
    MemoryPoolInitialize(&self->pools[3], 256,MM_POOL_BASE_SIZE);
    MemoryPoolInitialize(&self->pools[4], 512,MM_POOL_BASE_SIZE);
    MemoryPoolInitialize(&self->pools[5], 1024,MM_POOL_BASE_SIZE);
    MemoryPoolInitialize(&self->pools[6], 2048,MM_POOL_BASE_SIZE);
}

VOID* MemoryHeapAllocate(struct MEMORY_HEAP* self, UINTPTR bytes, enum HEAP_TYPE type)
{
    if (bytes == 0)
    {
        return NULL;
    }
    if (bytes <= MM_MAX_SMALL_SIZE)
    {
        return MemoryHeapAllocSmall(self, bytes);
    }
    else
    {
        if (type == NON_PAGED_HEAP || type == NON_PAGED_HEAP_ZEROED)
        {
            return MemoryHeapAllocBig(self, bytes, PalGetNonPagedPoolVirtualAddress());
        }
        else
        {
            return MemoryHeapAllocBig(self, bytes, PalGetNonPagedPoolPhysicalAddress());
        }
    }
}

VOID MemoryHeapFree(struct MEMORY_HEAP* self, VOID* ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    const struct MEMORY_POOL_BLOCK* block = ptr - sizeof(struct MEMORY_POOL_BLOCK);
    if (block->type == POOL_BLOCK_SMALL)
    {
        const INTPTR index = MemoryHeapFindPoolIndex(self, block->size);
        MemoryPoolFree(&self->pools[index], ptr);
    }
    else if (block->type == POOL_BLOCK_LARGE)
    {
        PfnDatabaseFreePages(MmPfnGlobalDatabase, (UINTPTR)ptr / HalGetPageSize(), block->size / HalGetPageSize());
    }
    else
    {
        INU_BUGCHECK("Heap free failed!");
    }
}

VOID* MemoryHeapAllocBig(struct MEMORY_HEAP* self, UINTPTR byteCount, UINTPTR fixupAddress)
{
    INU_ASSERT(self);

    const UINTPTR pageCount = byteCount / HalGetPageSize() + 2;
    struct MEMORY_POOL_BLOCK* block = NULL;
    VOID* mem = NULL;

    PfnDatabaseAllocatePages(MmPfnGlobalDatabase, pageCount, (VOID*)self->startBoundary, (VOID*)self->endBoundary,
                             (VOID**)&block);

    mem = block;
    mem += fixupAddress;
    mem += HalGetPageSize() - sizeof(struct MEMORY_POOL_BLOCK);
    block = mem;
    block->size = pageCount * HalGetPageSize();
    block->type = POOL_BLOCK_LARGE;
    return mem + sizeof(struct MEMORY_POOL_BLOCK);
}

VOID* MemoryHeapAllocSmall(struct MEMORY_HEAP* self, UINTPTR byteCount)
{
    const INTPTR index = MemoryHeapFindPoolIndex(self, byteCount);

    if (index == -1)
    {
        return NULL;
    }
    else
    {
        return MemoryPoolAlloc(&self->pools[index]);
    }
}

INTPTR MemoryHeapFindPoolIndex(const struct MEMORY_HEAP* self, const UINTPTR byteCount)
{
    int index = -1;
    for (int i = 0; i < MM_POOL_COUNT; i++)
    {
        if (byteCount <= self->pools[i].blockSize)
        {
            index = i;
            break;
        }
    }
    return index;
}
