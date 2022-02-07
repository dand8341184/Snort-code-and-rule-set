#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "main.h"
#include "statistic.h"
#include "clock_count.h"
#include "profile.h"
#include "util_file.h"

#include "snort_wu_manber.h"
#include "simulate_snort_wu_search.h"

typedef struct WuPattern{
	unsigned int length;
	unsigned char* pattern;
	unsigned int id;
	struct WuPattern* next;
} WuPattern;

typedef struct WuInfo{
	unsigned int protocol;
	unsigned int portType;
	unsigned int id;
	unsigned int patternListLength;
	struct WuPattern* patternList;
	MWM_STRUCT* instance;
	struct WuInfo* next;
} WuInfo;

typedef struct TextInfo{
	unsigned int acId;
	unsigned char* traceText;
	unsigned int traceLength;
	unsigned int searchMethod;
	unsigned int traceType;
	WuInfo* matchAc;
	struct TextInfo* next;
} TextInfo;

static const unsigned int IP = 0;
static const unsigned int ICMP = 1;
static const unsigned int UDP = 2;
static const unsigned int TCP = 3;

static const unsigned int SOURCE = 0;
static const unsigned int DESTINATION = 1;

static const unsigned int HTTP_URI_CONTENT = 0;
static const unsigned int HTTP_HEADER_CONTENT = 1;
static const unsigned int HTTP_CLIENT_BODY_CONTENT = 2;
static const unsigned int CONTENT_DECODE = 3;
static const unsigned int CONTENT_FILE = 4;
static const unsigned int CONTENT = 5;

static WuInfo* parseSnortAcFile(Config* config){
	FILE* acFile = fopen(config->patternFile, "r");
	unsigned char buffer[2048];
	unsigned char countPattern = 0;
	unsigned char* valueGroup[10]; 
	unsigned int protocol;
	unsigned int portType;

	WuInfo* acGroup = calloc(1, sizeof(WuInfo));
	WuInfo* currentAc = acGroup;
	WuPattern* currentPattern = 0;
	while(fscanf(acFile, "%s\n", buffer) != EOF){
		unsigned char* currentBuffer = buffer;
		for(unsigned int i = 0; i < 10; i++){
			valueGroup[i] = strtok(currentBuffer, ",");
			currentBuffer = 0;
		}

		unsigned char* tagText = valueGroup[0];
		if(strcmp(tagText, "AC_GROUP") == 0){
			unsigned char* protocolText = valueGroup[1];
			unsigned char* portTypeText = valueGroup[2];

			if(strcmp(protocolText, "IP") == 0){
				protocol = IP;
			}else if(strcmp(protocolText, "ICMP") == 0){
				protocol = ICMP; 
			}else if(strcmp(protocolText, "TCP") == 0){
				protocol = TCP;
			}else if(strcmp(protocolText, "UDP") == 0){
				protocol = UDP;
			}

			if(strcmp(portTypeText, "SOURCE") == 0){
				portType = SOURCE;
			}else if(strcmp(portTypeText, "DESTINATION") == 0){
				portType = DESTINATION;
			}
		}else if(strcmp(tagText, "AC_OBJECT") == 0){
			currentAc->next = calloc(1, sizeof(WuInfo));
			currentAc = currentAc->next;
			currentAc->protocol = protocol;
			currentAc->portType = portType;
			currentAc->instance = mwmNew();

			unsigned char* idText = valueGroup[1];
			currentAc->id = atoi(idText);
		
			currentAc->patternList = calloc(1, sizeof(WuPattern));
			currentPattern = currentAc->patternList;
		}else if(strcmp(tagText, "AC_PATTERN") == 0){
			unsigned char* hexText = valueGroup[1];

			currentPattern->next = calloc(1, sizeof(WuPattern));
			currentPattern = currentPattern->next;
			currentPattern->length = hexTextLength(hexText);
			currentPattern->pattern = hexTextToHexArray(hexText);
			
			currentPattern->id = currentAc->patternListLength;
			currentAc->patternListLength++;
		}
	}

	// printAcGroup(acGroup);

	unsigned int numberOfPattern = 0;
	unsigned int numberOfCharacter = 0;
	unsigned int maxLength = 0;
	unsigned int minLength = -1;
	float avgLength = 0;

	unsigned int numberOfWu = 0;
	unsigned int maxNumberOfPattern = 0;
	unsigned int minNumberOfPattern = -1;
	float avgNumberOfPattern = 0;

	for(WuInfo* currentAc = acGroup->next; currentAc != NULL; 
		currentAc = currentAc->next){
		
		numberOfWu++;
		unsigned numberOfPatternInWu = 0;

		for(WuPattern* currentPattern = currentAc->patternList->next; 
			currentPattern != NULL; currentPattern = currentPattern->next){
			
			unsigned char* pattern = currentPattern->pattern;
			unsigned int length = currentPattern->length;

			numberOfPatternInWu++;
			numberOfPattern++;
			numberOfCharacter += length;
			if(length > maxLength){
				maxLength = length;
			}
			if(length < minLength){
				minLength = length;
			}

			if(config->noLenOne == 1){
				if(length < 2){
					continue;
				}
			}

			int nocase = 0;
		    int offset = 0;
		    int depth = 0;
		    int negative = 0;
		    void* value = pattern;
		    int id = currentPattern->id;
		    mwmAddPatternEx(currentAc->instance, pattern, length, nocase, 0, 0, value /* ID */, id /* IID -internal id*/ );
		}

		if(numberOfPatternInWu > maxNumberOfPattern){
			maxNumberOfPattern = numberOfPatternInWu;
		}
		if(numberOfPatternInWu < minNumberOfPattern){
			minNumberOfPattern = numberOfPatternInWu;
		}

		mwmPrepPatterns(currentAc->instance); 
	}

	avgLength = (float)numberOfCharacter / (float)numberOfPattern;
	avgNumberOfPattern = (float)numberOfPattern / (float)numberOfWu;

	printf("[INFO][WU] Number of Pattern: %u \n", numberOfPattern);
	printf("[INFO][WU] Number of Character: %u \n", numberOfCharacter);
	printf("[INFO][WU] Max Length of Pattern: %u \n", maxLength);
	printf("[INFO][WU] Min Length of Pattern: %u \n", minLength);
	printf("[INFO][WU] Avg Length of Pattern: %f \n", avgLength);
	printf("\n");

	printf("[INFO][WU] Number of Wu: %u \n", numberOfWu);
	printf("[INFO][WU] Max Length of Pattern: %u \n", maxNumberOfPattern);
	printf("[INFO][WU] Min Length of Pattern: %u \n", minNumberOfPattern);
	printf("[INFO][WU] Avg Length of Pattern: %f \n", avgNumberOfPattern);
	printf("\n");

	return acGroup;
}

static TextInfo* parseSnortTraceFile(Config* config){
	FILE* textFile = fopen(config->traceFile, "r");
	unsigned char buffer[2048];
	unsigned char* valueGroup[10];

	TextInfo* traceHead = calloc(1, sizeof(TextInfo));
	TextInfo* currentTrace = traceHead;
	while(fscanf(textFile, "%s\n", buffer) != EOF){
		unsigned char* currentBuffer = buffer;
		for(unsigned int i = 0; i < 10; i++){
			valueGroup[i] = strtok(currentBuffer, ",");
			currentBuffer = 0;
		}

		unsigned char* tagText = valueGroup[0];
		if(strcmp(tagText, "PACKET") == 0){

		}else if(strcmp(tagText, "PACKET_TEXT") == 0){
			currentTrace->next = calloc(1, sizeof(TextInfo));
			currentTrace = currentTrace->next;

			currentTrace->acId = atoi(valueGroup[1]);
			currentTrace->searchMethod = atoi(valueGroup[2]);
			currentTrace->traceLength = atoi(valueGroup[4]);

			unsigned char* traceTypeText = valueGroup[3];
			if(strcmp(traceTypeText, "HTTP_URI_CONTENT") == 0){
				currentTrace->traceType = HTTP_URI_CONTENT;
			}else if(strcmp(traceTypeText, "HTTP_HEADER_CONTENT") == 0){
				currentTrace->traceType = HTTP_HEADER_CONTENT;
			}else if(strcmp(traceTypeText, "HTTP_CLIENT_BODY_CONTENT") == 0){
				currentTrace->traceType = HTTP_CLIENT_BODY_CONTENT;
			}else if(strcmp(traceTypeText, "CONTENT_DECODE") == 0){
				currentTrace->traceType = CONTENT_DECODE;
			}else if(strcmp(traceTypeText, "CONTENT_FILE") == 0){
				currentTrace->traceType = CONTENT_FILE;
			}else if(strcmp(traceTypeText, "CONTENT") == 0){
				currentTrace->traceType = CONTENT;
			}

			unsigned int hexLengthWithNewLine = (currentTrace->traceLength * 2) + 1;
			unsigned char* traceBuffer = calloc(hexLengthWithNewLine, sizeof(unsigned char));
			fscanf(textFile, "%s\n", traceBuffer);
			currentTrace->traceText = hexTextToHexArray(traceBuffer);
			free(traceBuffer);
		}
	}

	return traceHead;
}

static void mapAcInTrace(WuInfo* acInfoGroup, TextInfo* traceInfoGroup){
	for(TextInfo* currentTrace = traceInfoGroup->next;
		currentTrace != NULL; currentTrace = currentTrace->next){

		unsigned int acId = currentTrace->acId;
		for(WuInfo* currentAc = acInfoGroup->next;
			currentAc != NULL; currentAc = currentAc->next){
			
			if(currentAc->id != acId){
				continue;
			}
			currentTrace->matchAc = currentAc;
		}

		if(currentTrace->matchAc == NULL){
			printf("[INFO] AC not found with id: %u \n", acId);
			exit(0);
		}
	}
}

int wuMatchFound(void* id, int index, void *data){
	foundCounter++;
	return 0;
}

void runSimulateSnortWuSearch(Config* config){
	if(config->isSimulateSnortWuManber == 0){
		printf("[INFO] Unsuitable search mode.\n");
		exit(0);
	}

	WuInfo* acInfoGroup = parseSnortAcFile(config);
	TextInfo* traceInfoGroup = parseSnortTraceFile(config);

	mapAcInTrace(acInfoGroup, traceInfoGroup);
	showWuMemoryInfo();

	// FILE* clockCycleFile = 0;
	// if(config->mode == 3){
	// 	clockCycleFile = fopen("log/clock_cycle_list_m3", "w");
	// }else if(config->mode = 16){
	// 	clockCycleFile = fopen("log/clock_cycle_list_m16", "w");
	// }

	initClockTimer();
	ClockCount.reset();
	unsigned int countTrace = 0;
	for(TextInfo* currentTrace = traceInfoGroup->next;
		currentTrace != NULL; currentTrace = currentTrace->next){

		ClockCount.begin();
		unsigned int current_state = 0;
    	mwmSearch(currentTrace->matchAc->instance,  currentTrace->traceText, currentTrace->traceLength, wuMatchFound, 0 ); 
		unsigned long long int singleClock = ClockCount.end();

		// fprintf(clockCycleFile, "%u,%u,%u,%llu\n", 
		// 	currentTrace->acId, 
		// 	currentTrace->matchAc->patternListLength,
		// 	currentTrace->traceLength,
		// 	singleClock);

		// printf("[INFO] Single trace found patterns: %u, %u \n", countTrace, foundCounter);
		countTrace++;
	}

	// fclose(clockCycleFile);

	ClockCount.show();
	showStatisticResult();
}