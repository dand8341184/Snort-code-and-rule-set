#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aho_corasick.h"
#include "analysis_remain_file.h"


// 將 final state 上的 sub pattern 已倒轉形式儲存到 buffer 中，並將 hex value 轉為 \x00 的格式。
void textSubPatternDfaRegexNonHex(char* buffer, unsigned char* pattern, int length, int isOneStart, int isFrontDot){
  char printText[10] = "\\x%02x%s";
  char tmpBuffer[1000] = "";

  int i = 0;
  if(isOneStart){
    i = 1;
  }

  for(; i < length; i += 2){
    sprintf(tmpBuffer, "%s", buffer);
    sprintf(buffer, printText, pattern[i], tmpBuffer);
  }

  if(isFrontDot){
    sprintf(tmpBuffer, "%s", buffer);
    sprintf(buffer, ".%s", tmpBuffer);
  }
}


// 將 final state 上的 sub pattern 都倒轉後儲存到檔案中，在某些條件下前方會加上 dot。
void saveFinalStateSubPatternToFile(ACSM_STRUCT2 *acsm){
  
  // 檢查 states 資料夾是否存在。
  struct stat st = {0};
  if (stat("states", &st) == -1) {
      mkdir("states", 0777);
  }

  // 計算 final state 數量。
  unsigned int countFinalStates = 0;
  // 找出擁有最多 sub pattern 的 final state，並計算 sub pattern 數量。
  unsigned int countMaxRegexes = 0;
  // 找出擁有最多 sub pattern 的 final state id。
  unsigned int countMaxRegexesStateId = 0;

  // 每個 state 都檢查一次。
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    
    // 檢查是否為 final state。
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){

      // 寫入到 states 資料夾內的檔案，用 state id 命名。
      char fileName[100] = "";
      sprintf(fileName, "states/%06u", state);
      FILE *stateFile = fopen(fileName, "w");

      // 計算 final state 內有幾條 sub pattern。
      unsigned int countRegexes = 0;

      // 走訪 final state 內每一條 sub pattern，並印出倒轉後的 sub pattern。 
      ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
      for(; MatchList != NULL; MatchList = MatchList->next){
        char buffer[1000] = "";

        if(MatchList->same == 1){
          // 如果 odd pattern 和 even pattern 一樣長。
          if(MatchList->flag == 0){
            // 如果是 odd pattern。
            textSubPatternDfaRegexNonHex(buffer, MatchList->origin_patrn, MatchList->origin_length, 1, 0);
          }else{
            // 如果是 even pattern。
            textSubPatternDfaRegexNonHex(buffer, MatchList->origin_patrn, MatchList->origin_length, 0, 1);
          }
        }else{
          // 如果 odd pattern 和 even pattern 不一樣長。
          if(MatchList->flag == 0){
            textSubPatternDfaRegexNonHex(buffer, MatchList->origin_patrn, MatchList->origin_length, 1, 1);
          }else{
            textSubPatternDfaRegexNonHex(buffer, MatchList->origin_patrn, MatchList->origin_length, 0, 0);
          }
        }
        fprintf(stateFile, "%s\n", buffer);

        countRegexes++;
        
      }
      fclose(stateFile);
      printf("[INFO] State file %06u is saved. \n", state);

      // 替換最大值。
      countFinalStates++;
      if(countRegexes > countMaxRegexes){
        countMaxRegexes = countRegexes;
        countMaxRegexesStateId = state;
      }
    }
  }

  printf("[INFO] Count number of final state: %u \n", countFinalStates);
  printf("[INFO] Max number of regex in final state: %u \n", countMaxRegexes);
  printf("[INFO] Final state id with max number of regex: %06u \n", countMaxRegexesStateId);
}