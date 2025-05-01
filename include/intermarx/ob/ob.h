#pragma once
#include <intermarx/intermarx.h>
#include <intermarx/rtl/rtl.h>
#include <intermarx/far/far.h>
#include <intermarx/hp/hp.h>

struct RUNTIME_FIELD;

struct OBJECT_HEADER
{
    UINTPTR size;
    struct RUNTIME_TYPE* type;

    UINTPTR interop : 1;
    UINTPTR managedWrapper : 1;
    UINTPTR array : 1;
    UINTPTR reserved : 29;

    enum GC_COLOR color;
    VOID* forward;
};

struct INUMANAGED INUPACKED MANAGED_ARRAY
{
    struct OBJECT_HEADER header;

    struct RUNTIME_TYPE* elementType;
    UINTPTR count;

    union
    {
        VOID *pointer[0];
        INTPTR nint[0];
        INT32 int32[0];
        INT64 int64[0];
        INT16 int16[0];
        SINGLE single[0];
        DOUBLE duoble[0];
        BYTE byte[0];
        WCHAR characters[0];
    };
};

struct INUMANAGED INUPACKED MANAGED_WRAPPER
{
    struct OBJECT_HEADER header;

    NATIVE_HANDLE handle;
    VOID* fixUpAddress;
};

struct INUMANAGED INUPACKED MANAGED_STRING
{
    struct OBJECT_HEADER header;

    struct MANAGED_ARRAY* characters;
};

struct INUMANAGED INUPACKED MANAGED_EXCEPTION
{
    struct OBJECT_HEADER header;

    struct MANAGED_STRING* message;

    WCHAR* nativeMessage;
};

struct INUMANAGED INUPACKED MANAGED_DELEGATE
{
    struct OBJECT_HEADER header;

    struct MANAGED_ARRAY* thisObjects;
    struct MANAGED_ARRAY* callSites;
    INT32 reserved;
};


struct INUMANAGED INUPACKED MANAGED_ATTRIBUTE
{
    struct OBJECT_HEADER header;

    struct RUNTIME_METHOD *ctor;

    union
    {
        enum EXECUTIVE_OWNER_DESCRIPTOR *owner;
        struct RUNTIME_TYPE* typeOwner;
        struct RUNTIME_METHOD* methodOwner;
        struct RUNTIME_FIELD* fieldOwner;
    };

    struct VECTOR parametersList;
    struct VECTOR params;
};

struct INUMANAGED INUPACKED FAR_CALL_MANAGED_ATTRIBUTE
{
    struct MANAGED_ATTRIBUTE base;

    struct MANAGED_STRING* functionName;
    enum FAR_CALL_CALL_CONVENTION convention;
    enum FAR_CALL_STRING_ENCODING encoding;
    enum FAR_CALL_SYMBOL_SOURCE source;
};

VOID* ObManagedArrayInitialize(UINTPTR elementCount, UINTPTR elementSize);
VOID ObManagedAttributeInitialize(struct MANAGED_ATTRIBUTE* thisPtr);
VOID ObManagedExceptionInitialize(struct MANAGED_EXCEPTION* thisPtr, struct RUNTIME_TYPE* type, const WCHAR* message);
VOID ObManagedWrapperInitialize(struct MANAGED_WRAPPER* thisPtr, VOID* nativeHandle);
VOID ObManagedDelegateInitialize(struct MANAGED_DELEGATE* thisPtr, VOID** object, VOID** callSite, INTPTR entryCount, struct MANAGED_DELEGATE* base);