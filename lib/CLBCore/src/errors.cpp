#include "errors.h"

#include <Arduino.h>
#include "string.h"
#include "errors.h"
#include "settings.h"
#include "debug.h"
#include "ArduinoJson-v5.13.2.h"
#include "utils.h"
#include "sensors.h"
#include "processes.h"
#include "controller.h"
#include "registration.h"
#include "HullOS.h"

void decodeError(int errorNo, char *buffer, int bufferLength)
{
    String message;

    switch (errorNo)
    {
    case WORKED_OK:
        message =  F("Worked OK");
        break;
    case INVALID_HEX_DIGIT_IN_VALUE:
        message =  F("A number value contains an invalid hex digit");
        break;
    case INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER:
        message =  F("The hex value is too big for the input buffer");
        break;
    case INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH:
        message =  F("The hex value is the wrong length");
        break;
    case JSON_MESSAGE_COULD_NOT_BE_PARSED:
        message =  F("The JSON message could not be parsed.");
        break;
    case JSON_MESSAGE_MISSING_COMMAND_NAME:
        message =  F("There is no command name in the message");
        break;
    case JSON_MESSAGE_COMMAND_NAME_INVALID:
        message =  F("The command name is invalid");
        break;
    case JSON_MESSAGE_INVALID_DATA_TYPE:
        message =  F("The type of the data is wrong for this value");
        break;
    case JSON_MESSAGE_INVALID_DATA_VALUE:
        message =  F("The data value is invalid");
        break;
    case INVALID_OR_MISSING_TARGET_IN_RECEIVED_COMMAND:
        message =  F("The target for the command is missing or invalid");
        break;
    case COMMAND_FOR_DIFFERENT_TARGET:
        message =  F("The command is for a different target");
        break;
    case JSON_MESSAGE_PROCESS_NAME_MISSING:
        message =  F("The process name is missing from the command");
        break;
    case JSON_MESSAGE_PROCESS_NAME_INVALID:
        message =  F("The process name is invalid");
        break;
    case JSON_MESSAGE_COMMAND_MISSING_COMMAND:
        message =  F("There is no command name");
        break;
    case JSON_MESSAGE_COMMAND_COMMAND_NOT_FOUND:
        message =  F("There is no command to be performed");
        break;
    case JSON_MESSAGE_COMMAND_ITEM_NOT_FOUND:
        message =  F("There is no command item");
        break;
    case JSON_MESSAGE_COMMAND_ITEM_NOT_INT:
        message =  F("An integer command was expected but not received.");
        break;
    case JSON_MESSAGE_COMMAND_ITEM_INVALID_TYPE:
        message =  F("The type of the command item does not match.");
        break;
    case JSON_MESSAGE_COMMAND_ITEM_INVALID:
        message =  F("The command item is invalid");
        break;
    case JSON_MESSAGE_INVALID_COLOUR_NAME:
        message =  F("The colour name is invalid");
        break;
    case JSON_MESSAGE_SERVO_VALUE_TOO_LOW:
        message =  F("The servo value is too low");
        break;
    case JSON_MESSAGE_SERVO_VALUE_TOO_HIGH:
        message =  F("The servo value is too high");
        break;
    case JSON_MESSAGE_NO_ROOM_TO_STORE_LISTENER:
        message =  F("There is no room to store this listener");
        break;
    case JSON_MESSAGE_NO_MATCHING_SENSOR_FOR_LISTENER:
        message =  F("There is no sensor for this listener");
        break;
    case JSON_MESSAGE_NO_MATCHING_LISTENER_IN_SELECTED_SENSOR:
        message =  F("There is no listener of this name in the sensor");
        break;
    case JSON_MESSAGE_SENSOR_MISSING_TRIGGER:
        message =  F("There is no trigger specified for this sensor");
        break;
    case JSON_MESSAGE_SENSOR_ITEM_NOT_FOUND:
        message =  F("The sensor was not found");
        break;
    case JSON_MESSAGE_SERVO_NOT_AVAILABLE:
        message =  F("The servo is not available on this device");
        break;
    case JSON_MESSAGE_SENSOR_TRIGGER_NOT_FOUND:
        message =  F("There is no trigger with this name");
        break;
    case JSON_MESSAGE_DESTINATION_STRING_TOO_LONG:
        message =  F("The destination string is too long");
        break;
    case JSON_MESSAGE_LISTENER_COULD_NOT_BE_CREATED:
        message =  F("There is no room to create this listener");
        break;
    case JSON_MESSAGE_INVALID_CONSOLE_COMMAND:
        message =  F("The console command is invalid");
        break;
    case JSON_MESSAGE_INVALID_REGISTRATION_COMMAND:
        message =  F("The registration command is invalid");
        break;
    case JSON_MESSAGE_MAX7219_NOT_ENABLED:
        message =  F("The MAX7219 display is not enabled");
        break;
    case JSON_MESSAGE_SETTINGS_NOT_FOUND:
        message =  F("This setting was not found");
        break;
    case JSON_MESSAGE_MAX7219_INVALID_DEFAULT:
        message =  F("This is an invalid default for the MAX7219 display");
        break;
    case JSON_MESSAGE_PRINTER_NOT_ENABLED:
        message =  F("The printer is not enabled");
        break;
    case JSON_MESSAGE_SENSOR_NOT_FOUND_FOR_LISTENER_DELETE:
        message =  F("There is no sensor with this name in a listener delete");
        break;
    case JSON_MESSAGE_COULD_NOT_CREATE_STORE_FOLDER:
        message =  F("Could not create store folder");
        break;
    case JSON_MESSAGE_FILE_IN_PLACE_OF_STORE_FOLDER:
        message =  F("There is a file stored with this folder name. Folder store failed.");
        break;
    case JSON_MESSAGE_STORE_ID_MISSING_FROM_STORE_COMMAND:
        message =  F("Store ID missing from store command");
        break;
    case JSON_MESSAGE_STORE_FILENAME_INVALID:
        message =  F("Store filename invalid");
        break;
    case JSON_MESSAGE_STORE_FOLDERNAME_INVALID:
        message =  F("Store foldername invalid");
        break;
    case JSON_MESSAGE_STORE_FOLDER_DOES_NOT_EXIST:
        message =  F("Store folder does not exist");
        break;
    case JSON_MESSAGE_OUTPIN_NOT_AVAILABLE:
        message =  F("Output pin not available");
        break;
    }

    snprintf(buffer, bufferLength, message.c_str());
}
