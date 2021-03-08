#pragma once

#define MESSAGES_OK 400
#define MESSAGES_STOPPED 401

struct MessagesSettings {
	bool messagesEnabled;
};

enum ledFlashBehaviour { ledFlashOn, ledFlashNormalState, 
	ledFlashConfigState, ledFlashAlertState};

void displayMessage(int messageNumber, ledFlashBehaviour flashBehaviour, char * messageText);

void ledFlashBehaviourToString(ledFlashBehaviour severity, char * dest, int length);

bool bindMessageHandler(void(*newHandler)(int messageNumber, ledFlashBehaviour severity, char* messageText));

void messagesOff();

void messagesOn();

void messagesStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

extern struct MessagesSettings messagesSettings;

extern struct SettingItemCollection messagesSettingItems;

extern struct process messagesProcess;
