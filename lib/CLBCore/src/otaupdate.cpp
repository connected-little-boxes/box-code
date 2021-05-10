#include "utils.h"
#include "processes.h"
#include "settings.h"
#include "pixels.h"
#include "otaupdate.h"

char * Version = "1.0.0.60";

struct OtaUpdateSettings otaUpdateSettings;

boolean validateOtaUpdatePath(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, OTA_UPDATE_PATH_SIZE));
}
 
boolean validateOtaProductKey(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, OTA_UPDATE_PRODUCT_KEY_SIZE));
}

struct SettingItem otaUpdatePathSetting = {
	"Url of the OTA update server", 
	"otaupdateurl", 
	otaUpdateSettings.otaUpdatePath, 
	OTA_UPDATE_PATH_SIZE, 
	text, 
	setEmptyString, validateOtaUpdatePath};

struct SettingItem otaProductKeySetting = {
	"OTA product key", 
	"otaupdateproductkey", 
	otaUpdateSettings.otaUpdateProductKey, 
	OTA_UPDATE_PRODUCT_KEY_SIZE, 
	text, 
	setEmptyString, validateOtaProductKey};

struct SettingItem *otaUpdateSettingItemPointers[] =
	{
		&otaUpdatePathSetting,
		&otaProductKeySetting};

struct SettingItemCollection otaUpdateSettingItems = {
	"otaupdate",
	"Over The Air (OTA) update settings",
	otaUpdateSettingItemPointers,
	sizeof(otaUpdateSettingItemPointers) / sizeof(struct SettingItem *)};


#define USE_SERIAL Serial

void update_started() {
	Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
	Serial.println("CALLBACK:  HTTP update process finished");
	delay(1000);
	ESP.restart();
}

bool ota_update_init = true;

void update_progress(int cur, int total) {

	if (ota_update_init) {
		beginStatusDisplay();
		addStatusItem(true);
		renderStatusDisplay();
		ota_update_init = false;
	}

	Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);

	addStatusItem(true);

	renderStatusDisplay();
}

void update_error(int err) {
	Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void makeUpdateURL( char * buffer, int bufferLength)
{
	snprintf(buffer,bufferLength,
		"%s&s=%lu&_FirmwareInfo&k=%s&v=%s&FirmwareInfo_&",
		otaUpdateSettings.otaUpdatePath,
		PROC_ID,
		otaUpdateSettings.otaUpdateProductKey,
		Version);
}

void performOTAUpdate()
{
	WiFiClient client;
	
	char url[300];

	makeUpdateURL (url, 300);
	
#if defined(ARDUINO_ARCH_ESP8266)

//	url += "&s=" + String(PROC_ID);
//	url += MakeFirmwareInfo(OTA_PRODUCT_KEY, Version);

	Serial.println("Get firmware from url:");
	Serial.println(url);

	ESPhttpUpdate.onStart(update_started);
	ESPhttpUpdate.onEnd(update_finished);
	ESPhttpUpdate.onProgress(update_progress);
	ESPhttpUpdate.onError(update_error);

	t_httpUpdate_return ret = ESPhttpUpdate.update(client, url, Version);

#endif

#if defined(ARDUINO_ARCH_ESP32)

	url += "&s=" + String(PROC_ID);
	url += MakeFirmwareInfo(OTA_PRODUCT_KEY, Version);

	Serial.println("Get firmware from url:");
	Serial.println(url);

	t_httpUpdate_return ret = httpUpdate.update( client, url );

#endif


	switch (ret)
	{
	case HTTP_UPDATE_FAILED:
		Serial.println("Update failed!");
		break;
	case HTTP_UPDATE_NO_UPDATES:
		Serial.println("No new update available");
		break;
		// We can't see this, because of reset chip after update OK
	case HTTP_UPDATE_OK:
		Serial.println("Update OK");
		break;

	default:
		break;
	}
}

void initOtaUpdate()
{
	otaUpdateProcessDescriptor.status = OTAUPDATE_OFF;
}

void startOtaUpdate()
{
	otaUpdateProcessDescriptor.status = OTAUPDATE_OK;
}

void updateOtaUpdate()
{
}

void stopOtaUpdate()
{
	otaUpdateProcessDescriptor.status = OTAUPDATE_OFF;
}

bool otaUpdateStatusOK()
{
	return otaUpdateProcessDescriptor.status == OTAUPDATE_OK;
}

void otaUpdateStatusMessage(char *buffer, int bufferLength)
{
	switch (otaUpdateProcessDescriptor.status)
	{
	case OTAUPDATE_OK:
		snprintf(buffer, bufferLength, "OtaUpdate OK");
		break;
	case OTAUPDATE_OFF:
		snprintf(buffer, bufferLength, "OtaUpdate OFF");
		break;
	default:
		snprintf(buffer, bufferLength, "OtaUpdate status invalid");
		break;
	}
}

struct Command * otaUpdateCommandList[] = {
};

struct CommandItemCollection otaUpdateCommands =
	{
		"Control the Over The Air (OTA) update",
		otaUpdateCommandList,
		sizeof(otaUpdateCommandList) / sizeof(struct Command *)};

struct process otaUpdateProcessDescriptor = {
	"otaUpdate",
	initOtaUpdate,
	startOtaUpdate,
	updateOtaUpdate,
	stopOtaUpdate,
	otaUpdateStatusOK,
	otaUpdateStatusMessage,
	false,
	0,
	0,
	0,
	NULL,
	(unsigned char *)&otaUpdateSettings, sizeof(otaUpdateSettings), &otaUpdateSettingItems,
	&otaUpdateCommands,
	BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL};
