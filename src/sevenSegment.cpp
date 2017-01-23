#include "sevenSegment.h"
#include "config.h"

/*
* D8: DIN
* D7: CLK
* D5: CS
* We have only a single MAX72XX.
*/
//LedControl lc=LedControl(D8,D7,D5,1);
//the maximum length of a string to display
#define MAX_STR_LEN 50

const byte availableCharsCount = 8;
byte segmentLoopCount = 0;

//LedControl lc=LedControl(SEVEN_SEGMENT_CS,RF24_MOSI,RF24_SCK,8);
LedControl lc=LedControl(SEVEN_SEGMENT_CS,8);

char segmentString[MAX_STR_LEN];
boolean segmentDots[MAX_STR_LEN];

unsigned int strLength = 0;
boolean changed = true;

void sevenSegmentSetup() {
  for(int index=0;index<lc.getDeviceCount();index++) {
    //The MAX72XX is in power-saving mode on startup, we have to do a wakeup call
    lc.shutdown(index,false); 
    //0 == darkest, 15 == brightest
    lc.setIntensity(index,0);
  }
}

void sevenSegmentString(byte* toPrint, unsigned int length)
{
  if(length > MAX_STR_LEN)
  {
    return;
  }
  int realPos = 0;
  for(int i = 0; i < length; i++)
  {
    segmentDots[i] = false;
    if(toPrint[i] == '.' && realPos != 0)
    {
      segmentDots[realPos-1] = true;
    }
    else
    {
      segmentString[realPos] = toPrint[i];
      realPos++;
    }
  }
  
  segmentString[realPos] = 0;
  strLength = realPos; 
  changed = true;
}

void sevenSegmentLoop()
{ 
  if(strLength <= availableCharsCount && !changed )
  {
    //return immediately if nothing has changed and we don't need scrolling
    return;
  }
  changed = false;

  if(strLength > availableCharsCount )
  {
    segmentLoopCount++;
    if(segmentLoopCount >= strLength)
    {
      segmentLoopCount = 0;
    }

    lc.clearDisplay(0);

    for(int i = 0; i < availableCharsCount && segmentLoopCount + i < strLength; i++)
    {
      //setChar(int addr, int digit, char value, boolean decimalpoint)
      lc.setChar(0,availableCharsCount - i - 1,segmentString[segmentLoopCount + i],segmentDots[i]);
    }
  }
  else
  {
    lc.clearDisplay(0);
    for(int i = 0; i < strLength; i++)
    {
      //setChar(int addr, int digit, char value, boolean decimalpoint)
      lc.setChar(0,availableCharsCount - i - 1,segmentString[i],segmentDots[i]);
    }
  }
}

void sevenSegmentBrightness(uint8_t b)
{
  for(int index=0;index<lc.getDeviceCount();index++) {
    //0 == darkest, 15 == brightest
    //dividing by 17 maps the range 0..255 to 0..15
    lc.setIntensity(index,b/17);
  }
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
