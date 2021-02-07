#include "utils.h"
#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "apconfig.h"

const byte DNS_PORT = 53;
IPAddress apIP(8, 8, 4, 4); // The default android DNS
DNSServer dnsServer;

bool settingsApConfigStatusOK()
{
	return ApConfigProcess.status != APCONFIG_OFF;
}

void initApConfig()
{
	ApConfigProcess.status = APCONFIG_OFF;
}

void startApConfig()
{
	WiFi.mode(WIFI_AP);
	WiFi.softAP(settings.deviceName);
	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

	// if DNSServer is started with "*" for domain name, it will reply with
	// provided IP to all DNS request
	dnsServer.start(DNS_PORT, "*", apIP);

	ApConfigProcess.status = APCONFIG_HOSTING;
}

void updateApConfig()
{
	if (ApConfigProcess.status == APCONFIG_HOSTING)
	{
		dnsServer.processNextRequest();
	}
}

void stopApConfig()
{
	ApConfigProcess.status = APCONFIG_OFF;
}

void ApConfigStatusMessage(char *buffer, int bufferLength)
{
	switch (ApConfigProcess.status)
	{
	case APCONFIG_READY:
		snprintf(buffer, bufferLength, "Access point ready");
		break;
	case APCONFIG_OFF:
		snprintf(buffer, bufferLength, "Access point off");
		break;
	case APCONFIG_HOSTING:
		snprintf(buffer, bufferLength, "Access point active");
		break;
	default:
		snprintf(buffer, bufferLength, "Web server status invalid");
		break;
	}
}

struct process ApConfigProcess = {
	"SettingsApConfig",
	initApConfig,
	startApConfig,
	updateApConfig,
	stopApConfig,
	settingsApConfigStatusOK,
	ApConfigStatusMessage,
	false,
	0,
	0,
	NULL,
	NULL, 0, NULL, // no settings
	NULL,		   // no commands
	WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL}; // don't start the web server by default
