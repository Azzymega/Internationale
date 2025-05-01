#include <pal/pal.h>
#include <hal/hal.h>
#include <ke/ke.h>
#include <ps/ps.h>
#include <ps/sched.h>
#include <ps/cpu.h>
#include <ps/proc.h>
#include <rtl/rtl.h>

#include "psi.h"

INUGLOBAL struct LIST_ENTRY PsGlobalSchedulableObjectCollection;
INUGLOBAL struct LIST_ENTRY PsGlobalProcessCollection;

INUGLOBAL struct KERNEL_PROCESS PsGlobalKernelProcess;
INUGLOBAL struct VAS_DESCRIPTOR PsGlobalKernelVas;

VOID PsiInitializeKernelProcess()
{
    VasDescriptorInitialize(&PsGlobalKernelVas);
    ProcessInitialize(&PsGlobalKernelProcess,&PsGlobalKernelVas,PROCESS_KERNEL);
    VasDescriptorMapMemory(&PsGlobalKernelVas,(VOID*)0x80000000,(VOID*)0,1073741824,VAS_DESCRIPTOR_SU | VAS_DESCRIPTOR_RW);
    VasDescriptorMapMemory(&PsGlobalKernelVas,(VOID*)0,(VOID*)0,4194304,VAS_DESCRIPTOR_SU | VAS_DESCRIPTOR_RW);
}

VOID PsiIdle()
{
    struct KERNEL_THREAD* thread;
    PsCreateThread(&thread,&PsGlobalKernelProcess,KeEntry,NULL);
    PsUnlockThread(thread);

    while (TRUE)
    {
        PalYieldToDispatch();
    }
}

VOID PsiSchedule(VOID* trapFrame)
{
    struct KERNEL_THREAD* currentThread;
    PsGetCurrentThread(&currentThread);

    struct LIST_ENTRY* threadList = &PsGlobalSchedulableObjectCollection;
    struct KERNEL_THREAD* targetThread = NULL;

    INU_ASSERT(currentThread);

    HalSaveState(currentThread,trapFrame);

    targetThread = currentThread->schedulableCollection.owner;

    Select:

    targetThread = ListEntryNext(&targetThread->schedulableCollection);

    INU_ASSERT(targetThread);
    INU_ASSERT(threadList);

    if (targetThread->state != SCHEDULABLE_OBJECT_READY)
    {
        if (targetThread->isSleep == TRUE)
        {
            const UINTPTR clock = PalClock();

            if (targetThread->sleepStart + targetThread->sleepLength < clock)
            {
                targetThread->isSleep = FALSE;
                PsUnlockThread(targetThread);
            }
        }
        else
        {

        }

        goto Select;
    }

    PalGetCurrentCpuDescriptor()->schedulableObject = targetThread;

    HalSwitchState(targetThread,trapFrame);
}

VOID PsiInitializeIdleThread()
{
    struct KERNEL_THREAD *thread = NULL;
    INUSTATUS res =  PsCreateThread(&thread,&PsGlobalKernelProcess,PsiIdle,NULL);

    if (INU_FAIL(res))
    {
        INU_BUGCHECK("Failed to initialize idle thread!");
    }
    else
    {
        PalGetCurrentCpuDescriptor()->controlLevel = 0;
        PalGetCurrentCpuDescriptor()->schedulableObject = thread;

        PsUnlockThread(thread);
        HalJumpInKernelThread(thread);
    }
}

VOID PsiThreadPrologue(VOID *arg, VOID *func)
{
    THREAD_START entry = func;

    INU_ASSERT(func != NULL);

    int retCode = entry(arg);

    PalGetCurrentCpuDescriptor()->schedulableObject->returnCode = retCode;
    PalGetCurrentCpuDescriptor()->schedulableObject->state = SCHEDULABLE_OBJECT_EXITED;

    INU_BUGCHECK("KERNEL_THREAD DEATH IS NOT IMPLEMENTED!");

    while (TRUE);
}

VOID PsInitialize()
{
    ListEntryInitialize(&PsGlobalSchedulableObjectCollection,NULL);
    ListEntryInitialize(&PsGlobalProcessCollection,NULL);

    PalSetScheduler(PsiSchedule);

    PsiInitializeKernelProcess();
    PsiInitializeIdleThread();
}
