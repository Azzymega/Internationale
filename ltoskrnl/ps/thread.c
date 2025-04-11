#include <ltbase.h>
#include <fw/fw.h>
#include <hal/hal.h>
#include <ps/cpu.h>

INUSTATUS PsCreateThread(struct THREAD** reference, struct PROCESS* process, VOID* function, VOID* argument)
{
    INU_ASSERT(process);
    INU_ASSERT(function);

    if (process == NULL || function == NULL)
    {
        return STATUS_FAIL;
    }

    struct THREAD* thread = ThreadAllocate();

    ThreadInitialize(thread,process);
    ThreadLoad(thread,process,function,argument);
    ProcessAddSchedulableObject(process,thread);

    *reference = thread;

    return STATUS_SUCCESS;
}

INUSTATUS PsCreateThreadInCurrentProcess(struct THREAD** reference, VOID* function, VOID* argument)
{
    struct PROCESS* process;
    PsGetCurrentProcess(&process);

    INU_ASSERT(process);
    INU_ASSERT(function);

    if (process == NULL || function == NULL)
    {
        return STATUS_FAIL;
    }

    struct THREAD* thread = ThreadAllocate();

    ThreadInitialize(thread,process);
    ThreadLoad(thread,process,function,argument);
    ProcessAddSchedulableObject(process,thread);

    *reference = thread;

    return STATUS_SUCCESS;
}

INUSTATUS PsUnlockThread(struct THREAD* thread)
{
    INU_ASSERT(thread);

    if (thread == NULL)
    {
        return STATUS_FAIL;
    }

    ThreadUnlock(thread);

    return STATUS_SUCCESS;
}

INUSTATUS PsLockThread(struct THREAD* thread)
{
    INU_ASSERT(thread);

    if (thread == NULL)
    {
        return STATUS_FAIL;
    }

    ThreadLock(thread);

    return STATUS_SUCCESS;
}

INUSTATUS PsSleepThread(struct THREAD* target, UINTPTR count)
{
    ThreadSleep(target,count);
    return STATUS_SUCCESS;
}

INUSTATUS PsSleepCurrentThread(UINTPTR count)
{
    struct THREAD* current;
    if (INU_SUCCESS(PsGetCurrentThread(&current)))
    {
        PsSleepThread(current,count);
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_FAIL;
    }
}

INUSTATUS PsGetCurrentThread(struct THREAD** thread)
{
    INU_ASSERT(thread);

    struct THREAD* current = FwGetCurrentCpuDescriptor()->schedulableObject;

    if (thread == NULL)
    {
        INU_BUGCHECK("Corrupted state!");
        return STATUS_FAIL;
    }
    else
    {
        *thread = current;
        return STATUS_SUCCESS;
    }
}

INUSTATUS PsGetCurrentProcess(struct PROCESS** process)
{
    INU_ASSERT(process);

    if (FwGetCurrentCpuDescriptor()->schedulableObject == NULL)
    {
        INU_BUGCHECK("Corrupted state!");
        return STATUS_FAIL;
    }
    else
    {
        struct PROCESS* current = FwGetCurrentCpuDescriptor()->schedulableObject->owner;
        if (current == NULL)
        {
            return STATUS_FAIL;
        }
        else
        {
            *process = current;
            return STATUS_SUCCESS;
        }
    }
}
