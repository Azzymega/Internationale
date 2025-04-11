#pragma once
#include <ltbase.h>
#include <ob/object.h>
#include <ps/sched.h>
#include <rtl/list.h>

VOID PsInitialize();
INUSTATUS PsCreateThread(OUT struct THREAD** reference, IN struct PROCESS* process, IN VOID* function, IN VOID* argument);
INUSTATUS PsCreateThreadInCurrentProcess(OUT struct THREAD** reference, IN VOID* function, IN VOID* argument);

INUSTATUS PsUnlockThread(IN struct THREAD* thread);
INUSTATUS PsLockThread(IN struct THREAD* thread);

INUSTATUS PsSleepThread(IN struct THREAD* target, IN UINTPTR count);
INUSTATUS PsSleepCurrentThread(IN UINTPTR count);

INUSTATUS PsGetCurrentThread(OUT struct THREAD** thread);
INUSTATUS PsGetCurrentProcess(OUT struct PROCESS** process);