#include <intermarx/ex/runtime.h>
#include <intermarx/hp/hp.h>
#include <intermarx/ldr/ldr.h>
#include <intermarx/pal/pal.h>
#include <intermarx/pal/corelib.h>
#include <intermarx/mt/mt.h>
#include <mm/mm.h>
#include <pal/pal.h>


INUGLOBAL struct RUNTIME_DOMAIN* ExGlobalZeroDomain;
INUGLOBAL MONITOR ExGlobalStackTraceMonitor;

VOID MxDumpStackTrace(struct EXCEPTION_STATE* state)
{
    PalMonitorEnter(&ExGlobalStackTraceMonitor);

    struct MANAGED_EXCEPTION* exception = state->exception;

    PalPrint("%s", "\r\n");
    PalPrint("Exception in thread %i! ", PalThreadGetCurrentId());
    // wprintf(L"%ls", exception->header.type->fullName.characters);
    PalPrint("%s", " - ");

    if (exception->nativeMessage != NULL)
    {
        //wprintf(L"%ls", exception->nativeMessage);
    }
    else
    {
        for (int i = 0; i < exception->message->characters->count; ++i)
        {
            //putchar(exception->message->characters->characters[i]);
        }
    }

    PalPrint("%s", "\r\n");
    PalPrint("%s", "Stack trace: \r\n");

    for (int i = 0; i < state->stackTrace.count; ++i)
    {
        struct NSTRING* str = RtlVectorGet(&state->stackTrace, i);
        //wprintf(L"%ls\r\n", str->characters);
    }

    PalPrint("%s", "\r\n");

    PalMonitorExit(&ExGlobalStackTraceMonitor);

    while (TRUE);
    PalThreadExitCurrent(-1);
}

VOID MxStart(VOID* buffer)
{
    RuntimePalInitialize();
    HpInitialize();

    struct IMAGE_LOADER loader = {
        .image = buffer,
        .offset = 0
    };

    PalMonitorInitialize(&ExGlobalStackTraceMonitor);

    struct RUNTIME_DOMAIN* domain = LdrLoadDomain(&loader);
    ExGlobalZeroDomain = domain;

    ExInitialize();
    MtInitialize();

    LdrExecuteStaticConstructors(domain);
    LdrExecuteAttributesConstructors(domain);

    FarInitialize(domain);

    struct RUNTIME_TYPE* getType = ExDomainLocateType(domain, "Internationale.Runtime.Initializer");
    struct RUNTIME_METHOD* method = ExTypeLocateMethod(getType, "Initialize");

    if (MARX_SUCCESS(ExMethodPrologue(method)))
    {
        return;
    }
    else
    {
        MxDumpStackTrace(&MtThreadGetCurrent()->state);
    }
}
