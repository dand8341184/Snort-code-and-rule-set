#ifndef __HEADER_MAIN__
#define __HEADER_MAIN__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aho_corasick.h"

typedef struct Config{
	unsigned int isSimulateSnort;
	unsigned int isSimulateSnortWuManber;
	unsigned int isSingleSnortWu;

	unsigned char* patternFile;
	
	unsigned char* traceFile;

	unsigned short isRunAllTest;

	unsigned short isShowInfo1;
	unsigned short isShowInfo2;

	unsigned int mode;

	unsigned short isShowProfile;
	unsigned int searchRunTime;
	unsigned int noLenOne;
} Config;

int MatchFound(void* id, int index, void *data);

typedef struct AcPattern{
	unsigned int length;
	unsigned char* pattern;
	unsigned int id;
	struct AcPattern* next;
} AcPattern;

typedef struct AcInfo{
	unsigned int protocol;
	unsigned int portType;
	unsigned int id;
	unsigned int patternListLength;
	struct AcPattern* patternList;
	ACSM_STRUCT2* instance;
	struct AcInfo* next;
	unsigned int countCall;
	unsigned long long int countTraceCharacter;
} AcInfo;

typedef struct TraceInfo{
	unsigned int acId;
	unsigned char* traceText;
	unsigned int traceLength;
	unsigned int searchMethod;
	unsigned int traceType;
	AcInfo* matchAc;
	struct TraceInfo* next;
} TraceInfo;

typedef struct ThreadData{
    AcInfo* acInfoGroup;
    TraceInfo* traceInfoGroup;
    unsigned int threadMatchCount;
} ThreadData;

#endif