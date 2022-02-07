#include <stdio.h>
#include <stdlib.h>

#include "method_bitmap_tree.h"
#include "method_char_tree.h"
#include "aho_corasick.h"
#include "statistic.h"

static BitmapTree** bitmapTreeTable;

static void* callocOther(unsigned int size, unsigned int byte){
	return calloc(size, byte);
}

static void* callocBitmapTree(unsigned int size, unsigned int byte){
	// sizeBitmapTree += sizeof(AssumeSize) * size;
	return calloc(size, byte);
}

static void* callocFirstChar(unsigned int size, unsigned int byte){
	return calloc(size, byte);
}

static BitmapTree* createBitmapTree(unsigned int size){
	BitmapTree* new = callocBitmapTree(size, sizeof(BitmapTree));

	for(unsigned int x = 0; x < size; x++){
		for(unsigned int i = 0; i < 256; i++){
			new[x].childBitmap[i] = 256; 
		}

		for(unsigned int i = 0; i < 256; i++){
			new[x].firstCharBitmap[i] = 256;
		}		
	}

	return new;
}

static void traverseTree(BitmapTree* bitmapTree, CharTree* charTree){
	if(charTree == NULL){
		return;
	}

	bitmapTree->wildcardLength = charTree->wildcardLength;
	if(bitmapTree->wildcardLength != 0){
		bitmapTree->isFinalNode = 1;
	}

	unsigned int countChild = 0;
	for(unsigned int i = 0; i < 256; i++){
		if(charTree->next[i] != 0){
			bitmapTree->childBitmap[i] = countChild;
			countChild++;;
		}
	}
	bitmapTree->childList = createBitmapTree(countChild);


	unsigned int countFirstChar = 0;
	for(unsigned int i = 0; i < 256; i++){
		if(charTree->firstCharList[i] != NULL){
			bitmapTree->firstCharBitmap[i] = countFirstChar;
			countFirstChar++;
		}
	}
	bitmapTree->firstCharList = callocFirstChar(countFirstChar, sizeof(unsigned int));
	if(countFirstChar != 0){
		bitmapTree->isFinalNode = 1;
	}

	for(unsigned int i = 0; i < 256; i++){
		unsigned int index = bitmapTree->firstCharBitmap[i];
		if(index != 256){
			bitmapTree->firstCharList[index] = charTree->firstCharList[i]->patternLength;
		}
	}

	for(unsigned int i = 0; i < 256; i++){
		unsigned index = bitmapTree->childBitmap[i];

		BitmapTree* next = NULL;
		if(index != 256){
			next = &bitmapTree->childList[index];
		}

		traverseTree(next, charTree->next[i]);
	}
}

void createBitmapTableByCharTree(ACSM_STRUCT2 *acsm){
	bitmapTreeTable = callocOther(acsm->acsmNumStates, sizeof(BitmapTree*));

	for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
		acstate_t* transition = acsm->acsmNextState[state];
		if(transition[1]){
			BitmapTree* bitmapTree = createBitmapTree(1);
			CharTree* charTree = acsm->acsmCharTrees[state];

			traverseTree(bitmapTree, charTree);

			bitmapTreeTable[state] = bitmapTree;
			freeCharTree(charTree);
		}
	}
}

void fullMatchBitmapTree(ACSM_STRUCT2 *acsm, unsigned char* Tstart, unsigned char* T, unsigned int state){
	unsigned int offset = T - Tstart;

	unsigned char* lastChar = T - 1;
	unsigned char* currentChar = T - 3;

	BitmapTree* rootTree = bitmapTreeTable[state];
	if(rootTree == NULL){
		return;
	}

	BitmapTree* currentTree = rootTree;
	BitmapTree* lastFinalNode = NULL;
	while(currentTree != NULL){
		if(currentTree->isFinalNode){
			lastFinalNode = currentTree;
		}

		unsigned int index = currentTree->childBitmap[*currentChar];
		if(index != 256){
			currentTree = &(currentTree->childList[index]);
		}else{
			currentTree = NULL;
		}

		currentChar -= 2;
	}

	if(lastFinalNode == NULL){
		return;
	}

	unsigned int index = lastFinalNode->firstCharBitmap[*lastChar];
	if(index != 256){
		foundCounter += lastFinalNode->firstCharList[index];
	}

	foundCounter += lastFinalNode->wildcardLength;
}