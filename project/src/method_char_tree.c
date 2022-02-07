#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "method_char_tree.h"
#include "statistic.h"
#include "util_paser.h"

void* callocCharTree(unsigned int size, unsigned int byte){
  return calloc(size, byte);
}

void* callocFirstChar(unsigned int size, unsigned int byte){
  return calloc(size, byte);
}


void* callocPatternList(unsigned int size, unsigned int byte){
  return calloc(size, byte);
}

void freeCharTree(CharTree* node){
  if(node == NULL){
    return;
  }

  for(unsigned int i = 0; i < 256; i++){
    freeCharTree(node->next[i]);
    if(node->next[i] != NULL){
      free(node->next[i]);
    }
  }
}

void printfStringHex(unsigned char* pattern, unsigned int patternSize){
  printf("[INFO] String hex: ");
  for(unsigned int i = 0; i < patternSize; i++){
    printf("%02x", pattern[i]);
  }
  printf("\n");
}

void traverseTree(CharTree* node, unsigned int level){
  if(node == NULL){
    return;
  }

  printTab(level);
  printf("------------------\n");
  printTab(level);
  printf("[INFO] node: %p\n", node);
  printTab(level);
  printf("[INFO] final: %u\n", node->isFinalNode);

  if(node->isFinalNode){
    for(unsigned int i = 0; i < 256; i++){
      if(node->firstCharList[i] != NULL){
        printTab(level);
        printf("[INFO] first, len: %02x, %u \n", i, node->firstCharList[i]->patternLength);
      }
    }
    printTab(level);
    printf("[INFO] wild, len: %u, %u \n", node->wildcardList != NULL, node->wildcardLength);
  }



  for(unsigned int i = 0; i < 256; i++){
    if(node->next[i] != NULL){
      
      printTab(level);
      printf("[INFO] next: %02x -> %p\n\n", i, node->next[i]);
      traverseTree(node->next[i], level + 1);
    }
  }
}


// 將 final state 上的 sub pattern 已倒轉形式儲存到 buffer 中，並將第一個字元特別取出，並設定 flag。
void valueSubPatternReverseNoFirstChar(char* buffer, unsigned int* bufferLength, unsigned char* isWildCard, unsigned char* firstChar, unsigned char* pattern, int length, int isOneStart, int isFrontDot){
  // printfStringHex(pattern, length);

  char printText[10] = "%c%s";
  char tmpBuffer[1000] = "";

  int i = 0;
  if(isOneStart){
    i = 1;
  }

  if(isFrontDot == 0){
    length = length - 2;
  }

  unsigned int index = 0;
  for(; i < length; i += 2){
    buffer[index] = pattern[i];
    index++;
    (*bufferLength)++;
  }

  unsigned int left = 0;
  unsigned int right = index - 1;
  while(left < right){
    unsigned char tmp = buffer[left];
    buffer[left] = buffer[right];
    buffer[right] = tmp;
    left++;
    right--;
  }

  // printfStringHex(buffer, *bufferLength);

  if(isFrontDot){
    *isWildCard = 1;
    *firstChar = 0;
  }else{
    *isWildCard = 0;
    *firstChar = pattern[i];
  }

  // printf("[INFO] isWildCard: %u\n", *isWildCard);
  // printf("[INFO] Front char: %02x\n", *firstChar);
}

void processCharTree(ACSM_STRUCT2 *acsm, acstate_t state, unsigned int patternId, 
	char* buffer, unsigned int length, unsigned char isWildCard, 
	unsigned char firstChar){

  if(length == 0){
    return;
  }

  if(acsm->acsmCharTrees[state] == NULL){
    acsm->acsmCharTrees[state] = callocCharTree(1, sizeof(CharTree));
  }

  // printf("[INFO] char tree state: %u\n", state);

  CharTree* current = acsm->acsmCharTrees[state];
  unsigned int depth = 0;
  for(int i = 0; i < length; i++){
    // printf("[INFO]     char: %02x\n", (unsigned char)buffer[i]);
    unsigned char value = buffer[i];

    // printf("[INFO] state:%u, run node: %p, char:%02x\n", state, current, value);
    if(current->next[value] == NULL){
      CharTree* node = callocCharTree(1, sizeof(CharTree));
      current->next[value] = node;
    }

    current = current->next[value];
    depth++;
  }

  if(depth > acsm->acsmTreeDepths[state]){
    acsm->acsmTreeDepths[state] = depth;
  }

  // if(isWildCard){
  //   printf("[INFO]     first char: .\n");
  // }else{
  //   printf("[INFO]     first char: %02x\n", firstChar);
  // }
  
  if(isWildCard){
    if(current->wildcardList == NULL){
      PatternList* newNode = callocPatternList(1, sizeof(PatternList));
      newNode->id = patternId;
      current->wildcardList = newNode;
      current->wildcardLength++;
    }else{
      PatternList* newNode = callocPatternList(1, sizeof(PatternList));
      newNode->id = patternId;
      newNode->next = current->wildcardList;
      current->wildcardList = newNode;
      current->wildcardLength++;
    }
    // printf("[INFO] state:%u, add wild: %p, pattern id:%u \n", state, current, patternId);
  }else{
    if(current->firstCharList[firstChar] == NULL){
      current->firstCharList[firstChar] = callocFirstChar(1, sizeof(FirstCharList));
    }

    FirstCharList* firstCharList = current->firstCharList[firstChar];
    if(firstCharList->patternList == NULL){
      PatternList* newNode = callocPatternList(1, sizeof(PatternList));
      newNode->id = patternId;
      firstCharList->patternList = newNode;
      firstCharList->patternLength++;
    }else{
      PatternList* newNode = callocPatternList(1, sizeof(PatternList));
      newNode->id = patternId;
      newNode->next = firstCharList->patternList;
      firstCharList->patternList = newNode;
      firstCharList->patternLength++;
    }
    // printf("[INFO] state:%u, add patr: %p, pattern id:%u \n", state, current, patternId);
  }

  current->isFinalNode = 1;
}

void duplicateCharTreeFinalNode(CharTree* node, CharTree* prevFinalNode){
  if(node == NULL){
		return;
	}

	if(prevFinalNode != NULL){

		for(unsigned int i = 0; i < 256; i++){
			
			if(prevFinalNode->firstCharList[i] == NULL){
				continue;
			}

			FirstCharList* prevCharList = prevFinalNode->firstCharList[i];
			if(node->firstCharList[i] == NULL){
				node->firstCharList[i] = prevCharList;
        // printf("[INFO] duplicate %p->%p step 1 char: %02x\n", prevFinalNode, node, i);
				continue;
			}

			FirstCharList* charList = node->firstCharList[i];
			PatternList* prevPatternList = prevCharList->patternList;
      unsigned int prevPatternLength = prevCharList->patternLength;
      if(prevPatternList == NULL){
        // printf("[INFO] duplicate %p->%p step 2 char: %02x\n", prevFinalNode, node, i);
        continue;
      }

			if(charList->patternList == NULL){
				charList->patternList = prevPatternList;
        charList->patternLength = prevPatternLength;
        // printf("[INFO] duplicate %p->%p step 3 char: %02x\n", prevFinalNode, node, i);
				continue;
			}

			PatternList* currentPatternList = charList->patternList;
			while(currentPatternList->next != NULL){
				currentPatternList = currentPatternList->next;
			}

			currentPatternList->next = prevPatternList;
      charList->patternLength += prevPatternLength;

      // printf("[INFO] duplicate %p->%p step 4 char: %02x\n", prevFinalNode, node, i);
		}

		PatternList* prevWildcardList = prevFinalNode->wildcardList;
    unsigned int prevWildcardLength = prevFinalNode->wildcardLength;
    if(prevWildcardList != NULL){
      if(node->wildcardList == NULL){
        node->wildcardList = prevWildcardList;
        node->wildcardLength = prevWildcardLength;
        // printf("[INFO] duplicate %p->%p step 5 \n", prevFinalNode, node);
      }else{
        PatternList* currentWildcardList = node->wildcardList;
        while(currentWildcardList->next != NULL){
          currentWildcardList = currentWildcardList->next;
        }
        currentWildcardList->next = prevWildcardList;
        node->wildcardLength += prevWildcardLength;
        // printf("[INFO] duplicate %p->%p step 6 \n", prevFinalNode, node);
      }
    }

    node->isFinalNode = 1;
	}
	
  if(node->isFinalNode){
    prevFinalNode = node;
  }

	for(unsigned int i = 0; i < 256; i++){
		CharTree* nextNode = node->next[i];
		duplicateCharTreeFinalNode(nextNode, prevFinalNode);
	}
}

void leafPushingCharTree(ACSM_STRUCT2* acsm){
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
		CharTree* currentCharTree = acsm->acsmCharTrees[state];
    duplicateCharTreeFinalNode(currentCharTree, NULL);

    // printf("[INFO] Leaf pushing state: %u. \r", state);
	}
  printf("\n");
}

// 將 final state 上的 sub pattern 都倒轉後顯示，在某些條件下前方會加上 dot。
// 註解同 saveFinalStateSubPatternToFile 方法，只差在 printf 和 fprintf。
void processFinalStateSubPatternToCharTree(ACSM_STRUCT2 *acsm){
  acsm->acsmCharTrees = callocCharTree(acsm->acsmNumStates, sizeof(CharTree*));
  acsm->acsmTreeDepths = callocCharTree(acsm->acsmNumStates, sizeof(unsigned int));

  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    // printf("[INFO] Processing state char tree: %u. \r", state);

    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state];
      for(; MatchList != NULL; MatchList = MatchList->next){
        unsigned int length = 0;
        char buffer[1000] = "";
        char isWildCard = 0;
        char firstChar = 0;

        if(MatchList->same == 1){
          if(MatchList->flag == 0){
            valueSubPatternReverseNoFirstChar(buffer, &length, &isWildCard, &firstChar, MatchList->origin_patrn, MatchList->origin_length, 1, 0);
            // printf("[INFO] s = 1, f = 0 \n");
          }else{
            valueSubPatternReverseNoFirstChar(buffer, &length, &isWildCard, &firstChar, MatchList->origin_patrn, MatchList->origin_length, 0, 1);
            // printf("[INFO] s = 1, f = 1 \n");
          }
        }else{
          if(MatchList->flag == 0){
            valueSubPatternReverseNoFirstChar(buffer, &length, &isWildCard, &firstChar, MatchList->origin_patrn, MatchList->origin_length, 1, 1);
            // printf("[INFO] s = 0, f = 0 \n");
          }else{
            valueSubPatternReverseNoFirstChar(buffer, &length, &isWildCard, &firstChar, MatchList->origin_patrn, MatchList->origin_length, 0, 0);
            // printf("[INFO] s = 0, f = 1 \n");
          }
        }
        processCharTree(acsm, state, MatchList->iid, buffer, length, isWildCard, firstChar);
      }
    }
  }

  printf("\n");
  leafPushingCharTree(acsm);

  // for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
  //   if(acsm->acsmCharTrees[state] == NULL){
  //     continue;
  //   }

  //   printf("[INFO] Ac state: %u\n", state);
  //   traverseTree(acsm->acsmCharTrees[state], 0);
  // }
}

void fullMatchCharTree(ACSM_STRUCT2 *acsm, unsigned char* Tstart, unsigned char* T, unsigned int state){
  unsigned int offset = T - Tstart;

  // printf("[INFO] Ac match state: %u\n", state);

  unsigned char* lastChar = T - 1;
  unsigned char* currentChar = T - 3;

  CharTree* rootCharTree = acsm->acsmCharTrees[state];
  if(rootCharTree == NULL){
    return;
  }

  CharTree* currentCharTree = rootCharTree;
  CharTree* lastFinalNode = NULL;
  while(currentCharTree != NULL){
    // printf("[INFO]     compare char: %02x\n", *currentChar);
    
    // if(currentCharTree->isFinalNode){
    //   lastFinalNode = currentCharTree;
    // }

    // for(unsigned int i = 0; i < 256; i++){
    //   printf("[INFO]      first char: %02x : %p\n", i, currentCharTree->next[*currentChar]);
    // }

    lastFinalNode = currentCharTree;
    currentCharTree = currentCharTree->next[*currentChar];
    // if(currentCharTree == NULL){
    //   printf("[INFO]     compare char: fail\n");
    // }else{
    //   printf("[INFO]     compare char: success\n");
    // }

    currentChar -= 2;
  }

  if(lastFinalNode == NULL){
    return;
  }

  if(lastFinalNode->isFinalNode == 0){
    return;
  }

  FirstCharList* lastCharList = lastFinalNode->firstCharList[*lastChar];
  if(lastCharList != NULL){
    foundCounter += lastCharList->patternLength;

    // PatternList* patternList = lastCharList->patternList;
    // while(patternList != NULL){
    //   patternIdList[patternList->id]++;
    //   patternList = patternList->next;
    // }
  }

  foundCounter += lastFinalNode->wildcardLength;

  // PatternList* wildcardList = lastFinalNode->wildcardList;
  // while(wildcardList != NULL){
  //   patternIdList[wildcardList->id]++;
  //   wildcardList = wildcardList->next;
  // }

  // printf("[INFO] state:%u, offset: %u\n", state, offset);
}