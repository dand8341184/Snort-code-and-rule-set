#include "method_origin.h"
#include "aho_corasick.h"
#include "statistic.h"


void originMatch(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
	// printf("[INFO] Pattern Id: %u \n", acsm->acsmMatchList[state]->iid);
	foundCounter++;
}