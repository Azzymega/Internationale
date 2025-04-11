#include <ltbase.h>
#include <fw/fw.h>
#include <hal/hal.h>
#include <ke/ke.h>
#include <mm/mm.h>
#include <ps/ps.h>

VOID hell()
{
    while (TRUE);
}

VOID KeEntry()
{
    while (TRUE)
    {
        struct THREAD* thread;
        PsGetCurrentThread(&thread);

        FwDebugPrint("Time is %i ms\r\n",FwClock());

        FwYieldToDispatch();
    }
}
