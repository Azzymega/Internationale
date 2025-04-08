#include <mm/pfndb.h>
#include <hal/hal.h>

void PfnDatabaseInitialize(struct PFN_DATABASE *self, void *block, UINTPTR blockSize)
{
    UINTPTR pageSize = HalGetPageSize();
    UINTPTR totalPhysicalSize = blockSize + (UINTPTR) block;
    UINTPTR totalPhysicalPageCount = totalPhysicalSize / pageSize;

    UINTPTR bitmapSize = totalPhysicalPageCount * sizeof(struct PFN_FRAME);
    UINTPTR bitmapCount = bitmapSize / sizeof(struct PFN_FRAME);
    UINTPTR bitmapPageSize = bitmapSize / pageSize + 1;

    self->count = bitmapCount;
    block += bitmapPageSize * pageSize;
    blockSize -= bitmapPageSize * pageSize;

    PfnDatabaseMarkPages(self, 0, totalPhysicalPageCount, PfnFrameRestricted);

    UINTPTR freeMemoryPage = (UINTPTR) block / pageSize;
    UINTPTR freeMemoryPageCount = blockSize / pageSize;
    if ((UINTPTR) block % pageSize != 0)
    {
        freeMemoryPage++;
        freeMemoryPageCount--;
    }

    PfnDatabaseMarkPages(self, freeMemoryPage, freeMemoryPageCount, PfnFrameFree);
}

VOID *PfnDatabaseAllocatePages(struct PFN_DATABASE *self, UINTPTR pageCount)
{
    HalEnterLock(&self->monitor);

    INTPTR startPage = -1;
    INTPTR allocatedPages = -1;

    for (int i = 0; i < self->count; ++i)
    {
        if (startPage == -1)
        {
            if (self->frames[i].attributes == PfnFrameFree)
            {
                startPage = i;
                allocatedPages = 1;
            }
        }
        else
        {
            if (allocatedPages >= pageCount)
            {
                self->allocated += allocatedPages;
                PfnDatabaseMarkPages(self,
                                     startPage,
                                     pageCount,
                                     PfnFrameExecute | PfnFrameAllocated | PfnFrameRead | PfnFrameWrite);
                HalExitLock(&self->monitor);

                return (VOID *) (startPage * HalGetPageSize());
            }
            else if (self->frames[i].attributes != PfnFrameFree)
            {
                startPage = -1;
                allocatedPages = -1;
            }
            else
            {
                allocatedPages++;
            }
        }
    }

    HalExitLock(&self->monitor);

    HalBugcheck("Out of memory!");
    return NULL;
}

VOID PfnDatabaseFreePages(struct PFN_DATABASE *self, UINTPTR startPage, UINTPTR pageCount)
{
    HalEnterLock(&self->monitor);

    PfnDatabaseMarkPages(self, startPage, pageCount, PfnFrameFree);

    self->allocated -= pageCount;

    HalExitLock(&self->monitor);
}

VOID PfnDatabaseMarkPage(struct PFN_DATABASE *self, UINTPTR page, enum PFN_FRAME_ATTRIBUTES attributes)
{
    self->frames[page].attributes = attributes;
}

VOID PfnDatabaseMarkPages(struct PFN_DATABASE *self, UINTPTR pageStart, UINTPTR pageCount,
                          enum PFN_FRAME_ATTRIBUTES attributes)
{
    for (int i = 0; i < pageCount; ++i)
    {
        PfnDatabaseMarkPage(self, pageStart + i, attributes);
    }
}
