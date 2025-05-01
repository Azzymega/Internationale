#pragma once
#include <intermarx/intermarx.h>
#include <ps/crit.h>

#define PalStackAllocate(length) __builtin_alloca(length)
#define PalStackBufferAllocate(elementCount,elementLength) __builtin_alloca(elementCount*elementLength)

typedef UINTPTR (*THREAD_LAUNCH)(void *);
typedef struct CRITICAL_SECTION MONITOR;

void RuntimePalInitialize();

void PalDebugbreak(const CHAR* message);
UINTPTR PalThreadBootstrap(VOID* arg);

void* PalMemoryAllocate(UINTPTR count);
void PalMemoryFree(void* memory);

void PalMemoryCopy(void* dest, void* src, UINTPTR length);
void PalMemoryZero(void* dest, UINTPTR length);
void PalMemoryZeroBlock(void* dest, UINTPTR elementCount, UINTPTR elementLength);

UINTPTR PalStringLength(const CHAR* message);
UINTPTR PalWStringLength(const WCHAR* message);

NATIVE_HANDLE PalThreadGetCurrentHandle();
UINTPTR PalThreadGetCurrentId();
NATIVE_HANDLE PalThreadCreate(THREAD_LAUNCH delegate, void* argument);
void PalThreadExitCurrent(UINTPTR code);
void PalThreadResume(NATIVE_HANDLE thread);
void PalThreadSuspend(NATIVE_HANDLE thread);
UINTPTR PalThreadGetId(NATIVE_HANDLE thread);

void PalEnterMaintainenceMode();
void PalExitMaintainenceMode();
void PalSafepoint();

void PalMonitorInitialize(MONITOR* monitor);
void PalMonitorEnter(MONITOR* monitor);
void PalMonitorExit(MONITOR* monitor);

BOOLEAN PalTryEnterLock(INUVOLATILE BOOLEAN* monitor);
VOID PalExitLock(INUVOLATILE BOOLEAN* monitor);
VOID PalEnterLock(INUVOLATILE BOOLEAN* monitor);