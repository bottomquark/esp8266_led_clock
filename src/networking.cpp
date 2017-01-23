#include "networking.h"
#include "config.h"
#include "display.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

uint32_t mqttReconnectAttempt = 0;
uint32_t wifiReconnectAttempt = 0;
const unsigned char WIFI_NOT_CONNECTED_STR[] = "E 100";
bool displayedWifiNotConnected = false;

void displayWifiNotConnected();

void networkingSetup(MQTT_CALLBACK_SIGNATURE)
{
  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
}

void networkingLoop()
{
  if (WiFi.status() == WL_CONNECTED) {
    mqttClientLoop();
    ArduinoOTA.handle();
  }
  else {
    displayWifiNotConnected();
    setupWifi();
  }
}

void displayWifiNotConnected()
{
  if(displayedWifiNotConnected)
  {
    return;
  }
  displayString((unsigned char*)WIFI_NOT_CONNECTED_STR, 5);
  displayedWifiNotConnected = true;
}

void setupWifi() {

  uint32_t now = millis();
  if (now - wifiReconnectAttempt > 10000) {
    wifiReconnectAttempt = now;
    //need to disconnect completely and then set WIFI_STA (otherwise we are also in AP mode and therefore cannot connect to IPs 192.168.4.x)
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.onStart([]() {
        });
    ArduinoOTA.onEnd([]() {
        //Serial.println("\nEnd");
        });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
    ArduinoOTA.onError([](ota_error_t error) {
        /*Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        */
        });
    ArduinoOTA.begin();
    displayedWifiNotConnected = false;
  }
}

boolean mqttReconnect() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    // ... and resubscribe
    mqttClient.subscribe(IN_SUBSCRIBE_TOPIC "/#");
    //mqttClient.subscribe("clock/time/in");
    //mqttClient.subscribe("clock/display/#");
    //mqttClient.subscribe("clock/leds/#");
  }
  return mqttClient.connected();
}

/**
 * parse the given bytes and set the time to parsed values.
 * input format: 2016-01-02_23:33:44
 */
void setTimeFromString(byte* inputBytes, unsigned int length) {
  if( length != 19 ) {
    return;
  }

  char input[length + 1];
  for (int i = 0; i < length; i++) {
    input[i] = (char) inputBytes[i];
  }
  // Add the final 0 to end the C string
  input[length] = 0;

  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  char* curTok = strtok(input, ":_-");
  for (int i = 0; curTok != NULL; i++)
  {
    switch(i) {
      case 0: year = atoi(curTok);
      case 1: month = atoi(curTok);
      case 2: day = atoi(curTok);
      case 3: hour = atoi(curTok);
      case 4: minute = atoi(curTok);
      case 5: second = atoi(curTok);
    }
    // Find the next token in input string
    curTok = strtok(NULL, ":_-");
  }
  //int hr,int min,int sec,int day, int month, int yr
  setTime(hour, minute, second, day, month, year);
}

void mqttClientLoop() {
  //loop the MQTT client
  if (!mqttClient.connected()) {
    uint32_t now = millis();
    if (now - mqttReconnectAttempt > 5000) {
      mqttReconnectAttempt = now;
      // Attempt to reconnect
      if (mqttReconnect()) {
        mqttReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    mqttClient.loop();
  }

}

// vim:ai:cin:sts=2 sw=2 ft=cpp
