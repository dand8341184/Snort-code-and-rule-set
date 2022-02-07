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
#include "method_twice_odd_even.h"

typedef struct StateNumber{
	unsigned int value;
	unsigned int length;
	unsigned int count;
	struct StateNumber* next;
} StateNumber;

void sortLinkList(StateNumber* head){
    for(StateNumber* current = head; current != NULL; current = current->next){
        for(StateNumber* visit = current->next; visit != NULL; visit = visit->next){
            if(current->value > visit->value){
                unsigned int tmp = current->value;
                current->value = visit->value;
                visit->value = tmp;
            }
        }
    }
}

void uniqLinkList(StateNumber* head){
    for(StateNumber* current = head; current != NULL; current = current->next){
        StateNumber* repeat = NULL;
        for(repeat = current; repeat != NULL; repeat = repeat->next){
            if(current->value == repeat->value){
                current->count++;
            }else{
                break;
            }
        }
        if(current->next != repeat){
            free(current->next);
            current->next = repeat;
        }
    }
}

void printLinkList(StateNumber* head){
    for(StateNumber* current = head; current != NULL; current = current->next){
        printf("%u, %u\n", current->value, current->count);
    }
}

unsigned int sizeLinkList(StateNumber* head){
	unsigned int size = 0;
	for(StateNumber* current = head; current != NULL; current = current->next){
		size++;
	}
	return size;
}

void caleDataMaxLength(ACSM_STRUCT2* acsm){
  	acsm->finalStateDepthGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));
	for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
	    acstate_t* transition = acsm->acsmNextState[state];
	    if(transition[1] == 0){
	      continue;
	    }

	    ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];
	    unsigned int maxDepth = 0;
	   	for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
			matchPattern != NULL; matchPattern = matchPattern->next){
	      
	      	if(matchPattern->n > maxDepth){
	        	maxDepth = matchPattern->n;
	      	}
	    }
	    acsm->finalStateDepthGroup[state] = (maxDepth << 1) + 1;

		//if(maxDepth != 0){
 		//	printf("state:%u, depth:%u \n", state, acsm->finalStateDepthGroup[state]);
		// }
	}
}

void caleDataTwiceOddEven(ACSM_STRUCT2* acsm){
	acsm->lastOneStateGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int*));
	acsm->lastTwoStateGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int*));
	acsm->lastOneCountGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int*));
	acsm->lastTwoCountGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int*));
	acsm->sizeLastOneState = calloc(acsm->acsmNumStates, sizeof(unsigned int));
	acsm->sizeLastTwoState = calloc(acsm->acsmNumStates, sizeof(unsigned int));

	for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
		acstate_t* transition = acsm->acsmNextState[state];
		if(transition[1] == 0){
			continue;
		}

		StateNumber* lastOneState = NULL;
		StateNumber* lastTwoState = NULL;

		// printf("State Id: %u \n", state);

		for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
			matchPattern != NULL; matchPattern = matchPattern->next){

			// printf("	Match Id: %u, Sub Id:%u \n", matchPattern->selfPatternId, matchPattern->subPatternId);
			unsigned int subPatternId = matchPattern->subPatternId;
			for(unsigned int subState = 0; subState < acsm->acsmNumStates; subState++){
				acstate_t* subTransition = acsm->acsmNextState[subState];
				if(subTransition[1] == 0){
					continue;
				}

				for(ACSM_PATTERN2* subMatch = acsm->acsmMatchList[subState]; 
					subMatch != NULL; subMatch = subMatch->next){

					if(subMatch->selfPatternId != subPatternId){
						continue;
					}

					// printf("		SubState Id: %u, SubMatch Id:%u, flag:%u, same:%u \n", subState, subMatch->selfPatternId, matchPattern->flag, matchPattern->same);

					if(matchPattern->flag != matchPattern->same){
						if(lastOneState == NULL){
							lastOneState = calloc(1, sizeof(StateNumber));
							lastOneState->value = subState;
						}else{
							StateNumber* tmpLastState = calloc(1, sizeof(StateNumber));
							tmpLastState->value = subState;
							tmpLastState->next = lastOneState;
							lastOneState = tmpLastState;
						}
					}else{
						if(lastTwoState == NULL){
							lastTwoState = calloc(1, sizeof(StateNumber));
							lastTwoState->value = subState;
						}else{
							StateNumber* tmpLastState = calloc(1, sizeof(StateNumber));
							tmpLastState->value = subState;
							tmpLastState->next = lastTwoState;
							lastTwoState = tmpLastState;
						}
					}					
				}
			}
		}

		sizeLinkList(lastOneState);
		uniqLinkList(lastOneState);
		acsm->sizeLastOneState[state] = sizeLinkList(lastOneState);
		acsm->lastOneStateGroup[state] = calloc(acsm->sizeLastOneState[state], sizeof(unsigned int));
		acsm->lastOneCountGroup[state] = calloc(acsm->sizeLastOneState[state], sizeof(unsigned int));

		unsigned int* stateArray = acsm->lastOneStateGroup[state];
		unsigned int* countArray = acsm->lastOneCountGroup[state];
		for(StateNumber* current = lastOneState; current != NULL; current = current->next){
			*(stateArray++) = current->value;
			*(countArray++) = current->count;
		}

		// for(unsigned int i = 0; i < acsm->sizeLastOneState[state]; i++){
		// 	printf("[%u, %u] \n", acsm->lastOneStateGroup[state][i], acsm->lastOneCountGroup[state][i]);
		// }

		sizeLinkList(lastTwoState);
		uniqLinkList(lastTwoState);
		acsm->sizeLastTwoState[state] = sizeLinkList(lastTwoState);
		acsm->lastTwoStateGroup[state] = calloc(acsm->sizeLastTwoState[state], sizeof(unsigned int));
		acsm->lastTwoCountGroup[state] = calloc(acsm->sizeLastTwoState[state], sizeof(unsigned int));

		stateArray = acsm->lastTwoStateGroup[state];
		countArray = acsm->lastTwoCountGroup[state];
		for(StateNumber* current = lastTwoState; current != NULL; current = current->next){
			*(stateArray++) = current->value;
			*(countArray++) = current->count;
		}

		// for(unsigned int i = 0; i < acsm->sizeLastTwoState[state]; i++){
		// 	printf("{%u, %u} \n", acsm->lastTwoStateGroup[state][i], acsm->lastTwoCountGroup[state][i]);
		// }
	}

	caleDataMaxLength(acsm);
}

void fullMatchTwiceOddEvenNew(ACSM_STRUCT2* acsm, unsigned char* Tstart, unsigned char* T, unsigned int state){
	// printf("	[INFO] Search State: %u \n", state);						//
	// for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 		//
	// 		matchPattern != NULL; matchPattern = matchPattern->next){		//
																			
	// 	printf("		[INFO] Origin Pattern: ");							//
	// 	for(unsigned int i = 0; i < matchPattern->origin_length; i++){		//
	// 		printf("%02x", matchPattern->origin_patrn[i]);					//
	// 	}																	//
	// 	printf("\n");														//
																			
	// 	printf("		[INFO] Main Pattern: ");							//
	// 	for(unsigned int i = 0; i < matchPattern->n; i++){					//
	// 		printf("%02x", matchPattern->patrn[i]);							//
	// 	}																	//
	// 	printf("\n");														//
																			
	// 	printf("		[INFO] Sub Pattern: ");								//
	// 	for(unsigned int i = 0; i < matchPattern->other_length; i++){		//
	// 		printf("%02x", matchPattern->otherPattern[i]);					//
	// 	}																	//
	// 	printf("\n");														//
	// }																	//

	unsigned char* distance = T - acsm->finalStateDepthGroup[state];
	unsigned char* subText = acsm->subText;
	
	// printf("		[INFO] Sub Text: ");									//
	// for(unsigned int i = 0; i < acsm->finalStateDepthGroup[state]; i++){	//
	// 	printf("%02x", distance[i]);										//
	// }																	//
	// printf("\n");														//
                                                                    		
	// printf("		[INFO] Text: %u \n", (int)(T - distance));      		// 
	// printf("		[INFO] SubText: %u \n", (int)(T - subText));    		// 

	if(distance > subText){
		subText = distance;
		acsm->subState = 0;
	}

	// printf("		[INFO] New SubText: %u \n", (int)(T - subText)); 					// 

	unsigned int subState = acsm->subState; 
	unsigned int charIndex = 0;
	unsigned int* transitionTable;
    unsigned int** transitionGroup = acsm->acsmNextState;

	for(; subText < T - 2; subText += 2){
		transitionTable = transitionGroup[subState];
		charIndex = subText[0];
		subState = transitionTable[2 + charIndex];

		// printf("		[INFO] Search Sub State: %02x, %u \n", charIndex, subState);	// 
	}

	transitionTable = transitionGroup[subState];
	if(transitionTable[1]){
		unsigned int* lastTwoState = acsm->lastTwoStateGroup[state]; 
		unsigned int lastTwoSize = acsm->sizeLastTwoState[state];

		for(unsigned int i = 0; i < lastTwoSize; i++){
			if(lastTwoState[i] == subState){
				unsigned int* lastTwoCount = acsm->lastTwoCountGroup[state];
				foundCounter += lastTwoCount[i];
				break;
			}
		}
	}

	charIndex = subText[0];
	subState = transitionTable[2 + charIndex];
	transitionTable = transitionGroup[subState];

	// printf("		[INFO] Search Sub State: %02x, %u \n", charIndex, subState); 		//

	if(transitionTable[1]){
		unsigned int* lastOneState = acsm->lastOneStateGroup[state]; 
		unsigned int lastOneSize = acsm->sizeLastOneState[state];

		for(unsigned int i = 0; i < lastOneSize; i++){
			if(lastOneState[i] == subState){
				unsigned int* lastOneCount = acsm->lastOneCountGroup[state];
				foundCounter += lastOneCount[i];
				break;
			}
		}
	}

	acsm->subState = subState;
	acsm->subText = subText;
	// printf("		[INFO] FoundCunter: %u \n", foundCounter); 							//
}