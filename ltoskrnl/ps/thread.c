#include <ltbase.h>
#include <pal/pal.h>
#include <hal/hal.h>
#include <ps/cpu.h>

INUSTATUS PsCreateThread(struct KERNEL_THREAD** reference, struct KERNEL_PROCESS* process, VOID* function, VOID* argument)
{
    INU_ASSERT(process);
    INU_ASSERT(function);

    if (process == NULL || function == NULL)
    {
        return STATUS_FAIL;
    }

    struct KERNEL_THREAD* thread = ThreadAllocate();

    ThreadInitialize(thread,process);
    ThreadLoad(thread,process,function,argument);
    ProcessAddSchedulableObject(process,thread);

    *reference = thread;

    return STATUS_SUCCESS;
}

INUSTATUS PsCreateThreadInCurrentProcess(struct KERNEL_THREAD** reference, VOID* function, VOID* argument)
{
    struct KERNEL_PROCESS* process;
    PsGetCurrentProcess(&process);

    INU_ASSERT(process);
    INU_ASSERT(function);

    if (process == NULL || function == NULL)
    {
        return STATUS_FAIL;
    }

    struct KERNEL_THREAD* thread = ThreadAllocate();

    ThreadInitialize(thread,process);
    ThreadLoad(thread,process,function,argument);
    ProcessAddSchedulableObject(process,thread);

    *reference = thread;

    return STATUS_SUCCESS;
}

INUSTATUS PsUnlockThread(struct KERNEL_THREAD* thread)
{
    INU_ASSERT(thread);

    if (thread == NULL)
    {
        return STATUS_FAIL;
    }

    ThreadUnlock(thread);

    return STATUS_SUCCESS;
}

INUSTATUS PsLockThread(struct KERNEL_THREAD* thread)
{
    INU_ASSERT(thread);

    if (thread == NULL)
    {
        return STATUS_FAIL;
    }

    ThreadLock(thread);

    return STATUS_SUCCESS;
}

INUSTATUS PsSleepThread(struct KERNEL_THREAD* target, UINTPTR count)
{
    ThreadSleep(target,count);
    return STATUS_SUCCESS;
}

INUSTATUS PsSleepCurrentThread(UINTPTR count)
{
    struct KERNEL_THREAD* current;
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

INUSTATUS PsGetCurrentThread(struct KERNEL_THREAD** thread)
{
    INU_ASSERT(thread);

    struct KERNEL_THREAD* current = PalGetCurrentCpuDescriptor()->schedulableObject;

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

INUSTATUS PsGetCurrentProcess(struct KERNEL_PROCESS** process)
{
    INU_ASSERT(process);

    if (PalGetCurrentCpuDescriptor()->schedulableObject == NULL)
    {
        INU_BUGCHECK("Corrupted state!");
        return STATUS_FAIL;
    }
    else
    {
        struct KERNEL_PROCESS* current = PalGetCurrentCpuDescriptor()->schedulableObject->owner;
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
