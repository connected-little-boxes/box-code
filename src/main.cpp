#include <Arduino.h>

#include "settings.h"
#include "pixels.h"
#include "processes.h"
#include "sensors.h"
#include "pirSensor.h"
#include "buttonsensor.h"
#include "inputswitch.h"
#include "commands.h"
#include "statusled.h"
#include "messages.h"
#include "console.h"
#include "connectwifi.h"
#include "mqtt.h"
#include "servoproc.h"
#include "clock.h"
#include "registration.h"
#include "rotarySensor.h"
#include "potSensor.h"
#include "settingsWebServer.h"
#include "MAX7219Messages.h"
#include "printer.h"

// This function will be different for each build of the device.

void populateProcessList()
{
	addProcessToAllProcessList(&pixelProcess);
	addProcessToAllProcessList(&statusLedProcess);
	addProcessToAllProcessList(&inputSwitchProcess);
	addProcessToAllProcessList(&messagesProcess);
	addProcessToAllProcessList(&consoleProcessDescriptor);
	addProcessToAllProcessList(&WiFiProcessDescriptor);
	addProcessToAllProcessList(&MQTTProcessDescriptor);
	addProcessToAllProcessList(&controllerProcess);
	addProcessToAllProcessList(&ServoProcess);
	addProcessToAllProcessList(&RegistrationProcessDescriptor);
	addProcessToAllProcessList(&WebServerProcessDescriptor);
	addProcessToAllProcessList(&max7219MessagesProcess);
	addProcessToAllProcessList(&printerProcess);
}

void populateSensorList()
{
	addSensorToAllSensorsList(&pirSensor);
	addSensorToActiveSensorsList(&pirSensor);
	addSensorToAllSensorsList(&buttonSensor);
	addSensorToActiveSensorsList(&buttonSensor);
	addSensorToAllSensorsList(&clockSensor);
	addSensorToActiveSensorsList(&clockSensor);
	addSensorToAllSensorsList(&rotarySensor);
	addSensorToActiveSensorsList(&rotarySensor);
	addSensorToAllSensorsList(&potSensor);
	addSensorToActiveSensorsList(&potSensor);
}

void displayControlMessage(int messageNumber,  MessageLevel severity,  char *messageText)
{
	char buffer[20];

	messageSeverityToString(severity, buffer, 20);

	Serial.printf("%s: %d %s\n", buffer, messageNumber, messageText);
}

unsigned long heapPrintTime = 0;

#define HEAP_PRINT_GAP 500

void printString(char * string)
{
	while(*string)
	{
		Serial1.print(*string);
		Serial.print(*string);
		Serial1.flush();
		string++;
	}
}

void startDevice()
{
	Serial.begin(115200);

	delay(100);

	Serial.println("\n\n\n\n");

	loadBootReasonMessage();

	Serial.println(bootReasonMessage);

	Serial.printf("\n\nWelcome to Clever Little Boxes\n\n");
	Serial.printf("\n\nBuild date: %s %s\n\n", __DATE__, __TIME__);

	Serial.print("Initial Heap: ");
	Serial.println(ESP.getFreeHeap());
	heapPrintTime = millis();

	populateProcessList();

	populateSensorList();

	setupSettings();

	startstatusLedFlash(1000);

	beginStatusDisplay();

	initialiseAllProcesses();

	buildActiveProcessListFromMask(BOOT_PROCESS);

	startProcesses("Boot", true);

	bindMessageHandler(displayControlMessage);

	startSensors();

	delay(1000); // show the status for a while

	setupWalkingColour(BLUE_COLOUR);

	Serial.printf("Start complete\n\nType help and press enter for help\n\n");
}

void setup()
{
	startDevice();
}

uint32_t oldHeap = 0;

void heapMonitor()
{
	unsigned long m = millis();
	if ((m - heapPrintTime) > HEAP_PRINT_GAP)
	{
		uint32_t newHeap = ESP.getFreeHeap();
		if (newHeap != oldHeap)
		{
			Serial.print("Heap: ");
			Serial.println(newHeap);
			oldHeap=newHeap;
		}
		heapPrintTime = millis();
	}
}

void loop()
{
	// heapMonitor();
	updateSensors();
	updateProcesses();
	delay(1);
}