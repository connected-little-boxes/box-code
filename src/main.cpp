#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "LittleFS.h"

#include "debug.h"
#include "utils.h"

#include "settings.h"
#include "pixels.h"
#include "processes.h"
#include "sensors.h"
#include "pirSensor.h"
#include "buttonsensor.h"
#include "inputswitch.h"
#include "controller.h"
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
#include "BME280Sensor.h"
#include "HullOS.h"
#include "boot.h"
#include "otaupdate.h"
#include "outpin.h"


// This function will be different for each build of the device.

void populateProcessList()
{
	addProcessToAllProcessList(&bootProcessDescriptor);
	addProcessToAllProcessList(&pixelProcess);
	addProcessToAllProcessList(&statusLedProcess);
	addProcessToAllProcessList(&inputSwitchProcess);
	addProcessToAllProcessList(&messagesProcess);
	addProcessToAllProcessList(&consoleProcessDescriptor);
	addProcessToAllProcessList(&WiFiProcessDescriptor);
	addProcessToAllProcessList(&MQTTProcessDescriptor);
	addProcessToAllProcessList(&controllerProcess);
	addProcessToAllProcessList(&ServoProcess);
	addProcessToAllProcessList(&RegistrationProcess);
	addProcessToAllProcessList(&WebServerProcess);
	addProcessToAllProcessList(&max7219MessagesProcess);
	addProcessToAllProcessList(&printerProcess);
	addProcessToAllProcessList(&hullosProcess);
	addProcessToAllProcessList(&otaUpdateProcessDescriptor);
	addProcessToAllProcessList(&outPinProcess);
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
	addSensorToAllSensorsList(&bme280Sensor);
	addSensorToActiveSensorsList(&bme280Sensor);
}

void displayControlMessage(int messageNumber, ledFlashBehaviour severity, char *messageText)
{
	char buffer[20];

	ledFlashBehaviourToString(severity, buffer, 20);

	Serial.printf("%s: %d %s\n", buffer, messageNumber, messageText);
}

unsigned long heapPrintTime = 0;

void startDevice()
{
	Serial.begin(115200);

	delay(100);

	Serial.println("\n\n\n\n");

	Serial.printf("Connected Little Boxes Device\n");
	Serial.printf("www.connectedlittleboxes.com\n");
	Serial.printf("Version %s build date: %s %s\n", Version, __DATE__, __TIME__);
#ifdef DEBUG
	Serial.println("**** Debug output enabled");
#endif

	//	Serial.print("Initial Heap: ");
	//	Serial.println(ESP.getFreeHeap());
	heapPrintTime = millis();

	populateProcessList();

	populateSensorList();

	setupSettings();

	startstatusLedFlash(1000);

	initialiseAllProcesses();

	beginStatusDisplay();

	buildActiveProcessListFromMask(BOOT_PROCESS);

	startProcesses();

	bindMessageHandler(displayControlMessage);

	startSensors();

	delay(1000); // show the status for a while

	Serial.printf("Start complete\n\nType help and press enter for help\n\n");
}

uint32_t oldHeap = 0;

#define HEAP_PRINT_INTERVAL 500

void heapMonitor()
{
	unsigned long m = millis();
	if ((m - heapPrintTime) > HEAP_PRINT_INTERVAL)
	{
		uint32_t newHeap = ESP.getFreeHeap();
		if (newHeap != oldHeap)
		{
			Serial.print("Heap: ");
			Serial.println(newHeap);
			oldHeap = newHeap;
		}
		heapPrintTime = millis();
	}
}

void setup()
{
	startDevice();
}

void loop()
{
	updateSensors();
	updateProcesses();
	delay(1);
}