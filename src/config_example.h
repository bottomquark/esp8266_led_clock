#ifndef __CONFIG_H_INCLUDED
#define __CONFIG_H_INCLUDED

//the name of the WIFI network to connect to
#define WIFI_SSID "the_ssid"
//password of the WIFI network
#define WIFI_PASSWORD "the_password"
//IP address of the MQTT server
#define MQTT_SERVER "192.168.1.2" 

//if the following line is enabled, the clock runs in its reverse direction
//can be helpful if one glued the LED strip the wrong way around ;-)
//#define REVERSE_CLOCK_DISPLAY

//if 12 o' clock on the clock shouldn't be on LED index 0, specify the LED (starting with 0) here, where 12 o' clock should be
//i.e. if you want to/need to rotate the clock with already glued on LED strip
//#define CLOCK_DISPLAY_TOP_INDEX 32 

//D4 is LED_BUILTIN on d1_mini 
#define MOTION_PIN  D4
#define LDR_PIN     A0

#define LED_PIN     D2
#define NUM_LEDS    60
//initial brightness of LEDs (from 0..255)
#define BRIGHTNESS  255 
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

//if using DHT22, define it here. Otherwise we will seach for a connected DS18B20 on the same pin
#define USE_DHT22
#define DHT22_PIN D3

#define RF24_CE D0 
#define RF24_SCK D5
#define RF24_MISO D6
#define RF24_MOSI D7
#define RF24_CS D8
// this node is the first child of node 5. See http://tmrh20.github.io/RF24Network/Tuning.html
// children will be 0115 to 0515
#define RF24_NODEID 015

//set to 1 to enable, 0 to disable one of the following
#define DOT_MATRIX_ENABLED 0
#define SEVEN_SEGMENT_ENABLED 1

//seven segment version:
#define SEVEN_SEGMENT_CS D1
//dot matrix version:
#define DOT_MATRIX_CS D1

#define MQTT_CLIENT_ID "clock"

//definitions of MQTT topics
//in topics
#define IN_SUBSCRIBE_TOPIC MQTT_CLIENT_ID "/in"
#define IN_TOPIC_SECONDHAND IN_SUBSCRIBE_TOPIC "/leds/secondhand"
#define IN_TOPIC_MINUTEHAND IN_SUBSCRIBE_TOPIC "/leds/minutehand"
#define IN_TOPIC_HOURHAND IN_SUBSCRIBE_TOPIC "/leds/hourhand"
#define IN_TOPIC_TIME IN_SUBSCRIBE_TOPIC "/time"
#define IN_TOPIC_DISPLAY IN_SUBSCRIBE_TOPIC "/display"
#define IN_TOPIC_LED_BRIGHTNESS IN_SUBSCRIBE_TOPIC "/leds/brightness"
#define IN_TOPIC_DISPLAY_BRIGHTNESS IN_SUBSCRIBE_TOPIC "/display/brightness"
#define IN_TOPIC_5MINSECTIONS IN_SUBSCRIBE_TOPIC "/leds/fiveminutesections"
#define IN_TOPIC_QUARTERSECTIONS IN_SUBSCRIBE_TOPIC "/leds/quartersections"

//out topics
#define OUT_TOPIC_MOTION "motion/" MQTT_CLIENT_ID
#define OUT_TOPIC_LIGHT MQTT_CLIENT_ID "/light"
#define OUT_TOPIC_TIME MQTT_CLIENT_ID "/time/out"
#define OUT_TOPIC_TEMP "temps/" MQTT_CLIENT_ID "/temp"
#define OUT_TOPIC_HUM "temps/" MQTT_CLIENT_ID "/hum"

//stats topics:
#define OUT_TOPIC_STATS MQTT_CLIENT_ID "/stats"
#define OUT_TOPIC_STATS_IP OUT_TOPIC_STATS "/ip"
#define OUT_TOPIC_STATS_LOOPSPEED OUT_TOPIC_STATS "/loopspeed"
#define OUT_TOPIC_STATS_GITREV OUT_TOPIC_STATS "/gitRev"

#endif //__CONFIG_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
