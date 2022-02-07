#ifndef __HEADER_STATISTIC__
#define __HEADER_STATISTIC__

#include "util_paser.h"
#include "aho_corasick.h"

// extern unsigned int patternSize;
// extern unsigned int* patternIdList;

extern unsigned int foundCounter;

// extern unsigned long long int countMatchState;
// extern unsigned long long int countMatchCharater;
// extern unsigned long long int countAcCharacter;
// extern unsigned long long int countFullMatchCharacter;
// extern unsigned long long int countRepeatFullMatch;
// extern unsigned long long int countFullMatchTimes;

extern unsigned long long int countStateAccess;
extern unsigned long long int countNextStateAccess;
extern unsigned long long int countCharacterAccess;
extern unsigned long long int countSubPatternAccess;

extern unsigned long long int countFullMatchTimes;
extern unsigned long long int countFullMatchSuccess;
extern unsigned long long int countCompareCharater;

extern unsigned long long int countDirtyCharacter;

// extern unsigned long long int countMaxPatternInState;
// extern unsigned long long int countSumPatternInState;
// extern unsigned long long int countFinalState;

// extern unsigned long long int sizeCharTree;
// extern unsigned long long int sizeFirstChar;
// extern unsigned long long int sizePatternList;
// extern unsigned long long int sizeBitmapTree;
// extern unsigned long long int sizeFirstCharBitmap;

void showStatisticResult();

// void showStatisticResult(ACSM_STRUCT2* acsm, unsigned int patternSize, unsigned int* patternIdList, 
//   unsigned long long int searchClockTime);

// void statisticStateMachine(ACSM_STRUCT2 *acsm);

// void cleanStatistic();

#endif