#ifndef __HEADER_METHOD_SYNTHESIS__
#define __HEADER_METHOD_SYNTHESIS__

void acsmCaleSynthesis(ACSM_STRUCT2 *acsm);

void fullMatchSynthesis(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

void showStatisticStrategy();

#endif