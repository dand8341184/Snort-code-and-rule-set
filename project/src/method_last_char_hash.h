#ifndef __HEADER_METHOD_LAST_CAHR_HASH__
#define __HEADER_METHOD_LAST_CAHR_HASH__

extern unsigned short* sizeGroup;
extern unsigned char** filterGroup;
extern unsigned char** offsetGroup;
extern ACSM_PATTERN2*** subPatternGroup;

void acsmCaleLastCharHash(ACSM_STRUCT2 *acsm);

void fullMatchUsingLastCharHash(ACSM_STRUCT2* acsm, unsigned char* Tstart, unsigned char* T, unsigned int state);

#endif