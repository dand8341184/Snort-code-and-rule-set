#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "profile.h"
#include "aho_corasick.h"

unsigned int countSumOfPatternOnFinalState = 0;

unsigned int countMaxOfPattern1st = 0;
unsigned int countMaxOfPattern2nd = 0;
unsigned int countMaxOfPattern3rd = 0;

unsigned int stateMaxOfPattern1st = 0;
unsigned int stateMaxOfPattern2nd = 0;
unsigned int stateMaxOfPattern3rd = 0;

unsigned int countMinOfPattern1st = -1;
unsigned int countMinOfPattern2nd = -1;
unsigned int countMinOfPattern3rd = -1;

unsigned int stateMinOfPattern1st = 0;
unsigned int stateMinOfPattern2nd = 0;
unsigned int stateMinOfPattern3rd = 0;

unsigned int countSumOfChar = 0;
unsigned int countMaxOfChar = 0;
unsigned int countMinOfChar = -1;

unsigned int stateMaxOfChar = 0;
unsigned int stateMinOfChar = 0;

double averagePatternOnFinalState = 0;
double averageChar = 0;

void countTotalCharaterOfPattern(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  unsigned int count = 0;

  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int length = patternLengthGroup[i];
    count += length;
  }

  printf("[INFO] Count total characters: %u \n", count);
}

void countMostDistinctOfPattern(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  unsigned int maxDistinct = 0;

  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int charMap[256];
    for(unsigned int j = 0; j < 256; j++){
      charMap[j] = 0;
    }

    unsigned int length = patternLengthGroup[i];
    for(unsigned int j = 0; j < length; j++){
      unsigned char index = patternGroup[i][j];
      charMap[index] = 1;
    }

    unsigned int numberOfDistinct = 0;
    for(unsigned int j = 0; j < 256; j++){
      numberOfDistinct += charMap[j];
    }

    if(numberOfDistinct > maxDistinct){
      maxDistinct = numberOfDistinct;
    }
  }

  printf("[INFO] The most distinct characters in each pattern: %u \n", maxDistinct);
}

void countPatternLengthDistribution(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  unsigned long long int maxLength = 0;
  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int length = patternLengthGroup[i];

    if(length > maxLength){
      maxLength = length;
    }
  }

  unsigned int* lengthDistribution = calloc(maxLength, sizeof(unsigned int));
  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int index = patternLengthGroup[i] - 1;
    lengthDistribution[index]++; 
  }

  printf("[INFO] Show pattern length distribution.");
  for(unsigned int i = 0; i < maxLength; i++){
    printf("%u\n", lengthDistribution[i]);
  }
}

void countAvgLengthOfPattern(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  unsigned long long int sum = 0;

  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int length =  patternLengthGroup[i];
    sum += length;
  }

  double avg = sum / (double)numberOfPattern;
  printf("[INFO] The avg length of pattern: %f \n", avg);
}

void countMinLengthOfPattern(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  unsigned int minLength = -1;
  unsigned int minLengthIndex = 0;

  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int length = patternLengthGroup[i];
    if(length < minLength){
      minLength = length;
      minLengthIndex = i;
    }
  }

  printf("[INFO] The min length of pattern: %u \n", minLength);
}

void countMaxLengthOfPattern(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  unsigned int maxLength = 0;
  unsigned int maxLengthIndex = 0;

  for(unsigned int i = 0; i < numberOfPattern; i++){
    unsigned int length = patternLengthGroup[i];
    if(length > maxLength){
      maxLength = length;
      maxLengthIndex = i;
    }
  }

  printf("[INFO] The max length of pattern: %u \n", maxLength);
}

void countNumberOfPattern(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  printf("[INFO] Number of pattern: %u \n", numberOfPattern);
}

void profilePatterns(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup){
  countTotalCharaterOfPattern(numberOfPattern, patternGroup, patternLengthGroup);
  countNumberOfPattern(numberOfPattern, patternGroup, patternLengthGroup);
  countMaxLengthOfPattern(numberOfPattern, patternGroup, patternLengthGroup);
  countMinLengthOfPattern(numberOfPattern, patternGroup, patternLengthGroup);
  countAvgLengthOfPattern(numberOfPattern, patternGroup, patternLengthGroup);
  countPatternLengthDistribution(numberOfPattern, patternGroup, patternLengthGroup);
  countMostDistinctOfPattern(numberOfPattern, patternGroup, patternLengthGroup);
}

void countNumberOfPatternInFinalStateDistribution(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      
      unsigned int numberOfPattern = 0;
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        numberOfPattern++;
      }

      if(numberOfPattern > count){
        count = numberOfPattern;
      }
    }
  }

  unsigned int* numberDistribution = calloc(count, sizeof(unsigned int));
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      
      unsigned int numberOfPattern = 0;
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        numberOfPattern++;
      }

      unsigned int index = numberOfPattern - 1;
      numberDistribution[index]++;
    }
  }

  printf("[INFO] Show number of pattern in final state distribution.\n");
  for(unsigned int i = 0; i < count; i++){
    printf("%u\n", numberDistribution[i]);
  }
}

void countCharaterOfFullMatch(ACSM_STRUCT2* acsm){
  unsigned long long int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        count += MatchList->n;
      }
    }
  }

  printf("[INFO] Count Characters of fulll match: %llu \n", count);
}

void countAvgNumberOfPatternInFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      
      unsigned int numberOfPattern = 0;
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        numberOfPattern++;
      }

      count += numberOfPattern;
    }
  }

  printf("[INFO] Count Avg Number of pattern in final state: %f \n", count / (double)acsm->acsmNumStates);
}

void countMinNumberOfPatternInFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = -1;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      
      unsigned int numberOfPattern = 0;
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        numberOfPattern++;
      }

      if(numberOfPattern < count){
        count = numberOfPattern;
      }
    }
  }

  printf("[INFO] Count Min Number of pattern in final state: %u \n", count);
}

void countMaxNumberOfPatternInFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      
      unsigned int numberOfPattern = 0;
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        numberOfPattern++;
      }

      if(numberOfPattern > count){
        count = numberOfPattern;
      }
    }
  }

  printf("[INFO] Count Max Number of pattern in final state: %u \n", count);
}

void countNumberOfPatternInFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        count++;
      }
    }
  }

  printf("[INFO] Count Number of pattern in final state: %u \n", count);
}

void countDepthOfFinalStateDistribution(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      if(acsm->acsmDepth[state] > count){
        count = acsm->acsmDepth[state];
      }
    }
  }

  unsigned int* depthDistribution = calloc(count, sizeof(unsigned int));
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      unsigned int index = acsm->acsmDepth[state] - 1;
      depthDistribution[index]++;
    }
  }

  printf("[INFO] Show depth of final state distribution.\n");
  for(unsigned int i = 0; i < count; i++){
    printf("%u\n", depthDistribution[i]);
  }
}

void countAvgDepthOfFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      count += acsm->acsmDepth[state];
    }
  }

  printf("[INFO] Count Avg depth of final state: %f \n", count / (double)acsm->acsmNumStates);
}

void countMinDepthOfFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = -1;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      if(acsm->acsmDepth[state] < count){
        count = acsm->acsmDepth[state];
      }
    }
  }

  printf("[INFO] Count Min depth of final state: %u \n", count);
}

void countMaxDepthOfFinalState(ACSM_STRUCT2* acsm){
  unsigned int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      if(acsm->acsmDepth[state] > count){
        count = acsm->acsmDepth[state];
      }
    }
  }

  printf("[INFO] Count Max depth of final state: %u \n", count);
}

void countLeafOfState(ACSM_STRUCT2* acsm){
  unsigned long long int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    unsigned int isLeafNode = 1;

    for(unsigned int i = 0; i < 256; i++){
      if(acsm->acsmTrans[state][i] == 1){
        isLeafNode = 0;
        break;
      }
    }

    if(isLeafNode){
      count++;
    }
  }

  printf("[INFO] Count number of leaf state: %llu \n", count);
}

void countRootOfTransition(ACSM_STRUCT2* acsm){
  unsigned long long int count = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    for(unsigned int i = 0; i < 256; i++){
      acstate_t index = acsm->acsmNextState[state][2 + i];
      if(acsm->acsmDepth[index] == 1){
        count++;
      }
    }
  }

  printf("[INFO] Count number of root transition: %llu \n", count);
}

void countBackOfTransition(ACSM_STRUCT2* acsm){
  unsigned long long int count = 0;

  unsigned int transitionMap[acsm->acsmNumStates];

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    
    for(unsigned int i = 0; i < acsm->acsmNumStates; i++){
      transitionMap[i] = 0;
    }  

    for(unsigned int i = 0; i < 256; i++){
      if(acsm->acsmTrans[state][i] == 0){
        acstate_t index = acsm->acsmNextState[state][2 + i];
        transitionMap[index] = 1;
      }
    }

    for(unsigned int i = 0; i < acsm->acsmNumStates; i++){
      count = count + transitionMap[i];
    }
  }

  printf("[INFO] Count number of back transition: %llu \n", count);
}


void countNextOfTransition(ACSM_STRUCT2* acsm){
  unsigned long long int count = 0;

  unsigned int transitionMap[acsm->acsmNumStates];

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    
    for(unsigned int i = 0; i < acsm->acsmNumStates; i++){
      transitionMap[i] = 0;
    }  

    for(unsigned int i = 0; i < 256; i++){
      if(acsm->acsmTrans[state][i] == 1){
        acstate_t index = acsm->acsmNextState[state][2 + i];
        transitionMap[index] = 1;
      }
    }

    for(unsigned int i = 0; i < acsm->acsmNumStates; i++){
      count = count + transitionMap[i];
    }
  }

  printf("[INFO] Count number of next transition: %llu \n", count);
}

void countNumberOfTransition(ACSM_STRUCT2* acsm){
  unsigned long long int count = 0;

  unsigned int transitionMap[acsm->acsmNumStates];

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    
    for(unsigned int i = 0; i < acsm->acsmNumStates; i++){
      transitionMap[i] = 0;
    }  

    for(unsigned int i = 0; i < 256; i++){
      acstate_t index = acsm->acsmNextState[state][2 + i];
      transitionMap[index] = 1;
    }

    for(unsigned int i = 0; i < acsm->acsmNumStates; i++){
      count = count + transitionMap[i];
    }
  }

  printf("[INFO] Count number of transition: %llu \n", count);
}

void countNumberOfFinalState(ACSM_STRUCT2 *acsm){
  unsigned int counter = 0;

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      counter++;
    }
  }

  printf("[INFO] Count number of final state: %u \n", counter);
}

void countNumberOfState(ACSM_STRUCT2 *acsm){
  printf("[INFO] Count number of state: %u \n", acsm->acsmNumStates);
}

void profileFinalState(ACSM_STRUCT2 *acsm){
  countNumberOfState(acsm);
  countNumberOfFinalState(acsm);
  countNumberOfTransition(acsm);
  countNextOfTransition(acsm);
  countBackOfTransition(acsm);
  countRootOfTransition(acsm);
  countLeafOfState(acsm);
  countMaxDepthOfFinalState(acsm);
  countMinDepthOfFinalState(acsm);
  countAvgDepthOfFinalState(acsm);

  countDepthOfFinalStateDistribution(acsm);
  countNumberOfPatternInFinalState(acsm);
  countMaxNumberOfPatternInFinalState(acsm);
  countMinNumberOfPatternInFinalState(acsm);
  countAvgNumberOfPatternInFinalState(acsm);
  countCharaterOfFullMatch(acsm);
  countNumberOfPatternInFinalStateDistribution(acsm);
}

void showRemainText(ACSM_STRUCT2 *acsm){
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    ACSM_PATTERN2 *MatchList = acsm->acsmMatchList[state];
    
    if(MatchList == NULL){
        continue;
    }

    for(; MatchList != NULL; MatchList = MatchList->next){
      printf("%u,%u,", MatchList->flag == 1, MatchList->same == 1);
      if(MatchList->flag == 0){
        for(int i = 1; i < MatchList->origin_length; i += 2){
          printf("%02x", MatchList->origin_patrn[i]);
        }
      }else{
        for(int i = 0; i < MatchList->origin_length; i += 2){
          printf("%02x", MatchList->origin_patrn[i]);
        }
      }
      printf(",");
      for(unsigned int i = 0; i < MatchList->origin_length; i++){
        printf("%02x", MatchList->origin_patrn[i]);
      }

      printf(",");
      printf("match_pointor=%p", MatchList);
      printf(",");
      printf("state=%u", state);
      printf(",");
      printf("state_pointor=%p", acsm->acsmMatchList[state]);
      printf("\n");
    }
    printf("\n");
  }
}

void showAllStateTransition(ACSM_STRUCT2 *acsm){
  printf("STATE,TRANSITION,NEXT_STATE\n");
  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
    for(unsigned int i = 0; i < 256; i++){
      unsigned int nextState = acsm->acsmNextState[state][2 + i];
      if(nextState != 0){
        printf("%u,%u,%u\n", state, i, acsm->acsmNextState[state][2 + i]);
      }
    }
  }
}

void showExactWildcard(ACSM_STRUCT2 *acsm){
  unsigned int countWildcard = 0;
  unsigned int countExact = 0;

  unsigned int countFinalState = 0;
  unsigned int countWildcardState = 0;
  unsigned int countExactState = 0;

  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
    ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
    
    if(MatchList == NULL){
        continue;
    }

    unsigned int countInnerWildcard = 0;
    unsigned int countInnerExact = 0;
    while(MatchList != NULL){
      if(MatchList->wildcard){
        countWildcard++;
        countInnerWildcard++;
      }else{
        countExact++;
        countInnerExact++;
      }
      MatchList = MatchList->next;
    }

    countFinalState++;
    if(countInnerWildcard == 0){
      countExactState++;
    }else if(countInnerExact == 0){
      countWildcardState++;
    }
  }

  printf("[INFO] Count Wildcard Sub Pattern: %u \n", countWildcard);
  printf("[INFO] Count Exact Sub Pattern: %u \n", countExact);

  printf("[INFO] Count State: %u \n", acsm->acsmNumStates);
  printf("[INFO] Count Final State: %u \n", countFinalState);
  printf("[INFO] Count Wildcard State: %u \n", countWildcardState);
  printf("[INFO] Count Exact State: %u \n", countExactState);
  printf("[INFO] Count Mix State: %u \n", countFinalState - countWildcardState - countExactState);
}

void showExtremeSubPattern(ACSM_STRUCT2 *acsm){
  unsigned int countExtreme = 0;

  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){  
    ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
    
    if(MatchList == NULL){
        continue;
    }

    while(MatchList != NULL){
      if(MatchList->same == 0){
      }else if(MatchList->flag == 1){
      }else if(MatchList->origin_length - MatchList->n == 1){
        countExtreme++;
      }
      MatchList = MatchList->next;
    }
  }

  printf("[INFO] Count Extreme Sub Pattern: %u \n", countExtreme);
}