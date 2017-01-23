/*
 * schematic for RF24: https://www.mysensors.org/build/esp8266_gateway 
 *
 * pinout D1 mini (see .platformio/packages/framework-arduinoespressif8266/variants/d1_mini/pins_arduino.h):
 * MOSI = D7 = 13
 * MISO = D6 = 12
 * SS = D8 = 15
 * SCK = D5 = 14
 *
 * pinout
 * A0: LDR
 * D2: LED Data
 *
 * D0: RF24 CE
 * D5: "RF24 SCK" "dot matrix CLK"
 * D6: RF24 MISO
 * D7: "RF24 MOSI" "dot matrix DIN"
 * D8: RF24 CS
 *
 * D1: Dot matrix CS
 * D3: DHT22 Data
 * D4: motion sensor
 *
 */
#include "config.h"
#include <Arduino.h>
#include "Time.h"
#include "ledstrip.h"
#include "networking.h"
#include "temps.h"
//#include "rf24control.h"
#include "display.h"
#include "SPI.h"

static unsigned long lastMotionTime;

static uint32_t lastStatsTime = 0;
static uint8_t bytesToRgbResult[3];
uint16_t loopSpeedCounter = 0;

//multi purpose buffer for conversions etc.
static char buf[20];

void publishStats();
void readLight();
void motionIsr();
void mqttCallback(char* topic, byte* payload, unsigned int length);
int bytesToInt(byte* inputBytes, unsigned int length);
void bytesToRgb(byte* inputBytes, unsigned int length);

void setup() {
  delay( 3000 ); // power-up safety delay
  SPI.begin();

  ledSetup();

  networkingSetup(mqttCallback);
  //FIXME: maybe run networking loop for 5 seconds to be able to flash OTA if something goes wrong
  //as there is a 10 second delay between network connection attempts: should we wait that long?
  /*
  while(millis() < 8000)
  {
    networkingLoop();
  }
  */

  //rf24Setup();
  displaySetup();

  //set initial time
  //int hr,int min,int sec,int day, int month, int yr
  setTime(4, 59, 40, 1, 11, 2016);

  lastStatsTime = millis();

  tempsSetup();

  //LDR pin
  pinMode(LDR_PIN, INPUT);

  //motion pin
  pinMode(MOTION_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), motionIsr, RISING);
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  //set the clock to time received
  if (strcmp(topic, IN_TOPIC_TIME) == 0 && length == 19) {
    setTimeFromString(payload, length);
  }
  else if (strcmp(topic, IN_TOPIC_DISPLAY) == 0) {
    displayString(payload, length);
  }
  else if (strcmp(topic, IN_TOPIC_LED_BRIGHTNESS) == 0) {
    uint8_t b = bytesToInt(payload, length);
    setLedsBrightness(b);
  }
  else if (strcmp(topic, IN_TOPIC_HOURHAND) == 0) {
    bytesToRgb(payload, length);
    setHourHandColor(bytesToRgbResult[0], bytesToRgbResult[1], bytesToRgbResult[2]);
  }
  else if (strcmp(topic, IN_TOPIC_MINUTEHAND) == 0) {
    bytesToRgb(payload, length);
    setMinuteHandColor(bytesToRgbResult[0], bytesToRgbResult[1], bytesToRgbResult[2]);
  }
  else if (strcmp(topic, IN_TOPIC_SECONDHAND) == 0) {
    bytesToRgb(payload, length);
    setSecondHandColor(bytesToRgbResult[0], bytesToRgbResult[1], bytesToRgbResult[2]);
  }
  else if (strcmp(topic, IN_TOPIC_5MINSECTIONS) == 0) {
    bytesToRgb(payload, length);
    setFiveMinuteSectionColor(bytesToRgbResult[0], bytesToRgbResult[1], bytesToRgbResult[2]);
  }
  else if (strcmp(topic, IN_TOPIC_QUARTERSECTIONS) == 0) {
    bytesToRgb(payload, length);
    setQuarterSectionColor(bytesToRgbResult[0], bytesToRgbResult[1], bytesToRgbResult[2]);
  }
  else if (strcmp(topic, IN_TOPIC_DISPLAY_BRIGHTNESS) == 0) {
    uint8_t b = bytesToInt(payload, length);
    setDisplayBrightness(b);
  }
}

void loop()
{
  loopSpeedCounter++;

  networkingLoop();

  if(minute() == 0 && second() == 0) startHourAnimation(); 
  ledLoop();
  yield();

  networkingLoop();
  yield();

  tempsLoop();
  yield();

  //rf24Loop();
  yield();

  displayLoop();
  yield();

  uint32_t curmillis = millis();

  networkingLoop();

  if((curmillis - lastStatsTime) > 60000)
  {
    lastStatsTime = curmillis;
    publishStats();
    yield();

    readLight();
  }

  if(lastMotionTime != 0)
  {
    snprintf(buf, sizeof(buf), "%i", lastMotionTime );
    mqttClient.publish(OUT_TOPIC_MOTION, buf );

    lastMotionTime = 0;
  }
}

void readLight()
{
  //bright light: 0
  //darkness: 560
  uint16_t light = analogRead(A0);
  snprintf(buf, sizeof(buf), "%i", light );
  mqttClient.publish(OUT_TOPIC_LIGHT, buf );
}

void publishStats()
{
  snprintf(buf, sizeof(buf), "%i-%02i-%02i_%02i:%02i:%02i", year(), month(), day(), hour(), minute(), second());
  mqttClient.publish(OUT_TOPIC_TIME, buf );

  /* deactivated for now, don't need it
  snprintf(buf, sizeof(buf), "%i", millis());
  mqttClient.publish("clock/stats/uptime", buf );

  snprintf(buf, sizeof(buf), "%i", ESP.getFreeHeap());
  mqttClient.publish("clock/stats/freeHeap", buf );

  snprintf(buf, sizeof(buf), "%i", ESP.getFreeSketchSpace());
  mqttClient.publish("clock/stats/freeSketchSpace", buf );
  */

  mqttClient.publish(OUT_TOPIC_STATS_IP, WiFi.localIP().toString().c_str() );

  snprintf(buf, sizeof(buf), "%i", loopSpeedCounter);
  mqttClient.publish(OUT_TOPIC_STATS_LOOPSPEED, buf );
  loopSpeedCounter = 0;

#ifdef MY_SRC_REV
  mqttClient.publish(OUT_TOPIC_STATS_GITREV, MY_SRC_REV );
#endif
}

void motionIsr()
{
  lastMotionTime = millis();
}

int bytesToInt(byte* inputBytes, unsigned int length) 
{
  char input[length + 1];
  for (int i = 0; i < length; i++) {
    input[i] = (char) inputBytes[i];
  }
  input[length] = 0;
  return atoi(input);
}

void bytesToRgb(byte* inputBytes, unsigned int length)
{
  char input[length + 1];
  for (int i = 0; i < length; i++) {
    input[i] = (char) inputBytes[i];
  }
  // Add the final 0 to end the C string
  input[length] = 0;

  char* command = strtok(input, ", ");
  for (int i = 0; command != NULL; i++)
  {
    switch(i) {
      case 0: bytesToRgbResult[0] = atoi(command);
      case 1: bytesToRgbResult[1] = atoi(command);
      case 2: bytesToRgbResult[2] = atoi(command);
    }
    // Find the next token in input string
    command = strtok(NULL, ", ");
  }
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
