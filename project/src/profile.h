#ifndef __HEADER_PROFILE__
#define __HEADER_PROFILE__

#include "aho_corasick.h"

void profilePatterns(unsigned int numberOfPattern, unsigned char** patternGroup, unsigned int* patternLengthGroup);

void profileFinalState(ACSM_STRUCT2 *acsm);

void showRemainText(ACSM_STRUCT2 *acsm);

void showAllStateTransition(ACSM_STRUCT2 *acsm);

void showExactWildcard(ACSM_STRUCT2 *acsm);

void showExtremeSubPattern(ACSM_STRUCT2 *acsm);

#endif
