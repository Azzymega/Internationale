#include <ltbase.h>
#include <system.h>
#include <pal/pal.h>
#include <hal/hal.h>
#include <ke/ke.h>
#include <mm/mm.h>
#include <ps/ps.h>

INUIMPORT VOID MxStart(VOID* buffer);

VOID KeEntry()
{
    BYTE* buffer = __System_vkp;
    MxStart(buffer);

    while (TRUE);
}
