#ifndef __DISPLAY_H_INCLUDED
#define __DISPLAY_H_INCLUDED

#include "config.h"

#include "dotmatrix.h"
#include "sevenSegment.h"
#include <stdint.h>

void displaySetup();
void displayLoop();
void displayString(unsigned char* toPrint, unsigned int length);
void setDisplayBrightness(uint8_t b);

#endif //__DISPLAY_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
