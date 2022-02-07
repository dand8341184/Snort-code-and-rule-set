#ifndef __HEADER_METHOD_BITMAP_TREE__
#define __HEADER_METHOD_BITMAP_TREE__

#include "aho_corasick.h"

// typedef struct SizeList{
// 	unsigned int value;
// 	struct SizeList* next;
// } ValueList;

typedef struct BitmapTree{
  // struct BitmapTree* next;

  struct BitmapTree* childList;
  unsigned int childBitmap[256];

  unsigned int* firstCharList;
  unsigned int firstCharBitmap[256];

  unsigned int wildcardLength;
  unsigned int isFinalNode;
} BitmapTree;

typedef struct AssumeSize{
	// struct BitmapTree* next;
	struct BitmapTree* childList;
	unsigned int childBitmap[8];
	unsigned int* firstCharList;
	unsigned int firstCharBitmap[8];
	unsigned int wildcardLength;
} AssumeSize;

void createBitmapTableByCharTree(ACSM_STRUCT2 *acsm);

void fullMatchBitmapTree(ACSM_STRUCT2 *acsm, unsigned char* Tstart, unsigned char* T, acstate_t state);

void showClockTime(char* name, unsigned long long int searchClockTime);

#endif