#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "statistic.h"
#include "aho_corasick.h"
#include "method_code_consistency.h"

typedef struct Tree{
	unsigned char next[256];
	unsigned short patternSize;
} Tree;


void caleDataCodeConsistency(ACSM_STRUCT2* acsm){
	ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];

	for(; MatchList != NULL; MatchList = MatchList->next){
		unsigned char* origin_patrn = MatchList->origin_patrn;
		unsigned int indexEnd = MatchList->indexEnd;

		

		for(int i = indexEnd; i >= 0; i -= 2){
			origin_patrn[i]
		}
	}
}

void fullMatchCodeConsistency(ACSM_STRUCT2 *acsm, unsigned char * Tstart, 
	unsigned char* T, unsigned int state){
	
}
