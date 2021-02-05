#pragma once

#define MESSAGES_OK 400
#define MESSAGES_STOPPED 401

struct MessagesSettings {
	bool messagesEnabled;
};

enum MessageLevel { messageSeverityStarting, messageSeverityOk, 
	messageSeverityWarning, messageSeverityAlert};

void displayMessage(int messageNumber, MessageLevel severity, char * messageText);

void messageSeverityToString(MessageLevel severity, char * dest, int length);
int messageSeverityToFlashLengthMillis(MessageLevel severity);

bool bindMessageHandler(void(*newHandler)(int messageNumber, MessageLevel severity, char* messageText));

void messagesOff();

void messagesOn();

void messagesStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

extern struct MessagesSettings messagesSettings;

extern struct SettingItemCollection messagesSettingItems;

extern struct process messagesProcess;
