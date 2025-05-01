#pragma once
#include <intermarx/intermarx.h>
#include <intermarx/ob/ob.h>

VOID PalManagedThreadCreate(struct MANAGED_WRAPPER* thread, struct MANAGED_DELEGATE* delegate);
INT32 PalManagedThreadStart(struct MANAGED_WRAPPER* thread);
INT32 PalManagedThreadId();
VOID* PalManagedThreadGetCurrent();

INT64 PalTimerClock();
VOID PalLoggerPrint(const CHAR* message);

struct MANAGED_DELEGATE* PalManagedDelegateCombineImplNative(struct MANAGED_DELEGATE* thisPtr, struct MANAGED_DELEGATE* delegate);
struct MANAGED_DELEGATE* PalManagedDelegateRemoveImplNative(struct MANAGED_DELEGATE* thisPtr, struct MANAGED_DELEGATE* delegate);