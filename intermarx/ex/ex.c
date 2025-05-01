#include <intermarx/ex/ex.h>
#include <intermarx/pal/pal.h>
#include <intermarx/rtl/rtl.h>

INUGLOBAL UINTPTR ExGlobalDomainId;

INUIMPORT struct RUNTIME_TYPE *ExStringType;
INUIMPORT struct RUNTIME_TYPE *ExCharArrayType;
INUIMPORT struct RUNTIME_TYPE *ExCharType;
INUIMPORT struct RUNTIME_TYPE *ExThreadType;


BOOLEAN ExMetadataIs(enum METADATA_CHARACTERISTICS thisValue, enum METADATA_CHARACTERISTICS metadata)
{
    if ((thisValue & metadata) != 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID ExDomainInitialize(struct RUNTIME_DOMAIN *thisPtr)
{
    thisPtr->id = ExGlobalDomainId++;
    RtlVectorInitialize(&thisPtr->threads);
    RtlVectorInitialize(&thisPtr->types);
}

VOID ExTypeInitialize(struct RUNTIME_TYPE *thisPtr)
{
    thisPtr->descriptor = DESCRIPTOR_TYPE;
    thisPtr->super = NULL;
    thisPtr->inlined = BASE_OTHER;

    RtlVectorInitialize(&thisPtr->attributes);
    RtlVectorInitialize(&thisPtr->fields);
    RtlVectorInitialize(&thisPtr->interfaces);
    RtlVectorInitialize(&thisPtr->methods);
}

VOID ExFieldInitialize(struct RUNTIME_FIELD *thisPtr)
{
    thisPtr->descriptor = DESCRIPTOR_FIELD;

    RtlVectorInitialize(&thisPtr->attributes);
}

VOID ExMethodInitialize(struct RUNTIME_METHOD *thisPtr)
{
    thisPtr->descriptor = DESCRIPTOR_METHOD;

    PalMemoryZero(&thisPtr->farCall,sizeof(struct FAR_CALL));

    RtlVectorInitialize(&thisPtr->attributes);
    RtlVectorInitialize(&thisPtr->handlers);
    RtlVectorInitialize(&thisPtr->parameters);
    RtlVectorInitialize(&thisPtr->pool);
    RtlVectorInitialize(&thisPtr->variables);
    RtlVectorInitialize(&thisPtr->stringTable);
}

VOID ExThreadInitialize(struct RUNTIME_THREAD *thisPtr, struct RUNTIME_DOMAIN* domain)
{
    PalMemoryZero(thisPtr,sizeof(struct RUNTIME_THREAD));

    thisPtr->handle = PalThreadGetCurrentHandle();
    thisPtr->id = PalThreadGetCurrentId();
    thisPtr->domain = domain;
    thisPtr->wrapper = NULL;
    ExExceptionStateInitialize(&thisPtr->state);
}

VOID ExExceptionStateInitialize(struct EXCEPTION_STATE *thisPtr)
{
    thisPtr->exception = NULL;
    thisPtr->isUnwinding = FALSE;
    RtlVectorInitialize(&thisPtr->stackTrace);
}

void ExExceptionStateAppend(struct EXCEPTION_STATE *thisPtr, struct RUNTIME_METHOD *method)
{
    RtlVectorAdd(&thisPtr->stackTrace,&method->fullName);
}

void ExExceptionStateDrop(struct EXCEPTION_STATE *thisPtr)
{
    thisPtr->stackTrace.count = 0;
}

struct RUNTIME_METHOD * ExTypeLocateMethod(struct RUNTIME_TYPE *thisPtr, const CHAR *shortName)
{
    for (int i = 0; i < thisPtr->methods.count; ++i)
    {
        struct RUNTIME_METHOD* method = RtlVectorGet(&thisPtr->methods,i);

        if (RtlNStringCompare(method->shortName,shortName))
        {
            return method;
        }
    }

    return NULL;
}

struct RUNTIME_TYPE * ExDomainLocateType(struct RUNTIME_DOMAIN *thisPtr, const CHAR *string)
{
    for (int i = 0; i < thisPtr->types.count; ++i)
    {
        struct RUNTIME_TYPE* type = RtlVectorGet(&thisPtr->types,i);
        if (RtlNStringCompare(type->fullName,string))
        {
            return type;
        }
    }

    PalDebugbreak("Fail to find type!");
    return NULL;
}

