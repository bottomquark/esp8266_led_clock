#ifndef __NETWORKING_H_INCLUDED
#define __NETWORKING_H_INCLUDED

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "Time.h"

extern WiFiClient espClient;
extern PubSubClient mqttClient;

void networkingSetup(MQTT_CALLBACK_SIGNATURE);

//void callback(char*, byte*, unsigned int);
void setupWifi();
boolean mqttReconnect();
void setTimeFromString(byte* inputBytes, unsigned int length); 
void mqttClientLoop();
void networkingLoop();

#endif //__NETWORKING_H_INCLUDED
// vim:ai:cin:sts=2 sw=2 ft=cpp
