#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

int srtcasecmp(const char *string1, const char *string2);

unsigned long ulongDiff(unsigned long end, unsigned long start);

bool strContains(char* searchMe,char* findMe);

int getUnalignedInt(unsigned char * source);

float getUnalignedFloat(unsigned char * source);

void putUnalignedFloat(float fval, unsigned char * dest);

#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include <WebServer.h>
//#include <DNSServer.h>

#define PROC_ID (unsigned long)ESP.getEfuseMac()
#define PROC_NAME "ESP32"
#define LittleFS LITTLEFS

#endif

#if defined(ARDUINO_ARCH_ESP8266)

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define PROC_ID (unsigned long)ESP.getChipId()
#define PROC_NAME "ESP8266"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif


#endif



