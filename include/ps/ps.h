#pragma once
#include <ltbase.h>
#include <ob/object.h>
#include <ps/sched.h>
#include <rtl/list.h>

VOID PsInitialize();
INUSTATUS PsCreateThread(OUT struct KERNEL_THREAD** reference, IN struct KERNEL_PROCESS* process, IN VOID* function, IN VOID* argument);
INUSTATUS PsCreateThreadInCurrentProcess(OUT struct KERNEL_THREAD** reference, IN VOID* function, IN VOID* argument);

INUSTATUS PsUnlockThread(IN struct KERNEL_THREAD* thread);
INUSTATUS PsLockThread(IN struct KERNEL_THREAD* thread);

INUSTATUS PsSleepThread(IN struct KERNEL_THREAD* target, IN UINTPTR count);
INUSTATUS PsSleepCurrentThread(IN UINTPTR count);

INUSTATUS PsGetCurrentThread(OUT struct KERNEL_THREAD** thread);
INUSTATUS PsGetCurrentProcess(OUT struct KERNEL_PROCESS** process);