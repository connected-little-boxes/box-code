#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "statusled.h"
#include "processes.h"
#include "messages.h"
#include "printer.h"

unsigned long millisAtLastFlash;
unsigned long flashDurationInMillis = 0;
bool ledLit = false;

struct StatusLedSettings statusLedSettings;

void setDefaultstatusLedOutputPin(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = LED_BUILTIN;
}

struct SettingItem statusLedOutputPinSetting = {
    "Status LED output Pin",
    "statusledoutputpin",
    &statusLedSettings.statusLedOutputPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultstatusLedOutputPin,
    validateInt};

struct SettingItem statusLedOutputPinActiveLowSetting = {
    "Status LED output Active Low",
    "statusledoutputactivelow",
    &statusLedSettings.statusLedOutputPinActiveLow,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setTrue,
    validateYesNo};

struct SettingItem statusLedEnabled = {
    "Status LED enabled",
    "statusledactive",
    &statusLedSettings.statusLedEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setTrue,
    validateYesNo};

struct SettingItem *statusLedSettingItemPointers[] =
    {
        &statusLedOutputPinSetting,
        &statusLedOutputPinActiveLowSetting,
        &statusLedEnabled};

struct SettingItemCollection statusLedSettingItems = {
    "statusled",
    "Pin assignment, level (high or low) and enable/disable for status led output",
    statusLedSettingItemPointers,
    sizeof(statusLedSettingItemPointers) / sizeof(struct SettingItem *)};

void statusLedOff()
{
    if (!ledLit)
        return;

    if (statusLedSettings.statusLedOutputPinActiveLow)
        digitalWrite(statusLedSettings.statusLedOutputPin, true);
    else
        digitalWrite(statusLedSettings.statusLedOutputPin, false);

    ledLit = false;
}

void statusLedOn()
{
    if (ledLit)
        return;

    if (statusLedSettings.statusLedOutputPinActiveLow)
    {
        digitalWrite(statusLedSettings.statusLedOutputPin, false);
    }
    else{
        digitalWrite(statusLedSettings.statusLedOutputPin, true);
    }

    ledLit = true;
}

void ledToggle()
{
    millisAtLastFlash = millis();

    if (ledLit)
        statusLedOff();
    else
        statusLedOn();
}

void startstatusLedFlash(int flashLength)
{
    statusLedOn();

    millisAtLastFlash = millis();

    flashDurationInMillis = flashLength;
}

void updateStatusLedFlash()
{
    if (flashDurationInMillis == 0)
    {
        statusLedOff();
        return;
    }

    unsigned long millisSinceLastFlash = millis() - millisAtLastFlash;

    if (millisSinceLastFlash > flashDurationInMillis)
        ledToggle();
}

#define DIGIT_FLASH_INTERVAL 400
#define DIGIT_GAP 500

void flashStatusLedDigits(int flashes)
{
    for (int i = 0; i < flashes; i++)
    {
        statusLedOn();
        delay(DIGIT_FLASH_INTERVAL);
        statusLedOff();
        delay(DIGIT_FLASH_INTERVAL);
    }
}

void flashStatusLed(int flashes)
{
    int tenPowers = 1;

    while (flashes > tenPowers)
    {
        tenPowers = tenPowers * 10;
    }

    tenPowers = tenPowers / 10;

    while (tenPowers > 0)
    {
        int digit = (flashes / tenPowers) % 10;
        flashStatusLedDigits(digit);
        tenPowers = tenPowers / 10;
        delay(DIGIT_GAP);
    }
}

void setFlashLength(int flashLength)
{
    millisAtLastFlash = millis();
    flashDurationInMillis = flashLength;
    statusLedOn();
}

void displayMessageOnStatusLed(int messageNumber, ledFlashBehaviour severity, char *messageText)
{
    int length;

    switch(severity)
    {
        case ledFlashOn:
        length = 5000;
        break;

        case ledFlashNormalState:
        length = 1000;
        break;

        case ledFlashConfigState:
        length = 500;
        break;

        case ledFlashAlertState:
        length = 100;
        break;

        default:
        length = 100;
        break;
    }

    setFlashLength(length);
}

void initStatusLed()
{
    // When the device starts we light the status led while
    // everything initializes
    if (statusLedSettings.statusLedEnabled)
    {
#if defined(WEMOSD1MINI)
// The Wemos device uses pin 2 for both the printer and the builtin led
// Can't use both at once....
        if(printerSettings.printerEnabled && statusLedSettings.statusLedOutputPin==LED_BUILTIN)
        {
            Serial.println("The built in status led is disabled when using the printer output.");
        }
        else {
            pinMode(statusLedSettings.statusLedOutputPin, OUTPUT);
            statusLedOn();
        }
#else
        pinMode(statusLedSettings.statusLedOutputPin, OUTPUT);
        statusLedOn();
#endif
    }

    statusLedProcess.status = STATUS_LED_STOPPED;
}

void startStatusLed()
{
    if (statusLedSettings.statusLedEnabled)
    {
        bindMessageHandler(displayMessageOnStatusLed);
        statusLedProcess.status = STATUS_LED_OK;
    }
    else { 
        statusLedProcess.status = STATUS_LED_STOPPED;
    }
}

void updateStatusLed()
{
    if (statusLedProcess.status == STATUS_LED_STOPPED)
    {
        return ;
    }

    if (!statusLedSettings.statusLedEnabled)
    {
        statusLedOff();
    }

    updateStatusLedFlash();
}

void stopstatusLed()
{
    statusLedProcess.status = STATUS_LED_STOPPED;
}

bool statusLedStatusOK()
{
	return statusLedProcess.status == STATUS_LED_OK;
}

void statusLedStatusMessage(char *buffer, int bufferLength)
{
    if (statusLedProcess.status == STATUS_LED_STOPPED)
    {
        snprintf(buffer, bufferLength, "Status led stopped");
        return;
    }

    if (ledLit)
        snprintf(buffer, bufferLength, "Status led lit");
    else
        snprintf(buffer, bufferLength, "Status led off");
}

struct process statusLedProcess = {
    "statusled",
    initStatusLed,
    startStatusLed,
    updateStatusLed,
    stopstatusLed,
    statusLedStatusOK,
    statusLedStatusMessage,
    false,
    0,
    0,
    0,
    NULL,
    (unsigned char *)&statusLedSettings, sizeof(StatusLedSettings), &statusLedSettingItems,
    NULL,
    BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
    NULL,
    NULL,
    NULL,
    NULL, // no command options
    0     // no command options
};
