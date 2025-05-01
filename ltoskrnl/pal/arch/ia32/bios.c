#include <ltbase.h>
#include <hal/hal.h>
#include <pal/pal.h>
#include <rtl/rtl.h>

#include "pali.h"

#define PALI_BIOS_CALL_RELOCATION_PLACE 0x0007C00
#define PALI_BIOS_CALL_BUFFER_SIZE 4096
#define PALI_BIOS_FRAME_BUFFER_SIZE 100

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
    frame->ss = 1;
    frame->esp = 0xFFFF;

    VOID* source = PaliGetBiosFrameBuffer();
    HalDisableInterrupts();
    RtlCopyMemory(source,frame,sizeof(struct PaliX86BiosCallFrame));
    PaliX86BiosSwitchReal(interruptIndex, PaliGetBiosFrameBuffer());
    RtlCopyMemory(frame,source,sizeof(struct PaliX86BiosCallFrame));
}

VOID* PaliGetBiosCallBuffer()
{
    UINTPTR functionStartAddress = (UINTPTR)PaliX86BiosSwitch;
    UINTPTR functionEndAddress = (UINTPTR)PaliX86BiosSwitchEnd;
    UINTPTR functionLength = functionEndAddress - functionStartAddress;
    UINTPTR relocatedFunctionEnd = PALI_BIOS_CALL_RELOCATION_PLACE + functionLength;
    return (VOID*)(relocatedFunctionEnd - PALI_BIOS_CALL_BUFFER_SIZE);
}

VOID* PaliGetBiosFrameBuffer()
{
    return PaliGetBiosCallBuffer() - PALI_BIOS_FRAME_BUFFER_SIZE;
}
