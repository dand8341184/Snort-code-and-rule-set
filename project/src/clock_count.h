#ifndef __CLOCK_COUNT_H_COMPOLER__
#define __CLOCK_COUNT_H_COMPOLER__

typedef struct ClockList{
	unsigned long long clock;
	struct ClockList* next;
} ClockList;

typedef struct ClockCountStruct{
	unsigned long long int (*firstClock)();
	void (*reset)();
	void (*begin)();
	unsigned long long int (*end)();
	void (*show)();
	void (*print)(unsigned long long size, unsigned long long unit);
} ClockCountStruct;

extern ClockCountStruct ClockCount;

unsigned long long int rdtsc();

void initClockTimer();

#endif