#pragma once
#include <intermarx/intermarx.h>

VOID MtInitialize();
struct RUNTIME_THREAD* MtThreadGetCurrent();
VOID MtThreadRegister(struct RUNTIME_THREAD* thread);