#pragma once

extern struct process bootProcessDescriptor;

struct BootSettings 
{
    int accessPointTimeoutSecs;
};

extern struct BootSettings bootSettings;

void getBootReasonMessage(char *buffer, int bufferlength);

int getRestartCode();

int getInternalBootCode();
void setInternalBootCode(int value);
bool isSoftwareReset();

#define DEVICE_BOOT_MODE 1
#define CONFIG_BOOT_MODE 2
#define WARM_BOOT_MODE 3
#define CONFIG_BOOT_NO_TIMEOUT_MODE 4


#define BOOT_REASON_MESSAGE_SIZE 120 

#define BOOT_CONFIG_AP_DURATION_MIN_SECS 10
#define BOOT_CONFIG_AP_DURATION_MAX_SECS 6000
#define BOOT_CONFIG_AP_DURATION_DEFAULT_SECS 600

extern char bootReasonMessage [BOOT_REASON_MESSAGE_SIZE];

extern unsigned char bootMode;

void startBootManager();

void updateBootManager();

void internalReboot(unsigned char rebootCode);