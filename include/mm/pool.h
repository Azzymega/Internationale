#pragma once
#include <ltbase.h>
#include <ob/object.h>

#define MM_MAX_SMALL_SIZE 2048
#define MM_ALIGNMENT 8
#define MM_POOL_COUNT 7
#define MM_POOL_BASE_SIZE 16

enum MEMORY_POOL_TYPES {
    MmUnusedPool,
    MmNonPagedPool,
    MmNonPagedPoolExecute,
    MmNonPagedPoolZeroed,
    MmNonPagedPoolZeroedExecute,
    MmNonPagedPoolWriteCombining,
    MmMaximumPool
};

enum MEMORY_POOL_BLOCK_TYPES {
    POOL_BLOCK_UNKNOWN,
    POOL_BLOCK_SMALL,
    POOL_BLOCK_LARGE
};

struct MEMORY_POOL_BLOCK
{
    enum MEMORY_POOL_BLOCK_TYPES type;
    UINTPTR size;
    UINTPTR reserved;
    struct MEMORY_POOL_BLOCK* next;
};

struct MEMORY_POOL
{
    UINTPTR blockSize;
    BOOLEAN lock;
    BOOLEAN marked;
    struct MEMORY_POOL_BLOCK* list;
};

struct MEMORY_HEAP
{
    struct OBJECT_HEADER header;

    struct MEMORY_POOL pools[MM_POOL_COUNT];
};

VOID MemoryPoolInitialize(struct MEMORY_POOL* self, UINTPTR blockSize, UINTPTR pageCount);
VOID* MemoryPoolAlloc(struct MEMORY_POOL* self);
VOID MemoryPoolFree(struct MEMORY_POOL* self, VOID* ptr);
VOID MemoryPoolFillBuffer(struct MEMORY_POOL* self, UINTPTR pageCount);

VOID MemoryHeapInitialize(struct MEMORY_HEAP* self);
VOID* MemoryHeapAllocate(struct MEMORY_HEAP* self, UINTPTR bytes);
VOID MemoryHeapFree(struct MEMORY_HEAP* self, VOID* ptr);
VOID* MemoryHeapAllocBig(struct MEMORY_HEAP *self, UINTPTR byteCount);
VOID* MemoryHeapAllocSmall(struct MEMORY_HEAP *self, UINTPTR byteCount);
INTPTR MemoryHeapFindPoolIndex(const struct MEMORY_HEAP *self, UINTPTR byteCount);

