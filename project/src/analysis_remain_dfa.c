#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aho_corasick.h"
#include "analysis_remain_dfa.h"

// 呼叫 regex 程式將 regex 檔案轉換成 dfa 檔案。
void processRegexToDfa(unsigned int stateSize){
  
  // 檢查 dfa 資料夾是否存在，不存在就建立。
  struct stat st = {0};
  if (stat("dfa", &st) == -1) {
    mkdir("dfa", 0777);
  }

  // 每個 state 都走一次檢查 regex 檔案。
  for(unsigned int i = 0; i < stateSize; i++){    
    
    // 檢查 regex 檔案是否存在，不存在就跳過。
    char fileName[100] = "";
    sprintf(fileName, "states/%06u", i);
    if(access(fileName, F_OK) == -1){
      printf("[INFO] State file %06u is not existing. \n", i);
      continue;
    }


    // 呼叫 regex 程式，檢查 exit code 是否執行成功。
    char command[1000] = "";
    sprintf(command, "./regex -p states/%06u > /dev/null 2>&1", i);
    int exitCode = system(command);
    if(exitCode != 0){
      printf("[INFO] regex Exit code: %u \n", exitCode);
      exit(0);
    }

    // 呼叫 cp 複製 regex 程式產生的 dot 檔案。
    sprintf(command, "cp graph_dfa_mini ./dfa/%06u", i);
    exitCode = system(command);
    if(exitCode != 0){
       printf("[INFO] mv Exit code: %u \n", exitCode);
      exit(0);
    }

    printf("[INFO] State file %06u is transformed to Dfa file %06u. \n", i, i);
  }
}

// 將 Dfa 的檔案轉換成 Dfa 的資料結構，還沒寫完。
void processDfaFileToDfaData(ACSM_STRUCT2 *acsm){
  // typedef struct DfaNode{
  //   struct DfaNode* next[256];
  //   struct short isFinal; 
  // } DfaNode;

  // for(unsigned int i = 0; i < stateSize; i++){
  //   char fileName[100] = "";
  //   sprintf(fileName, "dfa/%06u", i);
  //   if(access(fileName, F_OK) == -1){
  //     printf("[INFO] Dfa file %06u is not existing. \n", i);
  //   }else{
  //     FILE *dfaFile = fopen(fileName, "r");
  //     char buffer[1024];
  //     while(fscanf(dfaFile, "%s\n", buffer) != EOF){
  //     }
  //   }
  // }
}