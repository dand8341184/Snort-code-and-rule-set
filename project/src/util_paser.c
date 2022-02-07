#include <string.h>
#include "util_paser.h"

// 將 value 以固定的單位顯示。
char* readableFixUnitValue(unsigned long long int value, unsigned short unit){
  char* valueText = calloc(10, sizeof(char));

  unsigned long long int times = value;
  float remain = 0;
  for(int i = 0; i < unit; i++){
    remain = (times % 1000) / 1000.0;
    times /= 1000;
  }

  float floatValue = times + remain;

  char* unitTexts[] = {"", "K", "M", "G", "T", "P"};
  sprintf(valueText, "%.3f %s", floatValue, unitTexts[unit]);

  return valueText;
}

void printTab(int count){
  for(unsigned int i = 0; i < count; i++){
    printf("\t");
  }
}

// 將 hex char 轉換為 hex value
char hexCharToInt(unsigned char value){
  if(value >= 'a' && value <= 'f'){
    return value - 'a' + 10;

  }else if(value >= 'A' && value <= 'F'){
    return value - 'A' + 10;
  
  }else if(value >= '0' && value <= '9'){
    return value - '0';
  }

  printf("[ERROR] Invaild hex char value: %d. \n", value);
  exit(1);
}

unsigned int hexTextLength(unsigned char* hexText){
  unsigned int hexTextLength = strlen(hexText);
  if(hexTextLength % 2 == 1){
    printf("[INFO] Invalid hex text length: %u \n", hexTextLength);
    printf("[INFO] Invalid hex text: %s \n", hexText);
    exit(0);
  }

  return hexTextLength >> 1;
}

unsigned char* hexTextToHexArray(unsigned char* hexText){
  unsigned int hexTextLength = strlen(hexText);
  if(hexTextLength % 2 == 1){
    printf("[INFO] Invalid hex text length: %u \n", hexTextLength);
    printf("[INFO] Invalid hex text: %s \n", hexText);
    exit(0);
  }

  unsigned int hexArrayLength = hexTextLength >> 1;
  unsigned char* hexArray = calloc(hexArrayLength, sizeof(unsigned char));
  for(unsigned int i = 0; i < hexTextLength; i += 2){
    unsigned char tenDigit = hexCharToInt(hexText[i]);
    unsigned char oneDigit = hexCharToInt(hexText[i+1]);
    unsigned char hexValue = (tenDigit << 4) | oneDigit;
    int index = i >> 1;
    hexArray[index] = hexValue;
  }

  return hexArray;
}

// 檢測 hex text 的長度是否為 2 的倍數，否則就是格式錯誤。
void assertHexString(int length, int patternId){
  if(length & 0x1){
    printf("[ERROR] Hex pattern length is not even at %d. \n", patternId);
    exit(1);
  }
}