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
#include "method_odd_even_snort.h"

static void* CALLOC(ACSM_STRUCT2* acsm, size_t nitems, size_t size){
  acsm->other_memory += size * nitems;
  return calloc(nitems, size);
}

void initAcBeforeSearch(ACSM_STRUCT2* acsm){
  acsm->lastState = 0;
  acsm->countConitnue = 0;
  acsm->lastChar = 0;
  acsm->continueChar = 0;
  acsm->lastFoundPatternCount = 0;
  acsm->continueFlag = 0;
}

// static void caleDataHash(ACSM_STRUCT2* acsm){
//   ACSM_PATTERN2** oneHashTable = calloc(65536, sizeof(ACSM_PATTERN2*));
//   ACSM_PATTERN2** twoHashTable = calloc(65536, sizeof(ACSM_PATTERN2*));

//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//       acstate_t* transition = acsm->acsmNextState[state];
//       if(transition[1] == 0){
//         continue;
//       }

//       // ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];
//       unsigned int maxDepth = 0;
//       for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//           matchPattern != NULL; matchPattern = matchPattern->next){
          
//           unsigned char* otherPattern = matchPattern->otherPattern;
//           unsigned int otherLastIndex = matchPattern->other_length - 1;
//           unsigned int otherLastChar = otherPattern[otherLastIndex];
//           unsigned int index = (state ^ otherLastChar) ^ ((state << 4) ^ otherLastChar) & 0x0000ffff;
//           // printf("state:%08x, char:%02x, hashindex = %u\n", state, otherLastChar, index);

//           ACSM_PATTERN2** hashTable = NULL;
//           if(matchPattern->flag != matchPattern->same){
//             hashTable = oneHashTable + index;
//           }else if(matchPattern->flag == matchPattern->same){
//             hashTable = twoHashTable + index;
//           }

//           if(*hashTable == NULL){
//             *hashTable = calloc(1, sizeof(ACSM_PATTERN2));
//             memcpy(*hashTable, matchPattern, sizeof(ACSM_PATTERN2));
//             (*hashTable)->next = NULL;
//             (*hashTable)->ownState = state;
//           }else{
//             ACSM_PATTERN2* newEntry = calloc(1, sizeof(ACSM_PATTERN2));
//             memcpy(newEntry, matchPattern, sizeof(ACSM_PATTERN2));
//             newEntry->next = *hashTable;
//             newEntry->ownState = state;
//             *hashTable = newEntry;
//           }
//       }
//   }

//   unsigned int oneCount = 0;
//   unsigned int oneExist = 0;
//   for(unsigned int i = 0; i < 65536; i++){
//     ACSM_PATTERN2* matchPattern = oneHashTable[i];
//     if(matchPattern == NULL){
//       continue;
//     }

//     oneExist++;
//     for(; matchPattern != NULL; matchPattern = matchPattern->next){
//       oneCount++;
//     }
//   }

//   unsigned int twoCount = 0;
//   unsigned int twoExist = 0;
//   for(unsigned int i = 0; i < 65536; i++){
//     ACSM_PATTERN2* matchPattern = twoHashTable[i];
//     if(matchPattern == NULL){
//       continue;
//     }

//     twoExist++;
//     for(; matchPattern != NULL; matchPattern = matchPattern->next){
//       twoCount++;
//     }
//   }

//   // printf("Show One Total:%u, Exist:%u, Avg:%f \n", oneCount, oneExist, ((float)oneCount) / ((float)oneExist));
//   // printf("Show Two Total:%u, Exist:%u, Avg:%f \n", twoCount, twoExist, ((float)twoCount) / ((float)twoExist));

//   acsm->oneHashTable = oneHashTable;
//   acsm->twoHashTable = twoHashTable;
// }

// unsigned int globalCountChar;
// unsigned int globalPatternIndex;

// static void caleDataPattern(ACSM_STRUCT2* acsm){
//   unsigned int* maxLengthGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));
//   unsigned int* numberPatternGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//       acstate_t* transition = acsm->acsmNextState[state];
//       if(transition[1] == 0){
//         continue;
//       }

//       unsigned int maxDepth = 0;
//       unsigned int number = 0;
//       for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//           matchPattern != NULL; matchPattern = matchPattern->next){
          
//           number++;
//           if(matchPattern->n > maxDepth){
//             maxDepth = matchPattern->n;
//           }
//       }
//       maxLengthGroup[state] = 3 + (maxDepth << 1) + 1;
//       numberPatternGroup[state] = number;
//   }

//   short*** patternGroup = calloc(acsm->acsmNumStates, sizeof(short**));
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//       acstate_t* transition = acsm->acsmNextState[state];
//       if(transition[1] == 0){
//         continue;
//       }

//       unsigned int number = numberPatternGroup[state];
//       unsigned int numberIndex = 0;
//       short** patternList = calloc(number, sizeof(short*));

//       unsigned int patternIndex = 0;
//       unsigned int maxDepth = maxLengthGroup[state];      
//       for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//         matchPattern != NULL; matchPattern = matchPattern->next){
        
//         unsigned char* origin_patrn = matchPattern->origin_patrn;
//         unsigned int origin_length = matchPattern->origin_length;

//         short* pattern = calloc(maxDepth, sizeof(short));
//         for(int i = 0; i < maxDepth; i++){
//           pattern[i] = 300;
//         }

//         if(matchPattern->flag != matchPattern->same){
//           pattern[0] = maxDepth - 1;
//         }else{
//           pattern[0] = maxDepth - 3;
//         }

//         pattern[1] = maxDepth;

//         if(matchPattern->next != 0){
//           pattern[2] = 301;
//         }else{
//           pattern[2] = 302;
//         }        

//         unsigned int charIndex = 0;
//         if(matchPattern->flag != matchPattern->same){
//           charIndex = maxDepth - 1;
//         }else{
//           charIndex = maxDepth - 2;
//         }

//         for(int i = origin_length - 1; i >= 0; i--){
//           pattern[charIndex] = origin_patrn[i];
//           charIndex--;
//         }

//         patternList[patternIndex] = pattern;
//         patternIndex++;

//         // for(int i = 0; i < maxDepth; i++){
//         //   printf("%u,", pattern[i]);
//         // }
//         // printf("\n");
//       }

//       patternGroup[state] = patternList;
//       // printf("\n");
//   }

//   unsigned int countFinal = 0;
//   unsigned int countChar = 0;
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//     acstate_t* transition = acsm->acsmNextState[state];
//     if(transition[1] == 0){
//       continue;
//     }

//     unsigned int number = numberPatternGroup[state];
//     unsigned int maxDepth = maxLengthGroup[state];      
    
//     countFinal += number;
//     for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//       matchPattern != NULL; matchPattern = matchPattern->next){
      
//       countChar += number * maxDepth;
//     }
//   }

//   // printf("Count Final: %u \n", countFinal);
//   // printf("Count Char: %u \n", countChar);

//   short*** finalPatternGroup = calloc(acsm->acsmNumStates, sizeof(short**));
//   short** finalPatternListPool = calloc(countFinal, sizeof(short*));
//   short* finalPatternCharPool = calloc(countChar, sizeof(short));

//   // for(int i = 0; i < countFinal; i++){
//   //   printf("%p,", finalPatternListPool[i]);
//   // }
//   // printf("\n");

//   // for(int i = 0; i < countChar; i++){
//   //   printf("%u,", finalPatternCharPool[i]);
//   // }
//   // printf("\n\n");

//   globalCountChar = countChar;

//   unsigned int listIndex = 0;
//   unsigned int patternIndex = 0;
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//     acstate_t* transition = acsm->acsmNextState[state];
//     if(transition[1] == 0){
//       continue;
//     }

//     unsigned int number = numberPatternGroup[state];
//     unsigned int maxDepth = maxLengthGroup[state];      
//     unsigned int poolLength = number * maxDepth;

//     // printf("> %p \n", finalPatternGroup[state]);

//     finalPatternGroup[state] = finalPatternListPool + listIndex;
//     // printf("listIndex: %u \n", listIndex);
//     listIndex += number;

//     unsigned int internalPatternIndex = 0;
//     for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//       matchPattern != NULL; matchPattern = matchPattern->next){
      
//       // printf("%u, %u, %u, %u \n", acsm, state, countChar, patternIndex);
//       // b method_odd_even_snort.c:247 if countChar == 8686 && patternIndex == 6884

//       finalPatternGroup[state][internalPatternIndex] = finalPatternCharPool + patternIndex;
//       patternIndex += maxDepth;

//       internalPatternIndex++;
//       // for(int i = 0; i < maxDepth; i++){
//       //   finalPatternGroup[state][internalPatternIndex][i] = patternGroup[state][internalPatternIndex][i];
//       // }
//     }
//     // printf("\n");

//     // printf("< %p \n", finalPatternGroup[state]);

//     // for(int i = 0; i < countFinal; i++){
//     //   printf("%p,", finalPatternListPool[i]);
//     // }
//     // printf("\n");

//     // for(int i = 0; i < countChar; i++){
//     //   printf("%u,", finalPatternCharPool[i]);
//     // }
//     // printf("\n\n");
//   }

//   free(maxLengthGroup);
//   free(numberPatternGroup);


//   acsm->finalPatternGroup = finalPatternGroup;
//   // for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//   //   acstate_t* transition = acsm->acsmNextState[state];
//   //   if(transition[1] == 0){
//   //     continue;
//   //   }

//   //   unsigned int number = numberPatternGroup[state];
//   //   unsigned int maxDepth = maxLengthGroup[state];      
//   //   for(unsigned int i = 0; i < number; i++){
//   //     free(patternGroup[state][i]);
//   //   }
//   //   free(patternGroup[state]);
//   // }
//   // free(patternGroup);

//   // exit(0);
// }

// static void caleDataPattern(ACSM_STRUCT2* acsm){
//   unsigned int* maxLengthGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));
//   unsigned int* numberPatternGroup = calloc(acsm->acsmNumStates, sizeof(unsigned int));
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//       acstate_t* transition = acsm->acsmNextState[state];
//       if(transition[1] == 0){
//         continue;
//       }

//       unsigned int maxDepth = 0;
//       unsigned int number = 0;
//       for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//           matchPattern != NULL; matchPattern = matchPattern->next){
          
//           number++;
//           if(matchPattern->n > maxDepth){
//             maxDepth = matchPattern->n;
//           }
//       }
//       maxLengthGroup[state] = 3 + (maxDepth << 1) + 1;
//       numberPatternGroup[state] = number;
//   }

//   unsigned int xTotal = acsm->acsmNumStates;
//   unsigned int yTotal = 0;
//   unsigned int* ySizeGroup = calloc(xTotal, sizeof(unsigned int));
//   unsigned int zTotal = 0;
//   unsigned int* zSizeGroup = 0;
//   for(unsigned int i = 0; i < xTotal; i++){
//     acstate_t* transition = acsm->acsmNextState[state];
//     if(transition[1] == 0){
//       continue;
//     }

//     yTotal++;
//     unsigned int zSize = 0
//     for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//         matchPattern != NULL; matchPattern = matchPattern->next){
        
//       zSize++;
//     }

//     zTotal; += zSize + 2;
//     ySizeGroup[state] = ySize + 2;
//   }

//   short*** x = calloc(xTotal, sizeof(short**));
//   short** yPool = calloc(yTotal, sizeof(short*));


//   short*** patternGroup = calloc(acsm->acsmNumStates, sizeof(short**));
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//       acstate_t* transition = acsm->acsmNextState[state];
//       if(transition[1] == 0){
//         continue;
//       }

//       unsigned int number = numberPatternGroup[state];
//       unsigned int numberIndex = 0;
//       short** patternList = calloc(number, sizeof(short*));

//       unsigned int patternIndex = 0;
//       unsigned int maxDepth = maxLengthGroup[state];      
//       for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//         matchPattern != NULL; matchPattern = matchPattern->next){
        
//         unsigned char* origin_patrn = matchPattern->origin_patrn;
//         unsigned int origin_length = matchPattern->origin_length;

//         short* pattern = calloc(maxDepth, sizeof(short));
//         for(int i = 0; i < maxDepth; i++){
//           pattern[i] = 300;
//         }

//         if(matchPattern->flag != matchPattern->same){
//           pattern[0] = maxDepth - 1;
//         }else{
//           pattern[0] = maxDepth - 3;
//         }

//         pattern[1] = maxDepth;

//         if(matchPattern->next != 0){
//           pattern[2] = 301;
//         }else{
//           pattern[2] = 302;
//         }        

//         unsigned int charIndex = 0;
//         if(matchPattern->flag != matchPattern->same){
//           charIndex = maxDepth - 1;
//         }else{
//           charIndex = maxDepth - 2;
//         }

//         for(int i = origin_length - 1; i >= 0; i--){
//           pattern[charIndex] = origin_patrn[i];
//           charIndex--;
//         }

//         patternList[patternIndex] = pattern;
//         patternIndex++;

//         // for(int i = 0; i < maxDepth; i++){
//         //   printf("%u,", pattern[i]);
//         // }
//         // printf("\n");
//       }

//       patternGroup[state] = patternList;
//       // printf("\n");
//   }

//   unsigned int countFinal = 0;
//   unsigned int countChar = 0;
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//     acstate_t* transition = acsm->acsmNextState[state];
//     if(transition[1] == 0){
//       continue;
//     }

//     unsigned int number = numberPatternGroup[state];
//     unsigned int maxDepth = maxLengthGroup[state];      
    
//     countFinal += number;
//     for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//       matchPattern != NULL; matchPattern = matchPattern->next){
      
//       countChar += number * maxDepth;
//     }
//   }

//   // printf("Count Final: %u \n", countFinal);
//   // printf("Count Char: %u \n", countChar);

//   short*** finalPatternGroup = calloc(acsm->acsmNumStates, sizeof(short**));
//   short** finalPatternListPool = calloc(countFinal, sizeof(short*));
//   short* finalPatternCharPool = calloc(countChar, sizeof(short));

//   // for(int i = 0; i < countFinal; i++){
//   //   printf("%p,", finalPatternListPool[i]);
//   // }
//   // printf("\n");

//   // for(int i = 0; i < countChar; i++){
//   //   printf("%u,", finalPatternCharPool[i]);
//   // }
//   // printf("\n\n");

//   globalCountChar = countChar;

//   unsigned int listIndex = 0;
//   unsigned int patternIndex = 0;
//   for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//     acstate_t* transition = acsm->acsmNextState[state];
//     if(transition[1] == 0){
//       continue;
//     }

//     unsigned int number = numberPatternGroup[state];
//     unsigned int maxDepth = maxLengthGroup[state];      
//     unsigned int poolLength = number * maxDepth;

//     // printf("> %p \n", finalPatternGroup[state]);

//     finalPatternGroup[state] = finalPatternListPool + listIndex;
//     // printf("listIndex: %u \n", listIndex);
//     listIndex += number;

//     unsigned int internalPatternIndex = 0;
//     for(ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state]; 
//       matchPattern != NULL; matchPattern = matchPattern->next){
      
//       // printf("%u, %u, %u, %u \n", acsm, state, countChar, patternIndex);
//       // b method_odd_even_snort.c:247 if countChar == 8686 && patternIndex == 6884

//       finalPatternGroup[state][internalPatternIndex] = finalPatternCharPool + patternIndex;
//       patternIndex += maxDepth;

//       internalPatternIndex++;
//       // for(int i = 0; i < maxDepth; i++){
//       //   finalPatternGroup[state][internalPatternIndex][i] = patternGroup[state][internalPatternIndex][i];
//       // }
//     }
//     // printf("\n");

//     // printf("< %p \n", finalPatternGroup[state]);

//     // for(int i = 0; i < countFinal; i++){
//     //   printf("%p,", finalPatternListPool[i]);
//     // }
//     // printf("\n");

//     // for(int i = 0; i < countChar; i++){
//     //   printf("%u,", finalPatternCharPool[i]);
//     // }
//     // printf("\n\n");
//   }

//   free(maxLengthGroup);
//   free(numberPatternGroup);


//   acsm->finalPatternGroup = finalPatternGroup;
//   // for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
//   //   acstate_t* transition = acsm->acsmNextState[state];
//   //   if(transition[1] == 0){
//   //     continue;
//   //   }

//   //   unsigned int number = numberPatternGroup[state];
//   //   unsigned int maxDepth = maxLengthGroup[state];      
//   //   for(unsigned int i = 0; i < number; i++){
//   //     free(patternGroup[state][i]);
//   //   }
//   //   free(patternGroup[state]);
//   // }
//   // free(patternGroup);

//   // exit(0);
// }

void acsmCaleFullMatchSnort(ACSM_STRUCT2* acsm){
  acsm->repeatSizeGroup = CALLOC(acsm, acsm->acsmNumStates, sizeof(unsigned short));
  acsm->repeatCharGroup = CALLOC(acsm, acsm->acsmNumStates, sizeof(unsigned char*));
  acsm->repeatValueGroup = CALLOC(acsm, acsm->acsmNumStates, sizeof(unsigned char*));

  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
    ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];
    if(matchPattern == NULL){
      continue;
    }

    unsigned char* maxCharMapRepeat = calloc(256, sizeof(unsigned char));
    for(; matchPattern != NULL; matchPattern = matchPattern->next){
      unsigned char* origin_patrn = matchPattern->origin_patrn;
      unsigned int indexEnd = matchPattern->indexEnd;
      unsigned char repeatChar = origin_patrn[indexEnd];

      unsigned char* charMapRepeat = calloc(256, sizeof(unsigned char));
      if(matchPattern->flag == matchPattern->same){
        for(int i = indexEnd; i >= 0; i -= 2){
          unsigned char index = origin_patrn[i];
          if(index != repeatChar){
            break;
          }

          if(charMapRepeat[index] == 0){
            charMapRepeat[index] = 2;
          }else{
            charMapRepeat[index] += 1;
          }
        }
      }else{
        for(int i = indexEnd; i >= 0; i -= 2){
          unsigned char index = origin_patrn[i];
          if(index != repeatChar){
            break;
          }

          charMapRepeat[index] += 1;
        }
      }

      for(unsigned int i = 0; i < 256; i++){
        if(charMapRepeat[i] > maxCharMapRepeat[i]){
          maxCharMapRepeat[i] = charMapRepeat[i];
        }
      }
    }
    
    unsigned char countRepeatTwice = 0;
    for(unsigned int i = 0; i < 256; i++){
      if(maxCharMapRepeat[i] > 1){
        countRepeatTwice++;
      }
    }

    unsigned char* repeatCharList = CALLOC(acsm, countRepeatTwice, sizeof(unsigned char));
    unsigned char* repeatValueList = CALLOC(acsm, countRepeatTwice, sizeof(unsigned char));
    unsigned int repeatIndex = 0;
    for(unsigned int i = 0; i < 256; i++){
      if(maxCharMapRepeat[i] > 1){
        repeatCharList[repeatIndex] = i;
        repeatValueList[repeatIndex] = maxCharMapRepeat[i];
        repeatIndex++;
      }
    }

    // printf("%u\n", countRepeatTwice);
    acsm->repeatSizeGroup[state] = countRepeatTwice;
    acsm->repeatCharGroup[state] = repeatCharList;
    acsm->repeatValueGroup[state] = repeatValueList;
  }
  // caleDataHash(acsm);
  // caleDataPattern(acsm);
}

void fullMatchSnort(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
  // printf("state: %u, last_char: %02x, counter: %u\n", state, acsm->lastChar, acsm->countConitnue);
  acsm->countFullMatch++;
  unsigned char* textEnd = acsm->textEnd;

  if(textEnd > T){
    if(acsm->lastState != state){
      acsm->countConitnue = 0;
      acsm->continueFlag = 0;
    }else{
       acsm->continueChar = *(T - 1);
      if(acsm->lastChar != acsm->continueChar){
        acsm->lastChar = acsm->continueChar;
        acsm->countConitnue = 1;
        acsm->continueFlag = 0;
      }else{
        acsm->countConitnue++;

        if(acsm->continueFlag){
          foundCounter += acsm->lastFoundPatternCount;
          acsm->countRepetitionVisit++;
          acsm->repetitionUsingTable[acsm->continueChar] = 1;
          // printf("# %lu, %u, %02x, %u \n", ((T - Tstart) * 2) + 1, foundCounter, acsm->continueChar, acsm->countConitnue);
          return;
        }

        unsigned short repeatSize = acsm->repeatSizeGroup[state];
        unsigned char* repeatCharList = acsm->repeatCharGroup[state];
        unsigned char* repeatValueList = acsm->repeatValueGroup[state];
        unsigned int isFoundRepetition = 0;
        for(unsigned int i = 0; i < repeatSize; i++){
			if(acsm->continueChar == repeatCharList[i]){
				isFoundRepetition == 1;
				if(acsm->countConitnue > repeatValueList[i]){
					// printf("%u\n", countConitnue);
					foundCounter += acsm->lastFoundPatternCount;
					// printf("+ %lu, %u, %02x, %u \n", ((T - Tstart) * 2) + 1, foundCounter, acsm->continueChar, acsm->countConitnue);
					acsm->continueFlag = 1;
					acsm->countRepetitionVisit++;
					acsm->repetitionUsingTable[acsm->continueChar] = 1;
					return;
				}
			}
        }

        if(isFoundRepetition == 0 && acsm->countConitnue > 2){
        	acsm->countRepetitionVisit++;
        	acsm->repetitionUsingTable[acsm->continueChar] = 1;
        	return;
        }
      }
    }
  }

  // printf("state: %u, continue; \n");

  // ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
  
  // printf("\nstate: %u\n", state);

  // unsigned short repeatSize = acsm->repeatSizeGroup[state];
  // unsigned char* repeatCharList = acsm->repeatCharGroup[state];
  // unsigned char* repeatValueList = acsm->repeatValueGroup[state];
  // for(unsigned int i = 0; i < repeatSize; i++){
  //   printf("%02x, %u \n", repeatCharList[i], repeatValueList[i]);
  // }

  // unsigned int lastIndexEnd = 0;
  // for(; MatchList != NULL; MatchList = MatchList->next){
  //   unsigned int indexEnd = MatchList->indexEnd;
  //   if(indexEnd > lastIndexEnd){
  //     lastIndexEnd = indexEnd;
  //   }
  // }

  // MatchList = acsm->acsmMatchList[state];
  // for(; MatchList != NULL; MatchList = MatchList->next){
  //   unsigned char* origin_patrn = MatchList->origin_patrn;
  //   unsigned int indexEnd = MatchList->indexEnd;

  //   if(MatchList->flag == MatchList->same){
  //     printf("..");
  //   }
  //   for(int i = indexEnd; i >= 0; i -= 2){
  //     printf("%02x", origin_patrn[i]);
  //   }
  //   printf("\n");

  //   unsigned char* start = T - MatchList->start;
  //   if(MatchList->flag == MatchList->same){
  //     printf("%02x", (start-1)[0]);
  //   }
  //   for(int i = indexEnd; i >= 0; i -= 2){
  //     printf("%02x", start[i]);
  //     if(origin_patrn[i] != start[i]){
  //       break;
  //     }
  //   }
  //   printf("\n");
  // }

  acsm->lastFoundPatternCount = 0;
  
  ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
  for(; MatchList != NULL; MatchList = MatchList->next){
  	acsm->countRestPatternComparison++;
    unsigned char* origin_patrn = MatchList->origin_patrn;
    unsigned char* start = T - MatchList->start;
    unsigned int indexEnd = MatchList->indexEnd;

    if(start < Tstart){
      continue;
    }

    if(start + indexEnd >= textEnd){
      continue;
    }

    int isFull = 1;
    for(int i = indexEnd; i >= 0; i -= 2){
      acsm->countCharacterComparison++;
      if(origin_patrn[i] != start[i]){
        isFull = 0;
        break;
      }
    }

    if(isFull){
      foundCounter++;
      acsm->lastFoundPatternCount++;
      // printf("= %lu, %u\n", ((T - Tstart) * 2) + 1, foundCounter);
      // printf("Pattern Id: %u\n", MatchList->iid);
    }

    // if(!memcmp(start, origin_patrn, MatchList->origin_length)){
    //   foundCounter++;
    // }
  }

  // printf("state: %u, %u\n", state, foundCounter);
}

void fullMatchOrigin(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
  unsigned char* textEnd = acsm->textEnd;

  ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
  for(; MatchList != NULL; MatchList = MatchList->next){
    acsm->countRestPatternComparison++;
    unsigned char* origin_patrn = MatchList->origin_patrn;
    unsigned char* start = T - MatchList->start;
    unsigned int indexEnd = MatchList->indexEnd;

    if(start < Tstart){
      continue;
    }

    if(start + indexEnd >= textEnd){
      continue;
    }

    int isFull = 1;
    for(int i = indexEnd; i >= 0; i -= 2){
    	acsm->countCharacterComparison++;
	    if(origin_patrn[i] != start[i]){
	    	isFull = 0;
	        break;
	    }
    }

    if(isFull){
      foundCounter++;
      acsm->lastFoundPatternCount++;
    }
  }
}

// void fullMatchSnort(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
//   unsigned char* textEnd = acsm->textEnd;

//   if(textEnd > T){
//     if(acsm->lastState != state){
//       acsm->countConitnue = 0;
//       acsm->continueFlag = 0;
//     }else{
//        acsm->continueChar = *(T - 1);
//       if(acsm->lastChar != acsm->continueChar){
//         acsm->lastChar = acsm->continueChar;
//         acsm->countConitnue = 1;
//         acsm->continueFlag = 0;
//       }else{
//         acsm->countConitnue++;

//         if(acsm->continueFlag){
//           foundCounter += acsm->lastFoundPatternCount;
//           // printf("# %lu, %u, %02x, %u \n", ((T - Tstart) * 2) + 1, foundCounter, acsm->continueChar, acsm->countConitnue);
//           return;
//         }

//         unsigned short repeatSize = acsm->repeatSizeGroup[state];
//         unsigned char* repeatCharList = acsm->repeatCharGroup[state];
//         unsigned char* repeatValueList = acsm->repeatValueGroup[state];
//         for(unsigned int i = 0; i < repeatSize; i++){
//           if(acsm->continueChar == repeatCharList[i]){
//             if(acsm->countConitnue > repeatValueList[i]){
//               // printf("%u\n", countConitnue);
//               foundCounter += acsm->lastFoundPatternCount;
//               // printf("+ %lu, %u, %02x, %u \n", ((T - Tstart) * 2) + 1, foundCounter, acsm->continueChar, acsm->countConitnue);
//               acsm->continueFlag = 1;
//               return;
//             }
//           }
//         }
//       }
//     }
//   }


//   acsm->lastFoundPatternCount = 0;
//   unsigned char oneChar = *(T - 1);
//   unsigned char twoChar = *(T - 3);
//   unsigned int oneIndex = (state ^ oneChar) ^ ((state << 4) ^ oneChar) & 0x0000ffff;
//   unsigned int twoIndex = (state ^ twoChar) ^ ((state << 4) ^ twoChar) & 0x0000ffff;

//   ACSM_PATTERN2* MatchList = acsm->oneHashTable[oneIndex];
//   for(; MatchList != NULL; MatchList = MatchList->next){
//     if(MatchList->ownState == state){
//       unsigned char* origin_patrn = MatchList->origin_patrn;
//       unsigned char* start = T - MatchList->start;
//       unsigned int indexEnd = MatchList->indexEnd;

//       if(start < Tstart){
//         continue;
//       }

//       if(start + indexEnd >= textEnd){
//         continue;
//       }

//       int isFull = 1;
//       for(int i = indexEnd; i >= 0; i -= 2){
//         if(origin_patrn[i] != start[i]){
//           isFull = 0;
//           break;
//         }
//       }

//       if(isFull){
//         foundCounter++;
//         acsm->lastFoundPatternCount++;
//         // printf("[INFO] One Pattern Id: %u \n", MatchList->iid);
//       }
//     }else{
//       continue;
//     }
//   }

//   MatchList = acsm->twoHashTable[twoIndex];
//   for(; MatchList != NULL; MatchList = MatchList->next){
//     if(MatchList->ownState == state){
//       unsigned char* origin_patrn = MatchList->origin_patrn;
//       unsigned char* start = T - MatchList->start;
//       unsigned int indexEnd = MatchList->indexEnd;

//       if(start < Tstart){
//         continue;
//       }

//       if(start + indexEnd >= textEnd){
//         continue;
//       }

//       int isFull = 1;
//       for(int i = indexEnd; i >= 0; i -= 2){
//         if(origin_patrn[i] != start[i]){
//           isFull = 0;
//           break;
//         }
//       }

//       if(isFull){
//         foundCounter++;
//         acsm->lastFoundPatternCount++;
//         // printf("[INFO] Two Pattern Id: %u \n", MatchList->iid);
//       }
//     }else{
//       continue;
//     }
//   }
// }