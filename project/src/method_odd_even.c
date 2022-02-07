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
#include "method_odd_even.h"

unsigned int* maxLengthGroup;
ACSM_PATTERN2** maxPatternGroup;

typedef struct Pattern{
  unsigned int id;
  unsigned int count;
  unsigned int exact;
} Pattern;

Pattern* patternIdGroup;
unsigned char** patternGroup;
unsigned char** hexPatternGroup;
unsigned int numberOfPattern;

void initFullMatchOddEven(unsigned int patternSize, unsigned char** patternList, unsigned char** hexPatternList){
  numberOfPattern = patternSize;
  patternIdGroup = calloc(patternSize, sizeof(Pattern));
  patternGroup = patternList;
  hexPatternGroup = hexPatternList;

  for(unsigned int i = 0; i < patternSize; i++){
    patternIdGroup[i].id = i;
  }
}

void acsmCaleFullMatch(ACSM_STRUCT2* acsm){
  maxLengthGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));
  maxPatternGroup = calloc(acsm->acsmNumStates, sizeof(ACSM_PATTERN2*));

  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1] == 0){
      continue;
    }

    ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];
    unsigned int maxLength = 0;
    ACSM_PATTERN2* maxPattern = NULL;
    while(matchPattern != NULL){
      if(matchPattern->origin_length > maxLength){
        maxLength = matchPattern->origin_length;
        maxPattern = matchPattern;
      }
      matchPattern = matchPattern->next;
    }
    maxLengthGroup[state] = maxLength;
    maxPatternGroup[state] = maxPattern;
  }
}

void full_match(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
  ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
  
  // printf("[INFO] MaxLength: %u, Pattern: ", maxLengthGroup[state]);
  // ACSM_PATTERN2* maxPattern = maxPatternGroup[state];
  // for(int i = 0; i < maxPattern->n; i++){
  //   printf("%02x", maxPattern->patrn[i]);
  // }
  // printf("\n");

  for(; MatchList != NULL; MatchList = MatchList->next){
    countFullMatchTimes++;
    countSubPatternAccess++;

    patternIdGroup[MatchList->iid].count++;
    unsigned char* origin_patrn = MatchList->origin_patrn;

    unsigned char* start = T - MatchList->start;
    unsigned int indexEnd = MatchList->indexEnd;

    int isFull = 1;
    for(int i = indexEnd; i >= 0; i -= 2){
      countCharacterAccess++;
      countCharacterAccess++;
      countCompareCharater++; 
      if(origin_patrn[i] != start[i]){
        isFull = 0;
        break;
      }
    }

    // int isFull = 1;
    // int i = flag == 0;
    // for(; i < origin_length; i += 2){
    //   countCharacterAccess++;
    //   countCharacterAccess++;

    //   if(origin_patrn[i] != start[i]){
    //     isFull = 0;
    //     break;
    //   }
    // }

    if(isFull){
      countFullMatchSuccess++;
      patternIdGroup[MatchList->iid].exact++;
      foundCounter++;
    }
  }
}

static int compare(const void *s1, const void *s2){
    return ((Pattern*)s2)->count - ((Pattern*)s1)->count;
}

static int compareExact(const void *s1, const void *s2){
    return ((Pattern*)s2)->exact - ((Pattern*)s1)->exact;
}

void showOddEvenResult(){
  qsort(patternIdGroup, numberOfPattern, sizeof(Pattern), compare);
  for(unsigned int i = 0; i < 50; i++){
    printf("[INFO] Most Full Match Pattern:%u, Count:%u, Exact:%u, Hex:%s \n", patternIdGroup[i].id, patternIdGroup[i].count,  patternIdGroup[i].exact, hexPatternGroup[patternIdGroup[i].id]);
  }

  qsort(patternIdGroup, numberOfPattern, sizeof(Pattern), compareExact);
  for(unsigned int i = 0; i < 50; i++){
    printf("[INFO] Most Exact Match Pattern:%u, Count:%u, Exact:%u, Hex:%s \n", patternIdGroup[i].id, patternIdGroup[i].count,  patternIdGroup[i].exact, hexPatternGroup[patternIdGroup[i].id]);
  }
}