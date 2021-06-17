#pragma once

#include <Arduino.h>

#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "pirSensor.h"
#include "controller.h"
#include "mqtt.h"

#define REGISTRATION_COMMAND_SIZE 30

#define MQTT_CONNECTED_TOPIC "connected"
#define MQTT_REGISTERED_TOPIC "registration"


#define REGISTRATION_OK 1000
#define REGISTRATION_OFF 1001
#define REGISTRATION_WAITING_FOR_MQTT 1002
#define WAITING_FOR_REGISTRATION_REPLY 1003

#define FRIENDLY_NAME_LENGTH 30

extern struct process RegistrationProcess;

struct RegistrationSettings 
{
    char friendlyName[FRIENDLY_NAME_LENGTH];
};

extern struct RegistrationSettings RegistrationSettings;
