#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "printer.h"
#include "processes.h"
#include "mqtt.h"
#include "errors.h"
#include "clock.h"

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
    "printeron",
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

void printMessage(char *messageText, char *option)
{
    TRACE("Printing message:");
    TRACE(messageText);
    TRACE(" with option ");
    TRACELN(option);

    if (option != NULL)
    {
        TRACELN("  Got options");

        if (strContains(option, "datestamp"))
        {
            TRACELN("    Got datestamp");
            if(getDateAndTime(PRINTERMessageBuffer,PRINTERMESSAGE_LENGTH ))
            {
                TRACELN("      printing datestamp");
                Serial1.println(PRINTERMessageBuffer);
            }
        }

        if (strContains(option, "sameline"))
        {
            TRACELN("    Got sameline");
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
    TRACELN("Print complete");
    Serial1.print(PRINTERMessageBuffer);
}

#define PRINTER_FLOAT_VALUE_OFFSET 0
#define PRINTER_MESSAGE_OFFSET (PRINTER_FLOAT_VALUE_OFFSET + sizeof(float))
#define PRINTER_OPTION_OFFSET (PRINTER_MESSAGE_OFFSET + PRINTERMESSAGE_LENGTH)
#define PRINTER_PRE_TEXT_OFFSET (PRINTER_OPTION_OFFSET + PRINTERMESSAGE_COMMAND_LENGTH)
#define PRINTER_POST_TEXT_OFFSET (PRINTER_PRE_TEXT_OFFSET + PRINTERMESSAGE_COMMAND_LENGTH)

boolean validatePrinterOptionString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, PRINTERMESSAGE_COMMAND_LENGTH));
}

boolean validatePRINTERMessageString(void *dest, const char *newValueStr)
{
    return (validateString((char *)dest, newValueStr, PRINTERMESSAGE_LENGTH));
}

struct CommandItem PrinterCommandOptionName = {
    "options",
    "printer options(sameline,datestamp)",
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

struct CommandItem printerPreText = {
    "pre",
    "printer pre text",
    PRINTER_PRE_TEXT_OFFSET,
    textCommand,
    validatePRINTERMessageString,
    setDefaultEmptyString};

struct CommandItem printerPostText = {
    "post",
    "printer post text",
    PRINTER_POST_TEXT_OFFSET,
    textCommand,
    validatePRINTERMessageString,
    setDefaultEmptyString};


struct CommandItem *PrintTextCommandItems[] =
    {
        &PrinterMessageText,
        &PrinterCommandOptionName,
        &printerPreText,
        &printerPostText};

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
        createJSONfromSettings("printer", &printMessageCommand, destination, settingBase, buffer, JSON_BUFFER_SIZE);
        return publishBufferToMQTTTopic(buffer, destination);
    }

    if (printerProcess.status != PRINTER_OK)
    {
        return JSON_MESSAGE_PRINTER_NOT_ENABLED;
    }

    char *message = (char *)(settingBase + PRINTER_MESSAGE_OFFSET);
    char *post = (char *)(settingBase +PRINTER_POST_TEXT_OFFSET);
    char *pre = (char *)(settingBase + PRINTER_PRE_TEXT_OFFSET);
    char *options = (char *)(settingBase + PRINTER_OPTION_OFFSET);
    char buffer[MAX_MESSAGE_LENGTH];

    snprintf(buffer, MAX_MESSAGE_LENGTH, "%s%s%s", pre, message, post);
    printMessage(buffer, options);

    TRACE("Printing: ");
    TRACELN(buffer);

    return WORKED_OK;
}

struct Command *PRINTERCommandList[] = {
    &printMessageCommand
    };

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

    if (printerSettings.printerEnabled)
    {
        char buffer[100];
        snprintf(buffer,100,"\n\nPrinter starting on:%s\n\n\n", settings.name);
        printerProcess.status = PRINTER_OK;
        printMessage(buffer, NULL);
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
