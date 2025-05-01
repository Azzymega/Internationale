#pragma once

VOID* HppTransferObject(struct OBJECT_HEADER* header);
VOID HppTraceStruct(VOID* structure, struct RUNTIME_TYPE* type);
VOID* HppTraceObject(struct OBJECT_HEADER* header);
VOID HppTraceField(struct RUNTIME_FIELD* field);