#include <rtl/list.h>

VOID ListEntryInitialize(struct LIST_ENTRY* self, VOID* owner)
{
    self->owner = owner;
    self->next = self;
    self->prev = self;
}

VOID ListEntryAdd(struct LIST_ENTRY* self, struct LIST_ENTRY* node)
{
    node->next = self->next;
    node->prev = self;
    self->next->prev = node;
    self->next = node;
}

VOID ListEntryAddBack(struct LIST_ENTRY* self, struct LIST_ENTRY* node)
{
    node->next = self;
    node->prev = self->prev;
    self->prev->next = node;
    self->prev = node;
}

VOID ListEntryRemove(struct LIST_ENTRY* self)
{
    self->prev->next = self->next;
    self->next->prev = self->prev;
    self->next = self;
    self->prev = self;
}

VOID* ListEntryNext(struct LIST_ENTRY* self)
{
    if (self->next->owner == NULL)
    {
        return self->next->next->owner;
    }
    else
    {
        return self->next->owner;
    }
}