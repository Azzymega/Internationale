#include <intermarx/ex/ex.h>
#include <intermarx/mt/mt.h>
#include <intermarx/ob/ob.h>
#include <intermarx/pal/pal.h>

INUIMPORT struct RUNTIME_DOMAIN* ExGlobalZeroDomain;

INUGLOBAL struct RUNTIME_THREAD MtGlobalBootstrapThread;
INUGLOBAL struct VECTOR MtGlobalThreadList;

VOID MtInitialize()
{
    RtlVectorInitialize(&MtGlobalThreadList);
    RtlVectorAdd(&MtGlobalThreadList,&MtGlobalBootstrapThread);

    ExThreadInitialize(&MtGlobalBootstrapThread,ExGlobalZeroDomain);
}

VOID MtThreadRegister(struct RUNTIME_THREAD *thread)
{
    RtlVectorAdd(&MtGlobalThreadList,thread);
}

struct RUNTIME_THREAD * MtThreadGetCurrent()
{
    UINTPTR id = PalThreadGetCurrentId();

    for (int i = 0; i < MtGlobalThreadList.count; ++i)
    {
        struct RUNTIME_THREAD* thread = RtlVectorGet(&MtGlobalThreadList,i);
        if (thread->id == id)
        {
            return thread;
        }
    }

    return NULL;
}
