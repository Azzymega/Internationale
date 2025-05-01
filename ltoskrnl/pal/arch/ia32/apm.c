#include "apm.h"

#include <hal/hal.h>
#include <rtl/rtl.h>

#include "pali.h"

BOOLEAN PaliApmIsPresent(BYTE version)
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x5300;
    frame.bx = 0;

    PaliX86BiosCall(0x15,&frame);

    if (frame.eflags.carry)
    {
        return FALSE;
    }

    if (frame.ax >= version)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID PaliApmEnable()
{
    PaliApmDisableInterface();
    PaliApmConnectRealModeInterface();
    PaliPromoteInterface(0x101);
}

VOID PaliApmShutdown()
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x5307;
    frame.bx = 0x0001;
    frame.cx = 0x0003;

    PaliX86BiosCall(0x15,&frame);
}

VOID PaliApmDisableInterface()
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x5304;
    frame.bx = 0;

    PaliX86BiosCall(0x15,&frame);
}

VOID PaliApmConnectRealModeInterface()
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x5301;
    frame.bx = 0;

    PaliX86BiosCall(0x15,&frame);
}

VOID PaliPromoteInterface(BYTE version)
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x530E;
    frame.bx = 0;
    frame.cx = version;

    PaliX86BiosCall(0x15,&frame);
}
