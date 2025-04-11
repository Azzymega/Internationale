#include <fw/fw.h>
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

INUGLOBAL struct PROCESS PsGlobalKernelProcess;
INUGLOBAL struct VAS_DESCRIPTOR PsGlobalKernelVas;

VOID PsiInitializeKernelProcess()
{
    VasDescriptorInitialize(&PsGlobalKernelVas);
    VasDescriptorMapMemory(&PsGlobalKernelVas,(VOID*)4096,(VOID*)4096,2147483648,VAS_DESCRIPTOR_SU | VAS_DESCRIPTOR_RW);
    ProcessInitialize(&PsGlobalKernelProcess,&PsGlobalKernelVas,PROCESS_KERNEL);
}

VOID PsiIdle()
{
    struct THREAD* thread;
    PsCreateThread(&thread,&PsGlobalKernelProcess,KeEntry,NULL);
    PsUnlockThread(thread);

    while (TRUE)
    {
        FwYieldToDispatch();
    }
}

VOID PsiSchedule(VOID* trapFrame)
{
    struct THREAD* currentThread;
    PsGetCurrentThread(&currentThread);

    struct LIST_ENTRY* threadList = &PsGlobalSchedulableObjectCollection;
    struct THREAD* targetThread = NULL;

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
            const UINTPTR clock = FwClock();

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

    FwGetCurrentCpuDescriptor()->schedulableObject = targetThread;

    HalSwitchState(targetThread,trapFrame);
}

VOID PsiInitializeIdleThread()
{
    struct THREAD *thread = NULL;

    INUSTATUS res =  PsCreateThread(&thread,&PsGlobalKernelProcess,PsiIdle,NULL);

    if (!INU_SUCCESS(res))
    {
        INU_BUGCHECK("Failed to initialize idle thread!");
    }
    else
    {
        FwGetCurrentCpuDescriptor()->controlLevel = 0;
        FwGetCurrentCpuDescriptor()->schedulableObject = thread;

        PsUnlockThread(thread);
        HalJumpInKernelThread(thread);
    }
}

VOID PsiThreadPrologue(VOID *arg, VOID *func)
{
    THREAD_START entry = func;

    INU_ASSERT(func != NULL);

    int retCode = entry(arg);

    FwGetCurrentCpuDescriptor()->schedulableObject->returnCode = retCode;
    FwGetCurrentCpuDescriptor()->schedulableObject->state = SCHEDULABLE_OBJECT_EXITED;

    INU_BUGCHECK("THREAD DEATH IS NOT IMPLEMENTED!");

    while (TRUE);
}

VOID PsInitialize()
{
    ListEntryInitialize(&PsGlobalSchedulableObjectCollection,NULL);
    ListEntryInitialize(&PsGlobalProcessCollection,NULL);

    FwSetScheduler(PsiSchedule);

    PsiInitializeKernelProcess();
    PsiInitializeIdleThread();
}
