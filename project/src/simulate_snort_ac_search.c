#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>
#include <sched.h>

#include "main.h"
#include "statistic.h"
#include "aho_corasick.h"
#include "method_char_tree.h"
#include "method_bitmap_tree.h"
#include "method_odd_even.h"
#include "clock_count.h"
#include "profile.h"
#include "util_file.h"
#include "method_origin.h"
#include "method_longest_sub_pattern.h"
#include "method_synthesis.h"
#include "method_empty.h"
#include "method_last_char_hash.h"
#include "method_hash_small_pattern.h"
#include "method_filter_digest.h"
#include "method_odd_even_ac_filter.h"
#include "method_odd_even_queue.h"
#include "method_odd_even_snort.h"
#include "method_twice_odd_even.h"
#include "method_count_dirty.h"

#include "simulate_snort_ac_search.h"

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

void setAcStrategy(Config* config, ACSM_STRUCT2* acsm){
	if(config->mode == 0){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, full_match);
	}else if(config->mode == 1){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchCharTree);
	}else if(config->mode == 2){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchBitmapTree);
	}else if(config->mode == 3){
	    setAcMode(acsm, 0);
	    setCallbackAcMatch(acsm, originMatch);
	}else if(config->mode == 4){
	    setAcMode(acsm, 2);
	    setCallbackAcMatch(acsm, fullMatchTwiceOddEven);
	}else if(config->mode == 5){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchUsingLongestSubPattern);
	}else if(config->mode == 6){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchSynthesis);
	}else if(config->mode == 7){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchDoNothing);
	}else if(config->mode == 8){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchUsingLastCharHash);
	}else if(config->mode == 9){
	    setAcMode(acsm, 3);
	    setCallbackAcMatch(acsm, full_match);
	    // setCallbackAcMatch(acsm, fullMatchUsingLastCharHash);
	    initSmallPattern();
	}else if(config->mode == 10){
	    setAcMode(acsm, 4);
	    // setCallbackAcMatch(acsm, full_match);
	}else if(config->mode == 11){
	    setAcMode(acsm, 5);
	    // setCallbackAcMatch(acsm, full_match);
	}else if(config->mode == 12){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchFilterDigest);
	}else if(config->mode == 13){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchOddEvenAcFilter);
	}else if(config->mode == 14){
	    setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchToQueue);
	}else if(config->mode == 15){
	    setAcMode(acsm, 7);
	    setCallbackAcMatch(acsm, fullMatchDoNothing);
	}else if(config->mode == 16){
		setAcMode(acsm, 1);
	    setCallbackAcMatch(acsm, fullMatchSnort);
	}else if(config->mode == 17){
		setAcMode(acsm, 8);
	    setCallbackAcMatch(acsm, fullMatchSnort);
	}else if(config->mode == 18){
		setAcMode(acsm, 8);
		setCallbackAcMatch(acsm, fullMatchTwiceOddEvenNew);
	}else if(config->mode == 19){
		setAcMode(acsm, 9);
		setCallbackAcMatch(acsm, fullMatchSnort);
	}else if(config->mode == 20){
		// Origin AC
		setAcMode(acsm, 10);
		setCallbackAcMatch(acsm, originMatch);
	}else if(config->mode == 21){
		// Odd-Even AC with One Character Pattern.
		setAcMode(acsm, 11);
		setCallbackAcMatch(acsm, fullMatchOrigin);
	}else if(config->mode == 22){
		// Odd-Even AC with One Character Pattern and Repetition Table
		setAcMode(acsm, 12);
		setCallbackAcMatch(acsm, fullMatchSnort);
	}else if(config->mode == 23){
		// Odd-Even AC
		setAcMode(acsm, 13);
		setCallbackAcMatch(acsm, fullMatchOrigin);
	}else if(config->mode == 24){
		// Origin AC Count Dirty
		setAcMode(acsm, 10);
		setCallbackAcMatch(acsm, fulllMatchCountDirty);
	}
}

void postprocessor(Config* config, ACSM_STRUCT2* acsm){
	if(config->mode == 0){
		acsmCaleFullMatch(acsm);
	}else if(config->mode == 1){
		processFinalStateSubPatternToCharTree(acsm);
	}else if(config->mode == 2){
		processFinalStateSubPatternToCharTree(acsm);
		createBitmapTableByCharTree(acsm);
	}else if(config->mode == 3){

	}else if(config->mode == 5){
		acsmCaleLongestSubPattern(acsm);
	}else if(config->mode == 6){
		acsmCaleSynthesis(acsm);
	}else if(config->mode == 8){
		acsmCaleLastCharHash(acsm);
	}else if(config->mode == 9){
		acsmCaleLastCharHash(acsm);
	}else if(config->mode == 10){
		acsmCaleFullMatch(acsm);
	}else if(config->mode == 11){
		acsmCaleLastCharHash(acsm);
	}else if(config->mode == 12){
		processFilterDigest(acsm);
	}else if(config->mode == 13){
		acsmCaleMaxPatternLength(acsm);
	}else if(config->mode == 14){

	}else if(config->mode == 15){

	}else if(config->mode == 16){
		acsmCaleFullMatchSnort(acsm);
	}else if(config->mode == 17){
		acsmCaleFullMatchSnort(acsm);
	}else if(config->mode == 18){
		caleDataTwiceOddEven(acsm);
	}else if(config->mode == 19){
		acsmCaleFullMatchSnort(acsm);
	}else if(config->mode == 20){

	}else if(config->mode == 21){
		acsmCaleFullMatchSnort(acsm);
	}else if(config->mode == 22){
		acsmCaleFullMatchSnort(acsm);
	}else if(config->mode == 23){
		acsmCaleFullMatchSnort(acsm);
	}else if(config->mode == 24){
		acsmCaleCountDirty(acsm);
	}

	// showFinalStateSubPattern(acsm);
	// showFinalStateSubPatternReverse(acsm);
	// saveFinalStateSubPatternToFile(acsm);
	// processRegexToDfa(acsm->acsmNumStates);

	if(config->isShowProfile){
		// profilePatterns(numberOfPattern, patternGroup, patternLengthGroup);
		// profileFinalState(acsm);
		// showRemainText(acsm);
		// showAllStateTransition(acsm);
		// showExactWildcard(acsm);
		// showExtremeSubPattern(acsm);
	}
}

static void printAcGroup(AcInfo* acGroup){
	AcInfo* currentAc = acGroup->next;
	while(currentAc){
		printf("[INFO] AC id: %u \n", currentAc->id);
		printf("[INFO] AC protocol: %u \n", currentAc->protocol);
		printf("[INFO] AC portType: %u \n", currentAc->portType);
		printf("[INFO] AC Size of Pattern: %u \n", currentAc->patternListLength);
		
		AcPattern* currentPattern = currentAc->patternList->next;
		while(currentPattern){
			printf("\t [INFO] Pattern: ");
			for(int x = 0; x < currentPattern->length; x++){
	          printf("%02x", currentPattern->pattern[x]);
	        }
	        printf("\n");
	        currentPattern = currentPattern->next;
		}
		currentAc = currentAc->next;
	}
}

void anaylsisAhoCorasick(ACSM_STRUCT2* acsm, unsigned long long int* finalStateNumber, 
	unsigned long long int* totalRestPattern, unsigned long long int* maxRestPattern, 
	unsigned long long int* minRestPattern){

  	for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
    	ACSM_PATTERN2* matchPattern = acsm->acsmMatchList[state];
    	if(matchPattern == NULL){
      		continue;
    	}

    	*finalStateNumber += 1;
    	unsigned long long int restPatternNumber = 0;

    	unsigned char* maxCharMapRepeat = calloc(256, sizeof(unsigned char));
    	for(; matchPattern != NULL; matchPattern = matchPattern->next){
    		restPatternNumber += 1;
    	}

    	*totalRestPattern += restPatternNumber;

    	if(restPatternNumber > *maxRestPattern){
    		*maxRestPattern = restPatternNumber;
    	}

    	if(restPatternNumber < *minRestPattern){
    		*minRestPattern = restPatternNumber;
    	}
  	}
}

static AcInfo* parseSnortAcFile(Config* config){
	FILE* acFile = fopen(config->patternFile, "r");
	unsigned char buffer[2048];
	unsigned char countPattern = 0;
	unsigned char* valueGroup[10]; 
	unsigned int protocol;
	unsigned int portType;

	AcInfo* acGroup = calloc(1, sizeof(AcInfo));
	AcInfo* currentAc = acGroup;
	AcPattern* currentPattern = 0;
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
			currentAc->next = calloc(1, sizeof(AcInfo));
			currentAc = currentAc->next;
			currentAc->protocol = protocol;
			currentAc->portType = portType;
			currentAc->instance = createAhoCorasick();
			setAcStrategy(config, currentAc->instance);

			unsigned char* idText = valueGroup[1];
			currentAc->id = atoi(idText);
		
			currentAc->patternList = calloc(1, sizeof(AcPattern));
			currentPattern = currentAc->patternList;
		}else if(strcmp(tagText, "AC_PATTERN") == 0){
			unsigned char* hexText = valueGroup[1];

			currentPattern->next = calloc(1, sizeof(AcPattern));
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

	unsigned int numberOfAc = 0;
	unsigned int maxNumberOfPattern = 0;
	unsigned int minNumberOfPattern = -1;
	float avgNumberOfPattern = 0;

	unsigned long long int finalStateNumber = 0;
	unsigned long long int totalRestPattern = 0;
	unsigned long long int maxRestPattern = 0;
	unsigned long long int minRestPattern = -1;

	for(AcInfo* currentAc = acGroup->next; currentAc != NULL; 
		currentAc = currentAc->next){
			
		numberOfAc++;
		unsigned numberOfPatternInAc = 0;

		setAcStrategy(config, currentAc->instance);
		for(AcPattern* currentPattern = currentAc->patternList->next; 
			currentPattern != NULL; currentPattern = currentPattern->next){

			unsigned char* pattern = currentPattern->pattern;
			unsigned int length = currentPattern->length;

			numberOfPatternInAc++;
			numberOfPattern++;
			numberOfCharacter += length;
			if(length > maxLength){
				maxLength = length;
			}
			if(length < minLength){
				minLength = length;
			}
			
			int nocase = 0;
			int offset = 0;
			int depth = 0;
			int negative = 0;
			void* value = pattern;
			int id = currentPattern->id;
			acsmAddPattern2(currentAc->instance, pattern, length, nocase, 
				offset, depth, negative, value, id);
		}

		if(numberOfPatternInAc > maxNumberOfPattern){
			maxNumberOfPattern = numberOfPatternInAc;
		}
		if(numberOfPatternInAc < minNumberOfPattern){
			minNumberOfPattern = numberOfPatternInAc;
		}

		acsmCompile2(currentAc->instance);
		anaylsisAhoCorasick(currentAc->instance, &finalStateNumber, &totalRestPattern, &maxRestPattern, &minRestPattern);

  		// acsmCaleStateDepth(currentAc->instance);
  		// acsmCalePatternDepth(currentAc->instance);
  		// acsmPrintInfo2(currentAc->instance);
		
		postprocessor(config, currentAc->instance);
	}

	avgLength = (float)numberOfCharacter / (float)numberOfPattern;
	avgNumberOfPattern = (float)numberOfPattern / (float)numberOfAc;

	printf("[INFO][AC] Number of Pattern: %u \n", numberOfPattern);
	printf("[INFO][AC] Number of Character: %u \n", numberOfCharacter);
	printf("[INFO][AC] Max Length of Pattern: %u \n", maxLength);
	printf("[INFO][AC] Min Length of Pattern: %u \n", minLength);
	printf("[INFO][AC] Avg Length of Pattern: %f \n", avgLength);
	printf("\n");

	printf("[INFO][AC] Number of Ac: %u \n", numberOfAc);
	printf("[INFO][AC] Max Length of Pattern: %u \n", maxNumberOfPattern);
	printf("[INFO][AC] Min Length of Pattern: %u \n", minNumberOfPattern);
	printf("[INFO][AC] Avg Length of Pattern: %f \n", avgNumberOfPattern);
	printf("\n");

	printf("[INFO][AC] Number of Final State: %u \n", finalStateNumber);
	printf("[INFO][AC] Max Rest Pattern: %u \n", maxRestPattern);
	printf("[INFO][AC] Min Rest Pattern: %u \n", minRestPattern);
	printf("[INFO][AC] Total Rest Pattern: %u \n", totalRestPattern);
	printf("[INFO][AC] Avg Rest Pattern: %f \n", totalRestPattern / finalStateNumber);
	printf("\n");

	return acGroup;
}

static TraceInfo* parseSnortTraceFile(Config* config){
	FILE* textFile = fopen(config->traceFile, "r");
	unsigned char buffer[2048];
	unsigned char* valueGroup[10];

	unsigned int numberOfText = 0;
	unsigned int numberOfCharacter = 0;
	unsigned int maxLength = 0;
	unsigned int minLength = -1;
	float avgLength = 0;

	TraceInfo* traceHead = calloc(1, sizeof(TraceInfo));
	TraceInfo* currentTrace = traceHead;
	while(fscanf(textFile, "%s\n", buffer) != EOF){
		unsigned char* currentBuffer = buffer;
		for(unsigned int i = 0; i < 10; i++){
			valueGroup[i] = strtok(currentBuffer, ",");
			currentBuffer = 0;
		}

		unsigned char* tagText = valueGroup[0];
		if(strcmp(tagText, "PACKET") == 0){

		}else if(strcmp(tagText, "PACKET_TEXT") == 0){
			currentTrace->next = calloc(1, sizeof(TraceInfo));
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

			numberOfText++;
			numberOfCharacter += currentTrace->traceLength;
			if(currentTrace->traceLength > maxLength){
				maxLength = currentTrace->traceLength;
			}
			if(currentTrace->traceLength < minLength){
				minLength = currentTrace->traceLength;
			}
		}
	}

	FILE* repetitionFile = fopen("log/count_repetition.csv", "w");
	unsigned int* countRepetition = calloc(10000, sizeof(unsigned int));
	currentTrace = traceHead->next;
	for(;currentTrace != NULL; currentTrace = currentTrace->next){
		unsigned char currentChar = currentTrace->traceText[0];
		unsigned int countChar = 0;
		for(unsigned int i = 0; i < currentTrace->traceLength; i++){
			if(currentChar == currentTrace->traceText[i]){
				countChar++;
			}else{
				countRepetition[countChar]++;
				currentChar = currentTrace->traceText[i];
				countChar = 1;
			}
		}
		countRepetition[countChar]++;
	}

	for(unsigned int i = 5; i < 2000; i++){
		// if(countRepetition[i] != 0){
		fprintf(repetitionFile, "count_repetition,%u,%u \n", i, countRepetition[i]);
		// }
	}

	fclose(repetitionFile);

	avgLength = (float)numberOfCharacter / (float)numberOfText;
	printf("[INFO][AC] Number of Text: %u \n", numberOfText);
	printf("[INFO][AC] Number of Character: %u \n", numberOfCharacter);
	printf("[INFO][AC] Max Length of Text: %u \n", maxLength);
	printf("[INFO][AC] Min Length of Text: %u \n", minLength);
	printf("[INFO][AC] Avg Length of Text: %f \n", avgLength);
	printf("\n");

	return traceHead;
}

static TraceInfo* checkRepetitionTableUsing(TraceInfo* traceInfoGroup){
	unsigned int* countRepetition = calloc(10000, sizeof(unsigned int));
	for(TraceInfo* currentTrace = traceInfoGroup->next; currentTrace != NULL; currentTrace = currentTrace->next){
		unsigned char currentChar = currentTrace->traceText[0];
		unsigned int countChar = 0;
		for(unsigned int i = 0; i < currentTrace->traceLength; i++){
			if(currentChar == currentTrace->traceText[i]){
				countChar++;
			}else{
				if(currentTrace->matchAc->instance->repetitionUsingTable[currentChar] == 1){
					countRepetition[countChar]++;
				}
				currentChar = currentTrace->traceText[i];
				countChar = 1;
			}
		}
		if(currentTrace->matchAc->instance->repetitionUsingTable[currentChar] == 1){
			countRepetition[countChar]++;
		}
	}

	FILE* repetitionFile = fopen("log/count_using_repetition.csv", "w");
	for(unsigned int i = 5; i < 2000; i++){
		// if(countRepetition[i] != 0){
		fprintf(repetitionFile, "count_using_repetition,%u,%u\n", i, countRepetition[i]);
		// }
	}
	fclose(repetitionFile);
}

static void mapAcInTrace(AcInfo* acInfoGroup, TraceInfo* traceInfoGroup){
	for(TraceInfo* currentTrace = traceInfoGroup->next;
		currentTrace != NULL; currentTrace = currentTrace->next){

		unsigned int acId = currentTrace->acId;
		for(AcInfo* currentAc = acInfoGroup->next;
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

static void showMemoryUsage(AcInfo* acInfoGroup){
	unsigned long long int totalMemory = 0;
	unsigned long long int acMemory = 0;
	unsigned long long int patternMemory = 0;
	unsigned long long int otherMemory = 0;
	for(AcInfo* currentAc = acInfoGroup->next;
		currentAc != NULL; currentAc = currentAc->next){
		
		totalMemory += currentAc->instance->acsm2_total_memory
			+ currentAc->instance->other_memory;
		patternMemory += currentAc->instance->acsm2_pattern_memory;
		patternMemory += currentAc->instance->acsm2_matchlist_memory;
		acMemory += currentAc->instance->acsm2_total_memory
			- currentAc->instance->acsm2_pattern_memory
			- currentAc->instance->acsm2_matchlist_memory;
		otherMemory += currentAc->instance->other_memory;
	}

	printf("[INFO] Total Memory: %f \n", totalMemory / 1000000.0);
	printf("[INFO] AC Memory: %f \n", acMemory / 1000000.0);
	printf("[INFO] Pattern Memory: %f \n", patternMemory / 1000000.0);
	printf("[INFO] Other Memory: %f \n", otherMemory / 1000000.0);
	printf("\n");
}

void* startThreadForOnLen(void* data){
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(3, &mask);
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
	  fprintf(stderr, "[INFO] Thread affinity failed. \n");
	  exit(0);
	}

	ThreadData* threadData = (ThreadData*)data;
	AcInfo* acInfoGroup = threadData->acInfoGroup;
	TraceInfo* traceInfoGroup = threadData->traceInfoGroup;
	unsigned int threadMatchCount = 0;
	for(TraceInfo* currentTrace = traceInfoGroup->next;
		currentTrace != NULL; currentTrace = currentTrace->next){

  		threadMatchCount += searchOneLenWithOddEven(currentTrace->matchAc->instance, 
  			currentTrace->traceText, currentTrace->traceLength);
	}

	threadData->threadMatchCount = threadMatchCount;
}

void runSimulateSnortAcSearch(Config* config){
	AcInfo* acInfoGroup = parseSnortAcFile(config);
	TraceInfo* traceInfoGroup = parseSnortTraceFile(config);

	mapAcInTrace(acInfoGroup, traceInfoGroup);

	if(config->mode != 16 && config->mode != 3 
		&& config->mode != 7 && config->mode != 17 
		&&  config->mode != 18 && config->mode != 19
		&& config->mode != 20 && config->mode != 0
		&& config->mode != 21 && config->mode != 22
		&& config->mode != 23 && config->mode != 24){
		printf("[INFO] Unsuitable search mode.\n");
		exit(0);
	}

	showMemoryUsage(acInfoGroup);

	FILE* clockCycleFile = 0;
	if(config->mode == 3){
		clockCycleFile = fopen("log/clock_cycle_list_m3", "w");
	}else if(config->mode = 16){
		clockCycleFile = fopen("log/clock_cycle_list_m16", "w");
	}else if(config->mode = 17){
		clockCycleFile = fopen("log/clock_cycle_list_m17", "w");
	}

	int cpuNum = sysconf(_SC_NPROCESSORS_CONF);
    printf("[INFO] System has %d processor(s)\n", cpuNum);

	initClockTimer();
	ClockCount.reset();
	unsigned int countTrace = 0;

	ThreadData* threadData;
	pthread_t* thread;
	if(config->mode == 19){
		cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(0, &mask);
		if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0){
		  fprintf(stderr, "[INFO] Thread affinity failed. \n");
		  exit(0);
		}

		threadData = calloc(1, sizeof(ThreadData));
		threadData->acInfoGroup = acInfoGroup;
		threadData->traceInfoGroup = traceInfoGroup;
		threadData->threadMatchCount = 0;

		thread = calloc(1, sizeof(pthread_t));
		pthread_create(thread, NULL, startThreadForOnLen, threadData);
	}

	unsigned long long int countTraceLength = 0;
	unsigned int countTracePassLenOne = 0;
	unsigned long long int countLenOneTraceLength = 0;

	struct timeval t1, t2;
	double elapsedTime;
	gettimeofday(&t1, NULL);

	for(TraceInfo* currentTrace = traceInfoGroup->next;
		currentTrace != NULL; currentTrace = currentTrace->next){

		currentTrace->matchAc->countCall++;
		currentTrace->matchAc->countTraceCharacter += currentTrace->traceLength;

		initAcBeforeSearch(currentTrace->matchAc->instance);
		ClockCount.begin();
		unsigned int current_state = 0;
    	acsmSearch2(currentTrace->matchAc->instance, 0, currentTrace->traceText, 
    		currentTrace->traceLength, MatchFound, (void *)0, &current_state);
		unsigned long long int singleClock = ClockCount.end();
		currentTrace->matchAc->instance->clockCycle += singleClock;

		fprintf(clockCycleFile, "%u,%u,%u,%u,%llu\n", 
			currentTrace->acId, 
			currentTrace->matchAc->patternListLength,
			countTrace,
			currentTrace->traceLength,
			singleClock);

		if(singleClock > 50000){
			for(unsigned int i = 0; i < currentTrace->traceLength; i++){
				fprintf(clockCycleFile, "%02x", currentTrace->traceText[i]);
			}
			fprintf(clockCycleFile, "\n");
		}

		if(currentTrace->matchAc->instance->oneLenFalg){
			countLenOneTraceLength += currentTrace->traceLength;
			countTracePassLenOne++;
		}
		// printf("[INFO] Single trace found patterns: %u, %u \n", countTrace, foundCounter);
		countTrace++;
		countTraceLength += currentTrace->traceLength;
	}
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    printf("[INFO] Search time: %f ms.\n", elapsedTime);
    printf("[INFO] Count Trace: %u \n", countTrace);
    printf("[INFO] Count Trace Length: %llu \n", countTraceLength);
    printf("[INFO] Count Trace Length Avg: %f \n", countTraceLength / (float) countTrace);
    printf("[INFO] Count Pass though Len One AC: %u.\n", countTracePassLenOne);
    printf("[INFO] Count Pass though Len One Trace Length: %llu.\n", countLenOneTraceLength);
    printf("[INFO] Count Pass though Len One Trace Length Avg: %f.\n", countLenOneTraceLength / (float)countTracePassLenOne);
    printf("\n");

    fclose(clockCycleFile);

    printf("count_ac_call,ac_id,number_of_input_text,character_comparision,full_match,input_text_character,rest_pattern_comparsion,repetition_times,clock_cycle\n");
    for(AcInfo* currentAc = acInfoGroup->next; currentAc != NULL; currentAc = currentAc->next){
    	if(currentAc->countCall != 0){
    		printf("count_ac_call,AC %u,%u,%llu,%llu,%llu,%llu,%llu,%llu\n", 
    			currentAc->id, currentAc->countCall, 
    			currentAc->instance->countCharacterComparison,
    			currentAc->instance->countFullMatch,
    			currentAc->countTraceCharacter,
    			currentAc->instance->countRestPatternComparison,
    			currentAc->instance->countRepetitionVisit,
    			currentAc->instance->clockCycle);
    	}
    }

    FILE* repetitionUsingFile = fopen("log/repetition_using_table.csv", "w");
	for(AcInfo* currentAc = acInfoGroup->next; currentAc != NULL; currentAc = currentAc->next){
		unsigned int isVaild = 0;
		for(unsigned int i = 0; i < 256; i++){
			if(currentAc->instance->repetitionUsingTable[i] != 0){
				isVaild = 1;
				break;
			}
		}

		if(isVaild == 0){
			continue;
		}

		fprintf(repetitionUsingFile, "repetition_using_table,%u", currentAc->id);
		for(unsigned int i = 0; i < 256; i++){
			if(currentAc->instance->repetitionUsingTable[i] != 0){
				fprintf(repetitionUsingFile, ",%02x", i);
			}
		}
		fprintf(repetitionUsingFile, "\n");
	}
	fclose(repetitionUsingFile);

	checkRepetitionTableUsing(traceInfoGroup);

    if(config->mode == 19){
    	pthread_join(*thread, NULL);
		foundCounter += threadData->threadMatchCount;
		printf("[INFO] Show Length One Match Count: %u \n", threadData->threadMatchCount);
	}

	ClockCount.show();
	showStatisticResult();
}