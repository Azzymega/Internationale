#include <intermarx/ex/ex.h>
#include <intermarx/ex/runtime.h>
#include <intermarx/far/far.h>
#include <intermarx/hp/hp.h>
#include <intermarx/ob/ob.h>
#include <intermarx/pal/corelib.h>
#include <intermarx/pal/pal.h>
#include <mm/mm.h>
#include <pal/pal.h>
#include <rtl/rtl.h>

INUNATIVE INUEXTERN INTPTR NgAsmNativeInvokeTrampolineIntPtr(VOID *constructedStack, VOID *callTarget);
INUNATIVE INUEXTERN INT32 NgAsmNativeInvokeTrampolineInt32(VOID *constructedStack, VOID *callTarget);
INUNATIVE INUEXTERN INT64 NgAsmNativeInvokeTrampolineInt64(VOID *constructedStack, VOID *callTarget);
INUNATIVE INUEXTERN SINGLE NgAsmNativeInvokeTrampolineSingle(VOID *constructedStack, VOID *callTarget);
INUNATIVE INUEXTERN DOUBLE NgAsmNativeInvokeTrampolineDouble(VOID *constructedStack, VOID *callTarget);
INUNATIVE INUEXTERN VOID *NgAsmNativeInvokeTrampolinePointer(VOID *constructedStack, VOID *callTarget);

INUIMPORT struct RUNTIME_TYPE *ExStringType;
INUIMPORT struct RUNTIME_TYPE *ExCharArrayType;
INUIMPORT struct RUNTIME_TYPE *ExCharType;
INUIMPORT struct RUNTIME_TYPE *ExThreadType;

MARX_STATUS FarInitialize(struct RUNTIME_DOMAIN *domain)
{
    struct FAR_IMPORT import[] = {
        {
            L"PalManagedThreadCreate", &PalManagedThreadCreate
        },
        {
            L"PalManagedThreadStart", &PalManagedThreadStart
        },
        {
            L"PalManagedThreadId", &PalManagedThreadId
        },
        {
            L"PalManagedThreadGetCurrent", &PalManagedThreadGetCurrent
        },
        {
            L"PalTimerClock", &PalTimerClock
        },
        {
            L"PalLoggerPrint", &PalLoggerPrint
        },
        {
            L"PalManagedDelegateCombineImplNative", &PalManagedDelegateCombineImplNative
        },
        {
            L"PalManagedDelegateRemoveImplNative", &PalManagedDelegateRemoveImplNative
        },
        {
            L"PalX86BiosCall",&PalX86BiosCall
        },
        {
            L"PalObjectToString",&PalObjectToString
        },
        {
            L"PalX86GetBiosCallBuffer", &PalX86GetBiosCallBuffer
        },
        {
            L"PalBufferMemoryCopy", &PalBufferMemoryCopy
        },
        {
            L"PalBufferMemorySet", &PalBufferMemorySet
        },
        {
            L"PalBufferMemorySetBlock", &PalBufferMemorySetBlock
        }
    };

    for (int i = 0; i < domain->types.count; ++i)
    {
        struct RUNTIME_TYPE *type = RtlVectorGet(&domain->types, i);
        for (int j = 0; j < type->methods.count; ++j)
        {
            struct RUNTIME_METHOD *method = RtlVectorGet(&type->methods, j);
            for (int z = 0; z < method->attributes.count; ++z)
            {
                struct FAR_CALL_MANAGED_ATTRIBUTE *attribute = RtlVectorGet(&method->attributes, z);
                if (RtlNStringCompare(attribute->base.header.type->fullName, ExFarCallTypeName))
                {
                    UINTPTR importLength = sizeof import / sizeof(struct FAR_IMPORT);
                    for (int r = 0; r < importLength; ++r)
                    {
                        BOOLEAN valid = TRUE;

                        if (attribute->functionName->characters->count != PalWStringLength(import[r].name))
                        {
                            valid = FALSE;
                        }

                        for (int p = 0; p < attribute->functionName->characters->count; ++p)
                        {
                            if (attribute->functionName->characters->characters[p] != import[r].name[p])
                            {
                                valid = FALSE;
                                break;
                            }
                        }

                        if (valid)
                        {
                            method->farCall.isInitialized = TRUE;
                            method->farCall.function = import[r].function;
                            method->farCall.convention = attribute->convention;
                            method->farCall.encoding = attribute->encoding;
                            method->farCall.source = attribute->source;

                            PalPrint("%w is imported and linked!\r\n", import[r].name);

                            break;
                        }
                    }
                }
            }
        }
    }

    return MARX_STATUS_SUCCESS;
}

MARX_STATUS FarLoadArgument(void *argument, UINTPTR argumentSize, void *stack, UINTPTR *stackPtr)
{
    *stackPtr -= argumentSize;
    const UINTPTR stackPointer = *stackPtr;
    PalMemoryCopy(&stack[stackPointer], argument, argumentSize);
    return MARX_STATUS_SUCCESS;
}

MARX_STATUS FarNativeMethodExecute(struct RUNTIME_FRAME *frame, struct RUNTIME_FRAME_BLOCK *returnValue)
{
    if (frame == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    if (frame->method == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    if (frame->method->farCall.function == NULL)
    {
        return MARX_STATUS_FAIL;
    }

    BYTE stackFrame[16384];
    UINTPTR stackFramePointer = 16384;

    for (int i = frame->method->parameters.count - 1; i >= 0; --i)
    {
        if (frame->args[i].type == MACHINE_OBJECT)
        {
            struct RUNTIME_TYPE *typeValueInfo = RtlVectorGet(&frame->method->parameters, i);
            if (RtlNStringCompare(typeValueInfo->fullName, ExStringTypeName))
            {
                struct MANAGED_STRING *str = frame->args[i].descriptor;

                if (str == NULL)
                {
                    VOID *nullPtr = NULL;
                    FarLoadArgument(&nullPtr, sizeof(void *), stackFrame, &stackFramePointer);
                }
                else
                {
                    switch (frame->method->farCall.encoding)
                    {
                        case STRING_ENCODING_ISO646:
                        {
                            char *buffer = PalStackAllocate((str->characters->count + 1) * sizeof(CHAR));

                            for (UINTPTR z = 0; z < str->characters->count; ++z)
                            {
                                buffer[z] = str->characters->characters[z];
                            }
                            buffer[str->characters->count] = '\0';

                            FarLoadArgument(&buffer, sizeof(void *), stackFrame, &stackFramePointer);
                            break;
                        }
                        case STRING_ENCODING_UCS2:
                        {
                            wchar_t *buffer = PalStackAllocate((str->characters->count + 1) * sizeof(WCHAR));

                            for (UINTPTR z = 0; z < str->characters->count; ++z)
                            {
                                buffer[z] = str->characters->characters[z];
                            }
                            buffer[str->characters->count] = '\0';

                            FarLoadArgument(&buffer, sizeof(void *), stackFrame, &stackFramePointer);
                            break;
                        }
                        case STRING_ENCODING_NONE:
                        {
                            FarLoadArgument(&str, sizeof(void *), stackFrame, &stackFramePointer);
                            break;
                        }
                        default:
                        {
                            return MARX_STATUS_FAIL;
                        }
                    }
                }
            }
            else
            {
                FarLoadArgument(&frame->args[i].descriptor, sizeof(void *), stackFrame, &stackFramePointer);
            }
        }
        else if (frame->args[i].type == MACHINE_INT32)
        {
            FarLoadArgument(&frame->args[i].int32, sizeof(INT32), stackFrame, &stackFramePointer);
        }
        else if (frame->args[i].type == MACHINE_INT64)
        {
            FarLoadArgument(&frame->args[i].int64, sizeof(INT64), stackFrame, &stackFramePointer);
        }
        else if (frame->args[i].type == MACHINE_INTPTR)
        {
            FarLoadArgument(&frame->args[i].pointer, sizeof(INTPTR), stackFrame, &stackFramePointer);
        }
        else if (frame->args[i].type == MACHINE_MANAGED_POINTER)
        {
            FarLoadArgument(&frame->args[i].link.pointer, sizeof(void *), stackFrame, &stackFramePointer);
        }
        else if (frame->args[i].type == MACHINE_STRUCT)
        {
            UINTPTR bufferSize = frame->args[i].valueType.type->size;

            BYTE* buffer = PalStackAllocate(bufferSize);
            PalMemoryZero(buffer,bufferSize);

            FarLoadArgument(&buffer, sizeof(void *), stackFrame, &stackFramePointer);
        }
        else if (frame->args[i].type == MACHINE_MFLOAT)
        {
            struct RUNTIME_TYPE *parameterType = RtlVectorGet(&frame->method->parameters, i);

            if (parameterType->inlined == BASE_SINGLE)
            {
                SINGLE value = frame->args[i].floating;
                FarLoadArgument(&value, sizeof(SINGLE), stackFrame, &stackFramePointer);
            }
            else if (parameterType->inlined == BASE_DOUBLE)
            {
                DOUBLE value = frame->args[i].floating;
                FarLoadArgument(&value, sizeof(DOUBLE), stackFrame, &stackFramePointer);
            }
            else
            {
                return MARX_STATUS_FAIL;
            }
        }
        else
        {
            return MARX_STATUS_FAIL;
        }
    }

    struct RUNTIME_FRAME_BLOCK slot;

    switch (frame->method->returnType->inlined)
    {
        case BASE_BYTE:
        case BASE_CHAR:
        case BASE_INT16:
        case BASE_INT32:
        {
            slot.type = MACHINE_INT32;
            slot.int32 = NgAsmNativeInvokeTrampolineInt32(&stackFrame[stackFramePointer],
                                                          frame->method->farCall.function);
            break;
        }
        case BASE_INTPTR:
        {
            slot.type = MACHINE_INTPTR;
            slot.pointer = NgAsmNativeInvokeTrampolineIntPtr(&stackFrame[stackFramePointer],
                                                             frame->method->farCall.function);
            break;
        }
        case BASE_VOID:
        {
            NgAsmNativeInvokeTrampolineInt32(&stackFrame[stackFramePointer], frame->method->farCall.function);

            break;
        }
        case BASE_INT64:
        {
            slot.type = MACHINE_INT64;
            slot.int64 = NgAsmNativeInvokeTrampolineInt64(&stackFrame[stackFramePointer],
                                                          frame->method->farCall.function);
            break;
        }
        case BASE_SINGLE:
        {
            slot.type = MACHINE_MFLOAT;
            slot.floating = NgAsmNativeInvokeTrampolineSingle(&stackFrame[stackFramePointer],
                                                              frame->method->farCall.function);
            break;
        }
        case BASE_DOUBLE:
        {
            slot.type = MACHINE_MFLOAT;
            slot.floating = NgAsmNativeInvokeTrampolineDouble(&stackFrame[stackFramePointer],
                                                              frame->method->farCall.function);
            break;
        }
        case BASE_OTHER:
        {
            slot.type = MACHINE_OBJECT;
            VOID* returnPointer = NgAsmNativeInvokeTrampolinePointer(&stackFrame[stackFramePointer],
                                                                 frame->method->farCall.function);

            if (frame->method->returnType == ExStringType)
            {
                struct MANAGED_STRING* newString = HpAllocateManaged(sizeof(struct MANAGED_STRING));

                struct RUNTIME_FRAME_BLOCK stringInfo = {
                    .type = MACHINE_OBJECT,
                    .descriptor = newString
                };
                ExPush(frame,stringInfo);
                struct RUNTIME_FRAME_BLOCK* blockPointer = &frame->stack[frame->sp-1];

                ((struct MANAGED_STRING*)blockPointer->descriptor)->header.type = ExStringType;

                if (frame->method->farCall.encoding == STRING_ENCODING_ISO646)
                {
                    CHAR* returnString = returnPointer;
                    UINTPTR returnStringLength = strlen(returnString);

                    struct MANAGED_ARRAY *array = ObManagedArrayInitialize(returnStringLength, sizeof(WCHAR));

                    array->elementType = ExCharType;
                    array->header.type = ExCharArrayType;
                    array->count = returnStringLength;
                    for (int i = 0; i < array->count; ++i)
                    {
                        array->characters[i] = returnString[i];
                    }
                    ((struct MANAGED_STRING*)blockPointer->descriptor)->characters = array;

                    slot.descriptor = blockPointer->descriptor;
                    ExPop(frame);
                    MmFreePoolMemory(returnPointer);
                }
                else if (frame->method->farCall.encoding == STRING_ENCODING_UCS2)
                {
                    WCHAR* returnString = returnPointer;
                    UINTPTR returnStringLength = wstrlen(returnString);

                    struct MANAGED_ARRAY *array = ObManagedArrayInitialize(returnStringLength, sizeof(WCHAR));

                    array->elementType = ExCharType;
                    array->header.type = ExCharArrayType;
                    array->count = returnStringLength;
                    for (int i = 0; i < array->count; ++i)
                    {
                        array->characters[i] = returnString[i];
                    }
                    ((struct MANAGED_STRING*)blockPointer->descriptor)->characters = array;

                    slot.descriptor = blockPointer->descriptor;
                    ExPop(frame);
                    MmFreePoolMemory(returnPointer);
                }
                else if (frame->method->farCall.encoding == STRING_ENCODING_NONE)
                {
                    ExPop(frame);
                    slot.descriptor = returnPointer;
                }
                else
                {
                    return MARX_STATUS_FAIL;
                }
            }
            else
            {
                slot.descriptor = returnPointer;
            }

            break;
        }
        default:
        {
            return MARX_STATUS_FAIL;
        }
    }

    *returnValue = slot;

    return MARX_STATUS_SUCCESS;
}
