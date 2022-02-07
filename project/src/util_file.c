#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "aho_corasick.h"
#include "util_paser.h"
#include "util_file.h"
#include "main.h"

// 檢查執行程式所需的檔案是否存在。
int checkFileExist(Config* config){
  if(access(config->patternFile, F_OK) == -1){
    printf("[INFO] Pattern file %s is not existing. \n", config->patternFile);
    exit(0);
  }else{
    printf("[INFO] Pattern file %s. \n", config->patternFile);
  }

  if(access(config->traceFile, F_OK) == -1){
    printf("[INFO] Trace file %s is not existing. \n", config->traceFile);
    exit(0);
  }else{
    printf("[INFO] Trace file %s. \n", config->traceFile);
  }

  printf("\n");
}

unsigned int getFileLines(unsigned char* fileName){
  FILE* file = fopen(fileName, "r");

  unsigned int countFileLines = 0;
  unsigned char buffer[1024];
  while(fscanf(file, "%s\n", buffer) != EOF){
    countFileLines++;
  }

  fclose(file);
  return countFileLines;
}

// // 讀取 Pattern 並加入 Aho Corasick。
void readPatternFile(unsigned char* fileName, unsigned int* size, 
  unsigned char*** patterns, unsigned int** lengths,
  unsigned char*** hexPatterns, unsigned int** hexLengths){
  
  unsigned int groupSize = getFileLines(fileName);
  unsigned char** patternGroup = calloc(groupSize, sizeof(unsigned char*));
  unsigned int* lengthGroup = calloc(groupSize, sizeof(unsigned int));
  unsigned char** hexPatternGroup = calloc(groupSize, sizeof(unsigned char*));
  unsigned int* hexLengthGroup = calloc(groupSize, sizeof(unsigned int));

  // 開啟 pattern 檔案。
  FILE* patternFile = fopen(fileName, "r");

  // 一次讀取一行 pattern。
  char buffer[1024];
  unsigned int countLines = 0;
  while(fscanf(patternFile, "%s\n", buffer) != EOF){
    unsigned int inputLength = strlen(buffer);
    assertHexString(inputLength, countLines);

    // 因為是 hex text 的形式，總長度除以 2。
    unsigned int patternLength = inputLength / 2;
    unsigned char* patternString = calloc(patternLength, sizeof(unsigned char*));
    hexPatternGroup[countLines] = calloc(inputLength, sizeof(unsigned char*));
    hexLengthGroup[countLines] = inputLength;
    for(unsigned int i = 0; i < inputLength; i++){
      hexPatternGroup[countLines][i] = buffer[i];
    }

    // 將 hex text 轉為 hex value。
    for(int i = 0; i < inputLength; i += 2){
      char tenDigit = hexCharToInt(buffer[i]);
      char oneDigit = hexCharToInt(buffer[i+1]);
      int index = i >> 1;
      char hexValue = (tenDigit << 4) | oneDigit;
      patternString[index] = hexValue;
    }

    patternGroup[countLines] = patternString;
    lengthGroup[countLines] = patternLength;

    countLines++;
  }

  *size = groupSize;
  *patterns = patternGroup;
  *lengths = lengthGroup;
  *hexPatterns = hexPatternGroup;
  *hexLengths = hexLengthGroup;
}


// 讀取 trace file，以 hex text 的形式。
void readTraceFile(char* traceFileName, unsigned long long int* size, char** trace){
  // 開啟 trace 檔案。
  FILE *traceFile = fopen(traceFileName, "r");

  // 每次讀取 1024 * 1024 + 1 這個長度的字串。
  unsigned long long int fileSize = 0;
  // The reason is fgets will add the trailing \0 at the end.
  unsigned int lengthBufferSize = 1024 * 1024 + 1;
  char lengthBuffer[lengthBufferSize];
  while(fgets(lengthBuffer, lengthBufferSize, traceFile) != NULL){
    fileSize += strlen(lengthBuffer);
  }
  printf("[INFO] Trace File Size: %llu \n", fileSize);
  printf("[INFO] Trace Input Real Size: %llu \n", fileSize >> 1);

  // 讓檔案指標回到原點。
  // printf("[INFO] File size: %llu \n", fileSize);
  rewind(traceFile);

  // 將 trace 一次全部讀取出來。
  // The reason is fgets will add the trailing \0 at the end.
  unsigned long long int traceBufferSize = fileSize + 1;
  char* traceBuffer = calloc(traceBufferSize, sizeof(char));
  if(traceBuffer == NULL){
      printf("[INFO] Trace File Memory Allocation Failed. \n");
      exit(0);
  }
  fgets(traceBuffer, traceBufferSize, traceFile);

  // 因為是 hex text 的形式，總長度除以 2。
  unsigned long long int inputLength = fileSize >> 1;
  // char inputString[inputLength];
  char* inputString = calloc(inputLength, sizeof(char));

  // 將 hex text 轉為 hex value。
  for(int i = 0; i < fileSize; i += 2){
    if(fileSize - i < 10){
      printf("%u\n", i);
    }
    char tenDigit = hexCharToInt(traceBuffer[i]);
    char oneDigit = hexCharToInt(traceBuffer[i+1]);
    int index = i >> 1;
    char hexValue = (tenDigit << 4) | oneDigit;
    inputString[index] = hexValue;
  }

  *size = inputLength;
  *trace = inputString;
}