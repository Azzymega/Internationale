#pragma once
#include <intermarx/intermarx.h>
#include <intermarx/ex/ex.h>
#include <intermarx/ob/ob.h>

INUSTATIC const char* const ExExecutionEngineErrorName = "System.ExecutionEngineException";
INUSTATIC const char* const ExNotImplementedName = "System.NotImplementedException";
INUSTATIC const char* const ExNullReferenceName = "System.NullReferenceException";
INUSTATIC const char* const ExIndexOutOfRangeName = "System.IndexOutOfRangeException";

//
//  Base function
//

MARX_STATUS ExInitialize();

MARX_STATUS ExMethodPrologueDelegate(struct RUNTIME_FRAME_BLOCK* delegate, struct RUNTIME_FRAME_BLOCK* args, struct RUNTIME_FRAME* previous, struct RUNTIME_FRAME_BLOCK* returnValue);

MARX_STATUS ExMethodPrologueArgs(struct RUNTIME_METHOD* method, struct RUNTIME_FRAME_BLOCK* args, struct RUNTIME_FRAME* previous, struct RUNTIME_FRAME_BLOCK* returnValue);
MARX_STATUS ExMethodPrologueCtor(struct RUNTIME_METHOD* method, struct RUNTIME_FRAME* previous, VOID* ptr, struct RUNTIME_FRAME_BLOCK* returnValue);

MARX_STATUS ExMethodPrologueArgsNative(struct RUNTIME_METHOD* method, struct RUNTIME_FRAME_BLOCK* args, struct RUNTIME_FRAME* previous, struct RUNTIME_FRAME_BLOCK* returnValue);
MARX_STATUS ExMethodPrologueCtorNative(struct RUNTIME_METHOD* method, struct RUNTIME_FRAME* previous, VOID* ptr, struct RUNTIME_FRAME_BLOCK* returnValue);

MARX_STATUS ExMethodPrologue(struct RUNTIME_METHOD* method);
MARX_STATUS ExMethodEpilogue(struct RUNTIME_FRAME* frame);
MARX_STATUS ExMethodExecute(struct RUNTIME_FRAME* frame, struct RUNTIME_FRAME_BLOCK* returnValue);

MARX_STATUS ExThrowException(struct MANAGED_EXCEPTION* exception);
MARX_STATUS ExHandleException(struct MANAGED_EXCEPTION** exception);
MARX_STATUS ExLocateHandler(struct RUNTIME_EXCEPTION_HANDLER* handler, struct RUNTIME_FRAME *frame);
MARX_STATUS ExLocateFinally(struct RUNTIME_EXCEPTION_HANDLER* handler, struct RUNTIME_FRAME *frame);
MARX_STATUS ExLocateVirtualMethod(struct RUNTIME_FRAME_BLOCK *header, struct RUNTIME_METHOD* method, struct RUNTIME_METHOD** returnTarget);

//
//  Helpers
//

MARX_STATUS ExNullCheck(struct RUNTIME_FRAME_BLOCK block);
MARX_STATUS ExBoundCheck(struct RUNTIME_FRAME_BLOCK block, UINTPTR accessIndex);

//
//  Opcodes
//

struct RUNTIME_FRAME_BLOCK ExPeek(struct RUNTIME_FRAME* frame);
struct RUNTIME_FRAME_BLOCK ExPop(struct RUNTIME_FRAME* frame);
VOID ExPush(struct RUNTIME_FRAME* frame, struct RUNTIME_FRAME_BLOCK block);
VOID* ExGetPoolElement(struct READER* reader, struct RUNTIME_FRAME* frame);


struct RUNTIME_FRAME_BLOCK ExAdd(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExSub(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExDiv(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExMul(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExRem(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExNeg(struct RUNTIME_FRAME_BLOCK first);
struct RUNTIME_FRAME_BLOCK ExShl(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExShr(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExAnd(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExOr(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExXor(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
struct RUNTIME_FRAME_BLOCK ExNot(struct RUNTIME_FRAME_BLOCK first);

MARX_STATUS ExIsInstance(struct RUNTIME_TYPE* castTarget, struct RUNTIME_TYPE* objectType);

BOOLEAN ExIsZero(struct RUNTIME_FRAME_BLOCK first);
BOOLEAN ExIsNonZero(struct RUNTIME_FRAME_BLOCK first);
BOOLEAN ExIsEquals(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
BOOLEAN ExIsUnEquals(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
BOOLEAN ExIsGreaterEquals(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
BOOLEAN ExIsGreater(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
BOOLEAN ExIsLowerEquals(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);
BOOLEAN ExIsLower(struct RUNTIME_FRAME_BLOCK first, struct RUNTIME_FRAME_BLOCK second);


struct RUNTIME_FRAME_BLOCK ExConvI8(struct RUNTIME_FRAME_BLOCK first);
struct RUNTIME_FRAME_BLOCK ExConvIntPtr(struct RUNTIME_FRAME_BLOCK first);
struct RUNTIME_FRAME_BLOCK ExConvI16(struct RUNTIME_FRAME_BLOCK first);
struct RUNTIME_FRAME_BLOCK ExConvI32(struct RUNTIME_FRAME_BLOCK first);
struct RUNTIME_FRAME_BLOCK ExConvI64(struct RUNTIME_FRAME_BLOCK first);
struct RUNTIME_FRAME_BLOCK ExConvMFloat(struct RUNTIME_FRAME_BLOCK first);