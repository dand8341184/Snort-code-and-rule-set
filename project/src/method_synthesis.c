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
#include "method_synthesis.h"

// Full Match
const unsigned short FULL = 0;
// One Length One Pattern in Final State
const unsigned short ONE = 1;
// Suffix Sub Pattern
const unsigned short SSP = 2;

unsigned short* finalStateTypeGroup = NULL;
unsigned int countSuffixSubPattern = 0;
unsigned int countLengthOnePattern = 0;
unsigned int countSearchSSP = 0;
unsigned int countSearchFULL = 0;

unsigned long long int countSuffixSubPatternSum = 0;

unsigned int createStrategyLengthOne(ACSM_STRUCT2 *acsm, unsigned int state){
	// unsigned int countSubPattern = 0;
	// ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];

	// if(MatchList == 0){
	// 	return 0;
	// }

	// if(MatchList->next != 0){
	// 	return 0;
	// }

	// if(MatchList->origin_length - n != 1){
	// 	return 0;
	// }

	// ACSM_PATTERN2* newSubPattern = calloc(1, sizeof(ACSM_PATTERN2));
	// memcpy(newSubPattern, MatchList, sizeof(ACSM_PATTERN2));

	// finalStateTypeGroup[state] = ONE;
	// acsm->acsmMatchList[state] = newSubPattern;

	// countLengthOnePattern++;
	// return 1;
	return 0;
}

unsigned int createStrategySuffixSubPattern(ACSM_STRUCT2 *acsm, unsigned int state){
	unsigned int flagMap[2] = {0};
	unsigned int longestLength = 0;
	ACSM_PATTERN2* longestSubPattern = NULL;
	for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
		if(MatchList->flag == MatchList->same){
			flagMap[0] = 1;
		}else{
			flagMap[1] = 1;
		}

		unsigned int length = MatchList->origin_length - MatchList->n;
		if(length > longestLength){
			longestLength = length;
			longestSubPattern = MatchList;
		}
	}

	unsigned int numberFlags = flagMap[0] + flagMap[1];
	if(numberFlags != 1){
		return 0;
	}

	for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
		unsigned int indexEnd = 0;
		if(MatchList->flag == MatchList->same){
			indexEnd = MatchList->origin_length - 2;
		}else{
			indexEnd = MatchList->origin_length - 1;
		}

		unsigned int indexDelta = longestSubPattern->origin_length - MatchList->origin_length;
		for(int i = indexEnd; i >= 0; i -= 2){
			if(longestSubPattern->origin_patrn[i + indexDelta] != MatchList->origin_patrn[i]){
				return 0;
			}
		}
	}

	ACSM_PATTERN2* newSubPattern = calloc(1, sizeof(ACSM_PATTERN2));
	memcpy(newSubPattern, longestSubPattern, sizeof(ACSM_PATTERN2));

	newSubPattern->subPatternArray = calloc(longestLength, sizeof(unsigned int));
	for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
		unsigned int index = MatchList->origin_length - MatchList->n - 1;
		newSubPattern->subPatternArray[index]++;

		countSuffixSubPatternSum += 1;
	}

	finalStateTypeGroup[state] = SSP;
	acsm->acsmMatchList[state] = newSubPattern;

	countSuffixSubPattern++;
	return 1;
}


void acsmCaleSynthesis(ACSM_STRUCT2 *acsm){
	finalStateTypeGroup = calloc(acsm->acsmNumStates, sizeof(unsigned short));

	for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
		acstate_t* transition = acsm->acsmNextState[state];
		if(transition[1]){
			// printf("[INFO] synthesis total:%u, state: %u\n", acsm->acsmNumStates, state);
			if(createStrategyLengthOne(acsm, state)){
				continue;
			}else if(createStrategySuffixSubPattern(acsm, state)){
				continue;
			}
		}
	}
}

void fullMatchOddEven(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	countSearchFULL += 1;

	ACSM_PATTERN2 *MatchList = acsm->acsmMatchList[state];

	for(; MatchList != NULL; MatchList = MatchList->next){
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

		int isFull = 1;
		if(flag == 0){
			for(int i = 1; i < origin_length; i += 2){
				if(origin_patrn[i] != start[i]){
					isFull = 0;
					break;
				}
			}
		}else{
			for(int i = 0; i < origin_length; i += 2){
				if(origin_patrn[i] != start[i]){
					isFull = 0;
					break;
				}
			}
		}

		if(isFull){
			foundCounter++;
			// printf("[INFO] Found pattern at position: %u.\n", (unsigned int)(T - Tstart));
		}
	}
}

// void fullMatchSuffixSubPattern(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
// 	ACSM_PATTERN2 *MatchList = acsm->acsmMatchList[state];

// 	unsigned char* origin_patrn = MatchList->origin_patrn;
//     unsigned int flag = MatchList->flag;
//     unsigned int origin_length = MatchList->origin_length;
//     unsigned int same = MatchList->same;

//     unsigned char* start = 0;
//     if(flag == 0){
//       if(same == 0){
//         start = T - origin_length - 1;
//       }else{
//         start = T - origin_length;
//       }
//     }else{
//       if(same == 0){
//         start = T - origin_length;
//       }else{
//         start = T - origin_length - 1;
//       }
//     }

//     if()
// }

void fullMatchSuffixSubPattern(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	countSearchSSP += 1;

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
    
 //    printf("[INFO] 		search pattern: ");
 //    for(unsigned int i = 0; i < origin_length; i++){
 //    	printf("%02x", origin_patrn[i]);
 //    }
 //    printf("\n");

 //    printf("[INFO] 		search text:    ");
 //    for(unsigned int i = 0; i < origin_length; i++){
 //    	printf("%02x", start[i]);
 //    }
 //    printf("\n");

	// printf("[INFO] 		array: ");
 //    for(unsigned int i = 0; i < MatchList->n; i++){
 //    	printf("%u,", MatchList->subPatternArray[i]);
 //    }
	// printf("\n");

    unsigned int lengthMatch = 0;
    if(flag == 0){
		for(int i = indexEnd; i >= 0; i -= 2){
			// printf("[INFO] 			compare char: %02x, %02x (", origin_patrn[i], start[i]);
			if(origin_patrn[i] != start[i]){
				// printf("JUMP)\n");
				break;
			}else{
				// printf("ADD: %u)\n", MatchList->subPatternArray[lengthMatch]);
				foundCounter += MatchList->subPatternArray[lengthMatch];
				lengthMatch++;
			}
		}
    }else{
    	for(int i = indexEnd; i >= 0; i -= 2){
			// printf("[INFO] 			compare char: %02x, %02x (", origin_patrn[i], start[i]);
			if(origin_patrn[i] != start[i]){
				// printf("JUMP)\n");
				break;
			}else{
				// printf("ADD: %u)\n", MatchList->subPatternArray[lengthMatch]);
				foundCounter += MatchList->subPatternArray[lengthMatch];
				lengthMatch++;
			}
		}
    }

    // printf("\n");
}

void fullMatchSynthesis(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	if(finalStateTypeGroup[state] == FULL){
		fullMatchOddEven(acsm, Tstart, T, state);
	}else if(finalStateTypeGroup[state] = SSP){
		fullMatchSuffixSubPattern(acsm, Tstart, T, state);
	}
}

void showStatisticStrategy(){
	printf("[INFO] Count length one pattern: %u \n", countLengthOnePattern);
	printf("[INFO] Count suffix sub pattern: %u \n", countSuffixSubPattern);

	printf("[INFO] Count avg sub pattern size in final state: %.2f \n", (float)countSuffixSubPatternSum / (float)countSuffixSubPattern);

	printf("[INFO] Count SearchFULL: %u \n", countSearchSSP);
	printf("[INFO] Count SearchSSP: %u \n", countSearchFULL);
}