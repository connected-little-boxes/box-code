#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "printer.h"
#include "processes.h"
#include "mqtt.h"
#include "errors.h"

struct PrinterSettings printerSettings;

char PRINTERMessageBuffer[PRINTERMESSAGE_LENGTH];

void setDefaultPRINTERDataPinNo(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 16;
}

struct SettingItem PrinterDataPinNo = {
    "PRINTER Data Pin",
    "printerdatapin",
    &printerSettings.dataPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultPRINTERDataPinNo,
    validateInt};

void setDefaultPrinterBaudRate(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = 19200;
}

struct SettingItem PrinterBaudRate = {
    "Printer baud rate",
    "printerbaud",
    &printerSettings.printerBaudRate,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultPrinterBaudRate,
    validateInt};

struct SettingItem printerEnabledSetting = {
    "Printer enabled",
    "printerMessagesactive",
    &printerSettings.printerEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

struct SettingItem *printerSettingItemPointers[] =
    {
        &printerEnabledSetting,
        &PrinterBaudRate,
        &PrinterDataPinNo,
};

struct SettingItemCollection printerMessagesSettingItems = {
    "printerSettings",
    "Printer setup",
    printerSettingItemPointers,
    sizeof(printerSettingItemPointers) / sizeof(struct SettingItem *)};

void printerMessagesOff()
{
    printerSettings.printerEnabled = false;
    saveSettings();
}

void printerMessagesOn()
{
    printerSettings.printerEnabled = true;
    saveSettings();
}

void printMessage(char *messageText, const char *option)
{
    if (option != NULL)
    {
        if (strcasecmp(option, "sameline") == 0)
        {
            snprintf(PRINTERMessageBuffer, PRINTERMESSAGE_LENGTH, messageText);
        }
        else
        {
            snprintf(PRINTERMessageBuffer, PRINTERMESSAGE_LENGTH, "%s\n", messageText);
        }
    }
    else {
        snprintf(PRINTERMessageBuffer, PRINTERMESSAGE_LENGTH, "%s\n", messageText);
    }
    Serial1.print(PRINTERMessageBuffer);
    Serial.printf("Printing:%s\n", PRINTERMessageBuffer);
}

#define PRINTER_FLOAT_VALUE_OFFSET 0
#define PRINTER_MESSAGE_OFFSET (PRINTER_FLOAT_VALUE_OFFSET + sizeof(float))
#define PRINTER_OPTION_OFFSET (PRINTER_MESSAGE_OFFSET + PRINTERMESSAGE_LENGTH)
#define PRINTER_TAIL_TEXT_OFFSET (PRINTER_OPTION_OFFSET + PRINTERMESSAGE_COMMAND_LENGTH)

boolean validatePrinterOptionString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, PRINTERMESSAGE_COMMAND_LENGTH));
}

boolean validatePRINTERMessageString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, PRINTERMESSAGE_LENGTH));
}

struct CommandItem printerfloatValueItem = {
    "value",
    "value",
    PRINTER_FLOAT_VALUE_OFFSET,
    floatCommand,
    validateFloat,
    noDefaultAvailable};

struct CommandItem PrinterCommandOptionName = {
    "option",
    "printer message option",
    PRINTER_OPTION_OFFSET,
    textCommand,
    validatePrinterOptionString,
    setDefaultEmptyString};

struct CommandItem PrinterMessageText = {
    "text",
    "printer message text",
    PRINTER_MESSAGE_OFFSET,
    textCommand,
    validatePRINTERMessageString,
    noDefaultAvailable};

struct CommandItem PrinterTailText = {
    "tail",
    "printer tail text",
    PRINTER_TAIL_TEXT_OFFSET,
    textCommand,
    validatePRINTERMessageString,
    setDefaultEmptyString};

struct CommandItem *PrintTextCommandItems[] =
    {
        &PrinterMessageText,
        &PrinterCommandOptionName};

int doPrintText(char *destination, unsigned char *settingBase);

struct Command printMessageCommand
{
    "print",
        "Prints text",
        PrintTextCommandItems,
        sizeof(PrintTextCommandItems) / sizeof(struct CommandItem *),
        doPrintText
};

int doPrintText(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("printerMessages", &printMessageCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (printerProcess.status != PRINTER_OK)
    {
        return JSON_MESSAGE_PRINTER_NOT_ENABLED;
    }

    char *message = (char *)(settingBase + PRINTER_MESSAGE_OFFSET);

    const char *option = (const char *)(settingBase + PRINTER_OPTION_OFFSET);

    printMessage(message, option);

    return WORKED_OK;
}

struct CommandItem *PrintValueCommandItems[] =
    {
        &printerfloatValueItem,
        &PrinterMessageText,
        &PrinterCommandOptionName,
        &PrinterTailText};

int doPrintValue(char *destination, unsigned char *settingBase);

struct Command printValueCommand
{
    "showvalue",
        "show a value",
        PrintValueCommandItems,
        sizeof(PrintValueCommandItems) / sizeof(struct CommandItem *),
        doPrintValue
};

int doPrintValue(char *destination, unsigned char *settingBase)
{
    if (*destination != 0)
    {
        // we have a destination for the command. Build the string
        char buffer[JSON_BUFFER_SIZE];
        createJSONfromSettings("printerMessages", &printValueCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (printerProcess.status != PRINTER_OK)
    {
        return JSON_MESSAGE_PRINTER_NOT_ENABLED;
    }

    unsigned char *valuePtr = (settingBase + PRINTER_FLOAT_VALUE_OFFSET);

    float value = getUnalignedFloat(valuePtr);

    char *message = (char *)(settingBase + PRINTER_MESSAGE_OFFSET);

    char *tail = (char *)(settingBase + PRINTER_TAIL_TEXT_OFFSET);

    const char *option = (const char *)(settingBase + PRINTER_OPTION_OFFSET);

    char buffer[PRINTERMESSAGE_LENGTH];

    snprintf(buffer, PRINTERMESSAGE_LENGTH, "%s%.1f%s", message, value, tail);

    printMessage(buffer, option);

    return WORKED_OK;
}

struct Command *PRINTERCommandList[] = {
    &printMessageCommand,
    &printValueCommand};

struct CommandItemCollection PRINTERCommands =
    {
        "Control printer",
        PRINTERCommandList,
        sizeof(PRINTERCommandList) / sizeof(struct Command *)};

unsigned long PRINTERmillisAtLastScroll;

void initPrinter()
{
    // perform all the hardware startup before we start the WiFi running

    printerProcess.status = PRINTER_OFF;

    if (printerSettings.printerEnabled)
    {
#if defined(ARDUINO_ARCH_ESP8266)
        Serial1.begin(printerSettings.printerBaudRate);
#else
        // TODO - create a serial port assigned to the specified pin

#endif
    }
}

void startPrinter()
{
    // all the hardware is set up in the init function. We just display the default message here
    //
    printMessage("Printer starting", NULL);

    if (printerSettings.printerEnabled)
    {
        printerProcess.status = PRINTER_OK;
    }
}

void updatePrinter()
{
    if (printerProcess.status != PRINTER_OK)
    {
        return;
    }
}

void stopPrinter()
{
    printerProcess.status = PRINTER_OFF;
}

bool printerStatusOK()
{
    return printerProcess.status == PRINTER_OK;
}

void printerStatusMessage(char *buffer, int bufferLength)
{
    if (printerProcess.status == PRINTER_OFF)
    {
        snprintf(buffer, bufferLength, "Printer off");
    }
    else
    {
        snprintf(buffer, bufferLength, "Printer on");
    }
}

struct process printerProcess = {
    "printer",
    initPrinter,
    startPrinter,
    updatePrinter,
    stopPrinter,
    printerStatusOK,
    printerStatusMessage,
    false,
    0,
    0,
    NULL,
    (unsigned char *)&printerSettings, sizeof(PrinterSettings), &printerMessagesSettingItems,
    &PRINTERCommands,
    BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS + WIFI_CONFIG_PROCESS,
    NULL,
    NULL,
    NULL,
    NULL, // no command options
    0     // no command options
};
