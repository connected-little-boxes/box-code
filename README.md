# box-code
This is the repository for all the code that runs inside the connected box. This code provides the interface to the sensors, settings managment, MQTT connection, a configuration web site and terminal connection. 

# Getting Started
## Building the code

The code can run on either an ESP8266 or ESP32. You can select the build target in the platformio.ini file.There are also build options for M5Stack and M5Stick devices. To build and deploy to a device connect it to your computer and perform a deploy from Platform IO. The default device configuration is for the Wemos D1 Mini.
## Configuring the device using your phone
When a new device is first powered up it detects that there are no WiFi settings configured and it starts an access point called **CLB**. You can connect to this access point with your phone or laptop. If your phone can use the camera configure a WiFi connection you can connect to the device using the following QR code:


![WiFi QR code](images/CLBWifiQRcode.png)

When you have connedted to the device you must browse to the address of the configuration server which is **192.168.4.1** The QR code for this adress is as follows:

![Configuration host address](images/CLBLocalHost.png)

Your phone will display the configuration web page for WiFi and MQTT. 

![Configuration page](images/configHome.png)

Type in the settings for the WiFi connection that you want the device to use. 

Now enter the details for the MQTT server the device will be using. If the server doesn't require a username or password you can leave them blank. If you want to use a secure MQTT connection you can change the "MQTT Secure sockets active" item to "yes" and update the port number to 8883 if this is appropriate. 

If you do not have an MQTT server of your own you can use a free open one, for example the one at **`broker.hivemq.com`** 

**Note that if you use an open MQTT server any messages that you send will be visible to all users on that server and anyone else on the server could send messages addressed to your device. You should only use an open server for testing. If you only want to use your Connected Little Boxes around your house you can use a Raspberry Pi running Mosquitto as an MQTT server for your home netowrk.**

When you have entered your settings press the Update button to store them in your device. When your settigns have been saved you can select the **reset** link to reset the device. It will then connect to the network using the credentials that you have entered. 
## Configiring the device using your computer
Another way to configure a device once you have deployed the code is to use the PlatformIO terminal program. You can start this by clicking the serial connection icon you can find at the bottom left hand side of Visual Studio:

![Terminal Icon](images/PlatformIOTerminal.png)

**Note that you might find that there are two terminal icons on the bottom row. I'm sorry about this, the one you want is the leftmost one, you can see it on the screenshot below.**

The terminal should automatically connect to your device and open a window at the bottom of the Visual Studio Code display. 

![Terminal Icon](images/PlatformIOTerminalWindow.png)


The terminal connection to the device will echo the keys that you enter and you can use the backspace key to move back down the line if you mis-type anything. Press the Enter key to submit the command to the device. When the device is restarted it will go through the startup sequence shown below. This device has automatically started a WiFi access point for configuration. 
```
Connected Little Boxes Device
www.connectedlittleboxes.com
Version 1.0.0.9 build date: Mar  3 2021 14:45:58
Reset reason: External system reset

Settings Setup
   settings occupy 3900 bytes of EEPROM
   device:CLB-302dd0
   settings loaded OK

Starting processes
   pixels:  PIXEL OK
   statusled:  Status led lit
   inputswitch:  Input switch stopped
   messages:  Messages stopped
   console:  Console OK
   WiFi: Starting config access point at CLB:  WiFi connection starting AP
   MQTT:  MQTT Starting
   controller:  Controller active
   servo:  Servo off
   registration:  Waiting for MQTT
   Settingswebserver:  Web server Ready
   max7219:  Messages stopped
   printer:  Printer off
   hullos:  HullOS stopped
Starting sensors
   PIR: PIR sensor not fitted
   button: Button sensor not fitted
   clock: Clock waiting for wifi
   rotary: Rotary sensor not fitted
   pot: Pot sensor not fitted
   BME280: BME280 not fitted
Start complete

Type help and press enter for help

   Hosting at 192.168.4.1 on CLB
```
You can set an individual setting by assigning a new value to it. These are the settings that you can start with for the MQTT. 

```
wifissid1=your WiFi ssid
wifipwd1=your WiFi password

mqttdevicename=CLB-302dd0
mqttactive=yes
mqtthost=your MQTT host name
mqttport=1883
mqttsecure=no
mqttuser=your MQTT username
mqttpwd=your MQTT password
```
Each value that you enter is stored in the device for future use. You can view the contents of any setting just by entering the setting name and pressing Enter.
### The dump command
The dump command shows you the content of multiple settings. If you just enter the command "dump" you will see all the setting values. The dump command can also be followed by a filter so that only settings containing the filter string will be displayed:

```
Processing: dump wifi 

switchinputwificonfig=0
wifiactive=yes
wifissid1=
wifipwd1=******
wifissid2=
wifipwd2=******
wifissid3=
wifipwd3=******
wifissid4=
wifipwd4=******
wifissid5=
wifipwd5=******
```
The command above shows the WiFi settings. Note that the password values are not displayed using the Terminal. 

### More commands
To see what other commands are availalbe type in the command **help**  and press enter. 
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
```
For full command descriptions consult the device manual.

## Set default connection defaults into code
If you are happy using the Platform.ini file you can configure it to put connection defaults for WiFi and MQTT connections into your devices when they are built. These will be stored in the device the first time that runs. 

Set the default values by entering them in the file **defaults.hsec** which is in the **lib\clb\src** folder. There is a sample file there called **defaults.hsec.sample** which you can use to get started. Fill in your details and  rename the file to defaults.hsec before building the solution.
```
#define DEFAULT_WIFI1_SSID "Your SSID"
#define DEFAULT_WIFI1_PWD "Your password"
#define DEFAULT_MQTT_HOST "Your MQTT Host"
#define DEFAULT_MQTT_USER "Your MQTT user"
#define DEFAULT_MQTT_PWD "Your MQTT password"
```
