#pragma once
#include <ltbase.h>
#include <ob/objectTypes.h>
#include <ob/object.h>

enum PFN_FRAME_ATTRIBUTES : BYTE
{
    PfnFrameRestricted = 0,
    PfnFrameFree       = 1,
    PfnFrameAllocated  = 2,
    PfnFrameExecute    = 4,
    PfnFrameRead       = 8,
    PfnFrameWrite      = 16,
};

struct PFN_FRAME
{
    enum PFN_FRAME_ATTRIBUTES attributes;
};

struct PFN_DATABASE
{
    struct OBJECT_HEADER header;

    BOOLEAN monitor;
    UINTPTR allocated;
    UINTPTR count;

    struct PFN_FRAME frames[];
};

VOID PfnDatabaseInitialize(struct PFN_DATABASE* self, VOID* block, UINTPTR blockSize);
VOID* PfnDatabaseAllocatePages(struct PFN_DATABASE* self, UINTPTR pageCount);
VOID PfnDatabaseFreePages(struct PFN_DATABASE* self, UINTPTR startPage, UINTPTR pageCount);
VOID PfnDatabaseMarkPages(struct PFN_DATABASE* self, UINTPTR pageStart, UINTPTR pageCount, enum PFN_FRAME_ATTRIBUTES attributes);
VOID PfnDatabaseMarkPage(struct PFN_DATABASE* self, UINTPTR page, enum PFN_FRAME_ATTRIBUTES attributes);
