#pragma once

#include "processes.h"
#include "connectwifi.h"

#define OTAUPDATE_OK 0
#define OTAUPDATE_OFF 1
#define OTAUPDATE_ERROR_NO_WIFI -1

void performOTAUpdate();

