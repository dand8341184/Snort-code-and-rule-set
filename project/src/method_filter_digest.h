#ifndef __HEADER_METHOD_FILTER_DIGEST__
#define __HEADER_METHOD_FILTER_DIGEST__

void processFilterDigest(ACSM_STRUCT2 *acsm);

void fullMatchFilterDigest(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

#endif