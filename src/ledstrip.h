#ifndef __LEDCONTROL_H_INCLUDED
#define __LEDCONTROL_H_INCLUDED

#include "FastLED.h"
#include "Time.h"

void ledLoop();
void ledSetup();
void setLedsBrightness(uint8_t b);
void doHourAnimation();
void startHourAnimation();
void setHourHandColor(uint8_t r, uint8_t g, uint8_t b);
void setMinuteHandColor(uint8_t r, uint8_t g, uint8_t b);
void setSecondHandColor(uint8_t r, uint8_t g, uint8_t b);
void setFiveMinuteSectionColor(uint8_t r, uint8_t g, uint8_t b);
void setQuarterSectionColor(uint8_t r, uint8_t g, uint8_t b);

#endif //__LEDCONTROL_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
