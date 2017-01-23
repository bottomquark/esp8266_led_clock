#ifndef __SEVEN_SEGMENT_H_INCLUDED
#define __SEVEN_SEGMENT_H_INCLUDED

#include "LedControl.h"

void sevenSegmentSetup();
void sevenSegmentLoop();
void sevenSegmentString(unsigned char* toPrint, unsigned int length);
void sevenSegmentBrightness(uint8_t b);

#endif //__SEVEN_SEGMENT_H_INCLUDED
// vim:ai:cin:sts=2 sw=2 ft=cpp
