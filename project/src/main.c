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

#include "snort_wu_manber.h"
#include "simulate_snort_ac_search.h"
#include "simulate_snort_wu_search.h"

// ac match 時會 call back 的 function。
int MatchFound(void* id, int index, void *data){
  // fprintf (stdout, "Found Pattern %s\n", (char *) id);
  // foundCounter++;
  return 0;
}

int WuManberMatchFound(void* id, int index, void *data){
  // fprintf (stdout, "Found Pattern %s\n", (char *) id);
  foundCounter++;
  return 0;
}

// 顯示程式的使用方法。
int showUsage(){
  printf("[INFO] Show command example: ./program -p PATTERN_FILE -t TRACE_FILE");
  exit(0);
}

void sumClock(double* sum, unsigned int* sumCount, unsigned long long int clockTime){
  double result = clockTime / 1024.0 / 1024.0;
  if(result > 100000){
    return;
  }

  *sum = *sum + result;
  *sumCount += 1;
}

/* parse the main call parameters */
static Config* parseArguments(int argc, char **argv)
{
  int i = 1;
  
  if(argc < 2){
    showUsage();
    return NULL;
  }

  Config* config = calloc(1, sizeof(Config));
  config->searchRunTime = 1;

  while(i < argc){
    // if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
    //   i++;
    //   if(i == argc){
    //     return 0;
    //   }
    // }

    if(strcmp(argv[i], "-p") == 0){
      i++;
      
      if(i == argc){
        printf("Pattern file name missing.");
        return NULL;  
      }

      config->patternFile = argv[i];
    }

    if(strcmp(argv[i], "-t") == 0){
      i++;

      if(i == argc){
        printf("Trace file name missing.");
        return NULL;
      }

      config->traceFile = argv[i];
    }

    if(strcmp(argv[i], "-m") == 0){
      i++;

      if(i == argc){
        printf("Mode missing.");
        return NULL;
      }

      config->mode = atoi(argv[i]);
    }

    if(strcmp(argv[i], "-a") == 0){
      config->isRunAllTest = 1;
    }

    if(strcmp(argv[i], "-l") == 0){
      config->isShowProfile = 1;
    }

    if(strcmp(argv[i], "-n") == 0){
      i++;

      if(i == argc){
        printf("Searcn run time missing.");
        return NULL;
      }

      config->searchRunTime = atoi(argv[i]);
    }

    if(strcmp(argv[i], "-snort") == 0){
      config->isSimulateSnort = 1;
    }

    if(strcmp(argv[i], "-wu") == 0){
      config->isSimulateSnortWuManber = 1;
    }

    if(strcmp(argv[i], "-no-len-one") == 0){
      config->noLenOne = 1;
    }

    if(strcmp(argv[i], "-wu-single") == 0){
      config->isSingleSnortWu = 1;
    }

    i++;
  }

  return config;
}

void addPatternIntoAc(ACSM_STRUCT2* acsm, unsigned int size, unsigned char** patterns, unsigned int* lengths){
  for(unsigned int i = 0; i < size; i++){
    // 將 pattern 加入 Aho Corasick，尚未編譯。
    int nocase = 0;
    int offset = 0;
    int depth = 0;
    int negative = 0;
    void* value = patterns[i];
    int id = i;
    acsmAddPattern2(acsm, patterns[i], lengths[i], nocase, offset, depth, negative, value, id);
  }
}

void runSingleSnortWu(Config* config){
  unsigned int numberOfPattern = 0;
  unsigned char** patternGroup = NULL;
  unsigned int* patternLengthGroup = NULL;
  unsigned char** hexPatternGroup = NULL;
  unsigned int* hexPatternLengthGroup = NULL;
  readPatternFile(config->patternFile, &numberOfPattern, &patternGroup, &patternLengthGroup, &hexPatternGroup, &hexPatternLengthGroup);

  MWM_STRUCT* wu = mwmNew();
  for(unsigned int i = 0; i < numberOfPattern; i++){
    // 將 pattern 加入 Aho Corasick，尚未編譯。
    int nocase = 0;
    int offset = 0;
    int depth = 0;
    int negative = 0;
    void* value = patternGroup[i];
    int id = i;
    mwmAddPatternEx(wu, patternGroup[i], patternLengthGroup[i], nocase, 0, 0, value, id);
  }

  mwmPrepPatterns(wu);

  initClockTimer();

  unsigned long long int traceSize = 0;
  char* trace = NULL; 
  readTraceFile(config->traceFile, &traceSize, &trace);

  ClockCount.reset();
  for(unsigned int i = 0; i < config->searchRunTime; i++){
    ClockCount.begin();
    unsigned int current_state = 0;
    mwmSearch(wu, trace, traceSize, WuManberMatchFound, 0);
    ClockCount.end();
  }
    
  ClockCount.show();
  showStatisticResult();
}

int main(int argc, char** argv){
  Config* config = parseArguments(argc, argv);
  checkFileExist(config);

  if(config->isSimulateSnort){
    runSimulateSnortAcSearch(config);
    exit(0);
  }else if(config->isSimulateSnortWuManber){
    runSimulateSnortWuSearch(config);
    exit(0);
  }else if(config->isSingleSnortWu){
    runSingleSnortWu(config);
    exit(0);
  }

  unsigned int numberOfPattern = 0;
  unsigned char** patternGroup = NULL;
  unsigned int* patternLengthGroup = NULL;
  unsigned char** hexPatternGroup = NULL;
  unsigned int* hexPatternLengthGroup = NULL;
  readPatternFile(config->patternFile, &numberOfPattern, &patternGroup, &patternLengthGroup, &hexPatternGroup, &hexPatternLengthGroup);

  ACSM_STRUCT2* acsm = createAhoCorasick();

  setAcStrategy(config, acsm);

  addPatternIntoAc(acsm, numberOfPattern, patternGroup, patternLengthGroup);
  acsmCompile2(acsm);
  acsmCaleStateDepth(acsm);
  acsmCalePatternDepth(acsm);
  acsmPrintInfo2(acsm);

  postprocessor(config, acsm);

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

  // return 0;

  initClockTimer();

  unsigned long long int traceSize = 0;
  char* trace = NULL; 
  readTraceFile(config->traceFile, &traceSize, &trace);
  
  if(config->mode == 13){
    setFilterSize(traceSize);
    createOriginAc(numberOfPattern, patternGroup, patternLengthGroup);
  }else if(config->mode == 14){
    intiOddEvenQueue();
  }else if(config->mode == 0){
    initFullMatchOddEven(numberOfPattern, patternGroup, hexPatternGroup);
  }

  ClockCount.reset();
  for(unsigned int i = 0; i < config->searchRunTime; i++){
    ClockCount.begin();
    unsigned int current_state = 0;
    acsmSearch2(acsm, 0, trace, traceSize, MatchFound, (void *)0, &current_state);
    
    if(config->mode == 13){
      searchOriginAcWithFlag(trace, traceSize);
    }else if(config->mode == 14){
      fullMatchFromQueue();
    }

    ClockCount.end();
  }
    
  ClockCount.show();
  showStatisticResult();

  if(config->mode == 6){
    showStatisticStrategy();
  }else if(config->mode == 0){
    showOddEvenResult();
  }

  return 0;
}

