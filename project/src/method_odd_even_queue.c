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
#include "method_odd_even_queue.h"

typedef struct Queue{
	unsigned char* text;
	unsigned char* pattern;
	unsigned short end;
	struct Queue* next;
} Queue;

Queue* queueHead = NULL;

void intiOddEvenQueue(){
	queueHead = calloc(1, sizeof(Queue));
}

void fullMatchToQueue(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
  ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];

  for(; MatchList != NULL; MatchList = MatchList->next){
  	Queue* newEntry = calloc(1, sizeof(Queue));
  	newEntry->pattern = MatchList->origin_patrn;
  	newEntry->text = T - MatchList->start;
  	newEntry->end = MatchList->indexEnd;

  	newEntry->next = queueHead->next;
  	queueHead->next = newEntry;
  }
}

void fullMatchFromQueue(){
	Queue* queueCurrent = queueHead->next;
	while(queueCurrent){
		int isFull = 1;
	    for(int i = queueCurrent->end; i >= 0; i -= 2){
			if(queueCurrent->pattern[i] != queueCurrent->text[i]){
				isFull = 0;
				break;
			}
	    }

	    if(isFull){
	      foundCounter++;
	    }

	    queueCurrent = queueCurrent->next;
	}

	queueHead->next = NULL;
}