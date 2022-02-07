#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "statistic.h"
#include "aho_corasick.h"
#include "method_last_char_hash.h"

unsigned short* sizeGroup;
unsigned char** filterGroup;
unsigned char** offsetGroup;
ACSM_PATTERN2*** subPatternGroup;

unsigned long long int costMemory = 0;

static void* CALLOC(size_t nitems, size_t size){
	costMemory += size * nitems;
	return calloc(nitems, size);
}

void acsmCaleLastCharHash(ACSM_STRUCT2* acsm){
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
			unsigned int lastCharIndex = currentPattern->other_length - 1;
			filterGroup[state][subPatternIndex] = currentPattern->otherPattern[lastCharIndex];
			offsetGroup[state][subPatternIndex] = currentPattern->start - currentPattern->indexEnd;
			subPatternGroup[state][subPatternIndex] = currentPattern;

			// printf("[INFO] Flag:%u, Same:%u, Origin: ", currentPattern->flag, currentPattern->same);
			// for(int i = 0; i < currentPattern->origin_length; i++){
			//   // printf("%02x", currentPattern->otherPattern[i]);
			//   printf("%c", currentPattern->origin_patrn[i]);
			// }
			// printf(", Pattern: ");
			// for(int i = 0; i < currentPattern->n; i++){
			//   // printf("%02x", currentPattern->otherPattern[i]);
			//   printf("%c", currentPattern->patrn[i]);
			// }
			// printf(", Other: ");
			// for(int i = 0; i < currentPattern->other_length; i++){
			//   // printf("%02x", currentPattern->otherPattern[i]);
			//   printf("%c", currentPattern->otherPattern[i]);
			// }
			// // printf(", %02x, Offset:%u \n", currentPattern->otherPattern[lastCharIndex], offsetGroup[state][subPatternIndex]);
			// printf(", %c, Offset:%u \n", currentPattern->otherPattern[lastCharIndex], offsetGroup[state][subPatternIndex]);

			subPatternIndex++;
		}
		// printf("\n");
	}

	printf("Cost Memory Bytes: %llu \n", costMemory);
}

void fullMatchUsingLastCharHash(ACSM_STRUCT2* acsm, unsigned char* Tstart, unsigned char* T, unsigned int state){
	unsigned char* filter = filterGroup[state];
	unsigned char* offset = offsetGroup[state];
	unsigned short size = sizeGroup[state];

	unsigned char* lastChar = 0;
	for(unsigned int i = 0; i < size; i++){
		lastChar = T - offset[i];
		if(filter[i] != lastChar[0]){
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
