#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "statistic.h"
#include "method_hash_small_pattern.h"

unsigned int* lengthOneTable = NULL;
unsigned int* lengthTwoTable = NULL;

void initSmallPattern(){
	lengthOneTable = calloc(256, sizeof(unsigned int));
	lengthTwoTable = calloc(65535, sizeof(unsigned int));
}

void addSmallPattern(unsigned char* pattern, unsigned int length){
	if(length == 1){
		unsigned int index = pattern[0];
		lengthOneTable[index]++;
	}else if(length == 2){
		unsigned int index = (pattern[0] * 256) + pattern[1];
		lengthTwoTable[index]++;
	}
}

void searchSmallPattern(unsigned int char1, unsigned int char2, unsigned int char3){
	unsigned int oddTwoIndex = (char1 << 8) + char2;
	unsigned int evenTwoIndex = (char2 << 8) + char3;

	foundCounter += lengthOneTable[char1];
	foundCounter += lengthOneTable[char2];
	foundCounter += lengthTwoTable[oddTwoIndex];
	foundCounter += lengthTwoTable[evenTwoIndex];
}