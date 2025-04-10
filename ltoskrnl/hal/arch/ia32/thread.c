#include <ltbase.h>
#include <hal/hal.h>

INUSTATUS PsCreateThread(REFERENCE* reference, struct PROCESS* process, VOID* function, VOID* argument)
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

    REFERENCE target;
    ObReferenceObject(&target,thread);
    *reference = target;

    return STATUS_SUCCESS;
}
