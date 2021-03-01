#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "MAX7219Messages.h"
#include "processes.h"
#include "mqtt.h"
#include "errors.h"

#include <MD_MAXPanel.h>

MD_MAX72XX::fontType_t _Fixed_5x3[] PROGMEM = {
    'F', 1, 32, 127, 5,
    2, 0, 0,       // 32 - 'Space'
    1, 23,         // 33 - '!'
    3, 3, 0, 3,    // 34 - '"'
    3, 31, 10, 31, // 35 - '#'
    3, 22, 31, 13, // 36 - '$'
    3, 9, 4, 18,   // 37 - '%'
    3, 10, 21, 26, // 38 - '&'
    1, 3,          // 39
    2, 14, 17,     // 40 - '('
    2, 17, 14,     // 41 - ')'
    3, 10, 4, 10,  // 42 - '*'
    3, 4, 14, 4,   // 43 - '+'
    2, 16, 8,      // 44 - ','
    3, 4, 4, 4,    // 45 - '-'
    1, 16,         // 46 - '.'
    3, 8, 4, 2,    // 47 - '/'
    3, 31, 17, 31, // 48 - '0'
    2, 0, 31,      // 49 - '1'
    3, 29, 21, 23, // 50 - '2'
    3, 17, 21, 31, // 51 - '3'
    3, 7, 4, 31,   // 52 - '4'
    3, 23, 21, 29, // 53 - '5'
    3, 31, 21, 29, // 54 - '6'
    3, 1, 1, 31,   // 55 - '7'
    3, 31, 21, 31, // 56 - '8'
    3, 23, 21, 31, // 57 - '9'
    1, 10,         // 58 - ':'
    2, 16, 10,     // 59 - ';'
    3, 4, 10, 17,  // 60 - '<'
    3, 10, 10, 10, // 61 - '='
    3, 17, 10, 4,  // 62 - '>'
    3, 1, 21, 3,   // 63 - '?'
    3, 14, 21, 22, // 64 - '@'
    3, 30, 5, 30,  // 65 - 'A'
    3, 31, 21, 10, // 66 - 'B'
    3, 14, 17, 17, // 67 - 'C'
    3, 31, 17, 14, // 68 - 'D'
    3, 31, 21, 17, // 69 - 'E'
    3, 31, 5, 1,   // 70 - 'F'
    3, 14, 17, 29, // 71 - 'G'
    3, 31, 4, 31,  // 72 - 'H'
    3, 17, 31, 17, // 73 - 'I'
    3, 8, 16, 15,  // 74 - 'J'
    3, 31, 4, 27,  // 75 - 'K'
    3, 31, 16, 16, // 76 - 'L'
    3, 31, 2, 31,  // 77 - 'M'
    3, 31, 14, 31, // 78 - 'N'
    3, 14, 17, 14, // 79 - 'O'
    3, 31, 5, 2,   // 80 - 'P'
    3, 14, 25, 30, // 81 - 'Q'
    3, 31, 5, 26,  // 82 - 'R'
    3, 18, 21, 9,  // 83 - 'S'
    3, 1, 31, 1,   // 84 - 'T'
    3, 15, 16, 15, // 85 - 'U'
    3, 7, 24, 7,   // 86 - 'V'
    3, 15, 28, 15, // 87 - 'W'
    3, 27, 4, 27,  // 88 - 'X'
    3, 3, 28, 3,   // 89 - 'Y'
    3, 25, 21, 19, // 90 - 'Z'
    2, 31, 17,     // 91 - '['
    3, 2, 4, 8,    // 92 - '\'
    2, 17, 31,     // 93 - ']'
    3, 2, 1, 2,    // 94 - '^'
    3, 16, 16, 16, // 95 - '_'
    2, 1, 2,       // 96 - '`'
    3, 12, 18, 28, // 97 - 'a'
    3, 31, 18, 12, // 98 - 'b'
    3, 12, 18, 18, // 99 - 'c'
    3, 12, 18, 31, // 100 - 'd'
    3, 12, 26, 20, // 101 - 'e'
    3, 4, 31, 5,   // 102 - 'f'
    3, 20, 26, 12, // 103 - 'g'
    3, 31, 2, 28,  // 104 - 'h'
    1, 29,         // 105 - 'i'
    2, 16, 13,     // 106 - 'j'
    3, 31, 8, 20,  // 107 - 'k'
    1, 31,         // 108 - 'l'
    3, 30, 6, 30,  // 109 - 'm'
    3, 30, 2, 28,  // 110 - 'n'
    3, 12, 18, 12, // 111 - 'o'
    3, 30, 10, 4,  // 112 - 'p'
    3, 4, 10, 30,  // 113 - 'q'
    2, 30, 4,      // 114 - 'r'
    3, 20, 30, 10, // 115 - 's'
    3, 4, 30, 4,   // 116 - 't'
    3, 14, 16, 30, // 117 - 'u'
    3, 14, 16, 14, // 118 - 'v'
    3, 14, 24, 14, // 119 - 'w'
    3, 18, 12, 18, // 120 - 'x'
    3, 22, 24, 14, // 121 - 'y'
    3, 26, 30, 22, // 122 - 'z'
    3, 4, 27, 17,  // 123 - '{'
    1, 27,         // 124 - '|'
    3, 17, 27, 4,  // 125 - '}'
    3, 6, 2, 3,    // 126 - '~'
    3, 31, 31, 31, // 127 - 'Full Block'
};

struct Max7219MessagesSettings max7219MessagesSettings;

MD_MAXPanel *mp = NULL;

char MAX7219MessageBuffer[MAX_MESSAGE_LENGTH];

int MAX7219scrollPos;

bool MAX7219scrolling = false;

int MAX7219FrameDelay;

int scrollDelayMS = 0;

void MAX7219setScrollDelay()
{
    MAX7219FrameDelay = (int)max7219MessagesSettings.maxFrameTimeMS *
                        max7219MessagesSettings.frameTimeFraction;
}

void setDefaultMAX7219DataPinNo(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 4;
}

struct SettingItem MAX7219DataPinNo = {
    "MAX7219 Data Pin",
    "max7219datapin",
    &max7219MessagesSettings.dataPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultMAX7219DataPinNo,
    validateInt};

void setDefaultMAX7219ChipSelectPinNo(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 5;
}

struct SettingItem MAX7219ChipSelectPinNo = {
    "MAX7219 Chip Select Pin",
    "max7219cspin",
    &max7219MessagesSettings.chipSelectPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultMAX7219ChipSelectPinNo,
    validateInt};

void setDefaultMAX7219ClockPinNo(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 15;
}

struct SettingItem MAX7219ClockPinNo = {
    "MAX7219 Clock Pin",
    "max7219clockpin",
    &max7219MessagesSettings.clockPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultMAX7219ClockPinNo,
    validateInt};

void setDefaultMAX7219XDevices(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 4;
}

struct SettingItem MAX7219XDevices = {
    "MAX7219 X devices",
    "max7219xdev",
    &max7219MessagesSettings.xDevices,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultMAX7219XDevices,
    validateInt};

void setDefaultMAXY219YDevices(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 1;
}

struct SettingItem MAX7219YDevices = {
    "MAX7219 Y devices",
    "max7219ydev",
    &max7219MessagesSettings.yDevices,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultMAXY219YDevices,
    validateInt};

void setDefaultMAXY219MaxFrameTime(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = MAX7219_MAX_FRAME_TIME_UPPER_LIMIT;
}

boolean validateMAX7219MaxFrameTime(void *dest, const char *newValueStr)
{
    int value;

    if (!validateInt(&value, newValueStr))
    {
        return false;
    }

    if (value < 0)
    {
        return false;
    }

    if (value > MAX7219_MAX_FRAME_TIME_UPPER_LIMIT)
    {
        return false;
    }

    *(int *)dest = value;

    MAX7219setScrollDelay();

    return true;
}

struct SettingItem MAX7219MaxFrameTimeMS = {
    "MAX7219 maximum frame time (millis)",
    "max7219frametime",
    &max7219MessagesSettings.maxFrameTimeMS,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultMAXY219MaxFrameTime,
    validateMAX7219MaxFrameTime};

void setDefaultMAXY219FrameTimeFraction(void *dest)
{
    float *destFloat = (float *)dest;
    *destFloat = MAX7219_DEFAULT_FRAME_TIME_FRACTION;
}

boolean validateMAX7219FrameTimeFraction(void *dest, const char *newValueStr)
{
    float value;

    if (!validateFloat0to1(&value, newValueStr))
    {
        return false;
    }

    *(float *)dest = value;

    MAX7219setScrollDelay();

    return true;
}

struct SettingItem MAX7219FrameTimeFraction = {
    "MAX7219 frame delay (0-1)",
    "max7219framedelay",
    &max7219MessagesSettings.frameTimeFraction,
    NUMBER_INPUT_LENGTH,
    floatValue,
    setDefaultMAXY219FrameTimeFraction,
    validateMAX7219FrameTimeFraction};

void setDefaultMAXY219Brightness(void *dest)
{
    float *destFloat = (float *)dest;
    *destFloat = MAX7219_DEFAULT_BRIGHTNESS_FRACTION;
}

struct SettingItem MAX7219BrightnessFraction = {
    "MAX7219 brightness (0-1)",
    "max7219brightness",
    &max7219MessagesSettings.brightness,
    NUMBER_INPUT_LENGTH,
    floatValue,
    setDefaultMAXY219Brightness,
    validateFloat0to1};

struct SettingItem max7219MessagesEnabled = {
    "Messages enabled",
    "max7219Messagesactive",
    &max7219MessagesSettings.max7219MessagesEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

void setDefaultMax7219Message(void *dest)
{
	strcpy((char *)dest, "Hello!");;
}

boolean validateMax7219Message(void *dest, const char *source)
{
    return validateString((char *)dest,source, MAX_MESSAGE_LENGTH);
}

struct SettingItem max7219defaultMessage = {
    "Text displayed at powerup",
    "max7219defaultMessage",
    &max7219MessagesSettings.max7219DefaultMessage,
    MAX_MESSAGE_LENGTH,
    text,
    setDefaultMax7219Message,
    validateMax7219Message};

struct SettingItem *max7219MessagesSettingItemPointers[] =
    {
        &max7219MessagesEnabled,
        &max7219defaultMessage,
        &MAX7219DataPinNo,
        &MAX7219ChipSelectPinNo,
        &MAX7219ClockPinNo,
        &MAX7219XDevices,
        &MAX7219YDevices,
        &MAX7219MaxFrameTimeMS,
        &MAX7219FrameTimeFraction,
        &MAX7219BrightnessFraction};

struct SettingItemCollection max7219MessagesSettingItems = {
    "max7219Messages",
    "Enable/disable for message output",
    max7219MessagesSettingItemPointers,
    sizeof(max7219MessagesSettingItemPointers) / sizeof(struct SettingItem *)};

void max7219MessagesOff()
{
    max7219MessagesSettings.max7219MessagesEnabled = false;
    saveSettings();
}

void max7219MessagesOn()
{
    max7219MessagesSettings.max7219MessagesEnabled = true;
    saveSettings();
}

void max7217SetBrightness()
{
    mp->setIntensity((int)(max7219MessagesSettings.brightness * 15.0));
}

// font defined in 
extern const uint8_t PROGMEM _sysfont[];  ///< System variable pitch font table

void startMAX7219MessageScroll()
{
    MAX7219scrollPos = 0;

    MAX7219scrolling = true;

    MAX7219setScrollDelay();
}

void stopMAX7219MessageScroll()
{
    MAX7219scrollPos = 0;

    MAX7219scrolling = false;
}

void displayMAX7219Message(char *messageText, char * options)
{
    snprintf(MAX7219MessageBuffer, MAX_MESSAGE_LENGTH, messageText);

    if (strContains(options, "scroll"))
    {
        startMAX7219MessageScroll();
    }
    else
    {
        stopMAX7219MessageScroll();
    }

    if(strContains(options,"small"))
    {
        mp->setFont(_Fixed_5x3);
    }
    else
    {
        mp->setFont(_sysfont);
    }

    mp->update(false);

    mp->clear();

    max7217SetBrightness();

    mp->drawText(0, mp->getYMax(), MAX7219MessageBuffer, MD_MAXPanel::ROT_0);

    mp->update();
}

#define MAX7219_FLOAT_VALUE_OFFSET 0
#define MAX7219_MESSAGE_OFFSET (MAX7219_FLOAT_VALUE_OFFSET + sizeof(float))
#define MAX7219_OPTION_OFFSET (MAX7219_MESSAGE_OFFSET + MAX_MESSAGE_LENGTH)
#define MAX7219_PRE_TEXT_OFFSET (MAX7219_OPTION_OFFSET + MAX7219MESSAGE_COMMAND_LENGTH)
#define MAX7219_POST_TEXT_OFFSET (MAX7219_PRE_TEXT_OFFSET + MAX7219MESSAGE_COMMAND_LENGTH)

boolean validateMAX7219OptionString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, MAX7219MESSAGE_COMMAND_LENGTH));
}

boolean validateMAX7219MessageString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, MAX_MESSAGE_LENGTH));
}

struct CommandItem MAX7219floatValueItem = {
    "value",
    "value",
    MAX7219_FLOAT_VALUE_OFFSET,
    floatCommand,
    validateFloat,
    noDefaultAvailable};

struct CommandItem MAX7219CommandOptionName = {
    "options",
    "max7219 message options (small,scroll)",
    MAX7219_OPTION_OFFSET,
    textCommand,
    validateMAX7219OptionString,
    setDefaultEmptyString};

struct CommandItem MAX7219MessageText = {
    "text",
    "max7219 message text",
    MAX7219_MESSAGE_OFFSET,
    textCommand,
    validateMAX7219MessageString,
    noDefaultAvailable};

struct CommandItem MAX7219PreText = {
    "pre",
    "max7219 pre text",
    MAX7219_PRE_TEXT_OFFSET,
    textCommand,
    validateMAX7219MessageString,
    setDefaultEmptyString};

struct CommandItem MAX7219PostText = {
    "post",
    "max7219 post text",
    MAX7219_POST_TEXT_OFFSET,
    textCommand,
    validateMAX7219MessageString,
    setDefaultEmptyString};

struct CommandItem *MAX7219DisplayMessageCommandItems[] =
    {
        &MAX7219MessageText,
        &MAX7219CommandOptionName,
        &MAX7219PreText,
        &MAX7219PostText};

int doSetMAX7219Message(char *destination, unsigned char *settingBase);

struct Command setMAX7219Message
{
    "display",
        "Displays a message",
        MAX7219DisplayMessageCommandItems,
        sizeof(MAX7219DisplayMessageCommandItems) / sizeof(struct CommandItem *),
        doSetMAX7219Message
};

int doSetMAX7219Message(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("max7219", &setMAX7219Message, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (max7219MessagesProcess.status != MAX7219MESSAGES_OK)
    {
        return JSON_MESSAGE_MAX7219_NOT_ENABLED;
    }

    char *message = (char *)(settingBase + MAX7219_MESSAGE_OFFSET);
    char *post = (char *)(settingBase + MAX7219_POST_TEXT_OFFSET);
    char *pre = (char *)(settingBase + MAX7219_PRE_TEXT_OFFSET);

    char buffer[MAX_MESSAGE_LENGTH];

    snprintf(buffer, MAX_MESSAGE_LENGTH, "%s%s%s", pre, message, post);

    char *options= (char *)(settingBase + MAX7219_OPTION_OFFSET);

    displayMAX7219Message(buffer, options);

    return WORKED_OK;
}

struct CommandItem *MAX7219DefaultMessageCommandItems[] =
    {
        &MAX7219MessageText
        };

int doSetMAX7219DefaultMessage(char *destination, unsigned char *settingBase);

struct Command setMAX7219DefaultMessage
{
    "default",
        "Sets the message displayed at power on",
        MAX7219DefaultMessageCommandItems,
        sizeof(MAX7219DefaultMessageCommandItems) / sizeof(struct CommandItem *),
        doSetMAX7219DefaultMessage
};

int doSetMAX7219DefaultMessage(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("max7219", &setMAX7219DefaultMessage, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    const char *message = (const char *)(settingBase + MAX7219_MESSAGE_OFFSET);

    if(!validateMax7219Message(max7219MessagesSettings.max7219DefaultMessage, message))
    {
        return JSON_MESSAGE_MAX7219_INVALID_DEFAULT;
    }

    saveSettings();

    return WORKED_OK;
}

struct CommandItem *MAX7219DisplayValueCommandItems[] =
    {
        &MAX7219floatValueItem,
        &MAX7219CommandOptionName,
        &MAX7219PreText,
        &MAX7219PostText};

int doShowMAX7219value(char *destination, unsigned char *settingBase);

struct Command showMAX7219value
{
    "showvalue",
        "show a value",
        MAX7219DisplayValueCommandItems,
        sizeof(MAX7219DisplayValueCommandItems) / sizeof(struct CommandItem *),
        doShowMAX7219value
};

int doShowMAX7219value(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("max7219", &showMAX7219value, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (max7219MessagesProcess.status != MAX7219MESSAGES_OK)
    {
        return JSON_MESSAGE_MAX7219_NOT_ENABLED;
    }

    unsigned char *valuePtr = (settingBase + MAX7219_FLOAT_VALUE_OFFSET);

    float value = getUnalignedFloat(valuePtr);

    char *post = (char *)(settingBase + MAX7219_POST_TEXT_OFFSET);
    char *pre = (char *)(settingBase + MAX7219_PRE_TEXT_OFFSET);

    char *options = (char *)(settingBase + MAX7219_OPTION_OFFSET);

    char buffer[MAX_MESSAGE_LENGTH];

    snprintf(buffer, MAX_MESSAGE_LENGTH, "%s%.1f%s", pre, value, post);

    displayMAX7219Message(buffer, options);

    return WORKED_OK;
}

struct CommandItem *MAX7219ScrollSpeedCommandItems[] =
    {
        &MAX7219floatValueItem};

int doSetMAX7219ScrollSpeed(char *destination, unsigned char *settingBase);

struct Command setMAX7219ScrollSpeed
{
    "scrollspeed",
        "Scroll speed (0-1)",
        MAX7219ScrollSpeedCommandItems,
        sizeof(MAX7219ScrollSpeedCommandItems) / sizeof(struct CommandItem *),
        doSetMAX7219ScrollSpeed
};

int doSetMAX7219ScrollSpeed(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("max7219", &setMAX7219ScrollSpeed, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (max7219MessagesProcess.status != MAX7219MESSAGES_OK)
    {
        return JSON_MESSAGE_MAX7219_NOT_ENABLED;
    }

    unsigned char *value = (settingBase + MAX7219_FLOAT_VALUE_OFFSET);

    max7219MessagesSettings.frameTimeFraction = getUnalignedFloat(value);

    saveSettings();

    MAX7219setScrollDelay();

    return WORKED_OK;
}

struct CommandItem *MAX7219BrightnessCommandItems[] =
    {
        &MAX7219floatValueItem};

int doSetMAX7219Brightness(char *destination, unsigned char *settingBase);

struct Command setMAX7219Brightness
{
    "brightness",
        "Brightness (0-1)",
        MAX7219BrightnessCommandItems,
        sizeof(MAX7219BrightnessCommandItems) / sizeof(struct CommandItem *),
        doSetMAX7219Brightness
};

int doSetMAX7219Brightness(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("max7219", &setMAX7219Brightness, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (max7219MessagesProcess.status != MAX7219MESSAGES_OK)
    {
        return JSON_MESSAGE_MAX7219_NOT_ENABLED;
    }

    unsigned char *value = (settingBase + MAX7219_FLOAT_VALUE_OFFSET);

    max7219MessagesSettings.brightness = getUnalignedFloat(value);

    saveSettings();

    return WORKED_OK;
}

struct Command *MAX7219CommandList[] = {
    &setMAX7219Message,
    &setMAX7219DefaultMessage,
    &setMAX7219ScrollSpeed,
    &setMAX7219Brightness,
    &showMAX7219value};

struct CommandItemCollection MAX7219Commands =
    {
        "Control MAX7219 display",
        MAX7219CommandList,
        sizeof(MAX7219CommandList) / sizeof(struct Command *)};

unsigned long MAX7219millisAtLastScroll;


void initMAX7219Messages()
{
    // perform all the hardware startup before we start the WiFi running

    max7219MessagesProcess.status = MAX7219MESSAGES_STOPPED;

    if (max7219MessagesSettings.max7219MessagesEnabled)
    {
        if (mp == NULL)
        {
            mp = new MD_MAXPanel(MAX7219_HARDWARE_TYPE,
                                 max7219MessagesSettings.dataPin,
                                 max7219MessagesSettings.clockPin,
                                 max7219MessagesSettings.chipSelectPin,
                                 max7219MessagesSettings.xDevices,
                                 max7219MessagesSettings.yDevices);

            mp->begin();
            mp->clear();

            MAX7219FrameDelay = (int)max7219MessagesSettings.maxFrameTimeMS *
                                max7219MessagesSettings.frameTimeFraction;

            MAX7219millisAtLastScroll = millis();

            max7219MessagesProcess.status = MAX7219MESSAGES_OK;

            MAX7219scrolling = false;
        }
    }
}

void startMAX7219Messages()
{
    // all the hardware is set up in the init function. We just display the default message here
    //

    if (max7219MessagesProcess.status != MAX7219MESSAGES_OK)
    {
        return ;
    }

    displayMAX7219Message(max7219MessagesSettings.max7219DefaultMessage,"");
}

void updateMAX7219Messages()
{
    if (MAX7219scrolling)
    {
        unsigned long currentMillis = millis();
        unsigned long millisSinceLastScroll = ulongDiff(currentMillis, MAX7219millisAtLastScroll);

        if ((int)millisSinceLastScroll < MAX7219FrameDelay)
        {
            return;
        }

        MAX7219scrollPos--;

        int scrollLimit = -MAX7219scrollPos - 8;

        mp->update(false);

        mp->clear();

        max7217SetBrightness();

        int drawLength = mp->drawText(MAX7219scrollPos, mp->getYMax(), MAX7219MessageBuffer, MD_MAXPanel::ROT_0);

        mp->update();

        if (scrollLimit > drawLength)
        {
            MAX7219scrollPos = mp->getXMax() + 5;
        }

        MAX7219millisAtLastScroll = currentMillis;
    }
}

void stopMAX7219Messages()
{
    max7219MessagesProcess.status = MAX7219MESSAGES_STOPPED;
}

bool MAX7219MessagesStatusOK()
{
    return max7219MessagesProcess.status == MAX7219MESSAGES_OK;
}

void MAX7219MessagesStatusMessage(char *buffer, int bufferLength)
{
    if (max7219MessagesProcess.status == MAX7219MESSAGES_STOPPED)
    {
        snprintf(buffer, bufferLength, "Messages stopped");
    }
    else
    {
        snprintf(buffer, bufferLength, "Messages enabled");
    }
}

struct process max7219MessagesProcess = {
    "max7219",
    initMAX7219Messages,
    startMAX7219Messages,
    updateMAX7219Messages,
    stopMAX7219Messages,
    MAX7219MessagesStatusOK,
    MAX7219MessagesStatusMessage,
    false,
    0,
    0,
    0,
    NULL,
    (unsigned char *)&max7219MessagesSettings, sizeof(Max7219MessagesSettings), &max7219MessagesSettingItems,
    &MAX7219Commands,
    BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
    NULL,
    NULL,
    NULL,
    NULL, // no command options
    0     // no command options
};
