#include <stdatomic.h>
#include <hal/hal.h>
#include <intermarx/pal/pal.h>
#include <intermarx/ex/runtime.h>
#include <intermarx/mt/mt.h>
#include <mm/mm.h>
#include <ps/crit.h>
#include <ps/ps.h>
#include <rtl/rtl.h>

INUGLOBAL BOOLEAN PalGlobalMaintainence;
INUGLOBAL MONITOR PalGlobalMaintainenceBump;

extern VOID MxDumpStackTrace(struct EXCEPTION_STATE* state);

VOID PalEnterLock(INUVOLATILE BOOLEAN* monitor)
{
    while (atomic_flag_test_and_set_explicit((struct atomic_flag*)monitor, memory_order_acquire))
    {
        asm("pause");
    }
}

VOID PalExitLock(INUVOLATILE BOOLEAN* monitor)
{
    atomic_flag_clear_explicit((struct atomic_flag*)monitor, memory_order_release);
}

BOOLEAN PalTryEnterLock(INUVOLATILE BOOLEAN* monitor)
{
    return !atomic_flag_test_and_set_explicit((struct atomic_flag*)monitor, memory_order_acquire);
}

void RuntimePalInitialize()
{
    PalMonitorInitialize(&PalGlobalMaintainenceBump);
    PalGlobalMaintainence = FALSE;
}

void PalDebugbreak(const CHAR *message)
{
    INU_BUGCHECK(message);
}

UINTPTR PalThreadBootstrap(void *arg)
{
    struct RUNTIME_FRAME_BLOCK ret;
    struct RUNTIME_FRAME_BLOCK delegate;

    delegate.type = MACHINE_OBJECT;
    delegate.descriptor = arg;

    if (MARX_SUCCESS(ExMethodPrologueDelegate(&delegate,NULL,NULL,&ret)))
    {
        return ret.int32;
    }
    else
    {
        MxDumpStackTrace(&MtThreadGetCurrent()->state);
    }

    return ret.int32;
}

VOID * PalMemoryAllocate(UINTPTR count)
{
    return MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED,count);
}

VOID PalMemoryFree(VOID *memory)
{
    MmFreePoolMemory(memory);
}

void PalMemoryCopy(void *dest, void *src, UINTPTR length)
{
    HalCopyMemory(dest,src,length);
}

void PalMemoryZero(void *dest, UINTPTR length)
{
    HalSetMemory(dest,0,length);
}

void PalMemoryZeroBlock(void *dest, UINTPTR elementCount, UINTPTR elementLength)
{
    PalMemoryZero(dest,elementCount*elementLength);
}

UINTPTR PalStringLength(const CHAR *message)
{
    return strlen(message);
}

UINTPTR PalWStringLength(const WCHAR *message)
{
    const WCHAR *start = message;

    while (*message != L'\0') {
        message++;
    }

    return message - start;
}

NATIVE_HANDLE PalThreadGetCurrentHandle()
{
    struct KERNEL_THREAD* thread;
    PsGetCurrentThread(&thread);

    return (NATIVE_HANDLE)thread;
}

UINTPTR PalThreadGetCurrentId()
{
    struct KERNEL_THREAD* thread;
    PsGetCurrentThread(&thread);

    return thread->id;
}

NATIVE_HANDLE PalThreadCreate(THREAD_LAUNCH delegate, VOID *argument)
{
    struct KERNEL_THREAD* thread;
    PsCreateThreadInCurrentProcess(&thread,delegate,argument);
    return (NATIVE_HANDLE)thread;
}

VOID PalThreadResume(NATIVE_HANDLE thread)
{
    struct KERNEL_THREAD* threadInfo = (struct KERNEL_THREAD*)thread;
    PsUnlockThread(threadInfo);
}

VOID PalThreadSuspend(NATIVE_HANDLE thread)
{
    struct KERNEL_THREAD* threadInfo = (struct KERNEL_THREAD*)thread;
    PsLockThread(threadInfo);
}

UINTPTR PalThreadGetId(NATIVE_HANDLE thread)
{
    struct KERNEL_THREAD* threadInfo = (struct KERNEL_THREAD*)thread;
    return threadInfo->id;
}

void PalEnterMaintainenceMode()
{
    PalMonitorEnter(&PalGlobalMaintainenceBump);

    PalGlobalMaintainence = TRUE;
}

void PalExitMaintainenceMode()
{
    PalMonitorExit(&PalGlobalMaintainenceBump);

    PalGlobalMaintainence = FALSE;
}

void PalSafepoint()
{
    if (PalGlobalMaintainence)
    {
        struct RUNTIME_THREAD* current = MtThreadGetCurrent();
        current->inSafePoint = TRUE;

        PalMonitorEnter(&PalGlobalMaintainenceBump);
        PalMonitorExit(&PalGlobalMaintainenceBump);

        current->inSafePoint = FALSE;
    }
}

void PalThreadExitCurrent(UINTPTR code)
{
    while (TRUE);
}

void PalMonitorInitialize(MONITOR *monitor)
{
    CriticalSectionInitialize(monitor);
}

void PalMonitorEnter(MONITOR *monitor)
{
    CriticalSectionEnter(monitor);
}

void PalMonitorExit(MONITOR *monitor)
{
    CriticalSectionExit(monitor);
}
