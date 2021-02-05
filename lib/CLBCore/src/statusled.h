#pragma once

#define WIFI_FLASH_MSECS 200
#define MQTT_FLASH_MSECS 500

#define STATUS_LED_OK 200
#define STATUS_LED_STOPPED 201

struct StatusLedSettings {
	int statusLedOutputPin;
	bool statusLedOutputPinActiveLow;
	bool statusLedEnabled;
};

void flashStatusLed(int flashes);

void statusLedOff();

void statusLedOn();

void statusLedToggle();

void startstatusLedFlash (int flashLength);

void stopStatusLedFlash();

void statusLedStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

extern struct StatusLedSettings statusLedSettings;

extern struct SettingItemCollection statusLedSettingItems;

extern struct process statusLedProcess;
