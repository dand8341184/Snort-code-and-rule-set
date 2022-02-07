#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "statistic.h"
#include "method_filter_digest.h"

static unsigned short* sizeGroup;
static unsigned char** filterGroup;
static unsigned char** offsetGroup;
static ACSM_PATTERN2*** subPatternGroup;

static unsigned long long int costMemory = 0;

static void* CALLOC(size_t nitems, size_t size){
	costMemory += size * nitems;
	return calloc(nitems, size);
}

void processFilterDigest(ACSM_STRUCT2 *acsm){
	sizeGroup = CALLOC(acsm->acsmNumStates, sizeof(unsigned short));
	filterGroup = CALLOC(acsm->acsmNumStates, sizeof(unsigned char*));
	offsetGroup = CALLOC(acsm->acsmNumStates, sizeof(unsigned char*));
	subPatternGroup = CALLOC(acsm->acsmNumStates, sizeof(ACSM_PATTERN2**));

	for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
		ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];

		if(matchPattern == NULL){
			continue;
		}

		unsigned short countSubPattern = 0;
		ACSM_PATTERN2* currentPattern = matchPattern;
		for(; currentPattern != NULL; currentPattern = currentPattern->next){
			countSubPattern++;
		}

		sizeGroup[state] = countSubPattern;
		filterGroup[state] = CALLOC(countSubPattern, sizeof(unsigned char));
		offsetGroup[state] = CALLOC(countSubPattern, sizeof(unsigned char));
		subPatternGroup[state] = CALLOC(countSubPattern, sizeof(ACSM_PATTERN2*));


		unsigned int subPatternIndex = 0;
		currentPattern = matchPattern;
		for(; currentPattern != NULL; currentPattern = currentPattern->next){
			if(currentPattern->other_length == 1){
				unsigned int lastCharIndex = currentPattern->other_length - 1;
				filterGroup[state][subPatternIndex] = currentPattern->otherPattern[lastCharIndex];
				
				if(currentPattern->start - currentPattern->indexEnd == 1){
					offsetGroup[state][subPatternIndex] = 0;
				}else if(currentPattern->start - currentPattern->indexEnd == 3){
					offsetGroup[state][subPatternIndex] = 1;
				}
			}else if(currentPattern->other_length == 2){
				unsigned int lastCharIndex = currentPattern->other_length - 1;
				unsigned char char1 = currentPattern->otherPattern[lastCharIndex - 1];
				unsigned char char2 = currentPattern->otherPattern[lastCharIndex];
				unsigned char digest = (char1 << 4) + char2;
				filterGroup[state][subPatternIndex] = digest;

				if(currentPattern->start - currentPattern->indexEnd == 1){
					offsetGroup[state][subPatternIndex] = 2;
				}else if(currentPattern->start - currentPattern->indexEnd == 3){
					offsetGroup[state][subPatternIndex] = 3;
				}
			}else if(currentPattern->other_length >= 3){
				unsigned int lastCharIndex = currentPattern->other_length - 1;
				unsigned char char1 = currentPattern->otherPattern[lastCharIndex - 2];
				unsigned char char2 = currentPattern->otherPattern[lastCharIndex - 1];
				unsigned char char3 = currentPattern->otherPattern[lastCharIndex];
				unsigned char digest = (char1 << 4) + (char2 << 2) + char3;
				filterGroup[state][subPatternIndex] = digest;

				if(currentPattern->start - currentPattern->indexEnd == 1){
					offsetGroup[state][subPatternIndex] = 4;
				}else if(currentPattern->start - currentPattern->indexEnd == 3){
					offsetGroup[state][subPatternIndex] = 5;
				}
			}
			
			subPatternGroup[state][subPatternIndex] = currentPattern;
			subPatternIndex++;
		}
	}

	printf("Cost Memory Bytes: %llu \n", costMemory);
	// printf("Check Offset: %u \n", checkOffset);
}

void fullMatchFilterDigest(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	unsigned char* filter = filterGroup[state];
	unsigned char* offset = offsetGroup[state];
	unsigned short size = sizeGroup[state];

	unsigned char digest[6] = {0, 0, 0, 0, 0, 0};
	
	unsigned char* lastChar;
	unsigned char char1;
	unsigned char char2;
	unsigned char char3;

	lastChar = T - 1;
	char1 = *(lastChar - 2);
	char2 = *(lastChar - 1);
	char3 = *(lastChar);
	digest[0] = char3;
	digest[2] = (char2 << 4) + char3;
	digest[4] = (char1 << 4) + (char2 << 2) + char3;

	lastChar = T - 3;
	char1 = *(lastChar - 2);
	char2 = *(lastChar - 1);
	char3 = *(lastChar);
	digest[1] = char3;
	digest[3] = (char2 << 4) + char3;
	digest[5] = (char1 << 4) + (char2 << 2) + char3;

	for(unsigned int i = 0; i < size; i++){
		unsigned int indexDigest = offset[i];
		if(filter[i] != digest[indexDigest]){
			continue;
		}

		ACSM_PATTERN2* pattern = subPatternGroup[state][i];
		unsigned char* origin_patrn = pattern->origin_patrn;
		unsigned char* start = T - pattern->start;
		unsigned int indexEnd = pattern->indexEnd;

	    int isFull = 1;
	    for(int i = indexEnd - 2; i >= 0; i -= 2){
			if(origin_patrn[i] != start[i]){
				isFull = 0;
				break;
			}
	    }

		if(isFull){
	      foundCounter++;
	    }
	}
}