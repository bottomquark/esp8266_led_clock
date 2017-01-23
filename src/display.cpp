#include "display.h"
#include "config.h"

uint8_t displayCounter = 0;

void displaySetup()
{

  if(DOT_MATRIX_ENABLED) 
  {
    matrixSetup();
  }
  if(SEVEN_SEGMENT_ENABLED)
  {
    sevenSegmentSetup();
  }
}

void displayLoop()
{
  //slow down a bit. delay() is not good here
  displayCounter++;
  if(displayCounter < 30)
  {
    return;
  }
  displayCounter = 0;

  if(DOT_MATRIX_ENABLED) 
  {
    matrixLoop();
  }
  if(SEVEN_SEGMENT_ENABLED)
  {
    sevenSegmentLoop();
  }
}

void displayString(unsigned char* toPrint, unsigned int length)
{
  if(DOT_MATRIX_ENABLED) 
  {
    matrixDisplayString(toPrint, length);
  }
  if(SEVEN_SEGMENT_ENABLED)
  {
    sevenSegmentString(toPrint, length);
  }
}

void setDisplayBrightness(uint8_t b)
{
  if(DOT_MATRIX_ENABLED) 
  {
    matrixDisplayBrightness(b);
  }
  if(SEVEN_SEGMENT_ENABLED)
  {
    sevenSegmentBrightness(b);
  }

}

// vim:ai:cin:sts=2 sw=2 ft=cpp
