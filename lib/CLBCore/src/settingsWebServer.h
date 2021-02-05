#pragma once

#include "settings.h"
#include "processes.h"
#include "connectwifi.h"

#define WEBSERVER_HOSTING 1100
#define WEBSERVER_OFF 1101
#define WEBSERVER_READY 1002

bool startHostingConfigWebsite();

extern struct process WebServerProcessDescriptor;

void webserverStatusMessage(char * buffer, int bufferLength);
