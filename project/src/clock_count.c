#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock_count.h"

static ClockList* clockListHead;
static unsigned long long int clockSize;
static unsigned long long int clockMin, clockMax, clockTotal, clockAvg, clockMid, clockQ1, clockQ3;
static unsigned long long int clockMin2nd, clockMax2nd;
static unsigned long long int clockMin3rd, clockMax3rd;

static unsigned long long int clockMinIndex, clockMaxIndex;
static unsigned long long int clockMin2ndIndex, clockMax2ndIndex;
static unsigned long long int clockMin3rdIndex, clockMax3rdIndex;

static unsigned long long int start, period, clockNumber;
static unsigned long* clockGroup;

// static inline unsigned long long int rdtsc(){
// 	unsigned long long int x;
// 	__asm volatile ("rdtsc" : "=A" (x));
// 	return x;
// }

unsigned long long int rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long int)hi << 32) | lo;
}

void initClockTimer(){
  for(unsigned int i = 0; i < 1000; i++){
    unsigned int tmp = rdtsc();
  }
}

static void reset(){
	clockMin = 18446744073709551615ULL;
	clockMax = 0;
	clockTotal = 0;
	clockAvg = 0;
	clockNumber = 0;

	clockListHead = (ClockList*)calloc(1, sizeof(ClockList));
	free(clockGroup);
	clockGroup = NULL;
}

static inline void begin(){
	start = rdtsc();
}

static inline unsigned long long int end(){
	period = rdtsc() - start;

	ClockList* newNode = (ClockList*)malloc(sizeof(ClockList));
	newNode->clock = period;
	newNode->next = clockListHead;
	clockListHead = newNode;
	clockNumber++;

	return period;
}

static void count(unsigned long long size, unsigned long long unit){
	clockGroup = (unsigned long*)malloc(sizeof(unsigned long) * size);

	unsigned int clockIndex;
	for(ClockList* current = clockListHead; current->next != NULL; current = current->next){
		clockTotal += current->clock;
		clockMax = (current->clock > clockMax) ? current->clock : clockMax;
		clockMin = (current->clock < clockMin) ? current->clock : clockMin;

		clockIndex = current->clock / unit;
		clockIndex = (clockIndex >= (size - 1)) ? size - 1 : clockIndex;
		clockGroup[clockIndex] += 1;
	}
	clockAvg = clockTotal / clockNumber;
}

int compare(const void * a, const void * b){
    if( *(unsigned long long int*)a < *(unsigned long long int*)b){
        return -1;
    }

    if( *(unsigned long long int*)a > *(unsigned long long int*)b){
        return 1;
    }
    
    return 0;
}

static void cale(){
	unsigned int countSize = 0;
	for(ClockList* current = clockListHead; current->next != NULL; current = current->next){
		clockTotal += current->clock;
		if(current->clock > clockMax){
			clockMax3rd = clockMax2nd;
			clockMax3rdIndex = clockMax2ndIndex;

			clockMax2nd = clockMax;
			clockMax2ndIndex = clockMaxIndex;

			clockMax = current->clock;
			clockMaxIndex = countSize;
		}

		if(current->clock < clockMin){
			clockMin3rd = clockMin2nd;
			clockMin3rdIndex = clockMin2ndIndex;

			clockMin2nd = clockMin;
			clockMin2ndIndex = clockMinIndex;

			clockMin = current->clock;
			clockMinIndex = countSize;
		}

		countSize++;
	}
	clockAvg = clockTotal / clockNumber;

	unsigned long long int clockArray[countSize];
	ClockList* current = clockListHead;
	for(unsigned int i = 0; i < countSize; i++){
		clockArray[i] = current->clock;
		current = current->next;
		// printf("[INFO] clock[%d]: %llu \n", i, clockArray[i]);
	}

	qsort(clockArray, countSize, sizeof(unsigned long long int), compare);

	// for(unsigned int i = 0; i < countSize; i++){
	// 	printf("[INFO] sorted clock[%d]: %llu \n", i, clockArray[i]);
	// }

	clockQ1 = clockArray[(unsigned int)(countSize * 0.25)];
	clockMid = clockArray[countSize / 2];
	clockQ3 = clockArray[(unsigned int)(countSize * 0.75)];
	clockSize = countSize;

	clockMaxIndex = countSize - clockMaxIndex;
	clockMax2ndIndex = countSize - clockMax2ndIndex;
	clockMax3rdIndex = countSize - clockMax3rdIndex;
	
	clockMinIndex = countSize - clockMinIndex;
	clockMin2ndIndex = countSize - clockMin2ndIndex;
	clockMin3rdIndex = countSize - clockMin3rdIndex;
}

static void show(){
	cale();

	printf("\n");

	printf("[INFO] Clock Size:\t %10llu \n", clockSize);
	printf("[INFO] Clock Total:\t %10f \n", clockTotal / 1000000.0);
	printf("[INFO] Clock Max:\t %10f (%llu) \n", clockMax / 1000000.0, clockMaxIndex);
	// printf("[INFO] Clock Max 2nd:\t %10llu (%llu) \n", clockMax2nd, clockMax2ndIndex);
	// printf("[INFO] Clock Max 3rd:\t %10llu (%llu) \n", clockMax3rd, clockMax3rdIndex);
	printf("[INFO] Clock Min:\t %10f (%llu) \n", clockMin / 1000000.0, clockMinIndex);
	// printf("[INFO] Clock Min 2nd:\t %10llu (%llu) \n", clockMin2nd, clockMin2ndIndex);
	// printf("[INFO] Clock Min 3rd:\t %10llu (%llu) \n", clockMin3rd, clockMin3rdIndex);
	printf("[INFO] Clock Avg:\t %10f \n", clockAvg / 1000000.0);
	printf("[INFO] Clock Q1:\t %10f \n", clockQ1 / 1000000.0);
	printf("[INFO] Clock Median:\t %10f \n", clockMid / 1000000.0);
	printf("[INFO] Clock Q3:\t %10f \n", clockQ3 / 1000000.0);
	printf("\n");
}

static void print(unsigned long long size, unsigned long long unit){
	count(size, unit);

	printf("=================================\n");
	printf("max: %10llu \n", clockMax);
	printf("min: %10llu \n", clockMin);
	printf("avg: %10llu \n", clockAvg);
	printf("\n");

	unsigned long long i;
	for(i = 0; i < size - 1; i++){
		printf("< %10llu : %10lu \n", unit * (i + 1), clockGroup[i]);
	}
	printf("> %10llu : %10lu \n", unit * i, clockGroup[i]);

	printf("--excel format--\n");
	printf("max: %10llu \n", clockMax);
	printf("min: %10llu \n", clockMin);
	printf("avg: %10llu \n", clockAvg);
	printf("\n");
	
	for(i = 0; i < size - 1; i++){
		printf("%10lu \n", clockGroup[i]);
	}
	printf("%10lu \n", clockGroup[i]);
	printf("=================================\n");
}

static unsigned long long int firstClock(){
	ClockList* current = clockListHead;
	return current->clock;
}

ClockCountStruct ClockCount = {
	.firstClock = firstClock,
	.reset = reset,
	.begin = begin,
	.end = end,
	.print = print,
	.show = show,
};