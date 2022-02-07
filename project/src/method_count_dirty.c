#include "method_count_dirty.h"
#include "aho_corasick.h"
#include "statistic.h"

void acsmCaleCountDirty(ACSM_STRUCT2* acsm){
  acsm->maxLengthGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));

  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1] == 0){
      continue;
    }

    ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];
    unsigned int maxLength = 0;
    ACSM_PATTERN2* maxPattern = NULL;
    while(matchPattern != NULL){
      if(matchPattern->n > maxLength){
        maxLength = matchPattern->n;
        maxPattern = matchPattern;
      }
      matchPattern = matchPattern->next;
    }
    acsm->maxLengthGroup[state] = maxLength;
    // if(maxLength != 0){
    //   printf("%u\n", maxLength);
    // }
  }
}

void fulllMatchCountDirty(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	foundCounter++;

	if(acsm->lastPosition == T - Tstart){
		return;
	}
  // countDirtyCharacter += acsm->acsmMatchList[state]->n;
  // acsm->lastPosition = T;

  unsigned int offset = T - Tstart;
	unsigned int distance = offset - acsm->lastPosition;
  // printf("%u: %p, %p, %d, %d\n", distance, Tstart, T, offset, acsm->lastPosition);

	unsigned int length = acsm->maxLengthGroup[state];
	if(length < distance){
		countDirtyCharacter += length;
	}else{
		countDirtyCharacter += distance;
	}
	acsm->lastPosition = T - Tstart;

	// printf("[INFO] Pattern Id: %u \n", acsm->acsmMatchList[state]->iid);
}