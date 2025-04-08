#include <ltbase.h>
#include <hal/hal.h>
#include <rtl/rtl.h>

VOID RtlCopyMemory(VOID *destination, VOID *source, UINTPTR length)
{
    HalCopyMemory(destination, source, length);
}

VOID RtlMoveMemory(VOID *destination, VOID *source, UINTPTR length)
{
    HalMoveMemory(destination,source,length);
}

VOID RtlSetMemory(VOID *destination, UINTPTR target, UINTPTR length)
{
    HalSetMemory(destination, target, length);
}
