#ifndef __HEADER_METHOD_ODD_EVEN__
#define __HEADER_METHOD_ODD_EVEN__

void initFullMatchOddEven(unsigned int patternSize, unsigned char** patternList, unsigned char** hexPatternList);

void acsmCaleFullMatch(ACSM_STRUCT2* acsm);

void full_match(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

void showOddEvenResult();

#endif