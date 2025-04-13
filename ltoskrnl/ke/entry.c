#include <ltbase.h>
#include <fw/fw.h>
#include <hal/hal.h>
#include <ke/ke.h>
#include <mm/mm.h>
#include <ps/ps.h>


VOID KeEntry()
{
    struct THREAD* thread;
    PsGetCurrentThread(&thread);

    while (TRUE)
    {
        FwDebugPrint("Time is %i ms\r\n",FwClock());
        //FwYieldToDispatch();
    }
}
