#ifndef __HEADER_AHO_CORASICK__
#define __HEADER_AHO_CORASICK__

#include <pthread.h>

enum {
  ACF_FULL,
  ACF_SPARSE,
  ACF_BANDED,
  ACF_SPARSEBANDS,
  ACF_FULLQ
};

enum {
  FSA_TRIE,
  FSA_NFA,
  FSA_DFA
};

struct _acsm_pattern2;
struct trans_node_s;
struct ACSM_STRUCT2;

#ifndef ACSMX2S_H
#define ACSMX2S_H
#define MAX_ALPHABET_SIZE 256
#define AC32

#ifdef AC32
typedef  unsigned int   acstate_t;
#define ACSM_FAIL_STATE2  0xffffffff
#else

typedef    unsigned short acstate_t;
#define ACSM_FAIL_STATE2 0xffff

#include "char_tree.h"

#endif

typedef struct _acsm_pattern2{
    struct  _acsm_pattern2 *next;

    unsigned char* origin_patrn;
    unsigned int origin_length;
    unsigned int flag;
    unsigned int same;
    unsigned int indexEnd;
    unsigned int start;
    unsigned int wildcard;
    unsigned int lastHashID;
    unsigned int selfPatternId;
    unsigned int subPatternId;

    unsigned int* subPatternArray;

    unsigned char* otherPattern;
    int other_length;

    unsigned char         *patrn;
    int      n;
    int      nocase;
    int      offset;
    int      depth;
    int      negative;
    void*    udata;
    int      iid;

    unsigned int matchDepth;
    unsigned int* selfMark;
    unsigned int* otherMark;

    unsigned int ownState;

} ACSM_PATTERN2;

/*
*    transition nodes  - either 8 or 12 bytes
*/
typedef struct trans_node_s {

  acstate_t    key;           /* The character that got us here - sized to keep structure aligned on 4 bytes */
                              /* to better the caching opportunities. A value that crosses the cache line */
                              /* forces an expensive reconstruction, typing this as acstate_t stops that. */
  acstate_t    next_state;    /*  */
  struct trans_node_s * next; /* next transition for this state */

} trans_node_t;

#define AC_MAX_INQ 32
typedef struct{
    unsigned inq;
    unsigned inq_flush;
    void * q[AC_MAX_INQ];
} PMQ;

/*
*   Aho-Corasick State Machine Struct - one per group of pattterns
*/
typedef struct ACSM_STRUCT2{
    int acsmMaxStates;
    int acsmNumStates;

    ACSM_PATTERN2    * acsmPatterns;
    acstate_t        * acsmFailState;
    ACSM_PATTERN2   ** acsmMatchList;

    unsigned char** acsmTrans;
    unsigned int* acsmLeaf;
    unsigned int* acsmDepth;

    struct CharTree** acsmCharTrees;
    unsigned int* acsmTreeDepths;

    /* list of transitions in each state, this is used to build the nfa & dfa */
    /* after construction we convert to sparse or full format matrix and free */
    /* the transition lists */
    trans_node_t ** acsmTransTable;

    acstate_t ** acsmNextState;
    int          acsmFormat;
    int          acsmNumTrans;
    int          acsmAlphabetSize;
    int          acsmFSA;
    int          numPatterns;
    PMQ q;
    int sizeofstate;
    int compress_states;

    unsigned int acMode;
    void (*callbackAcMatch)(struct ACSM_STRUCT2*, unsigned char*, unsigned char*, unsigned int);
    void* otherData;

    unsigned int lastState;
    unsigned int countConitnue;
    unsigned char lastChar;
    unsigned char continueChar;

    unsigned short* repeatSizeGroup;
    unsigned char** repeatCharGroup;
    unsigned char** repeatValueGroup;

    unsigned int lastFoundPatternCount;
    unsigned int continueFlag;

    int acsm2_total_memory;
    int acsm2_pattern_memory;
    int acsm2_matchlist_memory;
    int acsm2_transtable_memory;
    int acsm2_dfa_memory;
    int acsm2_dfa1_memory;
    int acsm2_dfa2_memory;
    int acsm2_dfa4_memory;
    int acsm2_failstate_memory;
    int other_memory;

    unsigned int oneLenFalg;
    unsigned int* oneLenTable;
    unsigned int subPatternSize;

    unsigned int* sizeLastOneState;
    unsigned int** lastOneStateGroup;
    unsigned int** lastOneCountGroup;

    unsigned int* sizeLastTwoState;
    unsigned int** lastTwoStateGroup;
    unsigned int** lastTwoCountGroup;

    unsigned int* finalStateDepthGroup;
    unsigned char* textEnd;
    unsigned char* subText;
    unsigned int subState;

    ACSM_PATTERN2** oneHashTable;
    ACSM_PATTERN2** twoHashTable;

    short*** finalPatternGroup;
    unsigned int* maxLengthGroup;
    unsigned int lastPosition;

    unsigned long long int countCharacterComparison;
    unsigned long long int countFullMatch;
    unsigned long long int countRestPatternComparison;
    unsigned long long int countRepetitionVisit;
    unsigned short repetitionUsingTable[256];
    unsigned long long int clockCycle;

} ACSM_STRUCT2;


ACSM_STRUCT2* createAhoCorasick();
ACSM_STRUCT2 * acsmNew2 ();
int acsmAddPattern2( ACSM_STRUCT2 * p, unsigned char * pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid );
int acsmCompile2 ( ACSM_STRUCT2 * acsm);

struct _SnortConfig;

int acsmSearch2 ( ACSM_STRUCT2 * acsm, unsigned long long int offset, unsigned char * T, int n, int (*Match)(void * id, int index, void *data), void * data, int* current_state );
int acsmPatternCount2 ( ACSM_STRUCT2 * acsm );
void acsmPrintInfo2( ACSM_STRUCT2 * p);

void setCallbackAcMatch(ACSM_STRUCT2 * acsm, void (*callback)(ACSM_STRUCT2*, unsigned char*, unsigned char*, unsigned int));


void setAcMode(ACSM_STRUCT2 * acsm, unsigned int mode);

void acsmCaleStateDepth(ACSM_STRUCT2* acsm);

void acsmCalePatternDepth(ACSM_STRUCT2* acsm);

void fullMatchTwiceOddEven(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state);

typedef struct OneLenThreadData{
    ACSM_STRUCT2 *acsm;
    unsigned char *text;
    int length;
    int matchFounder;
} OneLenThreadData;

int createOnLenThread(pthread_t* thread, OneLenThreadData* threadData);

int searchOneLenWithOddEven(ACSM_STRUCT2 *acsm, unsigned char *T, int n);

#endif

#endif