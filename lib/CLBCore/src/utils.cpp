#include <Arduino.h>
#include <limits.h>

#include "utils.h"

#define LED_BUILTIN 2

unsigned long ulongDiff(unsigned long end, unsigned long start)
{
    if (end >= start)
    {
        return end - start;
    }
    else
    {
        return ULONG_MAX - start + end + 1;
    }
}

bool strContains(char *searchMe, char *findMe)
{
    while (*searchMe != 0)
    {
        if (*searchMe == *findMe)
        {
            // found the start of the find string
            // try to match the rest of the find string from here
            char *s1 = searchMe;
            char *f1 = findMe;
            while (*s1 == *f1)
            {
                f1++;
                s1++;
                if (*f1 == 0)
                {
                    // hit the end of the find string - return with a win
                    return true;
                }
                if (*s1 == 0)
                {
                    // hit the end of the search string before we
                    // completed the find - return with a fail
                    return false;
                }
            }
        }
        searchMe++;
    }
    return false;
}

int getUnalignedInt(unsigned char *source)
{
    int result;
    memcpy((unsigned char *)&result, source, sizeof(int));
    return result;
}

float getUnalignedFloat(unsigned char *source)
{
    float result;
    memcpy((unsigned char *)&result, source, sizeof(float));
    return result;
}

void putUnalignedFloat(float fval, unsigned char *dest)
{
    unsigned char *source = (unsigned char *)&fval;
    memcpy(dest, source, sizeof(float));
}

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

char bootReasonMessage [BOOT_REASON_MESSAGE_SIZE];

void loadBootReasonMessage()
{
    getBootReasonMessage(bootReasonMessage, BOOT_REASON_MESSAGE_SIZE);
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