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
#include "method_longest_sub_pattern.h"

void acsmCaleLongestSubPattern(ACSM_STRUCT2 *acsm){
  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
  	acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
    	printf("[INFO] final state: %u \n", state);
    	ACSM_PATTERN2* longestSubPattern = NULL;
    	unsigned int longestLength = 0;
    	for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
    		printf("[INFO] 		pattern: %s \n", MatchList->patrn);
    		unsigned int subPatternLength = MatchList->origin_length - MatchList->n;
    		if(subPatternLength > longestLength){
    			longestLength = subPatternLength;
    			longestSubPattern = MatchList;
    		}
    	}

    	printf("[INFO] 		longest: %u \n", longestLength);
    	ACSM_PATTERN2* newSubPattern = calloc(1, sizeof(ACSM_PATTERN2));
    	memcpy(newSubPattern, longestSubPattern, sizeof(ACSM_PATTERN2));
    	newSubPattern->subPatternArray = calloc(longestLength, sizeof(unsigned int));

    	for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
    		unsigned int index = MatchList->n - 1;
    		newSubPattern->subPatternArray[index]++;
    	}

    	printf("[INFO] 		array: ");
    	for(unsigned int i = 0; i < longestLength; i++){
    		printf("%u,", newSubPattern->subPatternArray[i]);
    	}
    	printf("\n");

    	acsm->acsmMatchList[state] = newSubPattern;
    }
  }
}

void fullMatchUsingLongestSubPattern(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	ACSM_PATTERN2 *MatchList = acsm->acsmMatchList[state];

	unsigned char* origin_patrn = MatchList->origin_patrn;
    unsigned int flag = MatchList->flag;
    unsigned int origin_length = MatchList->origin_length;
    unsigned int same = MatchList->same;

    unsigned char* start = 0;
    if(flag == 0){
      if(same == 0){
        start = T - origin_length - 1;
      }else{
        start = T - origin_length;
      }
    }else{
      if(same == 0){
        start = T - origin_length;
      }else{
        start = T - origin_length - 1;
      }
    }

    unsigned int indexEnd = 0;
    if(flag == 0){
	    if(origin_length % 2 == 0){
	    	indexEnd = origin_length - 1;
	    }else{
	    	indexEnd = origin_length - 2;
	    }
    }else{
    	if(origin_length % 2 == 0){
	    	indexEnd = origin_length - 2;
	    }else{
	    	indexEnd = origin_length - 1;
	    }
    }
    
    printf("[INFO] 		search pattern: ");
    for(unsigned int i = 0; i < origin_length; i++){
    	printf("%02x", origin_patrn[i]);
    }
    printf("\n");

    printf("[INFO] 		search text:    ");
    for(unsigned int i = 0; i < origin_length; i++){
    	printf("%02x", start[i]);
    }
    printf("\n");

    printf("[INFO] 		array: ");
	for(unsigned int i = 0; i < MatchList->n; i++){
		printf("%u,", MatchList->subPatternArray[i]);
	}
	printf("\n");

    unsigned int lengthMatch = 0;
    if(flag == 0){
		for(int i = indexEnd; i >= 0; i -= 2){
		// for(int i = 1; i < origin_length; i += 2){

			printf("[INFO] 			compare char: %02x, %02x (", origin_patrn[i], start[i]);
			if(origin_patrn[i] != start[i]){
				printf("JUMP)\n");
				break;
			}else{
				printf("ADD: %u)\n", MatchList->subPatternArray[lengthMatch]);
				foundCounter += MatchList->subPatternArray[lengthMatch];
				lengthMatch++;
			}
		}
    }else{
    	for(int i = indexEnd; i >= 0; i -= 2){
		// for(int i = 0; i < origin_length; i += 2){

			printf("[INFO] 			compare char: %02x, %02x (", origin_patrn[i], start[i]);
			if(origin_patrn[i] != start[i]){
				printf("JUMP)\n");
				break;
			}else{
				printf("ADD: %u)\n", MatchList->subPatternArray[lengthMatch]);
				foundCounter += MatchList->subPatternArray[lengthMatch];
				lengthMatch++;
			}
		}
    }

    printf("\n");
}