#include <intermarx/ex/ex.h>
#include <intermarx/ex/runtime.h>
#include <intermarx/hp/hp.h>
#include <intermarx/ldr/ldr.h>
#include <intermarx/ob/ob.h>
#include <intermarx/pal/pal.h>
#include <intermarx/rtl/rtl.h>

#define LDR_MAGIC 0x50C1A715

INUIMPORT struct RUNTIME_TYPE *ExStringType;
INUIMPORT struct RUNTIME_TYPE *ExCharArrayType;
INUIMPORT struct RUNTIME_TYPE *ExCharType;
INUIMPORT struct RUNTIME_TYPE *ExThreadType;


VOID *LdrLoadStringTable(struct IMAGE_LOADER *thisPtr)
{
    UINT32 stringTableLenght;
    LdrImageRead(thisPtr, &stringTableLenght, sizeof stringTableLenght);

    struct NSTRING *stringTable = HpAllocateNative(sizeof(struct NSTRING) * stringTableLenght);

    for (int i = 0; i < stringTableLenght; ++i)
    {
        UINT32 stringLength;
        LdrImageRead(thisPtr, &stringLength, sizeof stringLength);

        CHAR buffer[stringLength];
        LdrImageRead(thisPtr, buffer, stringLength);

        WCHAR newBuffer[stringLength + 1];
        PalMemoryZero(newBuffer, (stringLength + 1) * sizeof(WCHAR));

        for (int z = 0; z < stringLength; ++z)
        {
            newBuffer[z] = buffer[z];
        }

        RtlNStringInitialize(&stringTable[i], newBuffer, stringLength);
    }

    thisPtr->stringTable = stringTable;
    return stringTable;
}

VOID *LdrLoadType(struct IMAGE_LOADER *thisPtr)
{
    struct RUNTIME_TYPE *type = HpAllocateNative(sizeof(struct RUNTIME_TYPE));
    ExTypeInitialize(type);

    type->fullName = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];
    type->shortName = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];

    if (RtlNStringCompare(type->fullName, ExByteTypeName))
    {
        type->inlined = BASE_BYTE;
    }
    else if (RtlNStringCompare(type->fullName, ExBooleanTypeName))
    {
        type->inlined = BASE_BYTE;
    }
    else if (RtlNStringCompare(type->fullName, ExCharTypeName))
    {
        type->inlined = BASE_CHAR;
    }
    else if (RtlNStringCompare(type->fullName, ExSByteTypeName))
    {
        type->inlined = BASE_BYTE;
    }
    else if (RtlNStringCompare(type->fullName, ExUInt16TypeName))
    {
        type->inlined = BASE_INT16;
    }
    else if (RtlNStringCompare(type->fullName, ExInt16TypeName))
    {
        type->inlined = BASE_INT16;
    }
    else if (RtlNStringCompare(type->fullName, ExUInt32TypeName))
    {
        type->inlined = BASE_INT32;
    }
    else if (RtlNStringCompare(type->fullName, ExInt32TypeName))
    {
        type->inlined = BASE_INT32;
    }
    else if (RtlNStringCompare(type->fullName, ExUInt64TypeName))
    {
        type->inlined = BASE_INT64;
    }
    else if (RtlNStringCompare(type->fullName, ExInt64TypeName))
    {
        type->inlined = BASE_INT64;
    }
    else if (RtlNStringCompare(type->fullName, ExSingleTypeName))
    {
        type->inlined = BASE_SINGLE;
    }
    else if (RtlNStringCompare(type->fullName, ExDoubleTypeName))
    {
        type->inlined = BASE_DOUBLE;
    }
    else if (RtlNStringCompare(type->fullName, ExUIntPtrTypeName))
    {
        type->inlined = BASE_INTPTR;
    }
    else if (RtlNStringCompare(type->fullName, ExIntPtrTypeName))
    {
        type->inlined = BASE_INTPTR;
    }
    else if (RtlNStringCompare(type->fullName, ExVoidTypeName))
    {
        type->inlined = BASE_VOID;
    }
    else
    {
        type->inlined = BASE_OTHER;
    }

    INT32 super = LdrImageReadIndex(thisPtr);
    if (super != -1)
    {
        type->super = thisPtr->sections[super];
    }

    INT32 interfaceCount = LdrImageReadIndex(thisPtr);
    for (int i = 0; i < interfaceCount; ++i)
    {
        RtlVectorAdd(&type->interfaces, thisPtr->sections[LdrImageReadIndex(thisPtr)]);
    }

    type->metadata = LdrImageReadIndex(thisPtr);

    return type;
}

VOID *LdrLoadField(struct IMAGE_LOADER *thisPtr)
{
    struct RUNTIME_FIELD *field = HpAllocateNative(sizeof(struct RUNTIME_FIELD));
    ExFieldInitialize(field);

    field->fullName = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];
    field->shortName = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];
    field->metadata = LdrImageReadIndex(thisPtr);
    field->owner = thisPtr->sections[LdrImageReadIndex(thisPtr)];
    field->declared = thisPtr->sections[LdrImageReadIndex(thisPtr)];

    RtlVectorAdd(&field->owner->fields, field);

    if (ExMetadataIs(field->metadata, MxExMetadataStatic))
    {
        HpRegisterRootField(field);
    }

    return field;
}

VOID *LdrLoadMethod(struct IMAGE_LOADER *thisPtr)
{
    struct RUNTIME_METHOD *method = HpAllocateNative(sizeof(struct RUNTIME_METHOD));
    ExMethodInitialize(method);

    method->fullName = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];
    method->shortName = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];
    method->metadata = LdrImageReadIndex(thisPtr);
    method->owner = thisPtr->sections[LdrImageReadIndex(thisPtr)];
    method->returnType = thisPtr->sections[LdrImageReadIndex(thisPtr)];

    if (RtlNStringCompare(method->returnType->fullName, ExVoidTypeName))
    {
        method->isReturns = FALSE;
    }
    else
    {
        method->isReturns = TRUE;
    }

    const INT32 parametersCount = LdrImageReadIndex(thisPtr);
    for (int i = 0; i < parametersCount; ++i)
    {
        RtlVectorAdd(&method->parameters, thisPtr->sections[LdrImageReadIndex(thisPtr)]);
    }
    RtlVectorAdd(&method->owner->methods, method);

    return method;
}

VOID *LdrLoadExecutable(struct IMAGE_LOADER *thisPtr)
{
    struct RUNTIME_METHOD *owner = thisPtr->sections[LdrImageReadIndex(thisPtr)];

    INT32 poolTypeSize = LdrImageReadIndex(thisPtr);
    for (int i = 0; i < poolTypeSize; ++i)
    {
        LdrImageLoaderWind(thisPtr,sizeof(INT32));
    }

    INT32 poolSize = LdrImageReadIndex(thisPtr);
    for (int i = 0; i < poolSize; ++i)
    {
        RtlVectorAdd(&owner->pool, thisPtr->sections[LdrImageReadIndex(thisPtr)]);
    }

    INT32 variableCount = LdrImageReadIndex(thisPtr);
    for (int i = 0; i < variableCount; ++i)
    {
        RtlVectorAdd(&owner->variables, thisPtr->sections[LdrImageReadIndex(thisPtr)]);
    }

    INT32 stringCount = LdrImageReadIndex(thisPtr);
    for (int i = 0; i < stringCount; ++i)
    {
        struct NSTRING *str = HpAllocateNative(sizeof(struct NSTRING));
        *str = thisPtr->stringTable[LdrImageReadIndex(thisPtr)];
        RtlVectorAdd(&owner->stringTable, str);
    }

    INT32 imageLength = LdrImageReadIndex(thisPtr);
    owner->bytecode = HpAllocateNative(imageLength);
    LdrImageRead(thisPtr, owner->bytecode, imageLength);

    return owner;
}

VOID *LdrLoadHandler(struct IMAGE_LOADER *thisPtr)
{
    struct RUNTIME_EXCEPTION_HANDLER *handler = HpAllocateNative(sizeof(struct RUNTIME_EXCEPTION_HANDLER));

    handler->handler = LdrImageReadIndex(thisPtr);

    if (handler->handler == HANDLER_FINALLY)
    {
        LdrImageReadIndex(thisPtr);
        handler->catch = NULL;
    }
    else
    {
        handler->catch = thisPtr->sections[LdrImageReadIndex(thisPtr)];
    }

    handler->owner = thisPtr->sections[LdrImageReadIndex(thisPtr)];
    handler->handlerStart = LdrImageReadIndex(thisPtr);
    handler->handlerEnd = LdrImageReadIndex(thisPtr);
    handler->tryStart = LdrImageReadIndex(thisPtr);
    handler->tryEnd = LdrImageReadIndex(thisPtr);

    RtlVectorAdd(&handler->owner->handlers, handler);
    return handler;
}

VOID *LdrLoadAttribute(struct IMAGE_LOADER *thisPtr)
{
    enum EXECUTIVE_OWNER_DESCRIPTOR *descriptor = thisPtr->sections[LdrImageReadIndex(thisPtr)];
    struct RUNTIME_TYPE *declared = thisPtr->sections[LdrImageReadIndex(thisPtr)];

    struct MANAGED_ATTRIBUTE *attribute = HpAllocateManaged(sizeof(struct MANAGED_ATTRIBUTE)+150); // TODO: BAD HACK! SCAN RUNTIME_TYPE SIZES BEFORE LOADING ATTRIBUTES!
    ObManagedAttributeInitialize(attribute);

    attribute->owner = descriptor;
    attribute->header.type = declared;
    attribute->header.interop = TRUE;
    attribute->ctor = thisPtr->sections[LdrImageReadIndex(thisPtr)];

    switch (*descriptor)
    {
        case DESCRIPTOR_TYPE:
        {
            struct RUNTIME_TYPE *value = (struct RUNTIME_TYPE *) attribute->owner;
            RtlVectorAdd(&value->attributes, attribute);
            break;
        }
        case DESCRIPTOR_FIELD:
        {
            struct RUNTIME_FIELD *value = (struct RUNTIME_FIELD *) attribute->owner;
            RtlVectorAdd(&value->attributes, attribute);
            break;
        }
        case DESCRIPTOR_METHOD:
        {
            struct RUNTIME_METHOD *value = (struct RUNTIME_METHOD *) attribute->owner;
            RtlVectorAdd(&value->attributes, attribute);
            break;
        }
        default:
        {
            PalDebugbreak("Fail!");
            break;
        }
    }

    return attribute;
}

VOID *LdrLoadArgument(struct IMAGE_LOADER *thisPtr)
{
    struct MANAGED_ATTRIBUTE *owner = thisPtr->sections[LdrImageReadIndex(thisPtr)];
    struct RUNTIME_TYPE *declared = thisPtr->sections[LdrImageReadIndex(thisPtr)];

    RtlVectorAdd(&owner->params, declared);

    if (RtlNStringCompare(declared->fullName, ExInt32TypeName))
    {
        const INT32 arrayCount = LdrImageReadIndex(thisPtr);
        const INT32 value = LdrImageReadIndex(thisPtr);

        struct MANAGED_ARRAY *newArray = ObManagedArrayInitialize(arrayCount,sizeof(INT32));
        newArray->header.type = ExDomainLocateType(thisPtr->domain, "System.Int32[]");
        newArray->elementType = declared;
        newArray->int32[arrayCount - 1] = value;

        RtlVectorAdd(&owner->parametersList, newArray);
    }
    else if (RtlNStringCompare(declared->fullName, "System.Int32[]"))
    {
        const INT32 arrayCount = LdrImageReadIndex(thisPtr);

        struct MANAGED_ARRAY *newArray = ObManagedArrayInitialize(arrayCount,sizeof(INT32));
        newArray->header.type = ExDomainLocateType(thisPtr->domain, "System.Int32[]");
        newArray->elementType = declared;

        for (int i = 0; i < arrayCount; ++i)
        {
            newArray->int32[i] = LdrImageReadIndex(thisPtr);
        }

        RtlVectorAdd(&owner->parametersList, newArray);
    }
    else if (RtlNStringCompare(declared->fullName, ExStringTypeName))
    {
        const INT32 arrayCount = LdrImageReadIndex(thisPtr);
        for (int i = 0; i < arrayCount; ++i)
        {
            const INT32 value = LdrImageReadIndex(thisPtr);
            RtlVectorAdd(&owner->parametersList, &thisPtr->stringTable[value]);
        }
    }
    else if (ExMetadataIs(declared->metadata, MxExMetadataEnum))
    {
        struct RUNTIME_FIELD *firstField = RtlVectorGet(&declared->fields, 0);

        if (RtlNStringCompare(firstField->declared->fullName, ExInt32TypeName))
        {
            const INT32 arrayCount = LdrImageReadIndex(thisPtr);
            const INT32 value = LdrImageReadIndex(thisPtr);

            struct MANAGED_ARRAY *newArray = ObManagedArrayInitialize(arrayCount,sizeof(INT32));
            newArray->header.type = ExDomainLocateType(thisPtr->domain, "System.Int32[]");
            newArray->elementType = declared;
            newArray->int32[arrayCount - 1] = value;

            RtlVectorAdd(&owner->parametersList, newArray);
        }
        else
        {
            PalDebugbreak("Bad enum underliyng type!");
        }
    }
    else
    {
        PalDebugbreak("Bad attribute type!");
    }

    return owner;
}

VOID LdrExecuteStaticConstructors(struct RUNTIME_DOMAIN *thisPtr)
{
    for (INTPTR i = 0; i < thisPtr->types.count; ++i)
    {
        struct RUNTIME_TYPE *type = RtlVectorGet(&thisPtr->types, i);

        for (INTPTR j = 0; j < type->methods.count; ++j)
        {
            struct RUNTIME_METHOD *method = RtlVectorGet(&type->methods, j);

            if (RtlNStringContainsNative(method->fullName, ".cctor"))
            {
                struct RUNTIME_FRAME_BLOCK ret;
                ExMethodPrologueArgs(method,NULL,NULL, &ret);
            }
        }
    }
}

VOID LdrExecuteAttributesConstructors(struct RUNTIME_DOMAIN *thisPtr)
{
    for (INTPTR i = 0; i < thisPtr->types.count; ++i)
    {
        struct RUNTIME_TYPE *type = RtlVectorGet(&thisPtr->types, i);

        for (INTPTR j = 0; j < type->attributes.count; ++j)
        {
            struct MANAGED_ATTRIBUTE *attribute = RtlVectorGet(&type->attributes, j);

            struct RUNTIME_FRAME_BLOCK parameters[attribute->ctor->parameters.count];

            parameters[0].type = MACHINE_OBJECT;
            parameters[0].descriptor = attribute;

            for (int z = attribute->params.count; z >= 1; --z)
            {
                struct RUNTIME_TYPE *paramType = RtlVectorGet(&attribute->params, z - 1);

                if (paramType->inlined == BASE_INT32)
                {
                    struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                    parameters[z].type = MACHINE_INT32;
                    parameters[z].int32 = arg->int32[0];
                }
                else if (RtlNStringCompare(paramType->fullName, "System.Int32[]"))
                {
                    struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                    parameters[z].type = MACHINE_OBJECT;
                    parameters[z].descriptor = arg;
                }
                else if (RtlNStringCompare(paramType->fullName, "System.String"))
                {
                    struct NSTRING *arg = RtlVectorGet(&attribute->parametersList, z - 1);

                    struct MANAGED_STRING* string = HpAllocateManaged(sizeof(struct MANAGED_STRING));
                    string->header.type = ExStringType;

                    struct MANAGED_ARRAY* array = ObManagedArrayInitialize(arg->length,sizeof(WCHAR));
                    array->elementType = ExCharType;
                    array->header.type = ExCharArrayType;
                    array->count = arg->length;

                    PalMemoryCopy(array->characters,arg->characters,sizeof(WCHAR)*array->count);

                    string->characters = array;

                    parameters[z].type = MACHINE_OBJECT;
                    parameters[z].descriptor = string;
                }
                else if (ExMetadataIs(paramType->metadata, MxExMetadataEnum))
                {
                    struct RUNTIME_FIELD *field = RtlVectorGet(&paramType->fields, 0);
                    if (field->declared->inlined == BASE_INT32)
                    {
                        const struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                        parameters[z].type = MACHINE_INT32;
                        parameters[z].int32 = arg->int32[0];
                    }
                    else
                    {
                        PalDebugbreak("Fail to load!");
                    }
                }
                else
                {
                    PalDebugbreak("Bad attriubte type!");
                }
            }

            struct RUNTIME_FRAME_BLOCK ret;
            ExMethodPrologueArgs(attribute->ctor, parameters,NULL, &ret);
        }

        for (UINTPTR l = 0; l < type->methods.count; ++l)
        {
            struct RUNTIME_METHOD *method = RtlVectorGet(&type->methods, l);

            for (UINTPTR j = 0; j < method->attributes.count; ++j)
            {
                struct MANAGED_ATTRIBUTE *attribute = RtlVectorGet(&method->attributes, j);

                struct RUNTIME_FRAME_BLOCK parameters[attribute->ctor->parameters.count];

                parameters[0].type = MACHINE_OBJECT;
                parameters[0].descriptor = attribute;

                for (int z = attribute->params.count; z >= 1; --z)
                {
                    struct RUNTIME_TYPE *paramType = RtlVectorGet(&attribute->params, z - 1);

                    if (paramType->inlined == BASE_INT32)
                    {
                        struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                        parameters[z].type = MACHINE_INT32;
                        parameters[z].int32 = arg->int32[0];
                    }
                    else if (RtlNStringCompare(paramType->fullName, "System.Int32[]"))
                    {
                        struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                        parameters[z].type = MACHINE_OBJECT;
                        parameters[z].descriptor = arg;
                    }
                    else if (RtlNStringCompare(paramType->fullName, "System.String"))
                    {
                        struct NSTRING *arg = RtlVectorGet(&attribute->parametersList, z - 1);

                        struct MANAGED_STRING* string = HpAllocateManaged(sizeof(struct MANAGED_STRING));
                        string->header.type = ExStringType;

                        struct MANAGED_ARRAY* array = ObManagedArrayInitialize(arg->length,sizeof(WCHAR));
                        array->elementType = ExCharType;
                        array->header.type = ExCharArrayType;
                        array->count = arg->length;

                        PalMemoryCopy(array->characters,arg->characters,sizeof(WCHAR)*array->count);

                        string->characters = array;

                        parameters[z].type = MACHINE_OBJECT;
                        parameters[z].descriptor = string;
                    }
                    else if (ExMetadataIs(paramType->metadata, MxExMetadataEnum))
                    {
                        struct RUNTIME_FIELD *enumField = RtlVectorGet(&paramType->fields, 0);
                        if (enumField->declared->inlined == BASE_INT32)
                        {
                            const struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                            parameters[z].type = MACHINE_INT32;
                            parameters[z].int32 = arg->int32[0];
                        }
                        else
                        {
                            PalDebugbreak("Fail to load!");
                        }
                    }
                    else
                    {
                        PalDebugbreak("Bad attriubte type!");
                    }
                }

                struct RUNTIME_FRAME_BLOCK ret;
                ExMethodPrologueArgs(attribute->ctor, parameters,NULL, &ret);
            }
        }

        for (UINTPTR l = 0; l < type->fields.count; ++l)
        {
            struct RUNTIME_FIELD *field = RtlVectorGet(&type->fields, l);

            for (UINTPTR j = 0; j < field->attributes.count; ++j)
            {
                struct MANAGED_ATTRIBUTE *attribute = RtlVectorGet(&field->attributes, j);

                struct RUNTIME_FRAME_BLOCK parameters[attribute->ctor->parameters.count];

                parameters[0].type = MACHINE_OBJECT;
                parameters[0].descriptor = attribute;

                for (int z = attribute->params.count; z >= 1; --z)
                {
                    struct RUNTIME_TYPE *paramType = RtlVectorGet(&attribute->params, z - 1);

                    if (paramType->inlined == BASE_INT32)
                    {
                        struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                        parameters[z].type = MACHINE_INT32;
                        parameters[z].int32 = arg->int32[0];
                    }
                    else if (RtlNStringCompare(paramType->fullName, "System.Int32[]"))
                    {
                        struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                        parameters[z].type = MACHINE_OBJECT;
                        parameters[z].descriptor = arg;
                    }
                    else if (RtlNStringCompare(paramType->fullName, "System.String"))
                    {
                        struct NSTRING *arg = RtlVectorGet(&attribute->parametersList, z - 1);

                        struct MANAGED_STRING* string = HpAllocateManaged(sizeof(struct MANAGED_STRING));
                        string->header.type = ExStringType;

                        struct MANAGED_ARRAY* array = ObManagedArrayInitialize(arg->length,sizeof(WCHAR));
                        array->elementType = ExCharType;
                        array->header.type = ExCharArrayType;
                        array->count = arg->length;

                        PalMemoryCopy(array->characters,arg->characters,sizeof(WCHAR)*array->count);

                        string->characters = array;

                        parameters[z].type = MACHINE_OBJECT;
                        parameters[z].descriptor = string;
                    }
                    else if (ExMetadataIs(paramType->metadata, MxExMetadataEnum))
                    {
                        struct RUNTIME_FIELD *enumField = RtlVectorGet(&paramType->fields, 0);
                        if (enumField->declared->inlined == BASE_INT32)
                        {
                            const struct MANAGED_ARRAY *arg = RtlVectorGet(&attribute->parametersList, z - 1);
                            parameters[z].type = MACHINE_INT32;
                            parameters[z].int32 = arg->int32[0];
                        }
                        else
                        {
                            PalDebugbreak("Fail to load!");
                        }
                    }
                    else
                    {
                        PalDebugbreak("Bad attriubte type!");
                    }
                }

                struct RUNTIME_FRAME_BLOCK ret;
                ExMethodPrologueArgs(attribute->ctor, parameters,NULL, &ret);
            }
        }
    }
}

VOID LdrImageLoaderInitialize(struct IMAGE_LOADER *thisPtr, VOID *imageAddress)
{
    thisPtr->image = imageAddress;
    thisPtr->offset = 0;
}

VOID LdrImageLoaderWind(struct IMAGE_LOADER *thisPtr, UINTPTR length)
{
    thisPtr->offset += length;
}

VOID LdrImageRead(struct IMAGE_LOADER *thisPtr, VOID *dest, UINTPTR length)
{
    PalMemoryCopy(dest, &thisPtr->image[thisPtr->offset], length);
    thisPtr->offset += length;
}

UINT32 LdrImageReadIndex(struct IMAGE_LOADER *thisPtr)
{
    UINT32 index;
    LdrImageRead(thisPtr, &index, sizeof index);
    return index;
}

VOID LdrCalculateTypeSizes(struct RUNTIME_DOMAIN *thisPtr)
{
    for (int i = 0; i < thisPtr->types.count; ++i)
    {
        struct RUNTIME_TYPE *type = RtlVectorGet(&thisPtr->types, i);

        type->size = LdrCalculateTypeSize(type);
    }
}

VOID LdrFillTypeInfo(struct RUNTIME_DOMAIN *thisPtr)
{
    for (int i = 0; i < thisPtr->types.count; ++i)
    {
        struct RUNTIME_TYPE *type = RtlVectorGet(&thisPtr->types, i);

        for (int j = 0; j < type->methods.count; ++j)
        {
            struct RUNTIME_METHOD* method = RtlVectorGet(&type->methods,j);

            if (RtlNStringCompare(method->shortName,"Finalize"))
            {
                type->hasFinalizer = TRUE;
            }
        }
    }
}

UINTPTR LdrCalculateFieldSize(struct RUNTIME_FIELD* thisPtr)
{
    if (RtlNStringCompare(thisPtr->declared->fullName, ExByteTypeName))
    {
        return sizeof(BYTE);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExBooleanTypeName))
    {
        return sizeof(BOOLEAN);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExCharTypeName))
    {
        return sizeof(WCHAR);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExSByteTypeName))
    {
        return sizeof(SBYTE);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExUInt16TypeName))
    {
        return sizeof(UINT16);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExInt16TypeName))
    {
        return sizeof(INT16);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExUInt32TypeName))
    {
        return sizeof(UINT32);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExInt32TypeName))
    {
        return sizeof(INT32);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExUInt64TypeName))
    {
        return sizeof(UINT64);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExInt64TypeName))
    {
        return sizeof(INT64);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExSingleTypeName))
    {
        return sizeof(SINGLE);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExDoubleTypeName))
    {
        return sizeof(DOUBLE);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExUIntPtrTypeName))
    {
        return sizeof(UINTPTR);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExIntPtrTypeName))
    {
        return sizeof(INTPTR);
    }
    else if (RtlNStringCompare(thisPtr->declared->fullName, ExVoidTypeName))
    {
        return 0;
    }
    else
    {
        if (ExMetadataIs(thisPtr->metadata, MxExMetadataStruct))
        {
            return LdrCalculateTypeSize(thisPtr->declared);
        }
        else
        {
            return sizeof(VOID*);
        }
    }
}

UINTPTR LdrCalculateTypeSize(struct RUNTIME_TYPE *thisPtr)
{
    UINTPTR structSize = 0;

    if (thisPtr->super != NULL)
    {
        structSize = LdrCalculateTypeSize(thisPtr->super);
    }

    for (UINTPTR i = 0; i < thisPtr->fields.count; i++)
    {
        struct RUNTIME_FIELD* field = RtlVectorGet(&thisPtr->fields, i);

        if (ExMetadataIs(field->metadata, MxExMetadataStatic))
        {
            field->offset = 0;
            field->dataSize = LdrCalculateFieldSize(field);
            field->staticValue = HpAllocateNative(field->dataSize);
        }
        else
        {
            UINTPTR size = LdrCalculateFieldSize(field);
            UINTPTR alignedOffset = (structSize + size - 1) / size * size;
            structSize = alignedOffset;

            field->offset = structSize;
            field->dataSize = size;
            structSize += size;
        }
    }

    return structSize;
}

struct RUNTIME_DOMAIN *LdrLoadDomain(struct IMAGE_LOADER *thisPtr)
{
    struct RUNTIME_DOMAIN *domain = HpAllocateNative(sizeof(struct RUNTIME_DOMAIN));
    ExDomainInitialize(domain);

    UINT32 magic;
    LdrImageRead(thisPtr, &magic, sizeof magic);

    if (magic != LDR_MAGIC)
    {
        PalDebugbreak("Bad image!");
    }

    UINT32 sectionCount;
    LdrImageRead(thisPtr, &sectionCount, sizeof sectionCount);

    VOID *stringTable = NULL;
    VOID **sections = HpAllocateNative(sectionCount * sizeof(VOID *));
    PalMemoryZero(sections, sectionCount * sizeof(VOID *));

    thisPtr->sections = sections;
    thisPtr->domain = domain;

    for (int i = 0; i < sectionCount; ++i)
    {
        struct LOADER_SECTION section;
        LdrImageRead(thisPtr, &section, sizeof section);

        UINTPTR prevOffset = thisPtr->offset;

        switch (section.type)
        {
            case NlSectionHeader:
            {
                break;
            }
            case NlSectionString:
            {
                sections[i] = LdrLoadStringTable(thisPtr);
                stringTable = sections[i];
                break;
            }

            case NlSectionType:
            {
                struct RUNTIME_TYPE *loadedType = LdrLoadType(thisPtr);
                sections[i] = loadedType;
                loadedType->domain = domain;
                RtlVectorAdd(&domain->types, sections[i]);
                break;
            }
            case NldSectionField:
            {
                struct RUNTIME_FIELD *value = LdrLoadField(thisPtr);
                sections[i] = value;
                break;
            }
            case NlSectionMethod:
            {
                sections[i] = LdrLoadMethod(thisPtr);
                break;
            }
            case NlSectionCode:
            {
                sections[i] = LdrLoadExecutable(thisPtr);
                break;
            }
            case NlSectionHandler:
            {
                sections[i] = LdrLoadHandler(thisPtr);
                break;
            }
            case NlSectionAttribute:
            {
                sections[i] = LdrLoadAttribute(thisPtr);
                break;
            }
            case NlSectionArgument:
            {
                sections[i] = LdrLoadArgument(thisPtr);
                break;
            }
            default:
            {
                PalDebugbreak("Bad section type!");
                break;
            }
        }

        thisPtr->offset = prevOffset + section.size - sizeof(struct LOADER_SECTION);
    }

    // HpFreeNative(sections);
    // HpFreeNative(stringTable);

    LdrCalculateTypeSizes(domain);
    LdrFillTypeInfo(domain);

    return domain;
}
