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
#include "method_odd_even_ac_filter.h"
#include "method_origin.h"

unsigned short* maxLengthGroup;
FilterFlag* filterFlagGroup;
unsigned int filterFlagSize;
ACSM_STRUCT2* filterOriginAC;

void setFilterSize(unsigned int traceLength){
	filterFlagSize = traceLength + 1;
	filterFlagGroup = calloc(traceLength, sizeof(FilterFlag));
}

static void addPatternIntoAc(ACSM_STRUCT2* acsm, unsigned int size, unsigned char** patterns, unsigned int* lengths){
  for(unsigned int i = 0; i < size; i++){
    // 將 pattern 加入 Aho Corasick，尚未編譯。
    int nocase = 0;
    int offset = 0;
    int depth = 0;
    int negative = 0;
    void* value = patterns[i];
    int id = i;
    acsmAddPattern2(acsm, patterns[i], lengths[i], nocase, offset, depth, negative, value, id);
  }
}

void createOriginAc(unsigned int size, unsigned char** patterns, unsigned int* lengths){
	filterOriginAC = createAhoCorasick();
	setAcMode(filterOriginAC, 6);
    setCallbackAcMatch(filterOriginAC, originMatch);

    addPatternIntoAc(filterOriginAC, size, patterns, lengths);
  	acsmCompile2(filterOriginAC);
  	acsmCaleStateDepth(filterOriginAC);
  	acsmCalePatternDepth(filterOriginAC);
  	acsmPrintInfo2(filterOriginAC);

  	filterOriginAC->otherData = filterFlagGroup;
}

void acsmCaleMaxPatternLength(ACSM_STRUCT2 *acsm){
	maxLengthGroup = calloc(acsm->acsmNumStates, sizeof(unsigned short));

	for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
		unsigned short maxLength = 0;
		for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
			if(MatchList->origin_length > maxLength){
				maxLength = MatchList->origin_length;
			}
		}
		maxLengthGroup[state] = maxLength;
	}
}

void fullMatchOddEvenAcFilter(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	unsigned int offset = (T - Tstart) + 1;
	unsigned int maxLength = maxLengthGroup[state] + 1;
	unsigned int start = offset - maxLength;

	for(unsigned int i = start; i <= offset; i++){
		filterFlagGroup[i].x = 1;
	}
}

static int MatchFound(void* id, int index, void *data){
  return 0;
}

void searchOriginAcWithFlag(unsigned char *trace, int traceSize){
	unsigned int countOne = 0;
	unsigned int countZero = 0;
	for(unsigned int i = 0; i < traceSize; i++){
		if(filterFlagGroup[i].x == 1){
			countOne++;
		}else if(filterFlagGroup[i].x == 0){
			countZero++;
		}
	}

	// printf("[INFO] count One:%u, Zero:%u \n", countOne, countZero);

	unsigned int current_state = 0;
    acsmSearch2(filterOriginAC, 0, trace, traceSize, MatchFound, (void *)0, &current_state);
}