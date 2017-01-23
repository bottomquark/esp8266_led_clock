#ifndef __TEMPS_H_INCLUDED
#define __TEMPS_H_INCLUDED

#include "config.h"
#include "networking.h"
#include <Adafruit_Sensor.h>
#ifdef USE_DHT22
#include <DHT.h>
#else
//for DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

void tempsSetup();
void tempsLoop();

#endif //__TEMPS_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
