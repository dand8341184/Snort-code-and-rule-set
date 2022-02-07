#ifndef __HEADER_METHOD_ODD_EVEN_SNORT__
#define __HEADER_METHOD_ODD_EVEN_SNORT__

// extern unsigned char* TextEnd;

void initAcBeforeSearch(ACSM_STRUCT2* acsm);

void fullMatchSnort(ACSM_STRUCT2* acsm, unsigned char* Tstart, unsigned char* T, unsigned int state);

void fullMatchOrigin(ACSM_STRUCT2* acsm, unsigned char* Tstart, unsigned char* T, unsigned int state);

void acsmCaleFullMatchSnort(ACSM_STRUCT2* acsm);

#endif