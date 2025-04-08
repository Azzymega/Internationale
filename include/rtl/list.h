#pragma once
#include <ltbase.h>

struct LIST_ENTRY
{
    VOID* owner;
    struct LIST_ENTRY *next;
    struct LIST_ENTRY *prev;
};

VOID ListEntryInitialize(struct LIST_ENTRY* self, VOID* owner);
VOID ListEntryAdd(struct LIST_ENTRY* self, struct LIST_ENTRY* node);
VOID ListEntryAddBack(struct LIST_ENTRY* self, struct LIST_ENTRY* node);
VOID ListEntryRemove(struct LIST_ENTRY* self);
VOID* ListEntryNext(struct LIST_ENTRY* self);
