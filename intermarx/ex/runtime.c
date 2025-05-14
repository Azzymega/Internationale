#include <intermarx/ex/runtime.h>
#include <intermarx/ex/ex.h>
#include <intermarx/hp/hp.h>
#include <intermarx/mt/mt.h>
#include <intermarx/ob/ob.h>
#include <intermarx/pal/pal.h>

INUIMPORT struct RUNTIME_DOMAIN *ExGlobalZeroDomain;

INUGLOBAL struct VECTOR ExGlobalDomainList;

INUGLOBAL struct RUNTIME_TYPE *ExStringType;
INUGLOBAL struct RUNTIME_TYPE *ExCharArrayType;
INUGLOBAL struct RUNTIME_TYPE *ExCharType;
INUGLOBAL struct RUNTIME_TYPE *ExThreadType;

INUGLOBAL struct MANAGED_EXCEPTION ExExecutionEngineError;
INUGLOBAL struct MANAGED_EXCEPTION ExNotImplemented;
INUGLOBAL struct MANAGED_EXCEPTION ExNullReference;
INUGLOBAL struct MANAGED_EXCEPTION ExIndexOutOfRange;


MARX_STATUS ExInitialize()
{
    ObManagedExceptionInitialize(&ExExecutionEngineError,
                                 ExDomainLocateType(ExGlobalZeroDomain, ExExecutionEngineErrorName),
                                 L"Execution engine error!");
    ObManagedExceptionInitialize(&ExNotImplemented,
                                 ExDomainLocateType(ExGlobalZeroDomain, ExNotImplementedName),
                                 L"Not implemented!");
    ObManagedExceptionInitialize(&ExNullReference,
                                 ExDomainLocateType(ExGlobalZeroDomain, ExNullReferenceName),
                                 L"Null reference!");
    ObManagedExceptionInitialize(&ExIndexOutOfRange,
                                 ExDomainLocateType(ExGlobalZeroDomain, ExIndexOutOfRangeName),
                                 L"Index out of range!");

    ExStringType = ExDomainLocateType(ExGlobalZeroDomain, ExStringTypeName);
    ExCharArrayType = ExDomainLocateType(ExGlobalZeroDomain, "System.Char[]");
    ExCharType = ExDomainLocateType(ExGlobalZeroDomain, ExCharTypeName);
    ExThreadType = ExDomainLocateType(ExGlobalZeroDomain, ExThreadTypeName);

    RtlVectorInitialize(&ExGlobalDomainList);
    RtlVectorAdd(&ExGlobalDomainList, ExGlobalZeroDomain);

    return MARX_STATUS_SUCCESS;
}

INUFORCEINLINE struct RUNTIME_FRAME_BLOCK ExPeek(struct RUNTIME_FRAME *frame)
{
    return frame->stack[frame->sp - 1];
}

INUFORCEINLINE struct RUNTIME_FRAME_BLOCK ExPop(struct RUNTIME_FRAME *frame)
{
    return frame->stack[--frame->sp];
}

INUFORCEINLINE VOID ExPush(struct RUNTIME_FRAME *frame, struct RUNTIME_FRAME_BLOCK block)
{
    frame->stack[frame->sp++] = block;
}

INUFORCEINLINE MARX_STATUS ExNullCheck(struct RUNTIME_FRAME_BLOCK block)
{
    switch (block.type)
    {
        case MACHINE_OBJECT:
        {
            if (block.descriptor == NULL)
            {
                return MARX_STATUS_FAIL;
            }
            else
            {
                return MARX_STATUS_SUCCESS;
            }
        }
        case MACHINE_MANAGED_POINTER:
        {
            if (block.link.pointer == NULL)
            {
                return MARX_STATUS_FAIL;
            }
            else
            {
                return MARX_STATUS_SUCCESS;
            }
        }
        case MACHINE_INTPTR:
        {
            if (block.pointer == (INTPTR) NULL)
            {
                return MARX_STATUS_FAIL;
            }
            else
            {
                return MARX_STATUS_SUCCESS;
            }
        }
        case MACHINE_STRUCT:
        {
            if (block.valueType.pointer == NULL)
            {
                return MARX_STATUS_FAIL;
            }
            else
            {
                return MARX_STATUS_SUCCESS;
            }
        }
        default:
        {
            return MARX_STATUS_FAIL;
        }
    }
}

MARX_STATUS ExMethodPrologueDelegate(struct RUNTIME_FRAME_BLOCK *delegate, struct RUNTIME_FRAME_BLOCK *args,
                                     struct RUNTIME_FRAME *previous, struct RUNTIME_FRAME_BLOCK *returnValue)
{
    struct RUNTIME_FRAME frame;
    struct RUNTIME_FRAME_BLOCK ret;

    if (delegate->descriptor == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    struct RUNTIME_METHOD *signatureMethod = ((struct MANAGED_DELEGATE *) delegate->descriptor)->callSites->pointer[0];
    struct RUNTIME_FRAME_BLOCK localArgs[signatureMethod->parameters.count + 1];

    if (previous != NULL)
    {
        for (int i = signatureMethod->parameters.count - 1; i >= 1; --i)
        {
            localArgs[i] = ExPop(previous);
        }
    }

    for (int t = 0; t < ((struct MANAGED_DELEGATE *) delegate->descriptor)->callSites->count; ++t)
    {
        struct RUNTIME_METHOD *method = ((struct MANAGED_DELEGATE *) delegate->descriptor)->callSites->pointer[t];

        if (method == NULL)
        {
            return MARX_STATUS_FAIL;
        }


        if (args == NULL)
        {
            localArgs[0].type = MACHINE_OBJECT;
            localArgs[0].descriptor = ((struct MANAGED_DELEGATE *) delegate->descriptor)->thisObjects->pointer[t];

            frame.args = localArgs;
        }
        else
        {
            frame.args = args;
        }


        UINTPTR stackSize = method->variables.count + method->parameters.count + 20;

        frame.stack = PalStackBufferAllocate(stackSize, sizeof(struct RUNTIME_FRAME_BLOCK));
        frame.sp = 0;
        frame.maxStack = stackSize;

        frame.variables = PalStackBufferAllocate(method->variables.count, sizeof(struct RUNTIME_FRAME_BLOCK));

        PalMemoryZeroBlock(frame.variables, method->variables.count, sizeof(struct RUNTIME_FRAME_BLOCK));

        for (int i = 0; i < method->variables.count; ++i)
        {
            struct RUNTIME_TYPE *variableType = RtlVectorGet(&method->variables, i);
            if (ExMetadataIs(variableType->metadata, MxExMetadataStruct) &&
                !ExMetadataIs(variableType->metadata, MxExMetadataPrimitive))
            {
                frame.variables[i].type = MACHINE_STRUCT;
                frame.variables[i].valueType.type = variableType;
                frame.variables[i].valueType.pointer = PalStackAllocate(variableType->size);
            }
        }

        frame.method = method;
        frame.domain = method->owner->domain;
        frame.next = NULL;

        if (previous != NULL)
        {
            frame.thread = previous->thread;
            frame.thread->currentFrame = &frame;
            previous->next = &frame;
        }
        else
        {
            frame.thread = MtThreadGetCurrent();
            frame.thread->currentFrame = &frame;
            frame.thread->firstFrame = &frame;
        }

        MARX_STATUS status = ExMethodExecute(&frame, &ret);
        *returnValue = ret;

        if (MARX_FAIL(status))
        {
            return status;
        }
    }

    return MARX_STATUS_SUCCESS;
}

MARX_STATUS ExMethodPrologue(struct RUNTIME_METHOD *method)
{
    struct RUNTIME_FRAME_BLOCK ret;
    return ExMethodPrologueArgs(method,NULL,NULL, &ret);
}

MARX_STATUS ExMethodPrologueArgs(struct RUNTIME_METHOD *method, struct RUNTIME_FRAME_BLOCK *args, struct RUNTIME_FRAME *previous,
                                 struct RUNTIME_FRAME_BLOCK *returnValue)
{
    struct RUNTIME_FRAME frame;
    struct RUNTIME_FRAME_BLOCK ret;

    if (method == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    struct RUNTIME_FRAME_BLOCK localArgs[method->parameters.count];

    if (args == NULL)
    {
        for (int i = method->parameters.count - 1; i >= 0; --i)
        {
            localArgs[i] = ExPop(previous);
        }
        frame.args = localArgs;
    }
    else
    {
        frame.args = args;
    }

    UINTPTR stackSize = method->variables.count + method->parameters.count + 20;

    frame.stack = PalStackBufferAllocate(stackSize,
                                         sizeof(struct RUNTIME_FRAME_BLOCK));
    frame.sp = 0;
    frame.maxStack = stackSize;

    frame.variables = PalStackBufferAllocate(method->variables.count, sizeof(struct RUNTIME_FRAME_BLOCK));

    PalMemoryZeroBlock(frame.variables, method->variables.count, sizeof(struct RUNTIME_FRAME_BLOCK));

    for (int i = 0; i < method->variables.count; ++i)
    {
        struct RUNTIME_TYPE *variableType = RtlVectorGet(&method->variables, i);
        if (ExMetadataIs(variableType->metadata, MxExMetadataStruct) &&
            !ExMetadataIs(variableType->metadata, MxExMetadataPrimitive))
        {
            frame.variables[i].type = MACHINE_STRUCT;
            frame.variables[i].valueType.type = variableType;
            frame.variables[i].valueType.pointer = PalStackAllocate(variableType->size);
        }
    }

    frame.method = method;
    frame.domain = method->owner->domain;
    frame.next = NULL;

    if (previous != NULL)
    {
        frame.thread = previous->thread;
        frame.thread->currentFrame = &frame;
        previous->next = &frame;
    }
    else
    {
        frame.thread = MtThreadGetCurrent();
        frame.thread->currentFrame = &frame;
        frame.thread->firstFrame = &frame;
    }

    MARX_STATUS status = ExMethodExecute(&frame, &ret);
    *returnValue = ret;
    return status;
}

MARX_STATUS ExMethodPrologueCtor(struct RUNTIME_METHOD *method, struct RUNTIME_FRAME *previous, VOID *ptr,
                                 struct RUNTIME_FRAME_BLOCK *returnValue)
{
    struct RUNTIME_FRAME frame;
    struct RUNTIME_FRAME_BLOCK ret;

    if (method == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    struct RUNTIME_FRAME_BLOCK args[method->parameters.count];

    if (ExMetadataIs(method->owner->metadata, MxExMetadataStruct))
    {
        args[0].type = MACHINE_MANAGED_POINTER;
        args[0].link.type = method->owner;
        args[0].link.pointer = ptr;
    }
    else
    {
        args[0].type = MACHINE_OBJECT;
        args[0].descriptor = ptr;
    }

    struct RUNTIME_FRAME_BLOCK thisPtr = ExPop(previous);
    for (int i = method->parameters.count - 1; i >= 1; --i)
    {
        args[i] = ExPop(previous);
    }
    ExPush(previous, thisPtr);

    frame.args = args;
    UINTPTR stackSize = method->variables.count + method->parameters.count + 20;

    frame.stack = PalStackBufferAllocate(stackSize,
                                         sizeof(struct RUNTIME_FRAME_BLOCK));
    frame.sp = 0;
    frame.maxStack = stackSize;
    frame.variables = PalStackBufferAllocate(method->variables.count, sizeof(struct RUNTIME_FRAME_BLOCK));

    PalMemoryZeroBlock(frame.variables, method->variables.count, sizeof(struct RUNTIME_FRAME_BLOCK));

    for (int i = 0; i < method->variables.count; ++i)
    {
        struct RUNTIME_TYPE *variableType = RtlVectorGet(&method->variables, i);
        if (ExMetadataIs(variableType->metadata, MxExMetadataStruct) &&
            !ExMetadataIs(variableType->metadata, MxExMetadataPrimitive))
        {
            frame.variables[i].type = MACHINE_STRUCT;
            frame.variables[i].valueType.type = variableType;
            frame.variables[i].valueType.pointer = PalStackAllocate(variableType->size);
        }
    }

    frame.method = method;
    frame.domain = method->owner->domain;
    frame.next = NULL;

    if (previous != NULL)
    {
        frame.thread = previous->thread;
        frame.thread->currentFrame = &frame;
        previous->next = &frame;
    }
    else
    {
        frame.thread = MtThreadGetCurrent();
        frame.thread->currentFrame = &frame;
        frame.thread->firstFrame = &frame;
    }

    MARX_STATUS status = ExMethodExecute(&frame, &ret);
    *returnValue = ret;
    return status;
}

MARX_STATUS ExMethodPrologueArgsNative(struct RUNTIME_METHOD *method, struct RUNTIME_FRAME_BLOCK *args, struct RUNTIME_FRAME *previous,
                                       struct RUNTIME_FRAME_BLOCK *returnValue)
{
    struct RUNTIME_FRAME frame;
    struct RUNTIME_FRAME_BLOCK ret;

    if (method == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    struct RUNTIME_FRAME_BLOCK localArgs[method->parameters.count];

    if (args == NULL)
    {
        for (int i = method->parameters.count - 1; i >= 0; --i)
        {
            localArgs[i] = ExPop(previous);
        }
        frame.args = localArgs;
    }
    else
    {
        frame.args = args;
    }

    frame.sp = 0;
    frame.method = method;
    frame.domain = method->owner->domain;
    frame.next = NULL;

    if (previous != NULL)
    {
        frame.thread = previous->thread;
        frame.thread->currentFrame = &frame;
        previous->next = &frame;
    }
    else
    {
        frame.thread = MtThreadGetCurrent();
        frame.thread->currentFrame = &frame;
        frame.thread->firstFrame = &frame;
    }

    MARX_STATUS status = FarNativeMethodExecute(&frame, &ret);
    *returnValue = ret;
    return status;
}

MARX_STATUS ExMethodPrologueCtorNative(struct RUNTIME_METHOD *method, struct RUNTIME_FRAME *previous, VOID *ptr,
                                       struct RUNTIME_FRAME_BLOCK *returnValue)
{
    struct RUNTIME_FRAME frame;
    struct RUNTIME_FRAME_BLOCK ret;

    if (method == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    struct RUNTIME_FRAME_BLOCK args[method->parameters.count];

    if (ExMetadataIs(method->owner->metadata, MxExMetadataStruct))
    {
        args[0].type = MACHINE_MANAGED_POINTER;
        args[0].link.type = method->owner;
        args[0].link.pointer = ptr;
    }
    else
    {
        args[0].type = MACHINE_OBJECT;
        args[0].descriptor = ptr;
    }

    struct RUNTIME_FRAME_BLOCK thisPtr = ExPop(previous);
    for (int i = method->parameters.count - 1; i >= 1; --i)
    {
        args[i] = ExPop(previous);
    }
    ExPush(previous, thisPtr);

    frame.args = args;
    frame.sp = 0;
    frame.method = method;
    frame.domain = method->owner->domain;
    frame.next = NULL;

    if (previous != NULL)
    {
        frame.thread = previous->thread;
        frame.thread->currentFrame = &frame;
        previous->next = &frame;
    }
    else
    {
        frame.thread = MtThreadGetCurrent();
        frame.thread->currentFrame = &frame;
        frame.thread->firstFrame = &frame;
    }

    MARX_STATUS status = FarNativeMethodExecute(&frame, &ret);
    *returnValue = ret;
    return status;
}

MARX_STATUS ExThrowException(struct MANAGED_EXCEPTION *exception)
{
    struct RUNTIME_THREAD *thread = MtThreadGetCurrent();

    thread->state.exception = exception;
    thread->state.isUnwinding = TRUE;

    return MARX_STATUS_SUCCESS;
}

MARX_STATUS ExHandleException(struct MANAGED_EXCEPTION **exception)
{
    struct RUNTIME_THREAD *thread = MtThreadGetCurrent();

    *exception = thread->state.exception;

    thread->state.exception = NULL;
    thread->state.isUnwinding = FALSE;

    return MARX_STATUS_SUCCESS;
}

MARX_STATUS ExLocateHandler(struct RUNTIME_EXCEPTION_HANDLER *handler, struct RUNTIME_FRAME *frame)
{
    struct RUNTIME_THREAD *thread = MtThreadGetCurrent();

    INT32 pc = frame->reader->offset;

    struct RUNTIME_EXCEPTION_HANDLER *lowest = NULL;
    for (int i = 0; i < frame->method->handlers.count; ++i)
    {
        struct RUNTIME_EXCEPTION_HANDLER *source = RtlVectorGet(&frame->method->handlers, i);

        if (source->handler == HANDLER_CATCH)
        {
            BOOLEAN casted = FALSE;
            struct RUNTIME_TYPE *exception = frame->thread->state.exception;

            while (exception != NULL)
            {
                if (handler->catch == exception)
                {
                    casted = TRUE;
                    break;
                }
                exception = exception->super;
            }

            if (casted)
            {
                if (pc >= handler->tryStart && pc <= handler->tryEnd)
                {
                    if (lowest == NULL)
                    {
                        lowest = handler;
                    }

                    if (lowest->tryStart > handler->tryStart && lowest->tryEnd > handler->tryEnd)
                    {
                        lowest = handler;
                    }
                }
            }
        }
    }

    if (lowest == NULL)
    {
        return MARX_STATUS_FAIL;
    }
    else
    {
        *handler = *lowest;
        return MARX_STATUS_SUCCESS;
    }
}

MARX_STATUS ExLocateFinally(struct RUNTIME_EXCEPTION_HANDLER *handler, struct RUNTIME_FRAME *frame)
{
    INT32 pc = frame->reader->offset;

    struct RUNTIME_EXCEPTION_HANDLER *lowest = NULL;
    for (INT32 i = 0; i < frame->method->handlers.count; ++i)
    {
        struct RUNTIME_EXCEPTION_HANDLER *source = RtlVectorGet(&frame->method->handlers, i);

        if (source->handler == HANDLER_FINALLY)
        {
            if (pc >= source->tryStart && pc <= source->tryEnd)
            {
                if (lowest == NULL)
                {
                    lowest = source;
                }

                if (lowest->tryStart > source->tryStart && lowest->tryEnd > source->tryEnd)
                {
                    lowest = source;
                }
            }
        }
    }

    if (lowest != NULL)
    {
        *handler = *lowest;
        return MARX_STATUS_SUCCESS;
    }
    else
    {
        return MARX_STATUS_FAIL;
    }
}

MARX_STATUS ExLocateVirtualMethod(struct RUNTIME_FRAME_BLOCK *header, struct RUNTIME_METHOD *method, struct RUNTIME_METHOD **returnTarget)
{
    struct RUNTIME_TYPE *objectType = NULL;

    if (header->type == MACHINE_OBJECT)
    {
        if (ExMetadataIs(method->owner->metadata, MxExMetadataStruct) && !ExMetadataIs(
                method->metadata,
                MxExMetadataStatic))
        {
            return MARX_STATUS_FAIL;
        }
        else
        {
            objectType = ((struct OBJECT_HEADER *) header->descriptor)->type;
        }
    }
    else if (header->type == MACHINE_MANAGED_POINTER)
    {
        objectType = header->link.type;
    }

    if (objectType == NULL || method == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    while (objectType != NULL)
    {
        for (int i = 0; i < objectType->methods.count; ++i)
        {
            struct RUNTIME_METHOD *targetMethod = RtlVectorGet(&objectType->methods, i);

            if (RtlNStringCompareObject(targetMethod->shortName, method->shortName))
            {
                if (targetMethod->parameters.count == method->parameters.count)
                {
                    BOOLEAN fail = FALSE;
                    for (int t = 1; t < targetMethod->parameters.count; ++t)
                    {
                        struct RUNTIME_TYPE *firstParameter = RtlVectorGet(&targetMethod->parameters, t);
                        struct RUNTIME_TYPE *secondParameter = RtlVectorGet(&method->parameters, t);

                        if (firstParameter != secondParameter)
                        {
                            fail = TRUE;
                            break;
                        }
                    }

                    if (fail)
                    {
                        continue;
                    }
                    else
                    {
                        *returnTarget = targetMethod;
                        return MARX_STATUS_SUCCESS;
                    }
                }
                else
                {
                    continue;
                }
            }
        }

        objectType = objectType->super;
    }

    return MARX_STATUS_FAIL;
}

MARX_STATUS ExBoundCheck(struct RUNTIME_FRAME_BLOCK block, UINTPTR accessIndex)
{
    struct MANAGED_ARRAY *array = block.descriptor;

    if (accessIndex >= array->count)
    {
        return MARX_STATUS_FAIL;
    }
    else
    {
        return MARX_STATUS_SUCCESS;
    }
}

MARX_STATUS ExMethodEpilogue(struct RUNTIME_FRAME *frame)
{
    if (frame == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    frame->next = NULL;
    frame->thread->currentFrame = frame;

    return MARX_STATUS_SUCCESS;
}

MARX_STATUS ExMethodExecute(struct RUNTIME_FRAME *frame, struct RUNTIME_FRAME_BLOCK *returnValue)
{
    struct READER reader;
    RtlReaderInitialize(&reader, frame->method->bytecode);
    frame->reader = &reader;

    goto ExceptionHandlingZoneEnd;

ExceptionHandlingZone:

    struct RUNTIME_EXCEPTION_HANDLER handler;
    MARX_STATUS status = ExLocateHandler(&handler, frame);

    if (MARX_FAIL(status))
    {
        ExExceptionStateAppend(&MtThreadGetCurrent()->state, frame->method);

        return MARX_STATUS_EXCEPTION;
    }
    else
    {
        ExExceptionStateDrop(&MtThreadGetCurrent()->state);

        struct RUNTIME_FRAME_BLOCK exception;
        exception.type = MACHINE_OBJECT;

        ExHandleException((struct MANAGED_EXCEPTION **) &exception.descriptor);

        RtlReaderSet(&reader, handler.handlerStart);
        ExPush(frame, exception);
    }

ExceptionHandlingZoneEnd:

    while (TRUE)
    {
        enum BYTECODE bytecode = ((BYTE *) reader.stream)[reader.offset++]; // RtlReaderReadByte(&reader);

        switch (bytecode)
        {
            case OpBreakpoint:
            {
                ExThrowException(&ExExecutionEngineError);
                goto ExceptionHandlingZone;
                break;
            }
            case OpNoOperation:
            {
                PalSafepoint();
                break;
            }
            case OpDup:
            {
                ExPush(frame, ExPeek(frame));
                break;
            }
            case OpPop:
            {
                ExPop(frame);
                break;
            }
            case OpAdd:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExAdd(first, second));
                break;
            }
            case OpSub:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExSub(first, second));
                break;
            }
            case OpMu:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExMul(first, second));
                break;
            }
            case OpDiv:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExDiv(first, second));
                break;
            }
            case OpRem:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExRem(first, second));
                break;
            }
            case OpNeg:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExNeg(first));
                break;
            }
            case OpAnd:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExAnd(first, second));
                break;
            }
            case OpOr:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExOr(first, second));
                break;
            }
            case OpXor:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExXor(first, second));
                break;
            }
            case OpNot:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExNot(first));
                break;
            }
            case OpShiftLeft:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExShl(first, second));
                break;
            }
            case OpShiftRight:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExShr(first, second));
                break;
            }
            case OpConvertI8:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExConvI8(first));
                break;
            }
            case OpConvertI16:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExConvI16(first));
                break;
            }
            case OpConvertI32:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExConvI32(first));
                break;
            }
            case OpConvertI64:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExConvI64(first));
                break;
            }
            case OpConvertDouble:
            case OpConvertFloat:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExConvMFloat(first));
                break;
            }
            case OpConvertIntPtr:
            {
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                ExPush(frame, ExConvIntPtr(first));
                break;
            }
            case OpLoadValueFieldAddress:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(value)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                VOID *rawPointer = NULL;

                switch (value.type)
                {
                    case MACHINE_INTPTR:
                    {
                        rawPointer = (VOID *) value.pointer;
                        break;
                    }
                    case MACHINE_OBJECT:
                    {
                        rawPointer = value.descriptor;
                        break;
                    }
                    case MACHINE_MANAGED_POINTER:
                    {
                        rawPointer = value.link.pointer;
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                    }
                }

                struct RUNTIME_FIELD *field = ExGetPoolElement(&reader, frame);
                rawPointer += field->offset;

                struct RUNTIME_FRAME_BLOCK block;
                block.type = MACHINE_MANAGED_POINTER;
                block.link.type = field->declared;
                block.link.pointer = rawPointer;

                ExPush(frame, block);
                break;
            }
            case OpLoadLocalVariableAddress:
            {
                INT32 index = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK block;

                block.type = MACHINE_MANAGED_POINTER;
                block.link.type = RtlVectorGet(&frame->method->variables, index);

                switch (frame->variables[index].type)
                {
                    case MACHINE_STRUCT:
                    {
                        block.link.pointer = frame->variables[index].valueType.pointer;
                        break;
                    }
                    case MACHINE_INT32:
                    {
                        block.link.pointer = &frame->variables[index].int32;
                        break;
                    }
                    case MACHINE_INT64:
                    {
                        block.link.pointer = &frame->variables[index].int64;
                        break;
                    }
                    case MACHINE_MFLOAT:
                    {
                        block.link.pointer = &frame->variables[index].floating;
                        break;
                    }
                    case MACHINE_INTPTR:
                    {
                        block.link.pointer = &frame->variables[index].pointer;
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                    }
                }

                ExPush(frame, block);
                break;
            }
            case OpLoadArgumentAddress:
            {
                INT32 index = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK block;

                block.type = MACHINE_MANAGED_POINTER;
                block.link.type = RtlVectorGet(&frame->method->parameters, index);

                switch (frame->args[index].type)
                {
                    case MACHINE_STRUCT:
                    {
                        block.link.pointer = frame->args[index].valueType.pointer;
                        break;
                    }
                    case MACHINE_INT32:
                    {
                        block.link.pointer = &frame->args[index].int32;
                        break;
                    }
                    case MACHINE_INT64:
                    {
                        block.link.pointer = &frame->args[index].int64;
                        break;
                    }
                    case MACHINE_MFLOAT:
                    {
                        block.link.pointer = &frame->args[index].floating;
                        break;
                    }
                    case MACHINE_INTPTR:
                    {
                        block.link.pointer = &frame->args[index].pointer;
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                    }
                }

                ExPush(frame, block);
                break;
            }
            case OpStoreValueToPointer:
            {
                struct RUNTIME_TYPE *loadTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK address = ExPop(frame);

                switch (address.type)
                {
                    case MACHINE_MANAGED_POINTER:
                    {
                        switch (loadTarget->inlined)
                        {
                            case BASE_BYTE:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                BYTE* target = pointerAddress;
                                *target = value.int32;
                                break;
                            }
                            case BASE_CHAR:
                            case BASE_INT16:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                INT16* target = pointerAddress;
                                *target = value.int32;
                                break;
                            }
                            case BASE_INT32:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                INT32* target = pointerAddress;
                                *target = value.int32;
                                break;
                            }
                            case BASE_INT64:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                INT64* target = pointerAddress;
                                *target = value.int64;
                                break;
                            }
                            case BASE_SINGLE:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                SINGLE* target = pointerAddress;
                                *target = value.floating;
                                break;
                            }
                            case BASE_DOUBLE:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                DOUBLE* target = pointerAddress;
                                *target = value.floating;
                                break;
                            }
                            case BASE_OTHER:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                VOID** target = pointerAddress;
                                *target = value.descriptor;
                                break;
                            }
                            case BASE_INTPTR:
                            {
                                VOID* pointerAddress = address.link.pointer;
                                INTPTR* target = pointerAddress;
                                *target = value.pointer;
                                break;
                            }
                            default:
                            {
                                ExThrowException(&ExExecutionEngineError);
                                goto ExceptionHandlingZone;
                            }
                        }
                        break;
                    }
                    case MACHINE_INTPTR:
                    {
                        switch (loadTarget->inlined)
                        {
                            case BASE_BYTE:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                BYTE* target = pointerAddress;
                                *target = value.int32;
                                break;
                            }
                            case BASE_CHAR:
                            case BASE_INT16:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                INT16* target = pointerAddress;
                                *target = value.int32;
                                break;
                            }
                            case BASE_INT32:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                INT32* target = pointerAddress;
                                *target = value.int32;
                                break;
                            }
                            case BASE_INT64:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                INT64* target = pointerAddress;
                                *target = value.int64;
                                break;
                            }
                            case BASE_SINGLE:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                SINGLE* target = pointerAddress;
                                *target = value.floating;
                                break;
                            }
                            case BASE_DOUBLE:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                DOUBLE* target = pointerAddress;
                                *target = value.floating;
                                break;
                            }
                            case BASE_OTHER:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                VOID** target = pointerAddress;
                                *target = value.descriptor;
                                break;
                            }
                            case BASE_INTPTR:
                            {
                                VOID* pointerAddress = (VOID*)address.pointer;
                                INTPTR* target = pointerAddress;
                                *target = value.pointer;
                                break;
                            }
                            default:
                            {
                                ExThrowException(&ExExecutionEngineError);
                                goto ExceptionHandlingZone;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                    };
                }
                break;
            }
            case OpLoadValueFromPointer:
            {
                struct RUNTIME_TYPE *loadTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK peek = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK value = {0};

                switch (peek.type)
                {
                    case MACHINE_MANAGED_POINTER:
                    {
                        switch (loadTarget->inlined)
                        {
                            case BASE_BYTE:
                            case BASE_CHAR:
                            case BASE_INT16:
                            case BASE_INT32:
                            {
                                value.type = MACHINE_INT32;
                                PalMemoryCopy(&value.int32, peek.link.pointer, loadTarget->size);
                                break;
                            }
                            case BASE_INT64:
                            {
                                value.type = MACHINE_INT64;
                                PalMemoryCopy(&value.int64, peek.link.pointer, loadTarget->size);
                                break;
                            }
                            case BASE_SINGLE:
                            {
                                value.type = MACHINE_MFLOAT;
                                SINGLE *floatingValue = peek.link.pointer;
                                value.floating = *floatingValue;
                                break;
                            }
                            case BASE_DOUBLE:
                            {
                                value.type = MACHINE_MFLOAT;
                                DOUBLE *floatingValue = peek.link.pointer;
                                value.floating = *floatingValue;
                                break;
                            }
                            case BASE_INTPTR:
                            {
                                value.type = MACHINE_INTPTR;
                                PalMemoryCopy(&value.pointer, peek.link.pointer, loadTarget->size);
                                break;
                            }
                            case BASE_OTHER:
                            {
                                if (ExMetadataIs(loadTarget->metadata, MxExMetadataStruct))
                                {
                                    value.type = MACHINE_STRUCT;
                                    value.valueType.type = loadTarget;
                                    value.valueType.pointer = PalStackAllocate(loadTarget->size);
                                    PalMemoryCopy(value.valueType.pointer, peek.link.pointer, loadTarget->size);
                                    break;
                                }
                                else
                                {
                                    value.type = MACHINE_OBJECT;
                                    value.descriptor = peek.link.pointer;
                                    break;
                                }
                            }
                            default:
                            {
                                ExThrowException(&ExExecutionEngineError);
                                goto ExceptionHandlingZone;
                            }
                        }
                        break;
                    }
                    case MACHINE_INTPTR:
                    {
                        switch (loadTarget->inlined)
                        {
                            case BASE_BYTE:
                            case BASE_CHAR:
                            case BASE_INT16:
                            case BASE_INT32:
                            {
                                value.type = MACHINE_INT32;
                                PalMemoryCopy(&value.int32, (void *) peek.pointer, loadTarget->size);
                                break;
                            }
                            case BASE_INT64:
                            {
                                value.type = MACHINE_INT64;
                                PalMemoryCopy(&value.int64, (void *) peek.pointer, loadTarget->size);
                                break;
                            }
                            case BASE_SINGLE:
                            {
                                value.type = MACHINE_MFLOAT;
                                SINGLE *floatingValue = (void *) peek.pointer;
                                value.floating = *floatingValue;
                                break;
                            }
                            case BASE_DOUBLE:
                            {
                                value.type = MACHINE_MFLOAT;
                                DOUBLE *floatingValue = (void *) peek.pointer;
                                value.floating = *floatingValue;
                                break;
                            }
                            case BASE_INTPTR:
                            {
                                value.type = MACHINE_INTPTR;
                                PalMemoryCopy(&value.pointer, (void *) peek.pointer, loadTarget->size);
                                break;
                            }

                            case BASE_OTHER:
                            {
                                if (ExMetadataIs(loadTarget->metadata, MxExMetadataStruct))
                                {
                                    value.type = MACHINE_STRUCT;
                                    value.valueType.type = loadTarget;
                                    value.valueType.pointer = PalStackAllocate(loadTarget->size);
                                    PalMemoryCopy(value.valueType.pointer, (void *) peek.pointer, loadTarget->size);
                                    break;
                                }
                                else
                                {
                                    value.type = MACHINE_OBJECT;
                                    value.descriptor = (VOID *) peek.pointer;
                                    break;
                                }
                            }
                            default:
                            {
                                ExThrowException(&ExExecutionEngineError);
                                goto ExceptionHandlingZone;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                    };
                }

                ExPush(frame, value);
                break;
            }
            case OpLoadValueField:
            {
                struct RUNTIME_FRAME_BLOCK peek = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(peek)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                struct RUNTIME_FIELD *target = ExGetPoolElement(&reader, frame);

                struct RUNTIME_FRAME_BLOCK slot = {0};

                void *ptr = NULL;
                switch (peek.type)
                {
                    case MACHINE_OBJECT:
                    {
                        ptr = peek.descriptor;
                        ptr += sizeof(struct OBJECT_HEADER);
                        ptr += target->offset;
                        break;
                    }
                    case MACHINE_STRUCT:
                    {
                        ptr = peek.valueType.pointer;
                        ptr += target->offset;
                        break;
                    }
                    case MACHINE_MANAGED_POINTER:
                    {
                        ptr = peek.link.pointer;
                        ptr += target->offset;
                        break;
                    }
                    case MACHINE_INTPTR:
                    {
                        ptr = (void *) peek.pointer;
                        ptr += target->offset;
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExNullReference);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                switch (target->declared->inlined)
                {
                    case BASE_BYTE:
                    case BASE_CHAR:
                    case BASE_INT16:
                    case BASE_INT32:
                    {
                        slot.type = MACHINE_INT32;
                        PalMemoryCopy(&slot.int32, ptr, target->dataSize);
                        break;
                    }
                    case BASE_INT64:
                    {
                        slot.type = MACHINE_INT64;
                        PalMemoryCopy(&slot.int64, ptr, target->dataSize);
                        break;
                    }
                    case BASE_SINGLE:
                    {
                        slot.type = MACHINE_MFLOAT;
                        SINGLE *ptrValue = ptr;
                        slot.floating = *ptrValue;
                        break;
                    }
                    case BASE_DOUBLE:
                    {
                        slot.type = MACHINE_MFLOAT;
                        DOUBLE *ptrValue = ptr;
                        slot.floating = *ptrValue;
                        break;
                    }
                    case BASE_INTPTR:
                    {
                        slot.type = MACHINE_INTPTR;
                        PalMemoryCopy(&slot.floating, ptr, target->dataSize);
                        break;
                    }
                    case BASE_OTHER:
                    {
                        if (!ExMetadataIs(target->declared->metadata, MxExMetadataStruct))
                        {
                            slot.type = MACHINE_OBJECT;
                            PalMemoryCopy(&slot.descriptor, ptr, target->dataSize);
                            break;
                        }
                        else
                        {
                            if (ExMetadataIs(target->declared->metadata,MxExMetadataEnum))
                            {
                                struct RUNTIME_FIELD* firstField = RtlVectorGet(&target->declared->fields,0);
                                switch (firstField->declared->inlined)
                                {
                                    case BASE_INTPTR:
                                    {
                                        slot.type = MACHINE_INTPTR;
                                        INTPTR *ptrValue = ptr;
                                        slot.pointer = *ptrValue;
                                        break;
                                    }
                                    case BASE_INT64:
                                    {
                                        slot.type = MACHINE_INT64;
                                        INT64 *ptrValue = ptr;
                                        slot.int64 = *ptrValue;
                                        break;
                                    }
                                    case BASE_INT32:
                                    {
                                        slot.type = MACHINE_INT32;
                                        INT32 *ptrValue = ptr;
                                        slot.int32 = *ptrValue;
                                        break;
                                    }
                                    case BASE_INT16:
                                    {
                                        slot.type = MACHINE_INT32;
                                        INT16 *ptrValue = ptr;
                                        slot.int32 = *ptrValue;
                                        break;
                                    }
                                    case BASE_SINGLE:
                                    {
                                        slot.type = MACHINE_MFLOAT;
                                        SINGLE *ptrValue = ptr;
                                        slot.floating = *ptrValue;
                                        break;
                                    }
                                    case BASE_DOUBLE:
                                    {
                                        slot.type = MACHINE_MFLOAT;
                                        DOUBLE *ptrValue = ptr;
                                        slot.floating = *ptrValue;
                                        break;
                                    }
                                    case BASE_BYTE:
                                    {
                                        slot.type = MACHINE_INT32;
                                        BYTE *ptrValue = ptr;
                                        slot.int32 = *ptrValue;
                                        break;
                                    }
                                    case BASE_CHAR:
                                    {
                                        slot.type = MACHINE_INT32;
                                        WCHAR *ptrValue = ptr;
                                        slot.int32 = *ptrValue;
                                        break;
                                    }
                                    default:
                                    {
                                        ExThrowException(&ExNullReference);
                                        goto ExceptionHandlingZone;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                slot.type = MACHINE_STRUCT;
                                slot.valueType.type = target->declared;
                                slot.valueType.pointer = __builtin_alloca(slot.valueType.type->size);
                                PalMemoryCopy(slot.valueType.pointer, ptr, slot.valueType.type->size);
                                break;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExNullReference);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                ExPush(frame, slot);
                break;
            }
            case OpLoadStaticField:
            {
                struct RUNTIME_FIELD *target = ExGetPoolElement(&reader, frame);

                struct RUNTIME_FRAME_BLOCK slot = {0};

                switch (target->declared->inlined)
                {
                    case BASE_BYTE:
                    case BASE_CHAR:
                    case BASE_INT16:
                    case BASE_INT32:
                    {
                        slot.type = MACHINE_INT32;
                        PalMemoryCopy(&slot.int32, target->staticValue, target->dataSize);
                        break;
                    }
                    case BASE_INT64:
                    {
                        slot.type = MACHINE_INT64;
                        PalMemoryCopy(&slot.int64, target->staticValue, target->dataSize);
                        break;
                    }
                    case BASE_SINGLE:
                    {
                        slot.type = MACHINE_MFLOAT;

                        void *ptr = target->staticValue;
                        SINGLE *ptrValue = ptr;
                        slot.floating = *ptrValue;

                        break;
                    }
                    case BASE_DOUBLE:
                    {
                        slot.type = MACHINE_MFLOAT;

                        void *ptr = target->staticValue;
                        DOUBLE *ptrValue = ptr;
                        slot.floating = *ptrValue;

                        break;
                    }
                    case BASE_INTPTR:
                    {
                        slot.type = MACHINE_INTPTR;

                        void *ptr = target->staticValue;

                        PalMemoryCopy(&slot.floating, ptr, target->dataSize);
                        break;
                    }
                    case BASE_OTHER:
                    {
                        if (!ExMetadataIs(target->declared->metadata, MxExMetadataStruct))
                        {
                            slot.type = MACHINE_OBJECT;
                            PalMemoryCopy(&slot.descriptor, target->staticValue, target->dataSize);
                            break;
                        }
                        else
                        {
                            slot.type = MACHINE_STRUCT;
                            slot.valueType.type = target->declared;
                            slot.valueType.pointer = __builtin_alloca(slot.valueType.type->size);
                            PalMemoryCopy(slot.valueType.pointer, target->staticValue, slot.valueType.type->size);
                            break;
                        }
                    }
                    default:
                    {
                        ExThrowException(&ExNullReference);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                ExPush(frame, slot);
                break;
            }
            case OpStoreValueField:
            {
                struct RUNTIME_FIELD *target = ExGetPoolElement(&reader, frame);

                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK source = ExPop(frame);

                struct OBJECT_HEADER *objData = source.descriptor;

                if (MARX_FAIL(ExNullCheck(source)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                void *storePointer = NULL;

                if (source.type == MACHINE_MANAGED_POINTER)
                {
                    storePointer = source.link.pointer;
                    storePointer += target->offset;
                }
                else if (source.type == MACHINE_OBJECT)
                {
                    storePointer = source.descriptor;

                    struct MANAGED_DELEGATE *delegate = storePointer;

                    storePointer += sizeof(struct OBJECT_HEADER);
                    storePointer += target->offset;
                }
                else
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                    break;
                }

                switch (target->declared->inlined)
                {
                    case BASE_BYTE:
                    case BASE_CHAR:
                    case BASE_INT16:
                    case BASE_INT32:
                    {
                        PalMemoryCopy(storePointer, &value.int32, target->dataSize);
                        break;
                    }
                    case BASE_INT64:
                    {
                        PalMemoryCopy(storePointer, &value.int64, target->dataSize);
                        break;
                    }
                    case BASE_SINGLE:
                    {
                        SINGLE slotValue = value.floating;
                        PalMemoryCopy(storePointer, &slotValue, target->dataSize);
                        break;
                    }
                    case BASE_DOUBLE:
                    {
                        DOUBLE slotValue = value.floating;
                        PalMemoryCopy(storePointer, &slotValue, target->dataSize);
                        break;
                    }
                    case BASE_INTPTR:
                    {
                        PalMemoryCopy(storePointer, &value.pointer, target->dataSize);
                        break;
                    }
                    case BASE_OTHER:
                    {
                        if (ExMetadataIs(target->declared->metadata, MxExMetadataStruct))
                        {
                            if (ExMetadataIs(target->declared->metadata, MxExMetadataEnum))
                            {
                                switch (value.type)
                                {
                                    case MACHINE_INT32:
                                    {
                                        PalMemoryCopy(storePointer, &value.int32, target->dataSize);
                                        break;
                                    }
                                    case MACHINE_INT64:
                                    {
                                        PalMemoryCopy(storePointer, &value.int64, target->dataSize);
                                        break;
                                    }
                                    case MACHINE_INTPTR:
                                    {
                                        PalMemoryCopy(storePointer, &value.pointer, target->dataSize);
                                        break;
                                    }
                                    case MACHINE_MFLOAT:
                                    {
                                        PalMemoryCopy(storePointer, &value.floating, target->dataSize);
                                        break;
                                    }
                                    default:
                                    {
                                        ExThrowException(&ExNullReference);
                                        goto ExceptionHandlingZone;
                                        break;
                                    };
                                }
                            }
                            else
                            {
                                PalMemoryCopy(storePointer, value.valueType.pointer, target->dataSize);
                                break;
                            }
                        }
                        else
                        {
                            PalMemoryCopy(storePointer, &value.descriptor, target->dataSize);
                            break;
                        }
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExNullReference);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                break;
            }
            case OpStoreStaticField:
            {
                struct RUNTIME_FIELD *target = ExGetPoolElement(&reader, frame);

                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);

                void *storePointer = target->staticValue;

                switch (target->declared->inlined)
                {
                    case BASE_BYTE:
                    case BASE_CHAR:
                    case BASE_INT16:
                    case BASE_INT32:
                    {
                        PalMemoryCopy(storePointer, &value.int32, target->dataSize);
                        break;
                    }
                    case BASE_INT64:
                    {
                        PalMemoryCopy(storePointer, &value.int64, target->dataSize);
                        break;
                    }
                    case BASE_SINGLE:
                    {
                        SINGLE slotValue = value.floating;
                        PalMemoryCopy(storePointer, &slotValue, target->dataSize);
                        break;
                    }
                    case BASE_DOUBLE:
                    {
                        DOUBLE slotValue = value.floating;
                        PalMemoryCopy(storePointer, &slotValue, target->dataSize);
                        break;
                    }
                    case BASE_INTPTR:
                    {
                        PalMemoryCopy(storePointer, &value.pointer, target->dataSize);
                        break;
                    }
                    case BASE_OTHER:
                    {
                        PalMemoryCopy(storePointer, &value.descriptor, target->dataSize);
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExNullReference);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                break;
            }
            case OpLoadArgument:
            {
                INT32 index = RtlReaderReadInt32(&reader);
                ExPush(frame, frame->args[index]);
                break;
            }
            case OpStoreArgument:
            {
                INT32 index = RtlReaderReadInt32(&reader);
                frame->args[index] = ExPop(frame);
                break;
            }
            case OpLoadLocal:
            {
                INT32 index = RtlReaderReadInt32(&reader);

                if (frame->variables[index].type == MACHINE_STRUCT)
                {
                    struct RUNTIME_FRAME_BLOCK clone = {0};
                    clone.type = MACHINE_STRUCT;
                    clone.valueType.type = frame->variables[index].valueType.type;
                    clone.valueType.pointer = PalStackAllocate(clone.valueType.type->size);
                    PalMemoryCopy(clone.valueType.pointer,
                           frame->variables[index].valueType.pointer,
                           clone.valueType.type->size);
                    ExPush(frame, clone);
                }
                else
                {
                    ExPush(frame, frame->variables[index]);
                }

                break;
            }
            case OpStoreLocal:
            {
                INT32 index = RtlReaderReadInt32(&reader);
                struct RUNTIME_FRAME_BLOCK peek = ExPop(frame);

                if (peek.type == MACHINE_STRUCT)
                {
                    PalMemoryCopy(frame->variables[index].valueType.pointer,
                           peek.valueType.pointer,
                           peek.valueType.type->size);
                }
                else
                {
                    frame->variables[index] = peek;
                }

                break;
            }
            case OpLoadNull:
            {
                struct RUNTIME_FRAME_BLOCK nullPtr;

                nullPtr.type = MACHINE_OBJECT;
                nullPtr.descriptor = NULL;

                ExPush(frame, nullPtr);
                break;
            }
            case OpLoadString:
            {
                INT32 index = RtlReaderReadInt32(&reader);
                struct NSTRING *ptr = RtlVectorGet(&frame->method->stringTable, index);

                struct MANAGED_STRING *string = HpAllocateManaged(sizeof(struct MANAGED_STRING));
                string->header.type = ExStringType;

                struct RUNTIME_FRAME_BLOCK block = {
                    .type = MACHINE_OBJECT,
                    .descriptor = string
                };

                ExPush(frame, block);

                struct MANAGED_ARRAY *array = ObManagedArrayInitialize(ptr->length, sizeof(WCHAR));

                array->elementType = ExCharType;
                array->header.type = ExCharArrayType;
                array->count = ptr->length;

                PalMemoryCopy(array->characters, ptr->characters, sizeof(WCHAR) * array->count);

                string = ExPeek(frame).descriptor;
                string->characters = array;
                break;
            }
            case OpLoadMethodDescriptor:
            {
                struct RUNTIME_METHOD *method = ExGetPoolElement(&reader, frame);

                struct RUNTIME_FRAME_BLOCK block;
                block.type = MACHINE_INTPTR;
                block.descriptor = method;

                ExPush(frame, block);
                break;
            }
            case OpLoadVirtualMethodDescriptor:
            {
                struct RUNTIME_METHOD *method = ExGetPoolElement(&reader, frame);
                struct RUNTIME_METHOD *virt;
                struct RUNTIME_FRAME_BLOCK obj = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(obj)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_SUCCESS(ExLocateVirtualMethod(&obj,method,&virt)))
                {
                    struct RUNTIME_FRAME_BLOCK block;
                    block.type = MACHINE_INTPTR;
                    block.descriptor = virt;

                    ExPush(frame, block);
                }
                else
                {
                    ExThrowException(&ExExecutionEngineError);
                    goto ExceptionHandlingZone;
                }

                break;
            }
            case OpNewArray:
            {
                struct RUNTIME_TYPE *type = ExGetPoolElement(&reader, frame);
                struct RUNTIME_TYPE *arrayType = ExGetPoolElement(&reader, frame);

                struct MANAGED_ARRAY *managed = NULL;

                struct RUNTIME_FRAME_BLOCK slot = ExPop(frame);

                if (ExMetadataIs(type->metadata, MxExMetadataStruct))
                {
                    managed = ObManagedArrayInitialize(slot.int32, type->size);
                }
                else
                {
                    managed = ObManagedArrayInitialize(slot.int32, sizeof(VOID *));
                }

                managed->header.type = arrayType;
                managed->elementType = type;
                managed->count = slot.int32;

                struct RUNTIME_FRAME_BLOCK newSlot = {0};
                newSlot.type = MACHINE_OBJECT;
                newSlot.descriptor = managed;

                ExPush(frame, newSlot);
                break;
            }
            case OpInitializeObject:
            {
                struct RUNTIME_TYPE *type = ExGetPoolElement(&reader, frame);

                struct RUNTIME_FRAME_BLOCK top = ExPeek(frame);

                if (top.type == MACHINE_MANAGED_POINTER)
                {
                    PalMemoryZero(top.link.pointer, type->size);
                }
                else if (top.type == MACHINE_INTPTR)
                {
                    PalMemoryZero((VOID *) top.pointer, type->size);
                }
                else
                {
                    ExThrowException(&ExExecutionEngineError);
                    goto ExceptionHandlingZone;
                }

                break;
            }
            case OpNewObject:
            {
                PalSafepoint();

                struct RUNTIME_METHOD *ctor = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK ret;
                struct RUNTIME_FRAME_BLOCK block;

                VOID *instance = NULL;
                if (ExMetadataIs(ctor->owner->metadata, MxExMetadataStruct))
                {
                    block.type = MACHINE_STRUCT;
                    instance = PalStackAllocate(ctor->owner->size);
                    PalMemoryZero(instance, ctor->owner->size);
                    block.valueType.type = ctor->owner;
                    block.valueType.pointer = instance;
                }
                else
                {
                    block.type = MACHINE_OBJECT;
                    instance = HpAllocateManaged(ctor->owner->size + sizeof(struct OBJECT_HEADER));
                    struct OBJECT_HEADER *obj = instance;
                    obj->type = ctor->owner;
                    block.descriptor = instance;
                }

                ExPush(frame, block);

                if (ExMetadataIs(ctor->metadata, MxExMetadataExtern))
                {
                    MARX_STATUS retInfo = ExMethodPrologueCtorNative(ctor, frame, instance, &ret);
                    if (MARX_FAIL(retInfo))
                    {
                        ExMethodEpilogue(frame);

                        if (retInfo != MARX_STATUS_EXCEPTION)
                        {
                            ExThrowException(&ExExecutionEngineError);
                        }

                        goto ExceptionHandlingZone;
                    }
                    ExMethodEpilogue(frame);
                }
                else
                {
                    MARX_STATUS retInfo = ExMethodPrologueCtor(ctor, frame, instance, &ret);
                    if (MARX_FAIL(retInfo))
                    {
                        ExMethodEpilogue(frame);

                        if (retInfo != MARX_STATUS_EXCEPTION)
                        {
                            ExThrowException(&ExExecutionEngineError);
                        }

                        goto ExceptionHandlingZone;
                    }
                    ExMethodEpilogue(frame);
                }

                break;
            }
            case OpLoadImmediateInt32:
            {
                struct RUNTIME_FRAME_BLOCK block;

                block.type = MACHINE_INT32;
                block.int32 = RtlReaderReadInt32(&reader);

                ExPush(frame, block);
                break;
            }
            case OpLoadImmediateInt64:
            {
                struct RUNTIME_FRAME_BLOCK block;

                block.type = MACHINE_INT64;
                block.int64 = RtlReaderReadInt64(&reader);

                ExPush(frame, block);
                break;
            }
            case OpLoadImmediateFloat:
            {
                struct RUNTIME_FRAME_BLOCK block;

                block.type = MACHINE_MFLOAT;
                block.floating = RtlReaderReadSingle(&reader);

                ExPush(frame, block);
                break;
            }
            case OpLoadImmediateDouble:
            {
                struct RUNTIME_FRAME_BLOCK block;

                block.type = MACHINE_MFLOAT;
                block.floating = RtlReaderReadDouble(&reader);

                ExPush(frame, block);
                break;
            }
            case OpLoadNativeIntFromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_INTPTR;
                newValue.pointer = arrayReference->nint[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadInt8FromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_INT32;
                newValue.int32 = arrayReference->byte[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadInt16FromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_INT32;
                newValue.int32 = arrayReference->int16[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadInt32FromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_INT32;
                newValue.int32 = arrayReference->int32[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadInt64FromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_INT64;
                newValue.int64 = arrayReference->int64[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadFloatFromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_MFLOAT;
                newValue.floating = arrayReference->single[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadDoubleFromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_MFLOAT;
                newValue.floating = arrayReference->duoble[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadObjectFromArray:
            {
                struct RUNTIME_FRAME_BLOCK index;
                struct RUNTIME_FRAME_BLOCK array;

                index = ExPop(frame);
                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_OBJECT;
                newValue.descriptor = arrayReference->pointer[index.int32];

                ExPush(frame, newValue);
                break;
            }
            case OpLoadArrayLength:
            {
                struct RUNTIME_FRAME_BLOCK array;

                array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayReference = array.descriptor;

                struct RUNTIME_FRAME_BLOCK newValue;

                newValue.type = MACHINE_INT32;
                newValue.int32 = arrayReference->count;

                ExPush(frame, newValue);
                break;
            }
            case OpStoreNativeIntToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->nint[index.int32] = value.pointer;
                break;
            }
            case OpStoreInt8ToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->byte[index.int32] = value.int32;
                break;
            }
            case OpStoreInt16ToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->int16[index.int32] = value.int32;
                break;
            }
            case OpStoreInt32ToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->int32[index.int32] = value.int32;
                break;
            }
            case OpStoreInt64ToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->int64[index.int32] = value.int64;
                break;
            }
            case OpStoreFloatToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->single[index.int32] = value.floating;
                break;
            }
            case OpStoreDoubleToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->duoble[index.int32] = value.floating;
                break;
            }
            case OpStoreObjectToArray:
            {
                struct RUNTIME_FRAME_BLOCK value = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK index = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK array = ExPop(frame);

                if (MARX_FAIL(ExNullCheck(array)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExBoundCheck(array,index.int32)))
                {
                    ExThrowException(&ExIndexOutOfRange);
                    goto ExceptionHandlingZone;
                }

                struct MANAGED_ARRAY *arrayValue = array.descriptor;
                arrayValue->pointer[index.int32] = value.descriptor;
                break;
            }
            case OpBranchIfEquals:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsEquals(first, second))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranchIfGreaterOrEqual:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsGreaterEquals(first, second))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranchIfGreater:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsGreater(first, second))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranchIfLessOrEqual:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsLowerEquals(first, second))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranchIfLess:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsLower(first, second))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranchIfZero:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsZero(first))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranchIfNonZero:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsNonZero(first))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpBranch:
            {
                PalSafepoint();

                INT32 offset = RtlReaderReadInt32(&reader);
                RtlReaderSet(&reader, offset);
                break;
            }
            case OpBranchIfUnequalUnordered:
            {
                INT32 offset = RtlReaderReadInt32(&reader);

                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                if (ExIsUnEquals(first, second))
                {
                    RtlReaderSet(&reader, offset);
                }
                break;
            }
            case OpPushOneIfEqual:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                struct RUNTIME_FRAME_BLOCK result;
                result.type = MACHINE_INT32;

                if (ExIsEquals(first, second))
                {
                    result.int32 = TRUE;
                }
                else
                {
                    result.int32 = FALSE;
                }

                ExPush(frame, result);
                break;
            }
            case OpPushOneIfGreaterUn:
            case OpPushOneIfGreater:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                struct RUNTIME_FRAME_BLOCK result;
                result.type = MACHINE_INT32;

                if (ExIsGreater(first, second))
                {
                    result.int32 = TRUE;
                }
                else
                {
                    result.int32 = FALSE;
                }

                ExPush(frame, result);
                break;
            }
            case OpPushOneIfLowerUn:
            case OpPushOneIfLower:
            {
                struct RUNTIME_FRAME_BLOCK second = ExPop(frame);
                struct RUNTIME_FRAME_BLOCK first = ExPop(frame);

                struct RUNTIME_FRAME_BLOCK result;
                result.type = MACHINE_INT32;

                if (ExIsLower(first, second))
                {
                    result.int32 = TRUE;
                }
                else
                {
                    result.int32 = FALSE;
                }

                ExPush(frame, result);
                break;
            }
            case OpCall:
            {
                PalSafepoint();

                struct RUNTIME_METHOD *target = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK ret;
                MARX_STATUS state;

                if (ExMetadataIs(target->metadata, MxExMetadataExtern))
                {
                    state = ExMethodPrologueArgsNative(target,NULL, frame, &ret);
                    ExMethodEpilogue(frame);
                }
                else
                {
                    state = ExMethodPrologueArgs(target,NULL, frame, &ret);
                    ExMethodEpilogue(frame);
                }

                if (MARX_SUCCESS(state))
                {
                    if (target->isReturns)
                    {
                        ExPush(frame, ret);
                    }
                }
                else
                {
                    if (state != MARX_STATUS_EXCEPTION)
                    {
                        ExThrowException(&ExExecutionEngineError);
                    }
                    goto ExceptionHandlingZone;
                }

                break;
            }
            case OpVirtualCall:
            {
                PalSafepoint();

                struct RUNTIME_METHOD *target = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK *hed = &frame->stack[frame->sp - target->parameters.count];
                struct RUNTIME_METHOD *virt;
                struct RUNTIME_FRAME_BLOCK ret;
                MARX_STATUS state;

                if (MARX_FAIL(ExNullCheck(*hed)))
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                if (MARX_FAIL(ExLocateVirtualMethod(hed,target,&virt)) && !ExMetadataIs(target->owner->metadata,
                        MxExMetadataDelegate))
                {
                    ExThrowException(&ExExecutionEngineError);
                    goto ExceptionHandlingZone;
                }

                if (ExMetadataIs(virt->owner->metadata, MxExMetadataDelegate) && RtlNStringCompare(
                        virt->shortName,
                        "Invoke"))
                {
                    struct RUNTIME_FRAME_BLOCK *frameInfo = &frame->stack[frame->sp - target->parameters.count];
                    state = ExMethodPrologueDelegate(frameInfo,NULL, frame, &ret);
                    ExPop(frame);
                    ExMethodEpilogue(frame);
                }
                else if (ExMetadataIs(virt->metadata, MxExMetadataExtern))
                {
                    state = ExMethodPrologueArgsNative(virt,NULL, frame, &ret);
                    ExMethodEpilogue(frame);
                }
                else
                {
                    state = ExMethodPrologueArgs(virt,NULL, frame, &ret);
                    ExMethodEpilogue(frame);
                }

                if (MARX_SUCCESS(state))
                {
                    if (target->isReturns)
                    {
                        ExPush(frame, ret);
                    }
                }
                else
                {
                    if (state != MARX_STATUS_EXCEPTION)
                    {
                        ExThrowException(&ExExecutionEngineError);
                    }
                    goto ExceptionHandlingZone;
                }

                break;
            }
            case OpReturn:
            {
                if (frame->method->isReturns)
                {
                    struct RUNTIME_FRAME_BLOCK block = ExPop(frame);
                    *returnValue = block;
                    return MARX_STATUS_SUCCESS;
                }
                else
                {
                    return MARX_STATUS_SUCCESS;
                }

                break;
            }
            case OpIsInstance:
            {
                struct RUNTIME_TYPE *castTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK block = ExPeek(frame);

                if (block.descriptor == NULL)
                {
                    break;
                }
                else
                {
                    ExPop(frame);
                }

                struct OBJECT_HEADER *header = block.descriptor;

                if (MARX_SUCCESS(ExIsInstance(castTarget,header->type)))
                {
                }
                else
                {
                    block.type = MACHINE_OBJECT;
                    block.descriptor = NULL;
                }

                ExPush(frame, block);
                break;
            }
            case OpCastClass:
            {
                struct RUNTIME_TYPE *castTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK block = ExPeek(frame);

                if (block.descriptor == NULL)
                {
                    break;
                }
                else
                {
                    ExPop(frame);
                }

                struct OBJECT_HEADER *header = block.descriptor;

                if (MARX_SUCCESS(ExIsInstance(castTarget,header->type)))
                {
                }
                else
                {
                    ExThrowException(&ExExecutionEngineError);
                    goto ExceptionHandlingZone;
                }

                ExPush(frame, block);
                break;
            }
            case OpBox:
            {
                PalSafepoint();

                struct RUNTIME_TYPE *loadTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK peek = ExPop(frame);

                if (!ExMetadataIs(loadTarget->metadata, MxExMetadataStruct))
                {
                    ExThrowException(&ExExecutionEngineError);
                    goto ExceptionHandlingZone;
                }

                struct OBJECT_HEADER *reference = HpAllocateManaged(loadTarget->size + sizeof(struct OBJECT_HEADER));
                reference->type = loadTarget;

                void *pointerToData = reference;
                pointerToData += sizeof(struct OBJECT_HEADER);

                switch (loadTarget->inlined)
                {
                    case BASE_BYTE:
                    case BASE_CHAR:
                    case BASE_INT16:
                    case BASE_INT32:
                    {
                        PalMemoryCopy(pointerToData, &peek.int32, loadTarget->size);
                        break;
                    }
                    case BASE_INT64:
                    {
                        PalMemoryCopy(pointerToData, &peek.int64, loadTarget->size);
                        break;
                    }
                    case BASE_SINGLE:
                    {
                        SINGLE valueTarget = peek.floating;
                        PalMemoryCopy(pointerToData, &valueTarget, loadTarget->size);
                        break;
                    }
                    case BASE_DOUBLE:
                    {
                        DOUBLE valueTarget = peek.floating;
                        PalMemoryCopy(pointerToData, &valueTarget, loadTarget->size);
                        break;
                    }
                    case BASE_INTPTR:
                    {
                        PalMemoryCopy(pointerToData, &peek.pointer, loadTarget->size);
                        break;
                    }
                    case BASE_OTHER:
                    {
                        PalMemoryCopy(pointerToData, &peek.valueType.pointer, loadTarget->size);
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                struct RUNTIME_FRAME_BLOCK boxed = {
                    boxed.type = MACHINE_OBJECT,
                    boxed.descriptor = reference
                };

                ExPush(frame, boxed);
                break;
            }
            case OpUnboxToPointer:
            {
                struct RUNTIME_TYPE *loadTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK peek = ExPop(frame);

                if (peek.type != MACHINE_OBJECT || !ExMetadataIs(loadTarget->metadata, MxExMetadataStruct))
                {
                    ExThrowException(&ExExecutionEngineError);
                    goto ExceptionHandlingZone;
                    break;
                }

                void *dataPointer = peek.descriptor;
                dataPointer += sizeof(struct OBJECT_HEADER);

                struct RUNTIME_FRAME_BLOCK unboxed = {
                    unboxed.type = MACHINE_MANAGED_POINTER,
                    unboxed.link.type = loadTarget
                };

                unboxed.link.pointer = PalStackAllocate(loadTarget->size);
                PalMemoryCopy(unboxed.link.pointer, dataPointer, loadTarget->size);

                ExPush(frame, unboxed);
                break;
            }
            case OpUnboxToValue:
            {
                struct RUNTIME_TYPE *loadTarget = ExGetPoolElement(&reader, frame);
                struct RUNTIME_FRAME_BLOCK peek = ExPop(frame);

                void *dataPointer = peek.descriptor;
                dataPointer += sizeof(struct OBJECT_HEADER);

                struct RUNTIME_FRAME_BLOCK unboxed = {
                    unboxed.type = MACHINE_OBJECT,
                    unboxed.link.type = loadTarget
                };

                switch (loadTarget->inlined)
                {
                    case BASE_BYTE:
                    case BASE_CHAR:
                    case BASE_INT16:
                    case BASE_INT32:
                    {
                        unboxed.type = MACHINE_INT32;
                        PalMemoryCopy(&unboxed.int32, dataPointer, loadTarget->size);
                        break;
                    }
                    case BASE_INT64:
                    {
                        unboxed.type = MACHINE_INT64;
                        PalMemoryCopy(&unboxed.int64, dataPointer, loadTarget->size);
                        break;
                    }
                    case BASE_SINGLE:
                    {
                        unboxed.type = MACHINE_MFLOAT;
                        SINGLE *floatingValue = dataPointer;
                        unboxed.floating = *floatingValue;
                        break;
                    }
                    case BASE_DOUBLE:
                    {
                        unboxed.type = MACHINE_MFLOAT;
                        DOUBLE *floatingValue = dataPointer;
                        unboxed.floating = *floatingValue;
                        break;
                    }
                    case BASE_INTPTR:
                    {
                        unboxed.type = MACHINE_INTPTR;
                        PalMemoryCopy(&unboxed.pointer, dataPointer, loadTarget->size);
                        break;
                    }
                    case BASE_OTHER:
                    {
                        unboxed.type = MACHINE_STRUCT;
                        unboxed.valueType.type = loadTarget;
                        unboxed.valueType.pointer = PalStackAllocate(loadTarget->size);
                        PalMemoryCopy(unboxed.valueType.pointer, dataPointer, loadTarget->size);
                        break;
                    }
                    default:
                    {
                        ExThrowException(&ExExecutionEngineError);
                        goto ExceptionHandlingZone;
                        break;
                    }
                }

                ExPush(frame, unboxed);
                break;
            }
            case OpRethrowException:
            case OpThrowException:
            {
                struct RUNTIME_FRAME_BLOCK block = ExPop(frame);

                if (MARX_SUCCESS(ExNullCheck(block)))
                {
                    ExThrowException(block.descriptor);
                    goto ExceptionHandlingZone;
                }
                else
                {
                    ExThrowException(&ExNullReference);
                    goto ExceptionHandlingZone;
                }

                break;
            }
            case OpEndFinallyException:
            {
                break;
            }
            case OpLeaveException:
            {
                struct RUNTIME_EXCEPTION_HANDLER finallyHandler;
                if (MARX_SUCCESS(ExLocateFinally(&finallyHandler,frame)))
                {
                    RtlReaderSet(&reader, finallyHandler.handlerStart);
                }

                break;
            }
            default:
            {
                ExThrowException(&ExExecutionEngineError);
                goto ExceptionHandlingZone;
                break;
            };
        }
    }
}
