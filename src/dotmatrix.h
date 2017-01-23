#ifndef __DOTMATRIX_H_INCLUDED
#define __DOTMATRIX_H_INCLUDED
#include <stdint.h>

void matrixDisplayString(unsigned char* toPrint, unsigned int length);
void printString(char* toPrint);
//void printNumber(int val, char offset);
void matrixLoop();
void matrixSetup();
void matrixDisplayBrightness(uint8_t b);

#endif //__DOTMATRIX_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
