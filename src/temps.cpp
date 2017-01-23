#include "temps.h"

#ifdef USE_DHT22
DHT dht(DHT22_PIN, DHT22);
#else
OneWire oneWire(DHT22_PIN);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
#endif

static unsigned long tempReadingRequested = 0;
uint32_t lastTempsTime;
//multi purpose buffer for conversions etc.
static char buf[20];

void readTemperature();
float median(float a, float b, float c);
float temps[3];
float hums[3];
uint8_t measurementCounter = 0;

void tempsSetup()
{
#ifdef USE_DHT22
  dht.begin();
#else
  sensors.begin();
  sensors.getAddress(insideThermometer, 0);
  sensors.setResolution(insideThermometer, 12);
  //request async reading
  sensors.setWaitForConversion(false);
#endif
}

void tempsLoop()
{
  uint32_t curmillis = millis();
  //temperature reading. If ds18b20 is used, it is read asynchronly
  if((curmillis - lastTempsTime) > 50000 || tempReadingRequested)
  {
    readTemperature();
    yield();
    lastTempsTime = curmillis;
  }
}

void readTemperature()
{
#ifdef USE_DHT22
  //DHT22 can be read every 2 seconds. Make it 3 seconds here to be on the safe side
  if(tempReadingRequested == 0 || ( measurementCounter < 3 && millis() - tempReadingRequested > 3000 ))
  {
    float hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float temp = dht.readTemperature();
    if(!isnan(hum) && !isnan(temp))
    {
      hums[measurementCounter] = hum;
      temps[measurementCounter] = temp;
      measurementCounter++;
    }

    tempReadingRequested = millis();

    if(measurementCounter == 3)
    {
      float temp = median(temps[0], temps[1], temps[2]);
      dtostrf(temp, 1, 1, buf);
      mqttClient.publish("temps/clock/temp", buf );

      float hum = median(hums[0], hums[1], hums[2]);
      dtostrf(hum, 1, 1, buf);
      mqttClient.publish("temps/clock/hum", buf );

      measurementCounter = 0;
      tempReadingRequested = 0;
    }
  }
#else
  if (tempReadingRequested == 0)
  {
    tempReadingRequested = millis();
    //request conversion
    sensors.requestTemperatures();
  }
  else if(millis() - tempReadingRequested > 1000)
  {
    //fetch result requested 1000 ms earlier
    float temp = sensors.getTempC(insideThermometer);
    //snprintf with float does not work on Arduino (has been removed)
    //snprintf(buf, sizeof(buf), "%f", temp );
    dtostrf(temp, 1, 2, buf);
    mqttClient.publish("temps/clock/temp", buf );
    tempReadingRequested = 0;
  }
#endif
}

float median(float a, float b, float c)
{
  return fmax(fmin(a,b), fmin(fmax(a,b),c));
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
