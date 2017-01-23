# what this is about
I saw cheap LED strips with 60 LEDs/m and thought "this might make a nice clock" ;-)

![finished](/imgs/finished.jpg)  
![video1](/imgs/vid_bright.mp4)  
![video2](/imgs/vid_dark.mp4)  

# things you need
## hardware
* WS2801 LED strip (or similar). See [FastLED](https://github.com/FastLED/FastLED) for supported ones
* Wemos D1 mini (you might have to change some pin mappings in src/config.h for different MCUs)
* power supply 5V, 1A minimum. Current draw is about 200mA with everything attached but can get much higher depending on the brightness values of LEDs and display.
Optional:
* DHT22 temperature and humidity sensor or DS18B20 temperature sensor
* 4.7k resistor for the DHT22 or DS18B20
* RF24 module
* dot matrix module(s)
* LDR for automatically adjusting the brightness of the LEDs depending on environmental light
* matching resistor for voltage divider for the LDR (something like 220k should be ok)
* a breadboard and wires or a PCB to solder everything together
* some cardboard
* 60 wood pieces, 1cm high, max. 1cm wide and about 10cm long
* a DIN A2 sheet of thin paper (80g/m²) or multiple A4 sheets

## software
To get started, you need a machine with the following software installed. A Rasperry Pi works nicely.
* python 2.x
* an MQTT broker (e.g. [Mosquitto](https://mosquitto.org/))
* a command line MQTT client for testing

for Ubuntu/Debian/Raspbian, this should install everything necessary:
```
sudo apt-get install mosquitto-clients mosquitto python2.7 python-pip python-flask
sudo pip install paho-mqtt
```

# assembling 
First, draw two circles on cardboard. The outer one should have a radius of 15,9 cm (assuming the LED strip is 1m long), the inner one depends on how long the wood pieces are.  
![step1](/imgs/IMG_3033.JPG)

Lay the wood pieces around the circle drawn to see if everything fits. You don't want to glue everything in place just to see that something doesn't match...  
![step2](/imgs/IMG_3034.JPG)

Cut out the cardboard to a nice circle around the outer drawn circle. This doesn't need to be very precise because it will be hidden behind paper later on.  
Draw two perpendicular lines through the center of the circle, so you have guide lines for gluing.  
![step3](/imgs/IMG_3036.JPG)

Stick the wood pieces to double sided tape, then cut them along with the double sided tape.  
![step4](/imgs/IMG_3037.JPG)

Start gluing the wood pieces to the cardboard. Start on the drawn lines. Now, wrap the LED strip around and glue the rest of the wood pieces between the LEDs. Adjust the wood pieces to keep the LED strip tight.  
![step5](/imgs/IMG_3038.JPG)

Nearly finished. The LED strip now should be held in place by a little tape.  
![step6](/imgs/IMG_3039.JPG)

Now, cut the sheet of paper in a nice circle. It should cover the whole clock. If you don't have a big one, glue 4 DIN A4 sheets together so the edges meet in the middle of the clock.  

Glue the paper on and connect the controller board.  
![step7](/imgs/finished.jpg)

# connecting the hardware
see src/main.cpp for which pins should connect to what in the default configuration. If you want to connect things differently, change the definitions in src/config_example.h.  

![wiring](/imgs/wiring.jpg)  
![controller](/imgs/controller.jpg)  

# configuration
Copy src/config_example.h to src/config.h, open src/config.h in a text editor and 
* put your WIFI credentials and MQTT server address in there
* edit `LED_TYPE`  if it doesn't match your LED strip type
* edit the `#define`s for which temperature sensor to use and which kind of display (dot matrix or seven segment)

If you don't use a display, rf24 module, ... just leave the corresponding values on their defaults

Open clock_controller/clock_controller.py in a text editor and change the IP address near the bottom to the one where your MQTT server is running.

# flashing 
You can use the Arduino IDE or PlatformIO. See platformio.ini for library dependencies.
Some information about [OTA updating](http://docs.platformio.org/en/stable/platforms/espressif8266.html#over-the-air-ota-update)

Flashing has to be done initially over USB. Afterwards you can flash OTA.
```
platformio run -t upload -e d1_mini
```

example for flashing device OTA listening on IP 192.168.3.131:
```
platformio run -t upload -e d1_mini --upload-port 192.168.3.131
```

# clock controller
The clock controller is a Python script which communicates with the clock over MQTT. It receives temperature measurements and sets the correct time and colors for the clock.  
Before running, make sure that the directory `targetdir` specified near the top of clock_controller.py exists. Temperatore and brightness measurements will be written to files in this directory.  
```
python clock_controller.py
```
Open [localhost:5000](localhost:5000) in your browser (replace localhost with the IP of the machine running it) to see what can be configured.

# MQTT topics
MQTT is used for communication from/to the clock.

## setting current time
publish the time in format `YYYY-MM-dd_HH:mm:ss` to the MQTT topic `clock/in/time`
```
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/time" -m "$(date +%F_%T)"
```

## setting clock LED brightness and colors 
```
#brightness values from 0 (darkest) to 255 (brightest)
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/leds/brightness" -m "10"
#message is R,G,B with 0..255 for each value
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/leds/hourhand" -m "255,0,0"
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/leds/minutehand" -m "0,255,0"
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/leds/secondhand" -m "0,0,255"
#every 5 minutes
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/leds/fiveminutesections" -m "0,0,255"
#every quarter hour
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/leds/quartersections" -m "0,0,255"
```

## dot matrix and seven segment brightness 
brightness values from 0 (darkest) to 255 (brightest)
```
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/display/brightness" -m "10"
```

## displaying a string on dot matrix or seven segment display 
see src/sevensegment.cpp and src/dotmatix.cpp for max string length
```
mosquitto_pub -h 192.168.2.3 -d -t "clock/in/display" -m "hello world"
```

# measurements and statistics
statistics and temperature measurements are published once a minute to MQTT topics `clock/#` and `temps/#`  
To see everything:
```
mosquitto_sub -h 192.168.2.3 -d -t "#"

# WIFI reach
A little more than 100m with 2 solid walls between the MCU and the AP
