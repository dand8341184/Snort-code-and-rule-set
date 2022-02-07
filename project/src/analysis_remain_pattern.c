#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aho_corasick.h"
#include "analysis_remain_pattern.h"

// 將 final state 上的 sub pattern 以正常形式顯示，並將 hex value 轉為 00 的格式。
// 前面有空缺的部分會補 dot，某些情況下最後會也會補 dot，目前這部分應該是有 bug 的。
void showFinalStateSubPattern(ACSM_STRUCT2 *acsm){
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){

      ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
      ACSM_PATTERN2* MaxMatchList = MatchList;
      unsigned int max = MatchList->origin_length;
      for(; MatchList != NULL; MatchList = MatchList->next){
        if(MatchList->origin_length > max){
          max = MatchList->origin_length;
          MaxMatchList = MatchList;
        }
      }
      unsigned int maxPatternSize = MaxMatchList->origin_length;
      unsigned int maxPatternFlag = MaxMatchList->flag;
      unsigned int maxPatternSame = (maxPatternSize % 2) == 0;
      unsigned int maxDfaLength = MaxMatchList->origin_length - MaxMatchList->n + 1;

      MatchList = acsm->acsmMatchList[state];
      for(; MatchList != NULL; MatchList = MatchList->next){
        
        char printText[5] = "%02x";
        if(maxPatternSame == 1){
          if(MatchList->same == 1){
            if(MatchList->flag == 0){
              int frontDotLength = maxDfaLength - MatchList->n;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 1; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }
              printf("\n");
            }else{
              int frontDotLength = maxDfaLength - MatchList->n - 1;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 0; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }

              printf(".");
              printf("\n");
            }
          }else{
            if(MatchList->flag == 0){
              int frontDotLength = maxDfaLength - MatchList->n - 1;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 1; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }

              printf(".");
              printf("\n");
            }else{
              int frontDotLength = maxDfaLength - MatchList->n;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 0; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }
              printf("\n");
            }
          }
        }else{
          if(MatchList->same == 1){
            if(MatchList->flag == 0){
              int frontDotLength = maxDfaLength - MatchList->n;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 1; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }
              printf("\n");
            }else{
              int frontDotLength = maxDfaLength - MatchList->n - 1;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 0; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }
              printf(".");
              printf("\n");
            }
          }else{
            if(MatchList->flag == 0){
              int frontDotLength = maxDfaLength - MatchList->n - 1;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 1; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }
              printf(".");
              printf("\n");
            }else{
              int frontDotLength = maxDfaLength - MatchList->n;
              for(int i = 0; i < frontDotLength; i++){
                printf(".");
              }

              for(int i = 0; i < MatchList->origin_length; i += 2){
                printf(printText, MatchList->origin_patrn[i]);
              }
              printf("\n");
            }
          }
        }
      }
      printf("\n");
    }
  }
}