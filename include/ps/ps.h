#pragma once
#include <ltbase.h>
#include <ob/object.h>
#include <ps/sched.h>
#include <rtl/list.h>

VOID PsInitialize();
INUSTATUS PsCreateThread(OUT REFERENCE* reference, IN struct PROCESS* process, IN VOID* function, IN VOID* argument);