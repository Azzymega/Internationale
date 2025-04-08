#pragma once
#include <ltbase.h>

VOID RtlCopyMemory(VOID *destination, VOID *source, UINTPTR length);
VOID RtlMoveMemory(VOID *destination, VOID *source, UINTPTR length);
VOID RtlSetMemory(VOID *destination, UINTPTR target, UINTPTR length);
