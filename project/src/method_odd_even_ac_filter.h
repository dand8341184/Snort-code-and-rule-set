#ifndef __HEADER_METHOD_ODD_EVEN_AC_FILTER__
#define __HEADER_METHOD_ODD_EVEN_AC_FILTER__

typedef struct FilterFlag{
    unsigned x:1;
} FilterFlag;

void setFilterSize(unsigned int traceLength);

void acsmCaleMaxPatternLength(ACSM_STRUCT2 *acsm);

void fullMatchOddEvenAcFilter(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

void searchOriginAcWithFlag(unsigned char *trace, int traceSize);

void createOriginAc(unsigned int size, unsigned char** patterns, unsigned int* lengths);

#endif