#ifndef __HEADER_METHOD_LONGEST_SUB_PATTERN__
#define __HEADER_METHOD_LONGEST_SUB_PATTERN__

void acsmCaleLongestSubPattern(ACSM_STRUCT2 *acsm);

void fullMatchUsingLongestSubPattern(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

#endif