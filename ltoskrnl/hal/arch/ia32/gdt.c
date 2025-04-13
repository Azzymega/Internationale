#include <hal/hal.h>
#include <rtl/rtl.h>

#include "hali.h"

INUEXTERN struct HaliX86GdtEntry FwGdtEntryTable[];

VOID HaliX86LoadGdtEntry(UINT32 index, UINT32 base, UINT32 limit, BYTE rw, BYTE executable, BYTE isSegment,
    BYTE privilegeLevel, BYTE granularity, BYTE sizeType)
{
    FwGdtEntryTable[index].lowBase = base & 0xFFFF;
    FwGdtEntryTable[index].midBase = ((base >> 16) & 0xFF);
    FwGdtEntryTable[index].highBase = ((base >> 24) & 0xFF);
    FwGdtEntryTable[index].lowLimit = limit & 0xFFFF;
    FwGdtEntryTable[index].highLimit = ((limit >> 16) & 0x0F);

    FwGdtEntryTable[index].accessed = TRUE;
    FwGdtEntryTable[index].rw = rw;
    FwGdtEntryTable[index].direction = 0;
    FwGdtEntryTable[index].executable = executable;
    FwGdtEntryTable[index].isSegment = isSegment;
    FwGdtEntryTable[index].privilegeLevel = privilegeLevel;
    FwGdtEntryTable[index].present = TRUE;
    FwGdtEntryTable[index].size = sizeType;
    FwGdtEntryTable[index].pageGranularity = granularity;
}

VOID HaliX86LoadTssEntry(UINT32 index, UINT32 base, UINT32 limit, BYTE tssType, BYTE privilegeLevel)
{
    struct HaliX86TssEntry entry = {0};

    entry.lowBase = base & 0xFFFF;
    entry.midBase = ((base >> 16) & 0xFF);
    entry.highBase = ((base >> 24) & 0xFF);
    entry.lowLimit = limit & 0xFFFF;
    entry.highLimit = ((limit >> 16) & 0x0F);

    entry.type = tssType;
    entry.privilegeLevel = privilegeLevel;
    entry.present = TRUE;
    entry.size = TRUE;
    entry.pageGranularity = TRUE;

    RtlCopyMemory(&FwGdtEntryTable[index],&entry,sizeof(struct HaliX86TssEntry));
}
