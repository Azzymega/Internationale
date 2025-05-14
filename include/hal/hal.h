#pragma once
#include <ltbase.h>
#include <ps/proc.h>

VOID HalInitialize();

UINTPTR HalGetPageSize();
UINTPTR HalGetSerializedStateSize();

VOID HalEnterLock(INUVOLATILE BOOLEAN* monitor);
VOID HalExitLock(INUVOLATILE BOOLEAN* monitor);
BOOLEAN HalTryEnterLock(INUVOLATILE BOOLEAN* monitor);

VOID HalAssert(const CHAR *file, const CHAR* func, const CHAR *line);
VOID HalBugcheck(const CHAR *message, const CHAR *file, const CHAR* func, const CHAR *line);

#define INU_BUGCHECK(message) HalBugcheck(message,__FILE__,__func__,__symbol2string( __LINE__ ));
#define INU_ASSERT(message) assert(message)
#define INU_STATIC_ASSERT(target) _Static_assert(target)

VOID HalCopyMemory(VOID* destination, VOID* source, UINTPTR length);
VOID HalSetMemory(VOID* destination, UINTPTR value, UINTPTR length);
VOID HalSetBlockMemory(VOID* destination, UINT32 value, UINTPTR length);
VOID HalMoveMemory(VOID *restrict destination, const VOID *restrict source, UINTPTR length);
DOUBLE HalFmod(DOUBLE x, DOUBLE y);

VOID HalSaveState(struct KERNEL_THREAD* thread, VOID* state);
VOID HalSwitchState(struct KERNEL_THREAD* target, VOID* state);

VOID HalEnableInterrupts();
VOID HalDisableInterrupts();
VOID HalSetInterrupt(TRAP_HANDLER ptr, UINTPTR index, CONTROL_LEVEL level);
CONTROL_LEVEL HalGetInterruptControlLevel(UINTPTR index);
VOID HalJumpInKernelThread(struct KERNEL_THREAD* thread);

VOID HalSetZeroDivTrap(TRAP_HANDLER handler);
VOID HalSetDebugTrap(TRAP_HANDLER handler);
VOID HalSetInvalidOpcodeTrap(TRAP_HANDLER handler);
VOID HalSetDoubleFaultTrap(TRAP_HANDLER handler);
VOID HalSetProtectionFaultTrap(TRAP_HANDLER handler);
VOID HalSetPageFaultTrap(TRAP_HANDLER handler);

VOID* HalAllocatePageTable();
VOID HalMemoryMap(struct VAS_DESCRIPTOR* vas, VOID* virtualAddress, VOID* kernelAddress, UINTPTR length, enum VAS_BLOCK_DESCRIPTOR_TYPES attributes);

VOID HalModifyFrame(VOID* self, VOID* addressSpace, VOID* newStack, VOID* func, VOID* arg, enum PROCESS_MODE mode);