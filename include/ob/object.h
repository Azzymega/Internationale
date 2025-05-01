#pragma once
#include <ltbase.h>
#include <ob/objectTypes.h>

struct KERNEL_OBJECT_HEADER
{
    INTPTR refCount;
    enum OBJECTS_TYPES type;
};

INUSTATUS ObReferenceObject(OUT REFERENCE* reference, IN VOID* value);
INUSTATUS ObDereferenceObject(IN VOID* reference);