#pragma once
#include <intermarx/intermarx.h>

struct VECTOR
{
    UINTPTR count;
    UINTPTR allocated;
    VOID** pointer;
};

struct NSTRING
{
    UINTPTR length;
    WCHAR* characters;
};

struct READER
{
    VOID* stream;
    UINTPTR offset;
};

VOID RtlNStringInitialize(struct NSTRING* thisPtr, WCHAR* characters, UINTPTR length);
BOOLEAN RtlNStringCompare(struct NSTRING thisPtr, const CHAR* characters);
BOOLEAN RtlNStringCompareObject(struct NSTRING first, struct NSTRING second);
BOOLEAN RtlNStringContainsNative(struct NSTRING first, const CHAR* second);

VOID RtlVectorInitialize(struct VECTOR* thisPtr);
VOID RtlVectorAdd(struct VECTOR* thisPtr, VOID* target);
VOID* RtlVectorGet(struct VECTOR* thisPtr, UINTPTR index);

VOID RtlReaderInitialize(struct READER* thisPtr, VOID* stream);
BYTE RtlReaderReadByte(struct READER* thisPtr);
INT32 RtlReaderReadInt32(struct READER* thisPtr);
INT64 RtlReaderReadInt64(struct READER* thisPtr);
SINGLE RtlReaderReadSingle(struct READER* thisPtr);
DOUBLE RtlReaderReadDouble(struct READER* thisPtr);
VOID RtlReaderSet(struct READER* thisPtr, UINTPTR offset);
VOID RtlReaderRead(struct READER* thisPtr, VOID* target, UINTPTR length);