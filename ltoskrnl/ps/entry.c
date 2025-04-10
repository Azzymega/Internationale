#include <fw/fw.h>
#include <hal/hal.h>
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
    while (TRUE)
    {
        FwDebugPrint("Time is %i ms\r\n",FwClock());
    }
}

VOID PsiSchedule(VOID* trapFrame)
{
    struct THREAD* currentThread = FwGetCurrentCpuDescriptor()->schedulableObject;
    struct LIST_ENTRY* threadList = &PsGlobalSchedulableObjectCollection;
    struct THREAD* targetThread = NULL;

    INU_ASSERT(currentThread);

    HalSaveState(currentThread,trapFrame);

    targetThread = ListEntryNext(currentThread->schedulableCollection.next);

    Select:

    targetThread = ListEntryNext(targetThread->schedulableCollection.next);

    INU_ASSERT(targetThread);
    INU_ASSERT(threadList);

    if (targetThread->state != SCHEDULABLE_OBJECT_READY)
    {
        goto Select;
    }

    FwGetCurrentCpuDescriptor()->schedulableObject = targetThread;

    HalSwitchState(targetThread,trapFrame);
}

VOID PsiInitializeIdleThread()
{
    struct THREAD *thread = ThreadAllocate();
    ThreadInitialize(thread,&PsGlobalKernelProcess);
    ThreadLoad(thread,&PsGlobalKernelProcess,PsiIdle,NULL);
    ProcessAddSchedulableObject(&PsGlobalKernelProcess,thread);

    FwGetCurrentCpuDescriptor()->controlLevel = 0;
    FwGetCurrentCpuDescriptor()->schedulableObject = thread;

    ThreadUnlock(thread);
    HalJumpInKernelThread(thread);
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
