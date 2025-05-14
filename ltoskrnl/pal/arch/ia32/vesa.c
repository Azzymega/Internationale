#include "vesa.h"

#include <hal/hal.h>
#include <rtl/rtl.h>

#include "pali.h"

#define PALI_VESA_VIDEO_MODE 0x118
#define PALI_VESA_BIOS_FONT_BUFFER_SIZE 4096

INUGLOBAL BYTE PaliVesaBiosFontBuffer[PALI_VESA_BIOS_FONT_BUFFER_SIZE];
INUGLOBAL struct PaliVesaPixel24* PaliVesaFramebufferAddress;

INUGLOBAL UINTPTR PaliVesaScreenWidth;
INUGLOBAL UINTPTR PaliVesaScreenHeight;

INUGLOBAL UINTPTR PaliVesaCurrentWidth;
INUGLOBAL UINTPTR PaliVesaCurrentHeight;



VOID PaliEnableVesa()
{
    if (PaliCheckVesa())
    {
        PaliSetVideoModeVesa(PALI_VESA_VIDEO_MODE,TRUE);
        PaliGetFramebufferAddress();
        PaliLoadBiosFont();
        PaliVesaClearScreen();
    }
    else
    {
        INU_BUGCHECK("VESA 2.0 IS NOT SUPPORTED!");
    }
}

VOID PaliVesaPrintText(const CHAR* string)
{
    UINTPTR length = strlen(string);
    for (int i = 0; i < length; ++i)
    {
        PaliVesaPrintCharacter(string[i]);
    }
}

VOID PaliVesaPrintCharacter(CHAR character)
{
    if (PaliVesaCurrentWidth + 8 >= PaliVesaScreenWidth)
    {
        PaliVesaCurrentHeight += 8;
        PaliVesaCurrentWidth = 0;
    }

    if (PaliVesaCurrentHeight + 8 >= PaliVesaScreenHeight)
    {
        PaliVesaClearScreen();
        PaliVesaCurrentHeight = 0;
        PaliVesaCurrentWidth = 0;
    }

    UINTPTR offset = character*16;
    VOID* ptr = (VOID*)offset;

    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {

        }
    }
}

VOID PaliVesaClearScreen()
{
    struct PaliVesaPixel24 pixel = {
        .blue = 199
    };

    UINTPTR total = PaliVesaScreenHeight * PaliVesaScreenWidth;
    struct PaliVesaPixel24 *p = PaliVesaFramebufferAddress;

    UINTPTR i;
    for (i = 0; i + 7 < total; i += 8)
    {
        p[i] = pixel;
        p[i + 1] = pixel;
        p[i + 2] = pixel;
        p[i + 3] = pixel;
        p[i + 4] = pixel;
        p[i + 5] = pixel;
        p[i + 6] = pixel;
        p[i + 7] = pixel;
    }

    for (; i < total; ++i)
    {
        p[i] = pixel;
    }
}

VOID PaliSetVideoModeVesa(UINT16 mode, BOOLEAN lfb)
{
    struct PaliX86BiosCallFrame frame = {0};

    struct PaliVesaSetMode info = {0};
    info.lfb = lfb;
    info.mode = mode;

    frame.eax = 0x4F02;
    RtlCopyMemory(&frame.bx,&info,2);
    PaliX86BiosCall(0x10,&frame);
}

BOOLEAN PaliCheckVesa()
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x4F00;
    frame.es = 0;
    frame.di = (UINTPTR)PaliGetBiosCallBuffer();

    PaliX86BiosCall(0x10,&frame);

    struct PaliVesaInfo* info = PaliGetBiosCallBuffer();
    if (info->VbeVersion >= 0x200)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UINTPTR PaliGetFramebufferAddress()
{
    UINT16 currentMode = PaliGetCurrentVideoModeVesa();

    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x4F01;
    frame.es = 0;
    frame.di = (UINTPTR)PaliGetBiosCallBuffer();
    frame.cx = currentMode;

    PaliX86BiosCall(0x10,&frame);

    struct PaliVesaModeInfo* info = PaliGetBiosCallBuffer();

    PaliVesaScreenHeight = info->height;
    PaliVesaScreenWidth = info->width;
    PaliVesaFramebufferAddress = (VOID*)info->framebuffer;

    return info->framebuffer;
}

UINT16 PaliGetCurrentVideoModeVesa()
{
    struct PaliX86BiosCallFrame frame = {0};
    frame.ax = 0x4F03;
    PaliX86BiosCall(0x10,&frame);
    return  frame.bx;
}

UINTPTR PaliGetFramebufferX()
{
    return PaliVesaScreenWidth;
}

UINTPTR PaliGetFramebufferY()
{
    return PaliVesaScreenHeight;
}

VOID PaliLoadBiosFont()
{
    struct PaliX86BiosCallFrame frame = {0};

    frame.ax = 0x1130;
    frame.bh = 6;

    PaliX86BiosCall(0x10,&frame);

    VOID* fontAddress = (VOID*)(frame.es * 0x10) + frame.bp;
    RtlCopyMemory(&PaliVesaBiosFontBuffer,fontAddress,sizeof(PaliVesaBiosFontBuffer));
}
