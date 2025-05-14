#pragma once
#include <intermarx/intermarx.h>
#include <intermarx/ob/ob.h>

struct PaliX86BiosCallFrame;

VOID PalManagedThreadCreate(struct MANAGED_WRAPPER* thread, struct MANAGED_DELEGATE* delegate);
INT32 PalManagedThreadStart(struct MANAGED_WRAPPER* thread);
INT32 PalManagedThreadId();
VOID* PalManagedThreadGetCurrent();

VOID PalX86BiosCall(BYTE interruptIndex, struct PaliX86BiosCallFrame *frame);
VOID* PalObjectToString(struct OBJECT_HEADER* header);
VOID* PalX86GetBiosCallBuffer();
VOID PalBufferMemoryCopy(VOID* destination, VOID* source, UINT32 length);
VOID PalBufferMemorySet(VOID *destination, BYTE value, UINT32 length);
VOID PalBufferMemorySetBlock(VOID *destination, UINT32 value, UINT32 length);

INT64 PalTimerClock();
VOID PalLoggerPrint(const CHAR* message);

struct MANAGED_DELEGATE* PalManagedDelegateCombineImplNative(struct MANAGED_DELEGATE* thisPtr, struct MANAGED_DELEGATE* delegate);
struct MANAGED_DELEGATE* PalManagedDelegateRemoveImplNative(struct MANAGED_DELEGATE* thisPtr, struct MANAGED_DELEGATE* delegate);