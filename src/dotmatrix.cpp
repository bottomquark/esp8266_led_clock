#include "dotmatrix.h"
#include "config.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#define MAX_STR_LEN 50

//const int pinCS = DOT_MATRIX_CS; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4;
const int numberOfVerticalDisplays = 1;
const byte pixelCount = numberOfHorizontalDisplays*8;
//the width of one character in pixels
const byte charWidth = 5;
//the number of characters that can be displayed at once
const byte availableCharsCount = pixelCount/charWidth;

Max72xxPanel matrix = Max72xxPanel(DOT_MATRIX_CS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

byte loopCount = 0;
int startPos = 0; 
unsigned int len = 0;
char theString[MAX_STR_LEN];

void matrixSetup() {

  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

  //the displays are rotated 90°
  matrix.setRotation(0, 1);
  matrix.setRotation(1, 1);
  matrix.setRotation(2, 1);
  matrix.setRotation(3, 1);
}

void matrixDisplayString(unsigned char* toPrint, unsigned int length)
{
  if(length >= MAX_STR_LEN)
  {
    return;
  }
  memcpy( theString, toPrint, length );
  theString[length] = 0;
  len = length; 
  startPos = 0;
  matrix.fillScreen(LOW);
}

void printString(char* toPrint)
{
  len = strlen( toPrint );
  if(len >= MAX_STR_LEN)
  {
    return;
  }
  strcpy(theString, toPrint);
  startPos = 0;
  matrix.fillScreen(LOW);
}

void matrixLoop() {

  yield();

  if(len > availableCharsCount )
  {
    loopCount++;
    //we need scrolling, string is too long
    if(loopCount >= charWidth) 
    {
      startPos++;
      loopCount = 0;
    }

    if(startPos >= len)
    {
      startPos = 0;
    }

    for(int i = 0; i < availableCharsCount + 1; i++)
    {
      int curpos = startPos + i;
      curpos = curpos % len;
      matrix.drawChar(i*charWidth - (loopCount % charWidth), 0, theString[curpos], HIGH, LOW, 1);
    }
  } 
  else
  {
    for (int i = 0; i < len; i++)
    {
      matrix.drawChar(i*6, 0, theString[i], HIGH, LOW, 1);
    }
  }
  yield();
  matrix.write(); // Send bitmap to display
}

void matrixDisplayBrightness(uint8_t b)
{
    //dividing by 17 maps the range 0..255 to 0..15
  matrix.setIntensity(b/17);
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
