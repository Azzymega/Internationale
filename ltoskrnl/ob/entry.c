#include <ltbase.h>
#include <ob/object.h>

INUSTATUS ObReferenceObject(OUT REFERENCE* reference, IN VOID* value)
{
    struct KERNEL_OBJECT_HEADER* header = value;
    header->refCount++;
    *reference = header;
    return STATUS_SUCCESS;
}

INUSTATUS ObDereferenceObject(IN VOID* reference)
{
    struct KERNEL_OBJECT_HEADER* header = reference;
    header->refCount--;
    return STATUS_SUCCESS;
}
