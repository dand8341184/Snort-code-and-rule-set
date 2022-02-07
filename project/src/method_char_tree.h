#ifndef __HEADER_METHOD_CHAR_TREE__
#define __HEADER_METHOD_CHAR_TREE__

struct PatternList;
struct FirstCharList;
struct CharTree;

#include "aho_corasick.h"

typedef struct PatternList{
  unsigned int id;
  struct PatternList* next;
} PatternList;

typedef struct FirstCharList{
  struct PatternList* patternList;
  unsigned int patternLength;
} FirstCharList;

typedef struct CharTree{
  struct CharTree* next[256];
  struct FirstCharList* firstCharList[256];
  struct PatternList* wildcardList;
  unsigned int wildcardLength;
  unsigned short isFinalNode;
} CharTree;

void processCharTree(ACSM_STRUCT2 *acsm, acstate_t state, unsigned int patternId, 
	char* buffer, unsigned int length, unsigned char isWildCard, 
	unsigned char firstChar);

void processFinalStateSubPatternToCharTree(ACSM_STRUCT2 *acsm);

void fullMatchCharTree(ACSM_STRUCT2 *acsm, unsigned char* Tstart, unsigned char* T, unsigned int state);

void freeCharTree(CharTree* node);

#endif