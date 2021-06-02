#pragma once

#include "processes.h"
#include "connectwifi.h"

#define OTAUPDATE_OK 1400
#define OTAUPDATE_OFF 1401

#define OTA_UPDATE_PATH_SIZE 200
#define OTA_UPDATE_PRODUCT_KEY_SIZE 100

#define Version "1.0.0.91"

void performOTAUpdate();

extern struct process otaUpdateProcessDescriptor;

struct OtaUpdateSettings 
{
    char otaUpdatePath [OTA_UPDATE_PATH_SIZE];
    char otaUpdateProdKey [OTA_UPDATE_PRODUCT_KEY_SIZE];
};

extern struct OtaUpdateSettings otaUpdateSettings;

