#include <ltbase.h>
#include <hal/hal.h>
#include <rtl/rtl.h>

#include "fwi.h"

INUEXTERN VOID PaliX86BiosSwitchEnd();
VOID (*PaliX86BiosSwitchReal)(BYTE, VOID*);
INUEXTERN VOID PaliX86BiosSwitch(BYTE indexNumber, struct PaliX86BiosCallFrame* frame);

VOID PaliX86BiosCallsInitialize()
{
    PaliX86BiosSwitchReal = (VOID*)0x0007C00;
    RtlCopyMemory(PaliX86BiosSwitchReal, PaliX86BiosSwitch, (UINTPTR)PaliX86BiosSwitchEnd - (UINTPTR)PaliX86BiosSwitch);
}

VOID PaliX86BiosCall(BYTE interruptIndex, struct PaliX86BiosCallFrame* frame)
{
    HalDisableInterrupts();
    PaliX86BiosSwitchReal(interruptIndex, frame);
}