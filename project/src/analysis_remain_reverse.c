#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aho_corasick.h"
#include "analysis_remain_reverse.h"

// 將 final state 上的 sub pattern 已倒轉形式儲存到 buffer 中，並將 hex value 轉為 00 的格式。
// 如果遇上 \ 或是 . 會加上 \ (5c) 做跳脫字元。
void textSubPatternDfaRegex(char* buffer, unsigned char* pattern, int length, int isOneStart, int isFrontDot){
  char escapeSlash[20] = "5c%02x%s";
  char printText[10] = "%02x%s";
  char tmpBuffer[1000] = "";

  int i = 0;
  if(isOneStart){
    i = 1;
  }

  for(; i < length; i += 2){
    sprintf(tmpBuffer, "%s", buffer);
    if(pattern[i] == '\\' | pattern[i] == '.'){
      sprintf(buffer, escapeSlash, pattern[i], tmpBuffer);
    }else{
      sprintf(buffer, printText, pattern[i], tmpBuffer);
    }
  }

  if(isFrontDot){
    sprintf(tmpBuffer, "%s", buffer);
    sprintf(buffer, "2e%s", tmpBuffer);
  }
}

// 將 final state 上的 sub pattern 都倒轉後顯示，在某些條件下前方會加上 dot。
// 註解同 saveFinalStateSubPatternToFile 方法，只差在 printf 和 fprintf。
void showFinalStateSubPatternReverse(ACSM_STRUCT2 *acsm){
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
      for(; MatchList != NULL; MatchList = MatchList->next){
        char buffer[1000] = "";

        if(MatchList->same == 1){
          if(MatchList->flag == 0){
            textSubPatternDfaRegex(buffer, MatchList->origin_patrn, MatchList->origin_length, 1, 0);
          }else{
            textSubPatternDfaRegex(buffer, MatchList->origin_patrn, MatchList->origin_length, 0, 1);
          }
        }else{
          if(MatchList->flag == 0){
            textSubPatternDfaRegex(buffer, MatchList->origin_patrn, MatchList->origin_length, 1, 1);
          }else{
            textSubPatternDfaRegex(buffer, MatchList->origin_patrn, MatchList->origin_length, 0, 0);
          }
        }
        printf("%s\n", buffer);
      }
      printf("\n");
    }
  }
}