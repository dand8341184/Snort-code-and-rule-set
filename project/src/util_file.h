#include "main.h"

#include "aho_corasick.h"

#ifndef __HEADER_UTIL_FILE__
#define __HEADER_UTIL_FILE__

int checkFileExist(Config* config);

void readPatternFile(unsigned char* fileName, unsigned int* size, 
  unsigned char*** patterns, unsigned int** lengths,
  unsigned char*** hexPatterns, unsigned int** hexLengths);

void readTraceFile(char* traceFileName, unsigned long long int* size, char** trace);

#endif