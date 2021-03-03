#pragma once

#include "debug.h"
#include "utils.h"
#include "processes.h"
#include "settings.h"
#include "messages.h"

#define CONFIG_ACCESS_POINT_SSID "CLB"

#define WIFI_CONNECT_TIMEOUT_MILLIS 20000

#define WIFI_OK 600
#define WIFI_TURNED_OFF 601
#define WIFI_SCANNING 602
#define WIFI_CONNECTING 603
#define WIFI_ERROR_NO_NETWORKS_FOUND 604
#define WIFI_ERROR_CONNECT_TIMEOUT 605
#define WIFI_ERROR_CONNECT_FAILED 606
#define WIFI_ERROR_NO_MATCHING_NETWORKS 607
#define WIFI_ERROR_DISCONNECTED 608
#define WIFI_ERROR_SCAN_TIMEOUT 609
#define WIFI_RECONNECT_TIMER 610
#define WIFI_CONFIG_HOSTING_WEBSITE 611
#define WIFI_CONFIG_STARTING_WEBSITE 612
#define WIFI_CONFIG_STARTING_AP 613

#define WIFI_MAX_NO_OF_FAILED_SCANS 2

#define WIFI_SCAN_RETRY_MILLIS 5000
#define WIFI_CONNECT_RETRY_MILLS 5000
#define WIFI_SCAN_TIMEOUT_MILLIS 5000
#define WIFI_NO_OF_CONNECT_ATTEMPTS 3
#define WIFI_MESSAGE_BUFFER_SIZE 120

#define WIFI_STATUS_OK_MESSAGE_NUMBER 1
#define WIFI_STATUS_OK_MESSAGE_TEXT "WiFi connected OK"

#define WIFI_STATUS_NO_MATCHING_NETWORKS_MESSAGE_NUMBER 12
#define WIFI_STATUS_NO_MATCHING_NETWORKS_MESSAGE_TEXT "No networks found that match stored network names"

#define WIFI_STATUS_CONNECT_FAILED_MESSAGE_NUMBER 13
#define WIFI_STATUS_CONNECT_FAILED_MESSAGE_TEXT "No networks found that match stored network names"

#define WIFI_STATUS_CONNECT_ABANDONED_MESSAGE_NUMBER 14
#define WIFI_STATUS_CONNECT_ABANDONED_MESSAGE_TEXT "Wifi connection abandoned"

struct WifiConnectionSettings
{
	boolean wiFiOn;
	char wifi1SSID[WIFI_SSID_LENGTH];
	char wifi1PWD[WIFI_PASSWORD_LENGTH];

	char wifi2SSID[WIFI_SSID_LENGTH];
	char wifi2PWD[WIFI_PASSWORD_LENGTH];

	char wifi3SSID[WIFI_SSID_LENGTH];
	char wifi3PWD[WIFI_PASSWORD_LENGTH];

	char wifi4SSID[WIFI_SSID_LENGTH];
	char wifi4PWD[WIFI_PASSWORD_LENGTH];

	char wifi5SSID[WIFI_SSID_LENGTH];
	char wifi5PWD[WIFI_PASSWORD_LENGTH];
};

struct WiFiSetting
{
	char * wifiSsid;
	char * wifiPassword;
};

extern struct WifiConnectionSettings wifiConnectionSettings;

extern struct SettingItemCollection wifiConnectionSettingItems;

enum WiFiConnectionState { WiFiOff, WiFiStarting, WiFiScanning, WiFiConnecting, WiFiConnected, ShowingWifiConnected, WiFiConnectFailed, ShowingWiFiFailed, WiFiNotConnected };

extern WiFiConnectionState wifiState;

extern struct process WiFiProcessDescriptor;

void startWiFiConfigAP();

