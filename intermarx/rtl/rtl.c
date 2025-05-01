#include <intermarx/hp/hp.h>
#include <intermarx/pal/pal.h>
#include <intermarx/rtl/rtl.h>
#include <rtl/rtl.h>

#define RTL_VECTOR_BASE_SIZE 10

VOID RtlNStringInitialize(struct NSTRING* thisPtr, WCHAR* characters, UINTPTR length)
{
    thisPtr->length = length;
    thisPtr->characters = PalMemoryAllocate((length+1)*sizeof(WCHAR));
    thisPtr->characters[length] = 0;

    PalMemoryCopy(thisPtr->characters,characters,length*sizeof(WCHAR));
}

BOOLEAN RtlNStringCompare(struct NSTRING thisPtr, const CHAR *characters)
{
    UINTPTR length = PalStringLength(characters);

    if (length != thisPtr.length)
    {
        return FALSE;
    }

    for (int i = 0; i < thisPtr.length; ++i)
    {
        if (thisPtr.characters[i] != characters[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN RtlNStringCompareObject(struct NSTRING first, struct NSTRING second)
{
    if (first.length != second.length)
    {
        return FALSE;
    }
    else
    {
        for (int i = 0; i < first.length; ++i)
        {
            if (first.characters[i] != second.characters[i])
            {
                return FALSE;
            }
        }

        return TRUE;
    }
}

BOOLEAN RtlNStringContainsNative(struct NSTRING first, const CHAR *second)
{
    const UINTPTR sourceLength = strlen(second);

    for (UINTPTR i = 0; i < first.length; ++i)
    {
        for (UINTPTR j = 0; j < sourceLength; ++j)
        {
            if (first.characters[i + j] != second[j])
            {
                break;
            }
            else
            {
                if (j + 1 >= sourceLength)
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

VOID RtlVectorInitialize(struct VECTOR *thisPtr)
{
    thisPtr->allocated = RTL_VECTOR_BASE_SIZE;
    thisPtr->count = 0;
    thisPtr->pointer = PalMemoryAllocate(RTL_VECTOR_BASE_SIZE*sizeof(VOID*));
}

VOID RtlVectorAdd(struct VECTOR *thisPtr, void *target)
{
    Start:

    if (thisPtr->count + 1 >= thisPtr->allocated)
    {
        UINTPTR newSize = thisPtr->allocated*1.5;
        UINTPTR byteSize = newSize*sizeof(VOID*);

        VOID* oldPointer = thisPtr->pointer;
        VOID* newPointer = HpAllocateNative(byteSize);

        PalMemoryCopy(newPointer,oldPointer,thisPtr->count*sizeof(VOID*));

        thisPtr->pointer = newPointer;
        thisPtr->allocated = newSize;
        goto Start;
    }

    thisPtr->pointer[thisPtr->count++] = target;
}

void * RtlVectorGet(struct VECTOR *thisPtr, UINTPTR index)
{
    if (thisPtr->count <= index)
    {
        PalDebugbreak("Fail!");
        return NULL;
    }
    else
    {
        return thisPtr->pointer[index];
    }
}

void RtlReaderInitialize(struct READER *thisPtr, void *stream)
{
    thisPtr->stream = stream;
    thisPtr->offset = 0;
}

BYTE RtlReaderReadByte(struct READER *thisPtr)
{
    BYTE* target = (BYTE*)&thisPtr->stream[thisPtr->offset];
    thisPtr->offset += sizeof(BYTE);
    return *target;
}

INT32 RtlReaderReadInt32(struct READER *thisPtr)
{
    INT32* target = (INT32*)&thisPtr->stream[thisPtr->offset];
    thisPtr->offset += sizeof(INT32);
    return *target;
}

INT64 RtlReaderReadInt64(struct READER *thisPtr)
{
    INT64 target;
    RtlReaderRead(thisPtr,&target, sizeof target);
    return target;
}

SINGLE RtlReaderReadSingle(struct READER *thisPtr)
{
    SINGLE target;
    RtlReaderRead(thisPtr,&target, sizeof target);
    return target;
}

DOUBLE RtlReaderReadDouble(struct READER *thisPtr)
{
    DOUBLE target;
    RtlReaderRead(thisPtr,&target, sizeof target);
    return target;
}

void RtlReaderSet(struct READER *thisPtr, UINTPTR offset)
{
    thisPtr->offset = offset;
}

void RtlReaderRead(struct READER *thisPtr, void *target, UINTPTR length)
{
    PalMemoryCopy(target,&thisPtr->stream[thisPtr->offset],length);
    thisPtr->offset += length;
}
