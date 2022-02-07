#include "aho_corasick.h"

#ifndef __HEADER_METHOD_COUNT_DIRTY__
#define __HEADER_METHOD_COUNT_DIRTY__

void acsmCaleCountDirty(ACSM_STRUCT2* acsm);

void fulllMatchCountDirty(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

#endif