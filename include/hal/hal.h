#pragma once
#include <ltbase.h>
#include <fw/fw.h>

VOID HalInitialize();

UINTPTR HalGetPageSize();
UINTPTR HalGetSerializedStateSize();

VOID HalEnterLock(INUVOLATILE BOOLEAN* monitor);
VOID HalExitLock(INUVOLATILE BOOLEAN* monitor);
BOOLEAN HalTryEnterLock(INUVOLATILE BOOLEAN* monitor);

VOID HalBugcheck(const CHAR* message);

VOID HalCopyMemory(VOID* destination, VOID* source, UINTPTR length);
VOID HalSetMemory(VOID* destination, UINTPTR target, UINTPTR length);
VOID HalMoveMemory(VOID* destination, VOID* source, UINTPTR length);

VOID HalEnableInterrupts();
VOID HalDisableInterrupts();
VOID HalSetInterrupt(TRAP_HANDLER ptr, UINTPTR index, CONTROL_LEVEL level);
CONTROL_LEVEL HalGetInterruptControlLevel(UINTPTR index);

VOID HalSetZeroDivTrap(TRAP_HANDLER handler);
VOID HalSetDebugTrap(TRAP_HANDLER handler);
VOID HalSetInvalidOpcodeTrap(TRAP_HANDLER handler);
VOID HalSetDoubleFaultTrap(TRAP_HANDLER handler);
VOID HalSetProtectionFaultTrap(TRAP_HANDLER handler);
VOID HalSetPageFaultTrap(TRAP_HANDLER handler);
