# box-code
This is the repository for all the code that runs inside the connected box. This code provides the interface to the sensors, settings managment, MQTT connection, a configuration web site and terminal connection. 

# Getting Started
## Building the code

The code can run on either an ESP8266 or ESP32. You can select the build target in the platformio.ini file.There are also build options for M5Stack and M5Stick devices. To build and deploy to a device connect it to your computer and perform a deploy from Platform IO. 

## Configiring the device via the terminal
The easiest way to configure a device once you have deployed the code is to use the PlatformIO terminal program. You can start this by clicking the serial connection item at the bottom of the screen. When the device boots it loads its setting information and starts the processes and sensors.

```
Connected Little Boxes Device      
Build date: Feb  5 2021 14:43:19   
Reset reason: External system reset

Settings Setup
   settings occupy 3759 bytes of EEPROM
   device:CLB-aa60a4 Version 1.0
   settings loaded OK

Starting processes
   pixels:  PIXEL OK
   statusled:  Status led lit
   inputswitch:  Input switch stopped
   messages:  Messages stopped
   console:  Console OK
   WiFi:  Scanning for networks
   MQTT:  MQTT Starting
   controller:  Controller active
   servo:  Servo off
   registration:  Waiting for MQTT
   Settingswebserver:  Web server Ready
   max7219:  Messages stopped
   printer:  Printer off
Starting sensors
   PIR: PIR sensor not fitted
   button: Button released
   clock: Clock waiting for wifi
   rotary: Rotary sensor not fitted
   pot: Pot sensor not fitted
Start complete

Type help and press enter for help
```
Type in the command **help**  and press enter. The terminal window will echo the keys that you enter and you can use the backspace key to move back down the line if you mis-type anything. 
```
These are all the available commands.

    help - show all the commands
    host - start the configuration web host
    settings - show all the setting values
    dump - dump all the setting values
    remote - show all the remote commands
    save - save all the setting values
    status - show the sensor status
    storage - show the storage use of sensors and processes
    pirtest - test the PIR sensor
    rotarytest - test the rotary sensor
    pottest - test the pot sensor
    colours - step through all the colours
    listeners - list the command listeners
    clearlisteners - clear the command listeners (also restarts the device)
    restart - restart the device
    otaupdate - update firmware over the air
    clear - clear all seeings and restart the device

You can view the value of any setting just by typing the setting name, for example:

    mqttdevicename

- would show you the MQTT device name.
You can assign a new value to a setting, for example:

     mqttdevicename=Rob

- would set the name of the mqttdevicename to Rob.

To see a list of all the setting names use the command settings.
This displays all the settings, their values and names.
To see a dump of settings (which can be restored to the device later) use dump
dump and settings can be followed by a filter string to match setting names
If you enter a JSON string this will be interpreted as a remote command.

```
You can assign a value to any of the settings:


## Setting Connection Defaults
You can set connection defaults for WiFi and MQTT connections. These will be stored in the device the first time that runs. 

Set the default values by entering them in the file defaults.hsec which is in the **lib\clb\src** folder. There is a sample file there called **defaults.hsec.sample** which you can use to get started. Fill in your details and  rename the file to defaults.hsec before building the solution.
```
#define DEFAULT_WIFI1_SSID "Your SSID"
#define DEFAULT_WIFI1_PWD "Your password"
#define DEFAULT_MQTT_HOST "Your MQTT Host"
#define DEFAULT_MQTT_USER "Your MQTT user"
#define DEFAULT_MQTT_PWD "Your MQTT password"
```
These default values will be stored in the eeprom of the device. If you would prefer not to have the defaults set in this way and you are happy to enter them manually you can remove the option **-DDEFAULTS_ON** from the **platformio.ini** file. if you use the default setting the WiFi and MQTT processes are turned off, so you will need to enable these via the terminal interface.
```
wifiactive=yes
mqttactive=yes
```