#ifndef __HEADER_UTIL_PASER__
#define __HEADER_UTIL_PASER__

#include <stdio.h>
#include <stdlib.h>

char* readableFixUnitValue(unsigned long long int, unsigned short);

void printTab(int count);

char hexCharToInt(unsigned char value);

unsigned int hexTextLength(unsigned char* hexText);

unsigned char* hexTextToHexArray(unsigned char* hexText);

void assertHexString(int length, int patternId);

#endif