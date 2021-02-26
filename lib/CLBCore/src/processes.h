#pragma once

#include <Arduino.h>
#include "sensors.h"
#include "settings.h"
#include "controller.h"

#define BOOT_PROCESS 1
#define ACTIVE_PROCESS 2
#define CONFIG_PROCESS 4
#define WIFI_CONFIG_PROCESS 8
#define FIRST_SHUTDOWN_PROCESS 16
#define SECOND_SHUTDOWN_PROCESS 32

// not used at the moment - will allow us to add behaviours to processes

struct processMessageListener{
	char * listenerName;
	char * destination;
	void (*processMessage)(processMessageListener * listener, void * value);
	struct processMessageListener * nextMessageListener;
};

struct process
{
	char *processName;
	void (*initProcess)();
	void (*startProcess)();
	void (*udpateProcess)();
	void (*stopProcess)();
	bool (*statusOK)();
	void (*getStatusMessage)(char *buffer, int bufferLength);
	boolean beingUpdated; // true if the process is running
	int status;			  // zero means OK - any other value is an error state
	unsigned long activeTime;
	unsigned long totalTime;
	void *processDetails;
	unsigned char *settingsStoreBase;
	int settingsStoreLength;
	SettingItemCollection *settingItems;
	struct CommandItemCollection *commands;
	int processStartMask;
	process *nextActiveProcess;
	process *nextAllProcesses;
	processMessageListener * listeners;
	unsigned char * commandItems;
	int commandItemSize;
};

void addProcessToAllProcessList(struct process *newProcess);
void addProcessToActiveProcessList(struct process *newProcess);
void buildActiveProcessListFromMask(int processMask);
struct process *findProcessByName(const char *name);
struct process *findActiveProcessByName(const char *name);
struct process *findProcessSettingCollectionByName(const char *name);
void initialiseAllProcesses();
void startProcess(process *proc);
struct process *startProcessByName(char *name);
void startProcesses();
void updateProcesses();
void dumpProcessStatus();
void updateProcess(struct process *process);
void iterateThroughAllProcesses(void (*func)(process *p));
void iterateThroughActiveProcesses(void (*func)(process *p));
void stopProcesses();
void iterateThroughProcessSettings(void (*func) (unsigned char * settings, int size));
void iterateThroughProcessSettingCollections(void (*func)(SettingItemCollection *s));
void iterateThroughProcessSettings(void (*func)(SettingItem *s));
void resetProcessesToDefaultSettings();
void iterateThroughProcessCommandCollections(void (*func)(CommandItemCollection *c));
void iterateThroughProcessCommands(void (*func)(Command *c));
Command * FindCommandInProcess(process * procPtr, const char *commandName);

Command *FindCommandByName(const char * processName, const char *name);

SettingItem *FindProcesSettingByFormName(const char *settingName);
