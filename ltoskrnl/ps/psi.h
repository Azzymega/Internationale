#pragma once
#include <ltbase.h>

typedef int (*THREAD_START)(VOID*);

VOID PsiInitializeKernelProcess();
VOID PsiInitializeIdleThread();
VOID PsiThreadPrologue(VOID *arg, VOID *func);
VOID PsiIdle();
VOID PsiSchedule(VOID* trapFrame);