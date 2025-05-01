#pragma once
#include <ltbase.h>

struct INUPACKED PaliVesaInfo
{
    CHAR VbeSignature[4];
    UINT16 VbeVersion;
    UINT16 OemStringPtr[2];
    BYTE Capabilities[4];
    UINT16 VideoModePtr[2];
    UINT16 TotalMemory;
    BYTE Reserved[492];
};

struct INUPACKED PaliVesaSetMode
{
    UINT16 mode : 14;
    UINT16 lfb : 1;
    UINT16 clearMem : 1;
};

struct INUPACKED PaliVesaPixel24
{
    BYTE blue;
    BYTE green;
    BYTE red;
};

struct INUPACKED PaliVesaModeInfo
{
    UINT16 attributes;
    BYTE windowA;
    BYTE windowB;
    UINT16 granularity;
    UINT16 window_size;
    UINT16 segmentA;
    UINT16 segmentB;
    UINT32 scary16bitSegmentFarCall;
    UINT16 pitch;
    UINT16 width;
    UINT16 height;
    BYTE wChar;
    BYTE yChar;
    BYTE planes;
    BYTE bpp;
    BYTE banks;
    BYTE memoryModel;
    BYTE bankSize;
    BYTE imagePages;
    BYTE reserved0;

    BYTE redMask;
    BYTE redPosition;
    BYTE greenMask;
    BYTE greenPosition;
    BYTE blueMask;
    BYTE bluePosition;
    BYTE reservedMask;
    BYTE reservedPosition;
    BYTE directColorAttributes;

    UINT32 framebuffer;
    UINT32 offScreenMemOffset;
    UINT16 offScreenMemSize;
    BYTE reserved1[206];
};

VOID PaliEnableVesa();
VOID PaliVesaPrintText(const CHAR* string);
VOID PaliVesaPrintCharacter(CHAR character);
VOID PaliVesaClearScreen();
VOID PaliSetVideoModeVesa(UINT16 mode, BOOLEAN lfb);
BOOLEAN PaliCheckVesa();
UINTPTR PaliGetFramebufferAddress();
UINT16 PaliGetCurrentVideoModeVesa();
UINTPTR PaliGetFramebufferX();
UINTPTR PaliGetFramebufferY();
VOID PaliLoadBiosFont();