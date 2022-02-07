include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "twice_aho_corasick.h"
#include "statistic.h"
#include "clock_count.h"

unsigned int acMode = 0;

static int acsm2_total_memory = 0;
static int acsm2_pattern_memory = 0;
static int acsm2_matchlist_memory = 0;
static int acsm2_transtable_memory = 0;
static int acsm2_dfa_memory = 0;
static int acsm2_dfa1_memory = 0;
static int acsm2_dfa2_memory = 0;
static int acsm2_dfa4_memory = 0;
static int acsm2_failstate_memory = 0;
static int s_verbose=0;

void (*callbackAcMatch)(ACSM_STRUCT2*, unsigned char*, unsigned char*, unsigned int);

void setAcMode(unsigned int mode){
  acMode = mode;
}

typedef enum _Acsm2MemoryType
{
    ACSM2_MEMORY_TYPE__NONE = 0,
    ACSM2_MEMORY_TYPE__PATTERN,
    ACSM2_MEMORY_TYPE__MATCHLIST,
    ACSM2_MEMORY_TYPE__TRANSTABLE,
    ACSM2_MEMORY_TYPE__FAILSTATE

} Acsm2MemoryType;

static void * AC_MALLOC( int n, Acsm2MemoryType type ) {
    void *p = calloc(1, n);

    if (p != NULL)
    {
        switch (type)
        {
            case ACSM2_MEMORY_TYPE__PATTERN:
                acsm2_pattern_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__MATCHLIST:
                acsm2_matchlist_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__TRANSTABLE:
                acsm2_transtable_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__FAILSTATE:
                acsm2_failstate_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__NONE:
                break;
            default:
                // FatalError("%s(%d) Invalid memory type\n", __FILE__, __LINE__);
                break;
        }

        acsm2_total_memory += n;
    }

    return p;
}

static void * AC_MALLOC_DFA( int n, int sizeofstate ) {
    void *p = calloc(1, n);

    if (p != NULL)
    {
        switch (sizeofstate)
        {
            case 1:
                acsm2_dfa1_memory += n;
                break;
            case 2:
                acsm2_dfa2_memory += n;
                break;
            case 4:
            default:
                acsm2_dfa4_memory += n;
                break;
        }

        acsm2_dfa_memory += n;
        acsm2_total_memory += n;
    }

    return p;
}


static void AC_FREE( void *p, int n, Acsm2MemoryType type ) {
    if (p != NULL){
        switch (type)
        {
            case ACSM2_MEMORY_TYPE__PATTERN:
                acsm2_pattern_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__MATCHLIST:
                acsm2_matchlist_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__TRANSTABLE:
                acsm2_transtable_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__FAILSTATE:
                acsm2_failstate_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__NONE:
            default:
                break;
        }

        acsm2_total_memory -= n;
        free(p);
    }
}

typedef struct _qnode{
  int state;
   struct _qnode *next;
} QNODE;

typedef struct _queue{
  QNODE * head, *tail;
  int count;
} QUEUE;

static void queue_init (QUEUE * s) {
  s->head = s->tail = 0;
  s->count= 0;
}

static int queue_find (QUEUE * s, int state) {
  QNODE * q;
  q = s->head;
  while( q )
  {
      if( q->state == state ) return 1;
      q = q->next;
  }
  return 0;
}

static void queue_add (QUEUE * s, int state) {
  QNODE * q;

  if( queue_find( s, state ) ) return;

  if (!s->head)
  {
      q = s->tail = s->head =
          (QNODE *)AC_MALLOC(sizeof(QNODE), ACSM2_MEMORY_TYPE__NONE);
      // MEMASSERT (q, "queue_add");
      q->state = state;
      q->next = 0;
  }
  else
  {
      q = (QNODE *)AC_MALLOC(sizeof(QNODE), ACSM2_MEMORY_TYPE__NONE);
      // MEMASSERT (q, "queue_add");
      q->state = state;
      q->next = 0;
      s->tail->next = q;
      s->tail = q;
  }
  s->count++;
}


static int queue_remove (QUEUE * s) {
  int state = 0;
  QNODE * q;
  if (s->head)
  {
      q       = s->head;
      state   = q->state;
      s->head = s->head->next;
      s->count--;

      if( !s->head )
      {
          s->tail = 0;
          s->count = 0;
      }
      AC_FREE(q, sizeof(QNODE), ACSM2_MEMORY_TYPE__NONE);
  }
  return state;
}


static int queue_count (QUEUE * s) {
  return s->count;
}


static void queue_free (QUEUE * s) {
  while (queue_count (s)){
      queue_remove (s);
  }
}

static int List_GetNextState( ACSM_STRUCT2 * acsm, int state, int input ) {
  trans_node_t * t = acsm->acsmTransTable[state];

  while( t )
  {
    if( t->key == (acstate_t)input )
    {
        return t->next_state;
    }
    t=t->next;
  }

  if( state == 0 ) return 0;

  return ACSM_FAIL_STATE2; /* Fail state ??? */
}

static int List_GetNextState2( ACSM_STRUCT2 * acsm, int state, int input ) {
  trans_node_t * t = acsm->acsmTransTable[state];

  while( t )
  {
    if( t->key == (acstate_t)input )
    {
      return t->next_state;
    }
    t = t->next;
  }

  return 0; /* default state */
}

static int List_PutNextState( ACSM_STRUCT2 * acsm, int state, int input, int next_state ) {
  trans_node_t * p;
  trans_node_t * tnew;

 // printf("   List_PutNextState: state=%d, input='%c', next_state=%d\n",state,input,next_state);


  /* Check if the transition already exists, if so just update the next_state */
  p = acsm->acsmTransTable[state];
  while( p )
  {
    /* transition already exists- reset the next state */
    if( p->key == (acstate_t)input )
    {
        p->next_state = next_state;
        return 0;
    }
    p=p->next;
  }

  /* Definitely not an existing transition - add it */
  tnew = (trans_node_t*)AC_MALLOC(sizeof(trans_node_t),
          ACSM2_MEMORY_TYPE__TRANSTABLE);
  if( !tnew ) return -1;

  tnew->key        = input;
  tnew->next_state = next_state;
  tnew->next       = 0;

  tnew->next = acsm->acsmTransTable[state];
  acsm->acsmTransTable[state] = tnew;

  acsm->acsmNumTrans++;

  return 0;
}

static int List_FreeTransTable( ACSM_STRUCT2 *acsm ) {
    int i;
    trans_node_t *t, *p;

    if (acsm->acsmTransTable == NULL)
        return 0;

    for (i = 0; i < acsm->acsmMaxStates; i++)
    {
        t = acsm->acsmTransTable[i];

        while (t != NULL)
        {
            p = t->next;
            AC_FREE(t, sizeof(trans_node_t), ACSM2_MEMORY_TYPE__TRANSTABLE);
            t = p;
        }
    }

    AC_FREE(acsm->acsmTransTable, sizeof(void*) * acsm->acsmMaxStates,
            ACSM2_MEMORY_TYPE__TRANSTABLE);

    acsm->acsmTransTable = NULL;

    return 0;
}

static int List_PrintTransTable( ACSM_STRUCT2 * acsm ) {
  int i;
  trans_node_t * t;
  ACSM_PATTERN2 * patrn;

  if( !acsm->acsmTransTable ) return 0;

  printf("Print Transition Table- %d active states\n",acsm->acsmNumStates);

  for(i=0;i< acsm->acsmNumStates;i++)
  {
     t = acsm->acsmTransTable[i];

     printf("state %3d: ",i);

     while( t )
     {
       if( isascii((int)t->key) && isprint((int)t->key) )
         printf("%3c->%-5d\t",t->key,t->next_state);
       else
         printf("%3d->%-5d\t",t->key,t->next_state);

       t = t->next;
     }

     patrn =acsm->acsmMatchList[i];

     while( patrn )
     {
         printf("%.*s ",patrn->n,patrn->patrn);

         patrn = patrn->next;
     }

     printf("\n");
   }
   return 0;
}

static int List_ConvToFull( ACSM_STRUCT2 *acsm, acstate_t state, acstate_t *full ) {
    int tcnt = 0;
    trans_node_t *t = acsm->acsmTransTable[state];

    memset(full, 0, acsm->sizeofstate * acsm->acsmAlphabetSize);

    if (t == NULL)
        return 0;

    while (t != NULL)
    {
        full[t->key] = t->next_state;
        tcnt++;
        t = t->next;
    }

    return tcnt;
}

static ACSM_PATTERN2* CopyMatchListEntry (ACSM_PATTERN2 * px) {
  ACSM_PATTERN2 * p = (ACSM_PATTERN2 *)AC_MALLOC(sizeof (ACSM_PATTERN2), ACSM2_MEMORY_TYPE__MATCHLIST);
  memcpy (p, px, sizeof (ACSM_PATTERN2));
  p->next = 0;
  return p;
}

static void AddMatchListEntry (ACSM_STRUCT2 * acsm, int state, ACSM_PATTERN2 * px) {
  ACSM_PATTERN2 * p;

  p = (ACSM_PATTERN2 *)AC_MALLOC(sizeof (ACSM_PATTERN2),
          ACSM2_MEMORY_TYPE__MATCHLIST);

  memcpy (p, px, sizeof (ACSM_PATTERN2));

  p->next = acsm->acsmMatchList[state];

  acsm->acsmMatchList[state] = p;
}

static void AddPatternStates (ACSM_STRUCT2 * acsm, ACSM_PATTERN2 * p) {
  int            state, next, n;
  unsigned char *pattern;

  n       = p->n;
  pattern = p->patrn;
  state   = 0;

  if(s_verbose)printf(" Begin AddPatternStates: acsmNumStates=%d\n",acsm->acsmNumStates);
  if(s_verbose)printf("    adding '%.*s', nocase=%d\n", n,p->patrn, p->nocase );

  /*
  *  Match up pattern with existing states
  */
  for (; n > 0; pattern++, n--)
  {
      if(s_verbose)printf(" find char='%c'\n", *pattern );

      next = List_GetNextState(acsm,state,*pattern);
      if ((acstate_t)next == ACSM_FAIL_STATE2 || next == 0)
      {
             break;
      }
      state = next;
  }

  /*
  *   Add new states for the rest of the pattern bytes, 1 state per byte
  */
  for (; n > 0; pattern++, n--)
  {
      if(s_verbose)printf(" add char='%c' state=%d NumStates=%d\n", *pattern, state, acsm->acsmNumStates );

      acsm->acsmNumStates++;
      List_PutNextState(acsm,state,*pattern,acsm->acsmNumStates);
      state = acsm->acsmNumStates;
  }

  AddMatchListEntry (acsm, state, p );

  if(s_verbose)printf(" End AddPatternStates: acsmNumStates=%d\n",acsm->acsmNumStates);
}

static void Build_NFA (ACSM_STRUCT2 * acsm) {
    int r, s, i;
    QUEUE q, *queue = &q;
    acstate_t     * FailState = acsm->acsmFailState;
    ACSM_PATTERN2 ** MatchList = acsm->acsmMatchList;
    ACSM_PATTERN2  * mlist,* px;

    /* Init a Queue */
    queue_init (queue);


    /* Add the state 0 transitions 1st, the states at depth 1, fail to state 0 */
    for (i = 0; i < acsm->acsmAlphabetSize; i++)
    {
      s = List_GetNextState2(acsm,0,i);
      if( s )
      {
          queue_add (queue, s);
          FailState[s] = 0;
      }
    }

    /* Build the fail state successive layer of transitions */
    while (queue_count (queue) > 0)
    {
        r = queue_remove (queue);

        /* Find Final States for any Failure */
        for (i = 0; i < acsm->acsmAlphabetSize; i++)
        {
           int fs, next;

           s = List_GetNextState(acsm,r,i);

           if( (acstate_t)s != ACSM_FAIL_STATE2 )
           {
                queue_add (queue, s);

                fs = FailState[r];

                /*
                 *  Locate the next valid state for 'i' starting at fs
                 */
                while ((acstate_t)(next = List_GetNextState(acsm,fs,i))
                       == ACSM_FAIL_STATE2 )
                {
                    fs = FailState[fs];
                }

                /*
                 *  Update 's' state failure state to point to the next valid state
                 */
                FailState[s] = next;

                /*
                 *  Copy 'next'states MatchList to 's' states MatchList,
                 *  we copy them so each list can be AC_FREE'd later,
                 *  else we could just manipulate pointers to fake the copy.
                 */
                for( mlist = MatchList[next];
                     mlist;
                     mlist = mlist->next)
                {
                    px = CopyMatchListEntry (mlist);

                    /* Insert at front of MatchList */
                    px->next = MatchList[s];
                    MatchList[s] = px;
                }
           }
        }
    }

    /* Clean up the queue */
    queue_free (queue);

    if( s_verbose)printf("End Build_NFA: NumStates=%d\n",acsm->acsmNumStates);
}

static void Convert_NFA_To_DFA (ACSM_STRUCT2 * acsm) {
    int i, r, s, cFailState;
    QUEUE  q, *queue = &q;
    acstate_t * FailState = acsm->acsmFailState;

    /* Init a Queue */
    queue_init (queue);

    acsm->acsmTrans = calloc(acsm->acsmNumStates, sizeof(unsigned char*));  // 為了計算 Depth
    acsm->acsmTrans[0] = calloc(256, sizeof(unsigned char));                // 為了計算 Depth

    /* Add the state 0 transitions 1st */
    for(i=0; i<acsm->acsmAlphabetSize; i++)
    {
      s = List_GetNextState(acsm,0,i);
      if ( s != 0 )
      {
          queue_add (queue, s);
          acsm->acsmTrans[r][i] = 1; // 為了計算 Depth
      }else{
          acsm->acsmTrans[r][i] = 0; // 為了計算 Depth
      }
    }

    /* Start building the next layer of transitions */
    while( queue_count(queue) > 0 )
    {
        r = queue_remove(queue);

        acsm->acsmTrans[r] = calloc(256, sizeof(unsigned char)); // 為了計算 Depth
        
        /* Process this states layer */
        for (i = 0; i < acsm->acsmAlphabetSize; i++)
        {
          s = List_GetNextState(acsm,r,i);

          if( (acstate_t)s != ACSM_FAIL_STATE2 && s!= 0)
          {
              queue_add (queue, s);
              acsm->acsmTrans[r][i] = 1; // 為了計算 Depth
          }
          else
          {
              acsm->acsmTrans[r][i] = 0; // 為了計算 Depth

              cFailState = List_GetNextState(acsm,FailState[r],i);

              if( cFailState != 0 && (acstate_t)cFailState != ACSM_FAIL_STATE2 )
              {
                  List_PutNextState(acsm,r,i,cFailState);
              }
          }
        }
    }

    /* Clean up the queue */
    queue_free (queue);

    if(s_verbose)printf("End Convert_NFA_To_DFA: NumStates=%d\n",acsm->acsmNumStates);

}

static int Conv_List_To_Full( ACSM_STRUCT2 *acsm ) {
    acstate_t k;
    acstate_t *p;
    acstate_t **NextState = acsm->acsmNextState;

    for (k = 0; k < (acstate_t)acsm->acsmNumStates; k++)
    {
        p = AC_MALLOC_DFA(acsm->sizeofstate * (acsm->acsmAlphabetSize + 2),
                acsm->sizeofstate);
        if (p == NULL)
            return -1;

        switch (acsm->sizeofstate)
        {
            case 1:
                List_ConvToFull(acsm, k, (acstate_t *)((uint8_t *)p + 2));
                *((uint8_t *)p) = ACF_FULL;
                *((uint8_t *)p + 1) = 0;
                break;
            case 2:
                List_ConvToFull(acsm, k, (acstate_t *)((uint16_t *)p + 2));
                *((uint16_t *)p) = ACF_FULL;
                *((uint16_t *)p + 1) = 0;
                break;
            default:
                List_ConvToFull(acsm, k, (p + 2));
                p[0] = ACF_FULL;
                p[1] = 0; /* no matches yet */
                break;
        }

        NextState[k] = p; /* now we have a full format row vector  */
    }

    return 0;
}

static void Print_DFA_MatchList( ACSM_STRUCT2 * acsm, int state) {
    for(int k = 0; k < acsm->acsmNumStates; k++){
      for (ACSM_PATTERN2* mlist = acsm->acsmMatchList[k]; mlist; mlist = mlist->next){
        printf("state:%d => %.*s \n", k, mlist->n, mlist->patrn);
      }
    }
}

static void printFullQ(ACSM_STRUCT2 * acsm){
  acstate_t **NextState = acsm->acsmNextState;

  printf("digraph \"patterns\" {\n");
  for(acstate_t state = 0; state < acsm->acsmNumStates; state++){
    acstate_t *ps = NextState[state];
    
    if(ps[1]){
      printf("  N%d [shape=doublecircle,label=\"%d\",color=\"black\"];\n", state, state);
    }else{
      printf("  N%d [shape=circle,label=\"%d\",color=\"black\"];\n", state, state);
    }

    for(int i = 0; i < acsm->acsmAlphabetSize; i++ ){
      int nextState = ps[2 + i];
      if(nextState != 0){
        printf("  N%d -> N%d [label=\"%02x\",color=\"black\"];\n", state, nextState, i);
      }
    }
  }
   printf("}\n");
}

static void Print_DFA(ACSM_STRUCT2 * acsm) {
  int  k,i;
  acstate_t * p, state, n, fmt, index, nb;
  acstate_t ** NextState = acsm->acsmNextState;

  printf("Print DFA - %d active states\n",acsm->acsmNumStates);

  for(k=0;k<acsm->acsmNumStates;k++)
  {
    p   = NextState[k];

    if( !p ) continue;

    fmt = *p++;

    printf("state %3d, fmt=%d: ",k,fmt);

    if( fmt ==ACF_SPARSE )
    {
       n = *p++;
       for( ; n>0; n--, p+=2 )
       {
         if( isascii((int)p[0]) && isprint((int)p[0]) )
         printf("%3c->%-5d\t",p[0],p[1]);
         else
         printf("%3d->%-5d\t",p[0],p[1]);
      }
    }
    else if( fmt ==ACF_BANDED )
    {

       n = *p++;
       index = *p++;

       for( ; n>0; n--, p++ )
       {
         if( isascii((int)p[0]) && isprint((int)p[0]) )
         printf("%3c->%-5d\t",index++,p[0]);
         else
         printf("%3d->%-5d\t",index++,p[0]);
      }
    }
    else if( fmt ==ACF_SPARSEBANDS )
    {
       nb    = *p++;
       for(i=0;(acstate_t)i<nb;i++)
       {
         n     = *p++;
         index = *p++;
         for( ; n>0; n--, p++ )
         {
           if( isascii((int)index) && isprint((int)index) )
           printf("%3c->%-5d\t",index++,p[0]);
           else
           printf("%3d->%-5d\t",index++,p[0]);
         }
       }
    }
    else if( fmt == ACF_FULL )
    {

      for( i = 0; i < acsm->acsmAlphabetSize; i++ )
      {
         state = p[i];

         if( state != 0 && state != ACSM_FAIL_STATE2 )
         {
           if( isascii(i) && isprint(i) )
             printf("%3c->%-5d\t",i,state);
           else
             printf("%3d->%-5d\t",i,state);
         }
      }
    }

    for (ACSM_PATTERN2* mlist = acsm->acsmMatchList[k]; mlist; mlist = mlist->next){
      printf("%.*s", mlist->n, mlist->patrn);
    }

    printf("\n");
  }
}
/*
*  Write a state table to disk
*/
/*
static void
Write_DFA(ACSM_STRUCT2 * acsm, char * f)
{
  int  k,i;
  acstate_t * p, n, fmt, index, nb, bmatch;
  acstate_t ** NextState = acsm->acsmNextState;
  FILE * fp;

  printf("Dump DFA - %d active states\n",acsm->acsmNumStates);

  fp = fopen(f,"wb");
  if(!fp)
   {
     printf("WARNING: could not write dfa to file - %s.\n",f);
     return;
   }

  fwrite( &acsm->acsmNumStates, 4, 1, fp);

  for(k=0;k<acsm->acsmNumStates;k++)
  {
    p   = NextState[k];

    if( !p ) continue;

    fmt = *p++;

    bmatch = *p++;

    fwrite( &fmt,    sizeof(acstate_t), 1, fp);
    fwrite( &bmatch, sizeof(acstate_t), 1, fp);

    if( fmt ==ACF_SPARSE )
    {
       n = *p++;
       fwrite( &n,     sizeof(acstate_t), 1, fp);
       fwrite(  p, n*2*sizeof(acstate_t), 1, fp);
    }
    else if( fmt ==ACF_BANDED )
    {
       n = *p++;
       fwrite( &n,     sizeof(acstate_t), 1, fp);

       index = *p++;
       fwrite( &index, sizeof(acstate_t), 1, fp);

       fwrite(  p, sizeof(acstate_t), n, fp);
    }
    else if( fmt ==ACF_SPARSEBANDS )
    {
       nb    = *p++;
       fwrite( &nb,    sizeof(acstate_t), 1, fp);
       for(i=0;i<nb;i++)
       {
         n     = *p++;
         fwrite( &n,    sizeof(acstate_t), 1, fp);

         index = *p++;
         fwrite( &index,sizeof(acstate_t), 1, fp);

         fwrite( p,     sizeof(acstate_t), 1, fp);
       }
    }
    else if( fmt == ACF_FULL )
    {
      fwrite( p,  sizeof(acstate_t), acsm->acsmAlphabetSize,  fp);
    }

    //Print_DFA_MatchList( acsm, k);

  }

  fclose(fp);
}
*/

ACSM_STRUCT2* acsmNew2() {
  ACSM_STRUCT2* p = (ACSM_STRUCT2 *)AC_MALLOC(sizeof (ACSM_STRUCT2), ACSM2_MEMORY_TYPE__NONE);

  if (p){
    memset (p, 0, sizeof (ACSM_STRUCT2));

    p->acsmFSA               = FSA_DFA;
    p->acsmFormat            = ACF_FULL;
    p->acsmAlphabetSize      = 256;
  }

  return p;
}

int acsmAddPatternWithOddEven (ACSM_STRUCT2 * p, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(n < 2){
    return 0;
  }

  int halfLength = (n >> 1) + 1;

  unsigned char** splitPatterns = AC_MALLOC(sizeof(unsigned char*) * 2, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[0] = AC_MALLOC(halfLength, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[1] = AC_MALLOC(halfLength, ACSM2_MEMORY_TYPE__PATTERN);

  int splitPatternLengths[2];
  splitPatternLengths[0] = 0;
  splitPatternLengths[1] = 0;

  unsigned char* splitPatternIndex = splitPatterns[0];
  for(int i = 0; i < n; i += 2){
    *splitPatternIndex = pat[i];
    splitPatternIndex++;
    splitPatternLengths[0]++;
  }

  splitPatternIndex = splitPatterns[1];
  for(int i = 1; i < n; i += 2){
    *splitPatternIndex = pat[i];
    splitPatternIndex++;
    splitPatternLengths[1]++;
  }

  unsigned int same = (splitPatternLengths[0] == splitPatternLengths[1]);
  
  for(int i = 0; i < 2; i++){
    unsigned char* splitPattern = splitPatterns[i];
    int patternLength = splitPatternLengths[i];

    ACSM_PATTERN2* plist = AC_MALLOC(sizeof(ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

    plist->patrn = AC_MALLOC(patternLength, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->patrn, splitPattern, patternLength);

    plist->origin_patrn = AC_MALLOC(n, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->origin_patrn, pat, n);

    plist->flag = i;
    plist->origin_length = n;

    plist->n      = patternLength;
    plist->nocase = nocase;
    plist->offset = offset;
    plist->depth  = depth;
    plist->negative = negative;
    plist->iid    = iid;
    plist->udata  = id;
    plist->same   = same; 

    plist->next     = p->acsmPatterns;
    p->acsmPatterns = plist;
    p->numPatterns++;
  }

  return 0;
}

int acsmAddPatternOrigin (ACSM_STRUCT2 * p, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  ACSM_PATTERN2 * plist;

  plist = (ACSM_PATTERN2 *)AC_MALLOC(sizeof (ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

  plist->patrn = (unsigned char *)AC_MALLOC(n, ACSM2_MEMORY_TYPE__PATTERN);
  memcpy (plist->patrn, pat, n);

  plist->n      = n;
  plist->nocase = nocase;
  plist->offset = offset;
  plist->depth  = depth;
  plist->negative = negative;
  plist->iid    = iid;
  plist->udata = id;

  plist->next     = p->acsmPatterns;
  p->acsmPatterns = plist;
  p->numPatterns++;

  return 0;
}

int acsmAddPattern2 (ACSM_STRUCT2 * p, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(acMode == 1){
    return acsmAddPatternWithOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else{
    return acsmAddPatternOrigin(p, pat, n, nocase, offset, depth, negative, id, iid);
  }
}

static void acsmUpdateMatchStates( ACSM_STRUCT2 *acsm ) {
    acstate_t state;
    acstate_t **NextState = acsm->acsmNextState;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    for (state = 0; state < (acstate_t)acsm->acsmNumStates; state++)
    {
        acstate_t *p = NextState[state];
        
        if (MatchList[state]){
            p[1] = 1;
        }
    }
}

static inline int _acsmCompile2( ACSM_STRUCT2* acsm )
{
    ACSM_PATTERN2* plist;

    for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next){
        acsm->acsmMaxStates += plist->n;
    }
    acsm->acsmMaxStates++;

    acsm->acsmTransTable = (trans_node_t**)AC_MALLOC(sizeof(trans_node_t*) * acsm->acsmMaxStates, ACSM2_MEMORY_TYPE__TRANSTABLE);
    acsm->acsmMatchList = (ACSM_PATTERN2 **)AC_MALLOC(sizeof(ACSM_PATTERN2*) * acsm->acsmMaxStates, ACSM2_MEMORY_TYPE__MATCHLIST);

    acsm->acsmNumStates = 0;
    for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next){
        AddPatternStates(acsm, plist);
    }
    acsm->acsmNumStates++;
    acsm->sizeofstate = 4;

    acsm->acsmFailState = (acstate_t*)AC_MALLOC(sizeof(acstate_t) * acsm->acsmNumStates, ACSM2_MEMORY_TYPE__FAILSTATE);
    acsm->acsmNextState = (acstate_t**)AC_MALLOC_DFA(acsm->acsmNumStates * sizeof(acstate_t*), acsm->sizeofstate);

    if ((acsm->acsmFSA == FSA_DFA) || (acsm->acsmFSA == FSA_NFA)){
        Build_NFA(acsm);
    }

    if (acsm->acsmFSA == FSA_DFA){
        Convert_NFA_To_DFA(acsm);
    }

    if (acsm->acsmFormat == ACF_FULLQ) {
        if (Conv_List_To_Full(acsm))
            return -1;

        AC_FREE(acsm->acsmFailState, sizeof(acstate_t) * acsm->acsmNumStates, ACSM2_MEMORY_TYPE__FAILSTATE);
        acsm->acsmFailState = NULL;
    }

    acsmUpdateMatchStates(acsm);
    List_FreeTransTable(acsm);

    return 0;
}

int acsmCompile2(ACSM_STRUCT2* acsm){
    int rval;

    if ((rval = _acsmCompile2(acsm)))
        return rval;

    return 0;
}

static inline int acsmSearchOrigin( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T++){
      
      ps = NextState[state];
      sindex = T[0];

      if (ps[1]){
        if (MatchList[state]){
          for(ACSM_PATTERN2* mlist = MatchList[state]; mlist != NULL; mlist = mlist->next){
              callbackAcMatch(acsm, Tstart, T, state);
              // patternIdList[mlist->iid]++;
              countFullMatchAccess++;
          }
        }
      }
      state = ps[2 + sindex];
      
      countStateAccess++;
      countNextStateAccess++;
      countCharacterAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      for(ACSM_PATTERN2* mlist = MatchList[state]; mlist != NULL; mlist = mlist->next){
          callbackAcMatch(acsm, Tstart, T, state);
          // patternIdList[mlist->iid]++;
           countFullMatchAccess++;
      }
    }

    return 0;
}

static inline int acsmSearchWithOddEven( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T += 2){
      ps = NextState[state];
      sindex = T[0];

      if (ps[1]){
        callbackAcMatch(acsm, Tstart, T, state);
      }

      state = ps[2 + sindex];

      countStateAccess++;
      countNextStateAccess++;
      countCharacterAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

int acsmSearch2(ACSM_STRUCT2 * acsm, unsigned long long int offset, unsigned char *Tx, int n, int (*Match)(void * id, int index, void *data), void *data, int* current_state ){
    if(acMode == 1){
      int result = acsmSearchWithOddEven(acsm, offset, Tx, n, Match, data, current_state);
      return result;
    }else{
      int result = acsmSearchOrigin(acsm, offset, Tx, n, Match, data, current_state);
      return result;
    }
}

void acsmPrintInfo2( ACSM_STRUCT2 * p)
{
    char * sf[]={
      "Full Matrix",
      "Sparse Matrix",
      "Banded Matrix",
      "Sparse Banded Matrix",
      "Full-Q Matrix"
    };
    char * fsa[]={
      "TRIE",
      "NFA",
      "DFA"
    };


    printf("+--[Pattern Matcher:Aho-Corasick]-----------------------------\n");
    printf("| Alphabet Size    : %d Chars\n",p->acsmAlphabetSize);
    if (p->compress_states)
    printf("| Sizeof State     : %d\n", p->sizeofstate);
    else
    printf("| Sizeof State     : %d bytes\n",(int)(sizeof(acstate_t)));
    printf("| Storage Format   : %s \n",sf[ p->acsmFormat ]);
    printf("| Num States       : %d\n",p->acsmNumStates);
    printf("| Num Transitions  : %d\n",p->acsmNumTrans);
    printf("| State Density    : %.1f%%\n",100.0*(double)p->acsmNumTrans/(p->acsmNumStates*p->acsmAlphabetSize));
    printf("| Finite Automaton : %s\n", fsa[p->acsmFSA]);
    
    if( acsm2_total_memory < 1024*1024 ){
      printf("| Memory           : %.2fKbytes\n", (float)acsm2_total_memory/1024 );
    }else{
      printf("| Memory           : %.2fMbytes\n", (float)acsm2_total_memory/(1024*1024) );
    }
    printf("+-------------------------------------------------------------\n");

    /* Print_DFA(acsm); */
}

// 建立 Aho Corasick。 
ACSM_STRUCT2* createAhoCorasick(){
  ACSM_STRUCT2* acsm = acsmNew2();
  if( !acsm ){
     printf("[INFO] Not enough memory to create aho corasick. \n");
     exit(0);
  }

  acsm->acsmFormat = ACF_FULLQ;
  acsm->acsmFSA = FSA_DFA;
}

void setCallbackAcMatch(void (*callback)(ACSM_STRUCT2*, unsigned char*, unsigned char*, unsigned int)){
  callbackAcMatch = callback;
}

void acsmAddDepthLoop(ACSM_STRUCT2 *acsm, acstate_t state, unsigned int depth){
  acsm->acsmDepth[state] = depth;

  unsigned int isLeafNode = 1;
  for(unsigned int i = 0; i < 256; i++){
    if(acsm->acsmTrans[state][i] == 1){
      isLeafNode = 0;
      break;
    }
  }

  if(isLeafNode){
    return;
  }

  for(unsigned int i = 0; i < 256; i++){
    if(acsm->acsmTrans[state][i] == 1){
      unsigned int nextState = acsm->acsmNextState[state][2 + i];
      acsmAddDepthLoop(acsm, nextState, depth + 1);
    }
  }
}

void acsmCaleStateDepth(ACSM_STRUCT2 *acsm){
  acsm->acsmDepth = calloc(acsm->acsmNumStates, sizeof(unsigned int));
  
  acsmAddDepthLoop(acsm, 0, 0);
}

void acsmUpdatePatternDepth(ACSM_STRUCT2 *acsm, acstate_t state, unsigned int depth){
  acsm->acsmDepth[state] = depth;

  unsigned int isLeafNode = 1;
  for(unsigned int i = 0; i < 256; i++){
    if(acsm->acsmTrans[state][i] == 1){
      isLeafNode = 0;
      break;
    }
  }

  if(isLeafNode){
    return;
  }

  for(unsigned int i = 0; i < 256; i++){
    if(acsm->acsmTrans[state][i] == 1){
      unsigned int nextState = acsm->acsmNextState[state][2 + i];
      acsmAddDepthLoop(acsm, nextState, depth + 1);
    }
  }
}

void acsmCalePatternDepth(ACSM_STRUCT2* acsm){
  for(unsigned int state = 0; state < acsm->acsmNUmStates; state++){
    unsigned int depth = acsm->acsmDepth[state];
    acstate_t* transition = acsm->acsmNextState[state];
    if(transition[1]){
      for(ACSM_PATTERN2* MatchList = acsm->acsmMatchList[state]; MatchList != NULL; MatchList = MatchList->next){
        if(MatchList->origin_length < 4){
          MatchList->matchDepth = depth - 1;
          continue;
        }

        MatchList->matchDepth = (MatchList->flag == 0 && MatchList->same == 0) ? depth - 1 : MatchList->matchDepth;
        MatchList->matchDepth = (MatchList->flag == 0 && MatchList->same == 1) ? depth : MatchList->matchDepth;
        MatchList->matchDepth = (MatchList->flag == 1 && MatchList->same == 0) ? depth : MatchList->matchDepth;
        MatchList->matchDepth = (MatchList->flag == 1 && MatchList->same == 1) ? depth - 1 : MatchList->matchDepth;
      }
    }
  }
}