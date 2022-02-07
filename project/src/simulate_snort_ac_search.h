#ifndef __HEADER_METHOD_SIMULATE_SNORT_SEARCH__
#define __HEADER_METHOD_SIMULATE_SNORT_SEARCH__


void runSimulateSnortAcSearch(Config* config);

void setAcStrategy(Config* config, ACSM_STRUCT2* acsm);

void postprocessor(Config* config, ACSM_STRUCT2* acsm);

#endif