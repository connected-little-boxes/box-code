#include "utils.h"

#include "settingsWebServer.h"
#include "settings.h"
#include "sensors.h"
#include "processes.h"

#define WEB_PAGE_BUFFER_SIZE 3000

char * webPageBuffer = NULL;

// not proud of this - but it will work

#if defined(ARDUINO_ARCH_ESP8266)
#define WebServer ESP8266WebServer
#endif

WebServer * webServer;

const char oldhomePageHeader[] =
"<html>"
"<head>"
//"<style>input {font-size: 1.2em; width: 100%; max-width: 360px; display: block; margin: 5px auto; } </style>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Connected Little Boxes</h1>"
"<h3>Version %s</h3>" // version  goes here;
"<h1>Settings</h1>";

const char oldhomePageFooter[] =
"<p> Select the link to the settings page that you want to edit.</p>"
"<p> Select the reset link below to reset the sensor when you have finished.</p>"
"<a href=""reset"">reset</a>"
"</body>"
"</html>";

const char homePageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<h1>Connected Little Boxes</h1>"
"<h2>%s</h2>" // configuration description goes here
"<form id='form' action='/%s' method='post'>"; // configuration short name goes here

const char homePageFooter[] =
"%s"// entire page goes here
"<input type='submit' value='Update'>"
"</form>"
"<p> Enter your settings and select Update to write them into the device.</p>"
"<p> Select the full settings page link below to view all the settings in the device.</p>"
"<a href=""full"">full settings</a>"
"<p> Select the reset link below to reset the device when you have finished.</p>"
"<a href=""reset"">reset</a>"
"</body>"
"</html>";

void addItem(SettingItemCollection * settings)
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <p style=\"margin-left: 20px; line-height: 50%%\"><a href=""%s"">%s</a> </p>\n",
		webPageBuffer,
		settings->collectionName,
		settings->collectionDescription);
}

extern struct SettingItem wifi1SSIDSetting;
extern struct SettingItem wifi1PWDSetting;
extern struct SettingItem mqttServerSetting;
extern struct SettingItem mqttPortSetting;
extern struct SettingItem mqttSecureSocketsSetting;
extern struct SettingItem mqttUserSetting;
extern struct SettingItem mqttPasswordSetting;


struct SettingItem * quickSettingPointers[] = {

		&wifi1SSIDSetting,
		&wifi1PWDSetting,
		&mqttServerSetting,
		&mqttPortSetting,
		&mqttSecureSocketsSetting,
		&mqttUserSetting,
		&mqttPasswordSetting
};

struct SettingItemCollection QuickSettingItems = {
	"Quick Settings",
	"WiFi and MQTT configuration",
	quickSettingPointers,
	sizeof(quickSettingPointers) / sizeof(struct SettingItem *)};

void buildCollectionSettingsPage(SettingItemCollection * settingCollection, const char * settingsPageHeader, const char * settingsPageFooter);

void buildHomePage()
{
	buildCollectionSettingsPage(&QuickSettingItems, homePageHeader, homePageFooter);
	Serial.println(webPageBuffer);
}

void buildFullSettingsHomePage()
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, homePageHeader, Version);

	iterateThroughProcessSettingCollections(addItem);

	iterateThroughSensorSettingCollections(addItem);

	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s %s",
		webPageBuffer, homePageFooter);

	Serial.println(webPageBuffer);
}

const char settingsPageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Settings</h1>"
"<h2>%s</h2>" // configuration description goes here
"<form id='form' action='/%s' method='post'>"; // configuration short name goes here

const char settingsPageFooter[] =
"%s"// entire page goes here
"<input type='submit' value='Update'>"
"</form>"
"</body>"
"</html>";

void buildCollectionSettingsPage(SettingItemCollection * settingCollection, const char * settingsPageHeader, const char * settingsPageFooter)
{
	int * valuePointer;
	float * floatPointer;
	double * doublePointer;
	boolean * boolPointer;
	uint32_t * loraIDValuePointer;
	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, settingsPageHeader, settingCollection->collectionDescription, settingCollection->collectionName);

	// Start the search at setting collection 0 so that the quick settings are used to build the web page
	for (int i = 0; i < settingCollection->noOfSettings; i++)
	{
		snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <label for='%s'> %s: </label>",
			webPageBuffer,
			settingCollection->settings[i]->formName,
			settingCollection->settings[i]->prompt);

		switch (settingCollection->settings[i]->settingType)
		{
		case text:
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%s' style=\"margin-left: 20px; line-height: 50%%\"><br>",
				webPageBuffer, settingCollection->settings[i]->formName, (char *)settingCollection->settings[i]->value);
			break;
		case password:
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'password' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, (char *)settingCollection->settings[i]->value);
			break;
		case integerValue:
			valuePointer = (int*)settingCollection->settings[i]->value;
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%d'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, *valuePointer);
			break;
		case floatValue:
			floatPointer = (float*)settingCollection->settings[i]->value;
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%f'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, *floatPointer);
			break;
		case doubleValue:
			doublePointer = (double*)settingCollection->settings[i]->value;
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%lf'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, *doublePointer);
			break;
		case yesNo:
			boolPointer = (boolean*)settingCollection->settings[i]->value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='yes'><br>",
					webPageBuffer, settingCollection->settings[i]->formName);
			}
			else
			{
				snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='no'><br>",
					webPageBuffer, settingCollection->settings[i]->formName);
			}
			break;

		case loraKey:
			dumpHexString(loraKeyBuffer, (uint8_t*)settingCollection->settings[i]->value, LORA_KEY_LENGTH);
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, loraKeyBuffer);
			break;

		case loraID:
			loraIDValuePointer = (uint32_t*)settingCollection->settings[i]->value;
			dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, loraKeyBuffer);
			break;

			}
		}

	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, settingsPageFooter, webPageBuffer);
}

const char replyPageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>%s</h1>"
"<h2>%s</h2>"; // configuration description goes here

const char replyPageFooter[] =
"%s"
"<p>Settings updated.</p>"
"<p><a href = ""/"">return to the settings home screen </a></p>"
"</body></html>";

void updateItem(SettingItemCollection* settings)
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <p><a href=""%s"">%s</a> </p>",
		webPageBuffer,
		settings->collectionName,
		settings->collectionDescription);
}

void updateSettings(WebServer *webServer, SettingItemCollection * settingCollection)
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, replyPageHeader, settingCollection->collectionDescription, settingCollection->collectionName);

	for (int i = 0; i < settingCollection->noOfSettings; i++)
	{
		String fName = String(settingCollection->settings[i]->formName);
		String argValue = webServer->arg(fName);

		if (!settingCollection->settings[i]->validateValue(
			settingCollection->settings[i]->value,
			argValue.c_str()))
		{
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <p>Invalid value %s for %s</p> ",
				webPageBuffer, argValue.c_str(), settingCollection->settings[i]->prompt);
		}
	}
	saveSettings();
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, replyPageFooter, webPageBuffer);
}

void serveHome(WebServer *webServer)
{
	// Serial.println("Serve request hit");

	if (webServer->args() == 0) {
		// Serial.println("Serving the home page");
		buildHomePage();
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
}

const char pageNotFoundMessage[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Sensor Configuration</h1>"
"<h2>Page not found</h2>"
"<p>Sorry about that.</p>"
"</body></html>";

const char sensorResetMessage[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Sensor Configuration</h1>"
"<h2>Reset</h2>"
"<p>Sensor will reset in a few seconds.</p>"
"</body></html>";


void pageNotFound(WebServer *webServer)
{
	String uriString = webServer->uri();

	const char * uriChars = uriString.c_str();
	const char * pageNameStart = uriChars + 1;

	Serial.printf("Not found hit:%s\n", uriChars);

	if(strcasecmp(pageNameStart, "reset")==0)
	{
		Serial.println("Resetting device");
		// reset the device
		webServer->sendHeader("Content-Length", String(strlen(sensorResetMessage)));
		webServer->send(200, "text/html", sensorResetMessage);
		delay(5000);
		ESP.restart();
	}

	if(strcasecmp(pageNameStart, "full")==0)
	{
		buildFullSettingsHomePage();
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}

	SettingItemCollection * items = NULL;

	if(strcasecmp("Quick\%20Settings",pageNameStart)==0)
	{
		items = &QuickSettingItems;
	}
	else 
	{
		items = findSettingItemCollectionByName(pageNameStart);
	}

	if (items != NULL)
	{
		// url refers to an existing set of settings
		if (webServer->args() == 0)
		{
			// Not a post - just serve out the settings form
			//Serial.printf("Got settings request for %s\n", items->collectionName);
			buildCollectionSettingsPage(items,settingsPageHeader,settingsPageFooter);
		}
		else
		{
			// Posting new values - now we need to read them
			//Serial.printf("Got new data for %s\n", items->collectionName);
			updateSettings(webServer, items);
		}
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
	else
	{
		//Serial.printf("Page not found: %s\n", uriString.c_str());

		webServer->sendHeader("Content-Length", String(strlen(pageNotFoundMessage)));
		webServer->send(200, "text/html", pageNotFoundMessage);
	}
}

void initWebServer()
{
	WebServerProcess.status = WEBSERVER_OFF;
}

void startWebServer()
{
	WebServerProcess.status = WEBSERVER_READY;
}

bool startHostingConfigWebsite()
{
	Serial.printf("Starting host %d\n", WebServerProcess.status);

	if(WebServerProcess.status == WEBSERVER_OFF)
	{
		return false;
	}

	if(WebServerProcess.status == WEBSERVER_HOSTING)
	{
		return true;
	}

	Serial.println("Starting web server");

	if(webPageBuffer==NULL)
	{
		webPageBuffer = new char [WEB_PAGE_BUFFER_SIZE+1];
	}

	buildHomePage();

	webServer = new WebServer(80);

	webServer->on("/", std::bind(serveHome, webServer));
	webServer->onNotFound(std::bind(pageNotFound, webServer));
	webServer->begin();
	WebServerProcess.status = WEBSERVER_HOSTING;

	return true;
}

void updateWebServer()
{
	if (WebServerProcess.status == WEBSERVER_HOSTING)
	{
		webServer->handleClient();
	}

}

void stopWebserver()
{
	WebServerProcess.status = WEBSERVER_OFF;
}

bool settingsWebServerStatusOK()
{
    return (WebServerProcess.status == WEBSERVER_READY) || 
	(WebServerProcess.status == WEBSERVER_HOSTING);
}

void webserverStatusMessage(char * buffer, int bufferLength)
{
	switch (WebServerProcess.status)
	{
	case WEBSERVER_READY:
		snprintf(buffer, bufferLength, "Web server Ready");
		break;
	case WEBSERVER_OFF:
		snprintf(buffer, bufferLength, "Web server OFF");
		break;
	case WEBSERVER_HOSTING:
		snprintf(buffer, bufferLength, "Web server hosting site");
		break;
	default:
		snprintf(buffer, bufferLength, "Web server status invalid");
		break;
	}
}

struct process WebServerProcess = { 
	"Settingswebserver", 
	initWebServer,
	startWebServer, 
	updateWebServer, 
	stopWebserver, 
	settingsWebServerStatusOK,
	webserverStatusMessage, 
	false, 
	0, 
	0, 
	0,
	NULL,
	NULL, 0, NULL, // no settings
	NULL, // no commands
	BOOT_PROCESS + ACTIVE_PROCESS + CONFIG_PROCESS+WIFI_CONFIG_PROCESS,
	NULL,
	NULL,
	NULL
	};// don't start the web server by default



