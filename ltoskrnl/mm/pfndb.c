#include <mm/pfndb.h>
#include <hal/hal.h>
#include <mm/mm.h>
#include <pal/pal.h>
#include <rtl/rtl.h>

INUSTATUS PfnDatabaseInitialize(struct PFN_DATABASE* self, struct PHYSICAL_MEMORY_BLOCK* blocks, UINTPTR count)
{
    RtlZeroMemory(self,sizeof(struct PFN_DATABASE));

    self->header.type = PFN_DATABASE_TYPE;
    self->frameCount = 0;

    ListEntryInitialize(&self->freePages,NULL);
    ListEntryInitialize(&self->allocatedPages,NULL);
    ListEntryInitialize(&self->reservedPages,NULL);

    UINTPTR pageSize = HalGetPageSize();

    for (int i = 0; i < count; ++i)
    {
        if (blocks[i].type == BLOCK_TYPE_UNUSED)
        {
            continue;
        }
        else
        {
            UINTPTR blockPageStart = blocks[i].start / pageSize;
            UINTPTR blockLength = (blocks[i].end - blocks[i].start) / pageSize;

            self->frameCount += blockLength;

            for (int j = 0; j < blockLength; ++j)
            {
                UINTPTR index = blockPageStart + j;

                ListEntryInitialize(&self->frames[index].entry, &self->frames[index]);

                switch (blocks[i].type)
                {
                case BLOCK_TYPE_UNUSED:
                    {
                        // TODO: THINK ABOUT IT! ListEntryAdd(&self->reservedPages, &self->frames[index].entry);
                        self->frames[index].attributes = PFN_FRAME_RESTRICTED;
                        break;
                    }
                case BLOCK_TYPE_USABLE:
                    {
                        // TODO: THINK ABOUT IT! ListEntryAdd(&self->freePages, &self->frames[index].entry);
                        self->frames[index].attributes = PFN_FRAME_FREE;
                        break;
                    }
                case BLOCK_TYPE_RESERVED:
                    {
                        // TODO: THINK ABOUT IT! ListEntryAdd(&self->reservedPages, &self->frames[index].entry);
                        self->frames[index].attributes = PFN_FRAME_RESTRICTED;
                        break;
                    }
                }
            }
        }
    }

    UINTPTR physicalPfnDatabaseAddress = (UINTPTR)self-PalGetNonPagedPoolVirtualAddress();
    UINTPTR physicalPfnDatabaseSize = sizeof(struct PFN_DATABASE)+self->frameCount*sizeof(struct PFN_FRAME);
    UINTPTR physicalPfnDatabaseAddressPage = physicalPfnDatabaseAddress/HalGetPageSize() - 1;
    UINTPTR physicalPfnDatabaseSizeInPages = physicalPfnDatabaseSize/HalGetPageSize() + 300;

    PfnDatabaseMarkPages(self,physicalPfnDatabaseAddressPage,physicalPfnDatabaseSizeInPages,PFN_FRAME_ALLOCATED);

    for (int i = 0; i < self->frameCount; ++i)
    {
        if (self->frames[i].attributes == PFN_FRAME_NONE)
        {
            PfnDatabaseMarkPage(self,i,PFN_FRAME_RESTRICTED);
        }
    }

    // UINTPTR blockStart = 0;
    // UINTPTR blockEnd = 0;
    // enum PFN_FRAME_ATTRIBUTES attributes = PFN_FRAME_RESTRICTED;
    //
    // for (int i = 0; i < self->frameCount; ++i)
    // {
    //     if (attributes != self->frames[i].attributes)
    //     {
    //         PalPrint("[PFN] BLOCK START: %x, BLOCK END: %x\r\n",blockStart*HalGetPageSize(),blockEnd*HalGetPageSize());
    //
    //         if (RtlHasFlag(attributes,PFN_FRAME_RESTRICTED))
    //         {
    //             PalPrint("%s","PFN_FRAME_RESTRICTED \r\n");
    //         }
    //
    //         if (RtlHasFlag(attributes,PFN_FRAME_FREE))
    //         {
    //             PalPrint("%s","PFN_FRAME_FREE \r\n");
    //         }
    //
    //         if (RtlHasFlag(attributes,PFN_FRAME_ALLOCATED))
    //         {
    //             PalPrint("%s","PFN_FRAME_ALLOCATED \r\n");
    //         }
    //
    //         if (RtlHasFlag(attributes,PFN_FRAME_EXECUTE))
    //         {
    //             PalPrint("%s","PFN_FRAME_EXECUTE \r\n");
    //         }
    //
    //         if (RtlHasFlag(attributes,PFN_FRAME_READ))
    //         {
    //             PalPrint("%s","PFN_FRAME_READ \r\n");
    //         }
    //
    //         if (RtlHasFlag(attributes,PFN_FRAME_WRITE))
    //         {
    //             PalPrint("%s","PFN_FRAME_WRITE \r\n");
    //         }
    //
    //         blockStart = blockEnd;
    //         attributes = self->frames[i].attributes;
    //     }
    //     else
    //     {
    //         blockEnd++;
    //     }
    // }
    //
    // PalPrint("[PFN] BLOCK START: %x, BLOCK END: %x\r\n",blockStart*HalGetPageSize(),blockEnd*HalGetPageSize());
    //
    // if (RtlHasFlag(attributes,PFN_FRAME_RESTRICTED))
    // {
    //     PalPrint("%s","PFN_FRAME_RESTRICTED \r\n");
    // }
    //
    // if (RtlHasFlag(attributes,PFN_FRAME_FREE))
    // {
    //     PalPrint("%s","PFN_FRAME_FREE \r\n");
    // }
    //
    // if (RtlHasFlag(attributes,PFN_FRAME_ALLOCATED))
    // {
    //     PalPrint("%s","PFN_FRAME_ALLOCATED \r\n");
    // }
    //
    // if (RtlHasFlag(attributes,PFN_FRAME_EXECUTE))
    // {
    //     PalPrint("%s","PFN_FRAME_EXECUTE \r\n");
    // }
    //
    // if (RtlHasFlag(attributes,PFN_FRAME_READ))
    // {
    //     PalPrint("%s","PFN_FRAME_READ \r\n");
    // }
    //
    // if (RtlHasFlag(attributes,PFN_FRAME_WRITE))
    // {
    //     PalPrint("%s","PFN_FRAME_WRITE \r\n");
    // }

     return STATUS_SUCCESS;
}

INUSTATUS PfnDatabaseAllocatePages(struct PFN_DATABASE* self, UINTPTR pageCount, VOID* startBoundary, VOID* endBoundary,
                                   VOID** mem)
{
    INU_ASSERT(self);
    INU_ASSERT(mem);

    SpinlockEnter(&self->lock);

    self->allocatedCount += pageCount;
    INTPTR startPage = -1;
    INTPTR allocatedPages = -1;

    if (startBoundary != NULL)
    {
        startPage = (UINTPTR)startBoundary / HalGetPageSize();
    }

    for (int i = 0; i < self->frameCount; ++i)
    {
        if (startPage == -1)
        {
            if (self->frames[i].attributes == PFN_FRAME_FREE)
            {
                startPage = i;
                allocatedPages = 1;
            }
        }
        else
        {
            if (allocatedPages >= pageCount)
            {
                if (startPage > (UINTPTR)endBoundary / HalGetPageSize())
                {
                    SpinlockExit(&self->lock);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                else
                {
                    PfnDatabaseMarkPages(self,
                                         startPage,
                                         pageCount,
                                         PFN_FRAME_EXECUTE | PFN_FRAME_ALLOCATED | PFN_FRAME_READ | PFN_FRAME_WRITE);
                    SpinlockExit(&self->lock);
                    *mem = (VOID*)(startPage * HalGetPageSize());
                    return STATUS_SUCCESS;
                }
            }
            else if (self->frames[i].attributes != PFN_FRAME_FREE)
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

    SpinlockExit(&self->lock);

    *mem = NULL;
    INU_BUGCHECK("OUT OF MEMORY!");
    return STATUS_INSUFFICIENT_RESOURCES;
}

INUSTATUS PfnDatabaseFreePages(struct PFN_DATABASE* self, UINTPTR startPage, UINTPTR pageCount)
{
    SpinlockEnter(&self->lock);

    PfnDatabaseMarkPages(self, startPage, pageCount, PFN_FRAME_FREE);

    SpinlockExit(&self->lock);

    return STATUS_SUCCESS;
}

INUSTATUS PfnDatabaseMarkPage(struct PFN_DATABASE* self, UINTPTR page, enum PFN_FRAME_ATTRIBUTES attributes)
{
    self->frames[page].attributes = attributes;

    return STATUS_SUCCESS;
}

INUSTATUS PfnDatabaseMarkPages(struct PFN_DATABASE* self, UINTPTR pageStart, UINTPTR pageCount,
                          enum PFN_FRAME_ATTRIBUTES attributes)
{
    for (int i = 0; i < pageCount; ++i)
    {
        PfnDatabaseMarkPage(self, pageStart + i, attributes);
    }

    return STATUS_SUCCESS;
}
