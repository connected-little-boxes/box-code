#include "utils.h"
#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "connectwifi.h"
#include "settingsWebServer.h"
#include "boot.h"
#include "pixels.h"

boolean validateWifiSSID(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, WIFI_SSID_LENGTH));
}

boolean validateWifiPWD(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, WIFI_PASSWORD_LENGTH));
}

struct WifiConnectionSettings wifiConnectionSettings;

struct SettingItem wifiOnOff = {
	"Wifi on", "wifiactive", &wifiConnectionSettings.wiFiOn, YESNO_INPUT_LENGTH, yesNo, setTrue, validateYesNo};

void setDefaultWiFi1SSID(void *dest)
{
	strcpy((char *)dest, DEFAULT_WIFI1_SSID);
}

void setDefaultWiFi1Pwd(void *dest)
{
	strcpy((char *)dest, DEFAULT_WIFI1_PWD);
}

struct SettingItem wifi1SSIDSetting = {
	"WiFiSSID1", "wifissid1", wifiConnectionSettings.wifi1SSID, WIFI_SSID_LENGTH, text, setDefaultWiFi1SSID, validateWifiSSID};
struct SettingItem wifi1PWDSetting = {
	"WiFiPassword1", "wifipwd1", wifiConnectionSettings.wifi1PWD, WIFI_PASSWORD_LENGTH, password, setDefaultWiFi1Pwd, validateWifiPWD};

struct SettingItem wifi2SSIDSetting = {
	"WiFiSSID2", "wifissid2", wifiConnectionSettings.wifi2SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID};
struct SettingItem wifi2PWDSetting = {
	"WiFiPassword2", "wifipwd2", wifiConnectionSettings.wifi2PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD};

struct SettingItem wifi3SSIDSetting = {
	"WiFiSSID3", "wifissid3", wifiConnectionSettings.wifi3SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID};
struct SettingItem wifi3PWDSetting = {
	"WiFiPassword3", "wifipwd3", wifiConnectionSettings.wifi3PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD};

struct SettingItem wifi4SSIDSetting = {
	"WiFiSSID4", "wifissid4", wifiConnectionSettings.wifi4SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID};
struct SettingItem wifi4PWDSetting = {
	"WiFiPassword4", "wifipwd4", wifiConnectionSettings.wifi4PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD};

struct SettingItem wifi5SSIDSetting = {
	"WiFiSSID5", "wifissid5", wifiConnectionSettings.wifi5SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID};
struct SettingItem wifi5PWDSetting = {
	"WiFiPassword5", "wifipwd5", wifiConnectionSettings.wifi5PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD};

struct SettingItem *wifiConnectionSettingItemPointers[] =
	{
		&wifiOnOff,
		&wifi1SSIDSetting,
		&wifi1PWDSetting,

		&wifi2SSIDSetting,
		&wifi2PWDSetting,

		&wifi3SSIDSetting,
		&wifi3PWDSetting,

		&wifi4SSIDSetting,
		&wifi4PWDSetting,

		&wifi5SSIDSetting,
		&wifi5PWDSetting};

struct SettingItemCollection wifiConnectionSettingItems = {
	"WiFi",
	"WiFi ssids and passwords",
	wifiConnectionSettingItemPointers,
	sizeof(wifiConnectionSettingItemPointers) / sizeof(struct SettingItem *)};

// Note that if we add new WiFi passwords into the settings
// we will have to update this table.

struct WiFiSetting wifiSettings[] =
	{
		{wifiConnectionSettings.wifi1SSID, wifiConnectionSettings.wifi1PWD},
		{wifiConnectionSettings.wifi2SSID, wifiConnectionSettings.wifi2PWD},
		{wifiConnectionSettings.wifi3SSID, wifiConnectionSettings.wifi3PWD},
		{wifiConnectionSettings.wifi4SSID, wifiConnectionSettings.wifi4PWD},
		{wifiConnectionSettings.wifi5SSID, wifiConnectionSettings.wifi5PWD}};

bool wifiConfigurationsEmpty()
{
	for (unsigned int i = 0; i < sizeof(wifiSettings) / sizeof(struct WiFiSetting); i++)
	{
		if (wifiSettings[i].wifiSsid[0] != 0)
		{
			return false;
		}
	}
	return true;
}

#define WIFI_SETTING_NOT_FOUND -1

int findWifiSetting(String ssidName)
{
	char ssidBuffer[WIFI_SSID_LENGTH];

	ssidName.toCharArray(ssidBuffer, WIFI_SSID_LENGTH);

	for (unsigned int i = 0; i < sizeof(wifiSettings) / sizeof(struct WiFiSetting); i++)
	{
		if (strcasecmp(wifiSettings[i].wifiSsid, ssidBuffer) == 0)
		{
			return i;
		}
	}
	return WIFI_SETTING_NOT_FOUND;
}

char wifiActiveAPName[WIFI_SSID_LENGTH];
int wifiError;

boolean firstRun = true;
unsigned long WiFiTimerStart;

int wifiConnectAttempts = 0;

void beginWiFiScanning()
{
	if (wifiConnectionSettings.wiFiOn == false)
	{
		WiFiProcessDescriptor.status = WIFI_TURNED_OFF;
		return;
	}

	WiFiTimerStart = millis();

	if (firstRun)
	{
		WiFi.mode(WIFI_OFF);
		delay(100);
		WiFi.mode(WIFI_STA);
		delay(100);
		firstRun = false;
		wifiConnectAttempts = 0;
	}

	WiFi.scanNetworks(true);
	WiFiProcessDescriptor.status = WIFI_SCANNING;
}

void startReconnectTimer()
{
	WiFiTimerStart = millis();
	WiFiProcessDescriptor.status = WIFI_RECONNECT_TIMER;
}

void updateReconnectTimer()
{
	if (ulongDiff(millis(), WiFiTimerStart) > WIFI_CONNECT_RETRY_MILLS)
	{
		beginWiFiScanning();
	}
}

void handleConnectFailure()
{
	if(bootMode==WARM_BOOT_MODE)
	{
		// if we have already rebooted once because of a connection failure 
		// we don't do it again. 
		return;
	}

	wifiConnectAttempts++;

	if (wifiConnectAttempts == WIFI_MAX_NO_OF_FAILED_SCANS)
	{
		TRACE("Reset due to WiFi lockup");
		internalReboot(WARM_BOOT_MODE);
	}
}

void handleFailedWiFiScan()
{
	TRACELN("No networks found that match stored network names");
	handleConnectFailure();
	displayMessage(WIFI_STATUS_NO_MATCHING_NETWORKS_MESSAGE_NUMBER, ledFlashAlertState, WIFI_STATUS_NO_MATCHING_NETWORKS_MESSAGE_TEXT);
	startReconnectTimer();
}

void checkWiFiScanResult()
{
	int noOfNetworks = WiFi.scanComplete();

	if (noOfNetworks == WIFI_SCAN_RUNNING)
	{
		if (ulongDiff(millis(), WiFiTimerStart) > WIFI_SCAN_TIMEOUT_MILLIS)
		{
			WiFiProcessDescriptor.status = WIFI_ERROR_SCAN_TIMEOUT;
			TRACELN("WiFi scan timeout");
		}
		return;
	}

	TRACE("Networks found: ");
	TRACELN(noOfNetworks);

	int settingNumber;

	// if we get here we have some networks
	for (int i = 0; i < noOfNetworks; ++i)
	{
		settingNumber = findWifiSetting(WiFi.SSID(i));

		if (settingNumber != WIFI_SETTING_NOT_FOUND)
		{
			snprintf(wifiActiveAPName, WIFI_SSID_LENGTH, "%s", wifiSettings[settingNumber].wifiSsid);
			TRACE("Connecting to ");
			TRACELN(wifiActiveAPName);
			WiFi.begin(wifiSettings[settingNumber].wifiSsid,
					   wifiSettings[settingNumber].wifiPassword);
			WiFiTimerStart = millis();
			WiFiProcessDescriptor.status = WIFI_CONNECTING;
			return;
		}
	}

	// if we get here we didn't find a matching network

	handleFailedWiFiScan();
}

void checkWiFiConnectResult()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		if (ulongDiff(millis(), WiFiTimerStart) > WIFI_CONNECT_TIMEOUT_MILLIS)
		{
			WiFiProcessDescriptor.status = WIFI_ERROR_CONNECT_TIMEOUT;
		}
		return;
	}

	// if we get here we are connected
	TRACELN("Deleting scan");
	WiFi.scanDelete();

	wifiError = WiFi.status();

	if (wifiError == WL_CONNECTED)
	{
		TRACELN("Wifi OK");
		WiFiProcessDescriptor.status = WIFI_OK;
		char messageBuffer[WIFI_MESSAGE_BUFFER_SIZE];
		snprintf(messageBuffer, WIFI_MESSAGE_BUFFER_SIZE, "%s %s", WIFI_STATUS_OK_MESSAGE_TEXT, WiFi.localIP().toString().c_str());
		displayMessage(WIFI_STATUS_OK_MESSAGE_NUMBER, ledFlashNormalState, messageBuffer);
		wifiConnectAttempts = 0;
		WiFiProcessDescriptor.status = WIFI_OK;
		return;
	}

	TRACE("Fail status:");
	TRACE_HEXLN(wifiError);
	displayMessage(WIFI_STATUS_CONNECT_FAILED_MESSAGE_NUMBER, ledFlashAlertState, WIFI_STATUS_CONNECT_FAILED_MESSAGE_TEXT);
	startReconnectTimer();
}

void displayWiFiStatus(int status)
{
	switch (status)
	{
	case WL_IDLE_STATUS:
		TRACE("Idle");
		break;
	case WL_NO_SSID_AVAIL:
		TRACE("No SSID");
		break;
	case WL_SCAN_COMPLETED:
		TRACE("Scan completed");
		break;
	case WL_CONNECTED:
		TRACE("Connected");
		break;
	case WL_CONNECT_FAILED:
		TRACE("Connect failed");
		break;
	case WL_CONNECTION_LOST:
		TRACE("Connection lost");
		break;
	case WL_DISCONNECTED:
		TRACE("Disconnected");
		break;
	default:
		TRACE("WiFi status value: ");
		TRACE(status);
		break;
	}
}

void checkWiFIOK()
{
	int wifiStatusValue = WiFi.status();

	if (wifiStatusValue != WL_CONNECTED)
	{
		startReconnectTimer();
	}
}

void stopWiFi()
{
	TRACELN("WiFi turned off");

	WiFiProcessDescriptor.status = WIFI_TURNED_OFF;
	WiFi.mode(WIFI_OFF);
	delay(500);
}

void startWiFiConfigAP()
{
	Serial.printf("Starting config access point at %s: ", CONFIG_ACCESS_POINT_SSID);


	WiFi.mode(WIFI_AP);

	delay(100);

	WiFi.softAP(CONFIG_ACCESS_POINT_SSID);

	delay(500);

	WiFiProcessDescriptor.status = WIFI_CONFIG_STARTING_AP;
}

void startWiFiConfigWebsite()
{
	if (startHostingConfigWebsite())
	{
		beginStatusDisplay();
		displayMessage(WIFI_STATUS_HOSTING_AP_MESSAGE_NUMBER, ledFlashConfigState, WIFI_STATUS_HOSTING_AP_MESSAGE_TEXT);
		Serial.printf("   Hosting at 192.168.4.1 on %s\n", CONFIG_ACCESS_POINT_SSID);
		WiFiProcessDescriptor.status = WIFI_CONFIG_HOSTING_WEBSITE;
	}
	else
	{
		stopWiFi();
	}
}

void initWifi()
{
	WiFiProcessDescriptor.status = WIFI_TURNED_OFF;
}

void startWifi()
{
	if (wifiConnectionSettings.wiFiOn)
	{
		if((bootMode == CONFIG_BOOT_MODE) || (bootMode == CONFIG_BOOT_NO_TIMEOUT_MODE))
		{
			// we need to get some configuration
			// changes state to WIFI_CONFIG_STARTING_AP
			startWiFiConfigAP();
		}
		else
		{
			// can start scanning for existing sites
			// changes state to WIFI_SCANNING
			beginWiFiScanning();
		}
	}
	else
	{
		WiFiProcessDescriptor.status = WIFI_TURNED_OFF;
		WiFi.mode(WIFI_OFF);
	}
}

void checkWiFiTurnedOff()
{
	if (wifiConnectionSettings.wiFiOn)
	{
		beginWiFiScanning();
	}
}

void updateWifiHostingConfigWebsite()
{
}

void updateNoMatchingNetworks()
{
}

void updateWifi()
{
	switch (WiFiProcessDescriptor.status)
	{
	case WIFI_OK:
		checkWiFIOK();
		break;

	case WIFI_TURNED_OFF:
		checkWiFiTurnedOff();
		break;

	case WIFI_SCANNING:
		checkWiFiScanResult();
		break;

	case WIFI_CONNECTING:
		checkWiFiConnectResult();
		break;

	case WIFI_RECONNECT_TIMER:
		updateReconnectTimer();
		break;

	case WIFI_CONFIG_STARTING_AP:
		startWiFiConfigWebsite();
		break;

	case WIFI_CONFIG_HOSTING_WEBSITE:
		updateWifiHostingConfigWebsite();
		break;
	}
}

bool connectWiFiStatusOK()
{
	return WiFiProcessDescriptor.status == WIFI_OK;
}

void wifiStatusMessage(char *buffer, int bufferLength)
{
	switch (WiFiProcessDescriptor.status)
	{
	case WIFI_OK:
		snprintf(buffer, bufferLength, "%s: %s", wifiActiveAPName, WiFi.localIP().toString().c_str());
		break;
	case WIFI_TURNED_OFF:
		snprintf(buffer, bufferLength, "Wifi OFF");
		break;
	case WIFI_SCANNING:
		snprintf(buffer, bufferLength, "Scanning for networks");
		break;
	case WIFI_ERROR_NO_NETWORKS_FOUND:
		snprintf(buffer, bufferLength, "No networks found");
		break;
	case WIFI_CONNECTING:
		snprintf(buffer, bufferLength, "Connecting to %s", wifiActiveAPName);
		break;
	case WIFI_RECONNECT_TIMER:
		snprintf(buffer, bufferLength, "WiFi connection waiting to retry");
		break;
	case WIFI_CONFIG_STARTING_AP:
		snprintf(buffer, bufferLength, "WiFi connection starting AP");
		break;
	case WIFI_CONFIG_HOSTING_WEBSITE:
		snprintf(buffer, bufferLength, "WiFi connection hosting AP");
		break;
	default:
		snprintf(buffer, bufferLength, "WiFi status invalid");
		break;
	}
}

struct process WiFiProcessDescriptor = {
	"WiFi",
	initWifi,
	startWifi,
	updateWifi,
	stopWiFi,
	connectWiFiStatusOK,
	wifiStatusMessage,
	false,
	0,
	0,
	0,
	NULL,
	(unsigned char *)&wifiConnectionSettings, sizeof(WifiConnectionSettings), &wifiConnectionSettingItems,
	NULL,
	BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS,
	NULL,
	NULL,
	NULL,
	NULL, // no command options
	0	  // no command options
};
