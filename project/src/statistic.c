#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "statistic.h"
#include "method_bitmap_tree.h"

// unsigned int patternSize = 0;
// unsigned int* patternIdList;

unsigned int foundCounter = 0;

unsigned long long int countStateAccess = 0;
unsigned long long int countNextStateAccess = 0;
unsigned long long int countCharacterAccess = 0;
unsigned long long int countSubPatternAccess = 0;

unsigned long long int countFullMatchTimes = 0;
unsigned long long int countFullMatchSuccess = 0;
unsigned long long int countFinalState = 0;
unsigned long long int countDirtyCharacter = 0;

unsigned long long int countCompareCharater = 0;

void showStatisticResult(){
	printf("[INFO] Count Found : %u \n", foundCounter);
	printf("[INFO] Count State Access: %llu \n", countStateAccess);
	printf("[INFO] Count NextState Access: %llu \n", countNextStateAccess);
	printf("[INFO] Count Character Access: %llu \n", countCharacterAccess);
	printf("[INFO] Count Sub Pattern Access: %llu \n", countSubPatternAccess);
	unsigned long long int totalAccess = countStateAccess + countNextStateAccess + countCharacterAccess + countSubPatternAccess;
	printf("[INFO] Count Total Memory Access: %llu \n", totalAccess);

	printf("\n");
	printf("[INFO] Count Full Match Times: %llu \n", countFullMatchTimes);
	printf("[INFO] Count Full Match Success: %llu \n", countFullMatchSuccess);
	printf("[INFO] Count Compare Charater: %llu \n", countCompareCharater);
	printf("[INFO] Count Dirty Charater: %llu \n", countDirtyCharacter);
}

// unsigned long long int countMatchState = 0;
// unsigned long long int countMatchCharater = 0;
// unsigned long long int countAcCharacter = 0; 
// unsigned long long int countFullMatchCharacter = 0;
// unsigned long long int countRepeatFullMatch = 0;
// unsigned long long int countFullMatchTimes = 0;



// unsigned long long int countMaxPatternInState = 0;
// unsigned long long int countSumPatternInState = 0;
// unsigned long long int countFinalState = 0;

// unsigned long long int sizeCharTree = 0;
// unsigned long long int sizeFirstChar = 0;
// unsigned long long int sizePatternList = 0;
// unsigned long long int sizeBitmapTree = 0;
// unsigned long long int sizeFirstCharBitmap = 0;

// unsigned long long int sizePattern = 0;



// // 顯示統計資訊。
// void showStatisticResult(ACSM_STRUCT2* acsm, unsigned int patternSize, unsigned int* patternIdList, 
//   unsigned long long int searchClockTime){

//    printf("[INFO] ------------------------------------------- \n");

//   // 顯示找到多少個 pattern。
//   printf("[INFO] Found Count: %u \n", foundCounter);
//   // 顯示字元比對多少次。
//   printf("[INFO] Match Charaters: %llu \n", countMatchCharater);

//   printf("\n");

//   // 顯示讀取幾次 state table。
//   printf("[INFO] State Table Access Count: %s \n", readableFixUnitValue(countStateTableAccess, 2));
//   // 顯示讀取幾次 match table。
//   printf("[INFO] Match Table Access Count: %s \n", readableFixUnitValue(countMatchTableAccess, 2));
//   // 顯示讀取幾次 next table。
//   printf("[INFO] Next Table Access Count: %s \n", readableFixUnitValue(countNextTableAccess, 2));
//   // 顯示讀取幾次 pattern 和 text 中的 character。
//   printf("[INFO] Character Access Count: %s \n", readableFixUnitValue(countCharacterAccess, 2));

//   // 將上述加起來，顯示全部幾次 memory access。
//   unsigned long long int countTotalAccess = countStateTableAccess + countMatchTableAccess + countNextTableAccess + countCharacterAccess;
//   printf("[INFO] Total Access Count: %s \n", readableFixUnitValue(countTotalAccess, 2));

//   // 計算哪些 pattern 是 match 和 mismatch，
//   unsigned int countMatchId = 0;
//   unsigned int countMissId = 0; 
//   for(int i = 0; i < patternSize; i++){
//     if(patternIdList[i] != 0){
//       countMatchId++;
//       // printf("[INFO] Match Id: %u \n", i);
//       // printf("[INFO] Match Count: %u \n", patternIdList[i]);
//     }else{
//       countMissId++;
//     }
//   }

//   printf("\n");
//   printf("[INFO] Miss Id Count: %u \n", countMissId);
//   printf("[INFO] Match Id Count: %u \n", countMatchId);
//   printf("\n");

//   statisticStateMachine(acsm);

//   // 找出最多 pattern 的 final state，並計算有幾條 sub pattern。
//   printf("[INFO] Max Pattern In State Count: %llu \n", countMaxPatternInState);
//   // 加總所有 final state 的 pattern。
//   printf("[INFO] Sum Pattern In State Count: %llu \n", countSumPatternInState);
//   // 計算有幾個 final state。
//   printf("[INFO] Final State Count: %llu \n", countFinalState);

//   printf("\n");
//   printf("[INFO] Size Char Tree Bytes: %llu \n", sizeCharTree);
//   printf("[INFO] Size Fisrt Char Bytes: %llu \n", sizeFirstChar);
//   printf("[INFO] Size Pattern List Bytes: %llu \n", sizePatternList);
//   printf("[INFO] Total Char Tree Bytes: %llu \n", sizeCharTree + sizeFirstChar + sizePatternList);
  
//   printf("[INFO] Size Bitmap Node Bytes: %lu \n", sizeof(BitmapTree));
//   printf("[INFO] Size Bitmap Tree Bytes: %llu \n", sizeBitmapTree);
//   printf("[INFO] Size Bitmap First Char Bytes: %llu \n", sizeFirstCharBitmap);

//   printf("\n");
//   printf("[INFO] Search clock time: %llu \n", searchClockTime);
//   printf("[INFO] Search clock time: %s \n", readableFixUnitValue(searchClockTime, 2));
//   // printFullQ(acsm);
//   // Print_DFA_MatchList(acsm, 0);
// }

// void showClockTime(char* name, unsigned long long int searchClockTime){
//   printf("[INFO] Search %s clock time: %s \n", name, readableFixUnitValue(searchClockTime, 2));
// }

// void statisticStateMachine(ACSM_STRUCT2 *acsm){
//   unsigned int sizePattern = 0;

//   for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
//     acstate_t* transition = acsm->acsmNextState[state];
//     if(transition[1]){
//        countFinalState++;

//        ACSM_PATTERN2 *MatchList = acsm->acsmMatchList[state];
//        unsigned int matchListSize = 0;
//        for(; MatchList != NULL; MatchList = MatchList->next){
//          matchListSize++;
//          sizePattern += MatchList->n;
//        }

//        countSumPatternInState += matchListSize;
//        if(matchListSize > countMaxPatternInState){
//          countMaxPatternInState = matchListSize;
//        }
//     }
//   }

//   printf("[INFO] Size pattern: %s \n", readableFixUnitValue(sizePattern, 2));
// }