#include "ledstrip.h"
#include "config.h"

CRGB leds[NUM_LEDS];
CRGBSet ledSet(leds, NUM_LEDS);

uint8_t hourAnimation = 0;
uint8_t hourAnimationCounter = 0;
CRGB hourHandColor = CRGB(255,0,0);
CRGB minuteHandColor = CRGB(51,238,0);
CRGB secondHandColor = CRGB(255,238,0);
CRGB fiveMinuteSectionColor = CRGB(0,0,69);
CRGB quarterSectionColor = CRGB(0,92,74);

void setHourLeds(int hour, int min); 
void setSecondHandLed(int sec);
int getIndexFor(int led);

void ledSetup()
{
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
}

void ledLoop()
{
  ledSet.fadeToBlackBy(1);

  //variant 1: move separator LEDs around
  if(hourAnimation)
  {
    doHourAnimation();
  }
  else
  {
    //light up 5 minute and quarter sections
    for(int i = 0; i < 60; i+=5)
    {
      if(i % 15 == 0)
      {
	leds[getIndexFor(i)] = quarterSectionColor;
      }
      else
      {
	leds[getIndexFor(i)] = fiveMinuteSectionColor;
      }
    }
  }

  int min = minute();
  int sec = second();
  int hour = hourFormat12();

  setHourLeds(hour, min);
  leds[getIndexFor(min)] = minuteHandColor; 
  //leds[getIndexFor(sec)] = secondHandColor;
  //we need HSV colors, makes dimming and brightning much easier
  setSecondHandLed(sec);
  //leds[getIndexFor(sec)] = secondHandColor; 

  yield();

  FastLED.show();
} 

int prevSec = 0;
uint8_t curSecBrightness = 0;
CRGB prevSec1;
CRGB prevSec2;
//float R = (255 * log10(2))/log10(255);
float R = 31.8975139158; 

uint8_t secondhandBrightness[2];
void dimmingVariant1(uint8_t curBrightness)
{
  secondhandBrightness[0] = pow(2,(curBrightness / R)) - 0.1;
  secondhandBrightness[1] = pow(2,((255 - curBrightness) / R)) - 0.1;
}
void dimmingVariant2(uint8_t curBrightness)
{
  secondhandBrightness[0] = pow(2,(curBrightness / R)) - 0.1;
  secondhandBrightness[1] = 255 - secondhandBrightness[0];
}

void setSecondHandLed(int sec)
{
  /*
  for(uint8_t i = sec; i < 60 + sec; i++)
  {
    leds[getIndexFor(i)] = CRGB(0,238,0);
  }
  */
  //if(curSecBrightness < sec) leds[getIndexFor(curSecBrightness)] = CRGB(0,238,0);
  
  int index = getIndexFor(sec);
  
  if(sec != prevSec)
  {
    prevSec = sec;
    curSecBrightness = 0;
    //remember previous colors for fading them
    prevSec2 = prevSec1;
    prevSec1 = leds[index];
  }
  
  // exponential brightness calculation from https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms/
  //uint8_t brightness1 = pow(2,(curSecBrightness / R)) - 0.1;
  //uint8_t brightness2 = pow(2,((255 - curSecBrightness) / R)) - 0.1;
  if(minute() % 2 == 0)
  {
    dimmingVariant1(curSecBrightness);
  }
  else
  {
    dimmingVariant2(curSecBrightness);
  }
  uint8_t brightness1 = secondhandBrightness[0];
  uint8_t brightness2 = secondhandBrightness[1];

  leds[index] = secondHandColor;
  leds[index].nscale8(brightness1);
  //leds[index] += prevSec1.nscale8(255 - pow(2,(curSecBrightness / R)) - 0.1);
  CRGB p1 = prevSec1;
  p1.nscale8(brightness2);
  leds[index] += p1;
  
  index = getIndexFor(sec - 1);
  leds[index] = secondHandColor;
  leds[index].nscale8(brightness2);
  //leds[index] += prevSec2;
  //leds[index] += prevSec2.nscale8(pow(2,(255 - curSecBrightness / R)) - 0.1);
  CRGB p2 = prevSec2;
  p2.nscale8(brightness1);
  leds[index] += p2;

  if(curSecBrightness < 255) curSecBrightness ++;
}

//variant 1: move separator LEDs around
void doHourAnimation()
{
  //slow down a bit. delay() is not good here
  hourAnimationCounter++;
  if(hourAnimationCounter < 10)
  {
    return;
  }
  hourAnimationCounter = 0;

  FastLED.clear(true);
  for(int i = 0; i < 60; i+=5)
  {
    leds[getIndexFor(hourAnimation + i)] = fiveMinuteSectionColor;
  }

  hourAnimation++;
  if(hourAnimation > 30) hourAnimation = 0;
}
/*
//variant 2: run hour hand around the clock
void doHourAnimation()
{
  
  hourAnimationCounter++;
  if(hourAnimationCounter%10 != 0)
  {
    return;
  }

  int hourLedIndex = (hour()-1)*5 + minute()/12;

  leds[(hourLedIndex + hourAnimation - 1)%60] = hourHandColor;
  leds[(hourLedIndex + hourAnimation)%60] = hourHandColor;
  leds[(hourLedIndex + hourAnimation + 1)%60] = hourHandColor;
  leds[(hourLedIndex + hourAnimation - 1)%60].nscale8(50);
  leds[(hourLedIndex + hourAnimation + 1)%60].nscale8(50);
  
  hourAnimation++;
  if(hourAnimation > 60) hourAnimation = 0;
}
*/

//if called, starts hour animation in next call to ledLoop()
void startHourAnimation()
{
  if(hourAnimation == 0) hourAnimation = 1;
}

void setLedsBrightness(uint8_t b)
{
  FastLED.setBrightness( b );
}

/**
* returns the LED index for the given LED.
* takes into account rotating and reversing the clock like configured
* also loops around if necessary, so it is safe to have led > NUM_LEDS or < 0
*/
int getIndexFor(int led)
{
  int index = led;
#ifdef CLOCK_DISPLAY_TOP_INDEX
  index += CLOCK_DISPLAY_TOP_INDEX;
#endif
#ifdef REVERSE_CLOCK_DISPLAY
  index = (int)NUM_LEDS - index;
#endif

  index %= (int)NUM_LEDS;

  if(index < 0) index += NUM_LEDS;

  return index;
}

void setHourLeds(int hour, int min) 
{
  int hourLedIndex = hour*5 + min/12;

  int leftHourLedIndex = getIndexFor(hourLedIndex - 1);
  int rightHourLedIndex = getIndexFor(hourLedIndex + 1);

  leds[getIndexFor(hourLedIndex)] = hourHandColor;
  leds[leftHourLedIndex] = hourHandColor;
  leds[rightHourLedIndex] = hourHandColor;
  // Reduce color to 20% (50/256ths) of its previous value
  leds[leftHourLedIndex].nscale8(50);
  leds[rightHourLedIndex].nscale8(50);
}

void setHourHandColor(uint8_t r, uint8_t g, uint8_t b)
{
  hourHandColor.setRGB( r, g, b );
}
void setMinuteHandColor(uint8_t r, uint8_t g, uint8_t b)
{
  minuteHandColor.setRGB( r, g, b );
}
void setSecondHandColor(uint8_t r, uint8_t g, uint8_t b)
{
  secondHandColor.setRGB( r, g, b );
}
void setFiveMinuteSectionColor(uint8_t r, uint8_t g, uint8_t b)
{
  fiveMinuteSectionColor.setRGB( r, g, b );
}
void setQuarterSectionColor(uint8_t r, uint8_t g, uint8_t b)
{
  quarterSectionColor.setRGB( r, g, b );
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
