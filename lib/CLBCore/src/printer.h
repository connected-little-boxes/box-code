#pragma once

#define PRINTER_OK 1300
#define PRINTER_OFF 1301

#define PRINTERMESSAGE_LENGTH 60
#define PRINTERMESSAGE_COMMAND_LENGTH 10

struct PrinterSettings
{
    bool printerEnabled;
    int printerBaudRate;
    int dataPin;
};

void printMessage(char *messageText);

void printerprinterMessagesOff();

void printerprinterMessagesOn();

extern struct PrinterSettings printerSettings;

extern struct SettingItemCollection printerSettingItems;

extern struct process printerProcess;
