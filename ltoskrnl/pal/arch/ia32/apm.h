#pragma once
#include <ltbase.h>

BOOLEAN PaliApmIsPresent(BYTE version);
VOID PaliApmEnable();
VOID PaliApmShutdown();
VOID PaliApmDisableInterface();
VOID PaliApmConnectRealModeInterface();
VOID PaliPromoteInterface(BYTE version);
