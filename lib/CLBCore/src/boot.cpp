#include <Arduino.h>
#include <limits.h>

#include "connectwifi.h"
#include "utils.h"
#include "boot.h"

struct BootSettings bootSettings;

char bootReasonMessage[BOOT_REASON_MESSAGE_SIZE];
unsigned char bootMode;

unsigned long bootStartMillis;

void setDefaultAccessPointTimeoutSecs(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = BOOT_CONFIG_AP_DURATION_DEFAULT_SECS;
}

boolean validateBootTimeout(void *dest, const char *newValueStr)
{
	int value;

    if(!validateInt(&value, newValueStr))
        return false;

    if( (value<BOOT_CONFIG_AP_DURATION_MIN_SECS) || (value>BOOT_CONFIG_AP_DURATION_MAX_SECS))
        return false;

    *(int *)dest = value;
    return true;
} 

struct SettingItem accessPointTimeoutSecs = {"Access Point Timeout seconds",
											 "accesspointtimeoutsecs",
											 &bootSettings.accessPointTimeoutSecs,
											 NUMBER_INPUT_LENGTH,
											 integerValue,
											 setDefaultAccessPointTimeoutSecs,
											 validateBootTimeout};

struct SettingItem *bootSettingItemPointers[] =
	{
		&accessPointTimeoutSecs};

struct SettingItemCollection bootSettingItems = {
	"boot",
	"Boot behaviour settings",
	bootSettingItemPointers,
	sizeof(bootSettingItemPointers) / sizeof(struct SettingItem *)};

void getBootReasonMessage(char *buffer, int bufferlength)
{
#if defined(ARDUINO_ARCH_ESP32)

    esp_reset_reason_t reset_reason = esp_reset_reason();

    switch (reset_reason)
    {
    case ESP_RST_UNKNOWN:
        snprintf(buffer, bufferlength, "Reset reason can not be determined");
        break;
    case ESP_RST_POWERON:
        snprintf(buffer, bufferlength, "Reset due to power-on event");
        break;
    case ESP_RST_EXT:
        snprintf(buffer, bufferlength, "Reset by external pin (not applicable for ESP32)");
        break;
    case ESP_RST_SW:
        snprintf(buffer, bufferlength, "Software reset via esp_restart");
        break;
    case ESP_RST_PANIC:
        snprintf(buffer, bufferlength, "Software reset due to exception/panic");
        break;
    case ESP_RST_INT_WDT:
        snprintf(buffer, bufferlength, "Reset (software or hardware) due to interrupt watchdog");
        break;
    case ESP_RST_TASK_WDT:
        snprintf(buffer, bufferlength, "Reset due to task watchdog");
        break;
    case ESP_RST_WDT:
        snprintf(buffer, bufferlength, "Reset due to other watchdogs");
        break;
    case ESP_RST_DEEPSLEEP:
        snprintf(buffer, bufferlength, "Reset after exiting deep sleep mode");
        break;
    case ESP_RST_BROWNOUT:
        snprintf(buffer, bufferlength, "Brownout reset (software or hardware)");
        break;
    case ESP_RST_SDIO:
        snprintf(buffer, bufferlength, "Reset over SDIO");
        break;
    }

    if (reset_reason == ESP_RST_DEEPSLEEP)
    {
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            snprintf(buffer, bufferlength, "In case of deep sleep: reset was not caused by exit from deep sleep");
            break;
        case ESP_SLEEP_WAKEUP_ALL:
            snprintf(buffer, bufferlength, "Not a wakeup cause: used to disable all wakeup sources with esp_sleep_disable_wakeup_source");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            snprintf(buffer, bufferlength, "Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            snprintf(buffer, bufferlength, "Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            snprintf(buffer, bufferlength, "Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            snprintf(buffer, bufferlength, "Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            snprintf(buffer, bufferlength, "Wakeup caused by ULP program");
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            snprintf(buffer, bufferlength, "Wakeup caused by GPIO (light sleep only)");
            break;
        case ESP_SLEEP_WAKEUP_UART:
            snprintf(buffer, bufferlength, "Wakeup caused by UART (light sleep only)");
            break;
        }
    }
    else
    {
        snprintf(buffer, bufferlength, "Unknown reset reason %d", reset_reason);
    }
#endif

#if defined(ARDUINO_ARCH_ESP8266)

    rst_info *resetInfo;

    resetInfo = ESP.getResetInfoPtr();

    switch (resetInfo->reason)
    {

    case REASON_DEFAULT_RST:
        snprintf(buffer, bufferlength, "Normal startup by power on");
        break;

    case REASON_WDT_RST:
        snprintf(buffer, bufferlength, "Hardware watch dog reset");
        break;

    case REASON_EXCEPTION_RST:
        snprintf(buffer, bufferlength, "Exception reset, GPIO status won't change");
        break;

    case REASON_SOFT_WDT_RST:
        snprintf(buffer, bufferlength, "Software watch dog reset, GPIO status won't change");
        break;

    case REASON_SOFT_RESTART:
        snprintf(buffer, bufferlength, "Software restart ,system_restart , GPIO status won't change");
        break;

    case REASON_DEEP_SLEEP_AWAKE:
        snprintf(buffer, bufferlength, "Wake up from deep-sleep");
        break;

    case REASON_EXT_SYS_RST:
        snprintf(buffer, bufferlength, "External system reset");
        break;

    default:
        snprintf(buffer, bufferlength, "Unknown reset cause %d", resetInfo->reason);
        break;
    };

#endif
}

int getRestartCode()
{
    int result = -1;

#if defined(ARDUINO_ARCH_ESP32)

    esp_reset_reason_t reset_reason = esp_reset_reason();
    result = reset_reason;

#endif

#if defined(ARDUINO_ARCH_ESP8266)

    rst_info *resetInfo;

    resetInfo = ESP.getResetInfoPtr();
    result = resetInfo->reason;

#endif

    return result;
}

int getInternalBootCode()
{
    uint32_t result;

    ESP.rtcUserMemoryRead(0, &result, 1);

    return (int)result;
}

void setInternalBootCode(int value)
{
    uint32_t storeValue = value;

    ESP.rtcUserMemoryWrite(0, &storeValue, 1);
}

bool isSoftwareReset()
{
#if defined(ARDUINO_ARCH_ESP8266)

    return getRestartCode() == REASON_SOFT_RESTART;

#endif

#if defined(ARDUINO_ARCH_ESP32)
    return getRestartCode() == ESP_RST_SW;
#endif
}



void initBoot()
{
    getBootReasonMessage(bootReasonMessage, BOOT_REASON_MESSAGE_SIZE);

    Serial.printf("Reset reason: %s\n", bootReasonMessage);

    if (isSoftwareReset())
    {
        Serial.printf("Internal reset - selecting boot mode\n");
        // we have been restarted by a call
        // Pull out the requested boot code and set the
        // device for this
        int storedCode = getInternalBootCode();
        switch (storedCode)
        {
        case DEVICE_BOOT_MODE:
            Serial.println("  Starting as a warm boot device\n");
            bootMode = WARM_BOOT_MODE;
            break;
        case CONFIG_BOOT_MODE:
            Serial.println("  Starting in config mode\n");
            bootMode = CONFIG_BOOT_MODE;
            break;
        default:
            Serial.printf("  Invalid boot code %d. Starting as a device\n", storedCode);
            bootMode = DEVICE_BOOT_MODE;
        }
    }
    else
    {
        if (wifiConfigurationsEmpty())
        {
            // start as an access point
            Serial.printf("Starting as configuration AP\n");
            bootMode = CONFIG_BOOT_MODE;
        }
        else
        {
            // got a configuration - start as a device
            Serial.printf("Starting as a device\n");
            bootMode = DEVICE_BOOT_MODE;
        }
    }
}

void startBoot()
{
    bootStartMillis = millis();
}


void updateBoot()
{
    if (bootMode == CONFIG_BOOT_MODE)
    {
        // config boot mode only lasts for a few seconds after boot
        // then we reset as a device
        unsigned long bootTime = ulongDiff(millis(), bootStartMillis);

        if (bootTime > bootSettings.accessPointTimeoutSecs*1000)
        {
            // reboot as a device
            internalReboot(DEVICE_BOOT_MODE);
        }
    }
}

void stopBoot()
{

}

bool bootStatusOK()
{
    return true;
}

void internalReboot(unsigned char rebootCode)
{
    setInternalBootCode(rebootCode);
    ESP.restart();
}

void bootStatusMessage(char *buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Boot reason:\"%s\" code:%d mode:%d", 
    bootReasonMessage, getRestartCode(), bootMode);
}

struct process bootProcessDescriptor = { 
	"boot", 
	initBoot,
	startBoot, 
	updateBoot, 
	stopBoot, 
	bootStatusOK,
	bootStatusMessage, 
	false, 
	0, 
	0, 
	0,
	NULL,
	(unsigned char*) &bootSettings, sizeof(bootSettings), &bootSettingItems,
	NULL,
	BOOT_PROCESS+ACTIVE_PROCESS+CONFIG_PROCESS+WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL};
