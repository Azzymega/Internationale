#include <ltbase.h>
#include <pal/pal.h>
#include <hal/hal.h>
#include <ke/ke.h>
#include <mm/mm.h>
#include <ps/ps.h>

VOID Dump()
{
    struct THREAD* thread;
    PsGetCurrentThread(&thread);

    while (TRUE)
    {
        PalPrint("Allocated memory: %i\r\n",MmGetOccupiedMemory());
        MmAllocatePoolMemory(NON_PAGED_HEAP_ZEROED,PalClock());
        PalYieldToDispatch();
    }
}

VOID KeEntry()
{
    struct THREAD* thread;
    PsGetCurrentThread(&thread);

    for (int i = 0; i < 500; ++i)
    {
        struct PROCESS* process;
        struct THREAD* newThread;

        PsGetCurrentProcess(&process);
        PsCreateThread(&newThread,process,Dump,NULL);
        PsUnlockThread(newThread);
    }

    while (TRUE)
    {
        PalPrint("Time is %i ms\r\n",PalClock());
        PalYieldToDispatch();
    }
}
