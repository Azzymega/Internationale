#pragma once
#include <ltbase.h>
#include <mm/vad.h>
#include <ps/sched.h>

enum PROCESS_MODE
{
    PROCESS_UNKNOWN,
    PROCESS_KERNEL,
    PROCESS_USER
};

struct PROCESS
{
    struct OBJECT_HEADER header;

    UINTPTR id;

    enum PROCESS_MODE mode;
    struct LIST_ENTRY processes;
    struct LIST_ENTRY scheduableObjects;
    struct LIST_ENTRY objects;

    struct VAS_DESCRIPTOR* vasDescriptor;
};

struct PROCESS* ProcessAllocate();
VOID ProcessInitialize(struct PROCESS* self, struct VAS_DESCRIPTOR* vas, enum PROCESS_MODE mode);
VOID ProcessRemoveSchedulableObject(struct PROCESS* self, struct THREAD* object);
VOID ProcessAddSchedulableObject(struct PROCESS* self, struct THREAD* object);
