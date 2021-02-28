
#include "utils.h"
#include "processes.h"
#include "settings.h"
#include "pixels.h"
#include "otaupdate.h"

#define ProductKey "efc8b1da-4927-48aa-95d1-c52a6cda8099"
#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"

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


void performOTAUpdate()
{
	String url = "http://otadrive.com/DeviceApi/GetEsp8266Update?";
	WiFiClient client;

	ota_update_init = true;

#if defined(ARDUINO_ARCH_ESP8266)

	url += "&s=" + String(PROC_ID);
	url += MakeFirmwareInfo(ProductKey, Version);

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
	url += MakeFirmwareInfo(ProductKey, Version);

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



