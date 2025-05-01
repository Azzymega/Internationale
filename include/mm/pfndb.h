#pragma once
#include <ltbase.h>
#include <ob/objectTypes.h>
#include <ob/object.h>
#include <mm/mm.h>
#include <ps/crit.h>
#include <rtl/list.h>

enum PFN_FRAME_ATTRIBUTES
{
    PFN_FRAME_NONE = 0,
    PFN_FRAME_RESTRICTED = 1,
    PFN_FRAME_FREE       = 2,
    PFN_FRAME_ALLOCATED  = 4,
    PFN_FRAME_EXECUTE    = 8,
    PFN_FRAME_READ       = 16,
    PFN_FRAME_WRITE      = 32,
};

struct PFN_FRAME
{
    enum PFN_FRAME_ATTRIBUTES attributes;
    struct LIST_ENTRY entry;
};

struct PFN_DATABASE
{
    struct OBJECT_HEADER header;
    struct SPINLOCK lock;

    UINTPTR frameCount;
    UINTPTR allocatedCount;

    struct LIST_ENTRY freePages;
    struct LIST_ENTRY allocatedPages;
    struct LIST_ENTRY reservedPages;

    struct PFN_FRAME frames[];
};

INUSTATUS PfnDatabaseInitialize(struct PFN_DATABASE* self, struct PHYSICAL_MEMORY_BLOCK* blocks, UINTPTR count);
INUSTATUS PfnDatabaseAllocatePages(struct PFN_DATABASE* self, UINTPTR pageCount, VOID* startBoundary, VOID* endBoundary, VOID** mem);
INUSTATUS PfnDatabaseFreePages(struct PFN_DATABASE* self, UINTPTR startPage, UINTPTR pageCount);
INUSTATUS PfnDatabaseMarkPages(struct PFN_DATABASE* self, UINTPTR pageStart, UINTPTR pageCount, enum PFN_FRAME_ATTRIBUTES attributes);
INUSTATUS PfnDatabaseMarkPage(struct PFN_DATABASE* self, UINTPTR page, enum PFN_FRAME_ATTRIBUTES attributes);
