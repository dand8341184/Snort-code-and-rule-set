#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>

#include "aho_corasick.h"
#include "statistic.h"
#include "clock_count.h"
#include "method_hash_small_pattern.h"
#include "method_last_char_hash.h"
#include "method_odd_even_ac_filter.h"

#include "method_odd_even_snort.h"

int s_verbose=0; 

void setAcMode(ACSM_STRUCT2 * acsm, unsigned int mode){
  acsm->acMode = mode;
}

typedef enum _Acsm2MemoryType
{
    ACSM2_MEMORY_TYPE__NONE = 0,
    ACSM2_MEMORY_TYPE__PATTERN,
    ACSM2_MEMORY_TYPE__MATCHLIST,
    ACSM2_MEMORY_TYPE__TRANSTABLE,
    ACSM2_MEMORY_TYPE__FAILSTATE

} Acsm2MemoryType;

static void * AC_MALLOC(ACSM_STRUCT2* acsm, int n, Acsm2MemoryType type ) {
    void *p = calloc(1, n);

    if (p != NULL)
    {
        switch (type)
        {
            case ACSM2_MEMORY_TYPE__PATTERN:
                acsm->acsm2_pattern_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__MATCHLIST:
                acsm->acsm2_matchlist_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__TRANSTABLE:
                acsm->acsm2_transtable_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__FAILSTATE:
                acsm->acsm2_failstate_memory += n;
                break;
            case ACSM2_MEMORY_TYPE__NONE:
                break;
            default:
                // FatalError("%s(%d) Invalid memory type\n", __FILE__, __LINE__);
                break;
        }

        acsm->acsm2_total_memory += n;
    }

    return p;
}

static void * AC_MALLOC_DFA(ACSM_STRUCT2* acsm, int n, int sizeofstate ) {
    void *p = calloc(1, n);

    if (p != NULL)
    {
        switch (sizeofstate)
        {
            case 1:
                acsm->acsm2_dfa1_memory += n;
                break;
            case 2:
                acsm->acsm2_dfa2_memory += n;
                break;
            case 4:
            default:
                acsm->acsm2_dfa4_memory += n;
                break;
        }

        acsm->acsm2_dfa_memory += n;
        acsm->acsm2_total_memory += n;
    }

    return p;
}


static void AC_FREE(ACSM_STRUCT2* acsm, void *p, int n, Acsm2MemoryType type ) {
    if (p != NULL){
        switch (type)
        {
            case ACSM2_MEMORY_TYPE__PATTERN:
                acsm->acsm2_pattern_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__MATCHLIST:
                acsm->acsm2_matchlist_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__TRANSTABLE:
                acsm->acsm2_transtable_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__FAILSTATE:
                acsm->acsm2_failstate_memory -= n;
                break;
            case ACSM2_MEMORY_TYPE__NONE:
            default:
                break;
        }

        acsm->acsm2_total_memory -= n;
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

static void queue_add (ACSM_STRUCT2* acsm, QUEUE * s, int state) {
  QNODE * q;

  if( queue_find( s, state ) ) return;

  if (!s->head)
  {
      q = s->tail = s->head =
          (QNODE *)AC_MALLOC(acsm, sizeof(QNODE), ACSM2_MEMORY_TYPE__NONE);
      // MEMASSERT (q, "queue_add");
      q->state = state;
      q->next = 0;
  }
  else
  {
      q = (QNODE *)AC_MALLOC(acsm, sizeof(QNODE), ACSM2_MEMORY_TYPE__NONE);
      // MEMASSERT (q, "queue_add");
      q->state = state;
      q->next = 0;
      s->tail->next = q;
      s->tail = q;
  }
  s->count++;
}


static int queue_remove (ACSM_STRUCT2* acsm, QUEUE * s) {
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
      AC_FREE(acsm, q, sizeof(QNODE), ACSM2_MEMORY_TYPE__NONE);
  }
  return state;
}


static int queue_count (QUEUE * s) {
  return s->count;
}


static void queue_free (ACSM_STRUCT2 * acsm, QUEUE * s) {
  while (queue_count (s)){
      queue_remove (acsm, s);
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
  tnew = (trans_node_t*)AC_MALLOC(acsm, sizeof(trans_node_t),
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
            AC_FREE(acsm, t, sizeof(trans_node_t), ACSM2_MEMORY_TYPE__TRANSTABLE);
            t = p;
        }
    }

    AC_FREE(acsm, acsm->acsmTransTable, sizeof(void*) * acsm->acsmMaxStates,
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

static int List_ConvToFull(ACSM_STRUCT2 *acsm, acstate_t state, acstate_t *full ) {
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

static ACSM_PATTERN2* CopyMatchListEntry (ACSM_STRUCT2* acsm, ACSM_PATTERN2 * px) {
  ACSM_PATTERN2 * p = (ACSM_PATTERN2 *)AC_MALLOC(acsm, sizeof (ACSM_PATTERN2), ACSM2_MEMORY_TYPE__MATCHLIST);
  memcpy (p, px, sizeof (ACSM_PATTERN2));
  p->next = 0;
  return p;
}

static void AddMatchListEntry (ACSM_STRUCT2 * acsm, int state, ACSM_PATTERN2 * px) {
  ACSM_PATTERN2 * p;

  p = (ACSM_PATTERN2 *)AC_MALLOC(acsm, sizeof (ACSM_PATTERN2),
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
          queue_add (acsm, queue, s);
          FailState[s] = 0;
      }
    }

    /* Build the fail state successive layer of transitions */
    while (queue_count (queue) > 0)
    {
        r = queue_remove (acsm, queue);

        /* Find Final States for any Failure */
        for (i = 0; i < acsm->acsmAlphabetSize; i++)
        {
           int fs, next;

           s = List_GetNextState(acsm,r,i);

           if( (acstate_t)s != ACSM_FAIL_STATE2 )
           {
                queue_add (acsm, queue, s);

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
                    px = CopyMatchListEntry(acsm, mlist);

                    /* Insert at front of MatchList */
                    px->next = MatchList[s];
                    MatchList[s] = px;
                }
           }
        }
    }

    /* Clean up the queue */
    queue_free (acsm, queue);

    if( s_verbose)printf("End Build_NFA: NumStates=%d\n",acsm->acsmNumStates);
}

static void Convert_NFA_To_DFA (ACSM_STRUCT2 * acsm) {
    int i = 0;
    int r = 0;
    int s = 0;
    int cFailState = 0;
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
          queue_add(acsm, queue, s);
          acsm->acsmTrans[r][i] = 1; // 為了計算 Depth
      }else{
          acsm->acsmTrans[r][i] = 0; // 為了計算 Depth
      }
    }

    /* Start building the next layer of transitions */
    while( queue_count(queue) > 0 )
    {
        r = queue_remove(acsm, queue);

        acsm->acsmTrans[r] = calloc(256, sizeof(unsigned char)); // 為了計算 Depth
        
        /* Process this states layer */
        for (i = 0; i < acsm->acsmAlphabetSize; i++)
        {
          s = List_GetNextState(acsm,r,i);

          if( (acstate_t)s != ACSM_FAIL_STATE2 && s!= 0)
          {
              queue_add(acsm, queue, s);
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
    queue_free(acsm, queue);

    if(s_verbose)printf("End Convert_NFA_To_DFA: NumStates=%d\n",acsm->acsmNumStates);

}

static int Conv_List_To_Full(ACSM_STRUCT2 *acsm ) {
    acstate_t k;
    acstate_t *p;
    acstate_t **NextState = acsm->acsmNextState;

    for (k = 0; k < (acstate_t)acsm->acsmNumStates; k++)
    {
        p = AC_MALLOC_DFA(acsm, acsm->sizeofstate * (acsm->acsmAlphabetSize + 2),
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
  ACSM_STRUCT2* acsm = (ACSM_STRUCT2 *)malloc(sizeof(ACSM_STRUCT2));

  if (acsm){
    memset (acsm, 0, sizeof (ACSM_STRUCT2));

    acsm->acsmFSA               = FSA_DFA;
    acsm->acsmFormat            = ACF_FULL;
    acsm->acsmAlphabetSize      = 256;

    acsm->acsm2_total_memory = sizeof(ACSM_STRUCT2);
    acsm->acsm2_pattern_memory = 0;
    acsm->acsm2_matchlist_memory = 0;
    acsm->acsm2_transtable_memory = 0;
    acsm->acsm2_dfa_memory = 0;
    acsm->acsm2_dfa1_memory = 0;
    acsm->acsm2_dfa2_memory = 0;
    acsm->acsm2_dfa4_memory = 0;
    acsm->acsm2_failstate_memory = 0;
    acsm->other_memory = 0;
    acsm->subPatternSize = 0;

    acsm->oneLenFalg = 0;
  }

  return acsm;
}

int acsmAddPatternWithTwiceOddEven(ACSM_STRUCT2 * acsm, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(n < 2){
    return 0;
  }

  int halfLength = (n >> 1) + 1;

  unsigned char** splitPatterns = AC_MALLOC(acsm, sizeof(unsigned char*) * 2, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[0] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[1] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);

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
  ACSM_PATTERN2* patternGroup[2];
  for(int i = 0; i < 2; i++){
    unsigned char* splitPattern = splitPatterns[i];
    int patternLength = splitPatternLengths[i];

    ACSM_PATTERN2* plist = AC_MALLOC(acsm, sizeof(ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

    plist->patrn = AC_MALLOC(acsm, patternLength, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->patrn, splitPattern, patternLength);

    plist->origin_patrn = AC_MALLOC(acsm, n, ACSM2_MEMORY_TYPE__PATTERN);
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

    plist->next     = acsm->acsmPatterns;
    acsm->acsmPatterns = plist;
    acsm->numPatterns++;

    patternGroup[i] = plist;
  }

  patternGroup[0]->selfMark = calloc(1, sizeof(unsigned int));
  patternGroup[1]->selfMark = calloc(1, sizeof(unsigned int));

  patternGroup[0]->otherMark = patternGroup[1]->selfMark;
  patternGroup[1]->otherMark = patternGroup[0]->selfMark;

  return 0;
}

int acsmAddPatternWithOddEven(ACSM_STRUCT2* acsm, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(n < 2){
    return 0;
  }

  int halfLength = (n >> 1) + 1;

  unsigned char** splitPatterns = AC_MALLOC(acsm, sizeof(unsigned char*) * 2, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[0] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[1] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);

  int splitPatternLengths[2];
  splitPatternLengths[0] = 0;
  splitPatternLengths[1] = 0;

  unsigned int splitPatternIds[2];
  splitPatternIds[0] = acsm->subPatternSize++;
  splitPatternIds[1] = acsm->subPatternSize++;

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
    unsigned int selfIndex = 0;
    unsigned int otherIndex = 0;
    if(i == 0){
      selfIndex = 0;
      otherIndex = 1;
    }else{
      selfIndex = 1;
      otherIndex = 0;
    }

    unsigned char* splitPattern = splitPatterns[selfIndex];
    unsigned char* otherPattern = splitPatterns[otherIndex];
    int patternLength = splitPatternLengths[selfIndex];
    int otherLength = splitPatternLengths[otherIndex];

    ACSM_PATTERN2* plist = AC_MALLOC(acsm, sizeof(ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

    plist->patrn = AC_MALLOC(acsm, patternLength, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->patrn, splitPattern, patternLength);

    plist->otherPattern = AC_MALLOC(acsm, otherLength, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->otherPattern, otherPattern, otherLength);

    plist->origin_patrn = AC_MALLOC(acsm, n, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->origin_patrn, pat, n);

    plist->flag = i;
    plist->origin_length = n;
    plist->other_length = otherLength;

    plist->n      = patternLength;
    plist->nocase = nocase;
    plist->offset = offset;
    plist->depth  = depth;
    plist->negative = negative;
    plist->iid    = iid;
    plist->udata  = id;
    plist->same   = same; 

    plist->next     = acsm->acsmPatterns;
    acsm->acsmPatterns = plist;
    acsm->numPatterns++;

    if(i == 0){
      plist->selfPatternId = splitPatternIds[0];
      plist->subPatternId = splitPatternIds[1];
    }else if(i == 1){
      plist->selfPatternId = splitPatternIds[1];
      plist->subPatternId = splitPatternIds[0];
    }

    if(plist->flag == plist->same){
      plist->start = plist->origin_length + 1;
      plist->wildcard = 1;
    }else{
      plist->start = plist->origin_length;
      plist->wildcard = 0;
    }

    unsigned int indexEnd = 0;
    if(plist->flag == 0){
      plist->indexEnd = plist->origin_length + same - 2;
    }else{
      plist->indexEnd = plist->origin_length - same - 1;
    }
  }

  return 0;
}

int acsmAddPatternWithOddEvenWithOneLen(ACSM_STRUCT2* acsm, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(acsm->oneLenTable == NULL){
      acsm->oneLenTable = calloc(256, sizeof(unsigned int));
    }

  if(n < 2){
    acsm->oneLenFalg = 1;
    unsigned int index = pat[0];
    // printf("=== %u", index);
    acsm->oneLenTable[index]++;
  }else{
    int halfLength = (n >> 1) + 1;

    unsigned char** splitPatterns = AC_MALLOC(acsm, sizeof(unsigned char*) * 2, ACSM2_MEMORY_TYPE__PATTERN);
    splitPatterns[0] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);
    splitPatterns[1] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);

    int splitPatternLengths[2];
    splitPatternLengths[0] = 0;
    splitPatternLengths[1] = 0;

    unsigned int splitPatternIds[2];
    splitPatternIds[0] = acsm->subPatternSize++;
    splitPatternIds[1] = acsm->subPatternSize++;

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
      unsigned int selfIndex = 0;
      unsigned int otherIndex = 0;
      if(i == 0){
        selfIndex = 0;
        otherIndex = 1;
      }else{
        selfIndex = 1;
        otherIndex = 0;
      }

      unsigned char* splitPattern = splitPatterns[selfIndex];
      unsigned char* otherPattern = splitPatterns[otherIndex];
      int patternLength = splitPatternLengths[selfIndex];
      int otherLength = splitPatternLengths[otherIndex];

      ACSM_PATTERN2* plist = AC_MALLOC(acsm, sizeof(ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

      plist->patrn = AC_MALLOC(acsm, patternLength, ACSM2_MEMORY_TYPE__PATTERN);
      memcpy (plist->patrn, splitPattern, patternLength);

      plist->otherPattern = AC_MALLOC(acsm, otherLength, ACSM2_MEMORY_TYPE__PATTERN);
      memcpy (plist->otherPattern, otherPattern, otherLength);

      plist->origin_patrn = AC_MALLOC(acsm, n, ACSM2_MEMORY_TYPE__PATTERN);
      memcpy (plist->origin_patrn, pat, n);

      plist->flag = i;
      plist->origin_length = n;
      plist->other_length = otherLength;

      plist->n      = patternLength;
      plist->nocase = nocase;
      plist->offset = offset;
      plist->depth  = depth;
      plist->negative = negative;
      plist->iid    = iid;
      plist->udata  = id;
      plist->same   = same; 

      plist->next     = acsm->acsmPatterns;
      acsm->acsmPatterns = plist;
      acsm->numPatterns++;

      if(i == 0){
        plist->selfPatternId = splitPatternIds[0];
        plist->subPatternId = splitPatternIds[1];
      }else if(i == 1){
        plist->selfPatternId = splitPatternIds[1];
        plist->subPatternId = splitPatternIds[0];
      }

      if(plist->flag == plist->same){
        plist->start = plist->origin_length + 1;
        plist->wildcard = 1;
      }else{
        plist->start = plist->origin_length;
        plist->wildcard = 0;
      }

      unsigned int indexEnd = 0;
      if(plist->flag == 0){
        plist->indexEnd = plist->origin_length + same - 2;
      }else{
        plist->indexEnd = plist->origin_length - same - 1;
      }
    }
  }

  return 0;
}


int acsmAddPatternOrigin(ACSM_STRUCT2 * acsm, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  ACSM_PATTERN2 * plist;

  plist = (ACSM_PATTERN2 *)AC_MALLOC(acsm, sizeof (ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

  plist->patrn = (unsigned char *)AC_MALLOC(acsm, n, ACSM2_MEMORY_TYPE__PATTERN);
  memcpy (plist->patrn, pat, n);

  plist->n      = n;
  plist->nocase = nocase;
  plist->offset = offset;
  plist->depth  = depth;
  plist->negative = negative;
  plist->iid    = iid;
  plist->udata = id;

  plist->next     = acsm->acsmPatterns;
  acsm->acsmPatterns = plist;
  acsm->numPatterns++;

  return 0;
}

int acsmAddPatternNoOneLen(ACSM_STRUCT2 * acsm, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(n < 2){
    return 0;
  }

  ACSM_PATTERN2 * plist;

  plist = (ACSM_PATTERN2 *)AC_MALLOC(acsm, sizeof (ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

  plist->patrn = (unsigned char *)AC_MALLOC(acsm, n, ACSM2_MEMORY_TYPE__PATTERN);
  memcpy (plist->patrn, pat, n);

  plist->n      = n;
  plist->nocase = nocase;
  plist->offset = offset;
  plist->depth  = depth;
  plist->negative = negative;
  plist->iid    = iid;
  plist->udata = id;

  plist->next     = acsm->acsmPatterns;
  acsm->acsmPatterns = plist;
  acsm->numPatterns++;

  return 0;
}

int acsmAddPatternHashSmallPattern(ACSM_STRUCT2 * acsm, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(n <= 2){
    addSmallPattern(pat, n);
    return 0;
  }

  int halfLength = (n >> 1) + 1;

  unsigned char** splitPatterns = AC_MALLOC(acsm, sizeof(unsigned char*) * 2, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[0] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);
  splitPatterns[1] = AC_MALLOC(acsm, halfLength, ACSM2_MEMORY_TYPE__PATTERN);

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
    unsigned int selfIndex = 0;
    unsigned int otherIndex = 0;
    if(i == 0){
      selfIndex = 0;
      otherIndex = 1;
    }else{
      selfIndex = 1;
      otherIndex = 0;
    }

    unsigned char* splitPattern = splitPatterns[selfIndex];
    unsigned char* otherPattern = splitPatterns[otherIndex];
    int patternLength = splitPatternLengths[selfIndex];
    int otherLength = splitPatternLengths[otherIndex];

    ACSM_PATTERN2* plist = AC_MALLOC(acsm, sizeof(ACSM_PATTERN2), ACSM2_MEMORY_TYPE__PATTERN);

    plist->patrn = AC_MALLOC(acsm, patternLength, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->patrn, splitPattern, patternLength);

    plist->otherPattern = AC_MALLOC(acsm, otherLength, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->otherPattern, otherPattern, otherLength);

    plist->origin_patrn = AC_MALLOC(acsm, n, ACSM2_MEMORY_TYPE__PATTERN);
    memcpy (plist->origin_patrn, pat, n);

    plist->flag = i;
    plist->origin_length = n;
    plist->other_length = otherLength;

    plist->n      = patternLength;
    plist->nocase = nocase;
    plist->offset = offset;
    plist->depth  = depth;
    plist->negative = negative;
    plist->iid    = iid;
    plist->udata  = id;
    plist->same   = same; 

    plist->next     = acsm->acsmPatterns;
    acsm->acsmPatterns = plist;
    acsm->numPatterns++;

    if(plist->flag == plist->same){
      plist->start = plist->origin_length + 1;
      plist->wildcard = 1;
    }else{
      plist->start = plist->origin_length;
      plist->wildcard = 0;
    }

    unsigned int indexEnd = 0;
    if(plist->flag == 0){
      plist->indexEnd = plist->origin_length + same - 2;
    }else{
      plist->indexEnd = plist->origin_length - same - 1;
    }
  }

  return 0;
}

int acsmAddPattern2 (ACSM_STRUCT2 * p, unsigned char *pat, int n, int nocase, int offset, int depth, int negative, void * id, int iid){
  if(p->acMode == 1){
    return acsmAddPatternWithOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 2){
    return acsmAddPatternWithTwiceOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 0){
    return acsmAddPatternOrigin(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 3){
    return acsmAddPatternHashSmallPattern(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 4){
    return acsmAddPatternWithOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 5){
    return acsmAddPatternWithOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 6){
    return acsmAddPatternOrigin(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 7){
    return acsmAddPatternWithOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 8){
    return acsmAddPatternWithOddEvenWithOneLen(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 9){
    return acsmAddPatternWithOddEvenWithOneLen(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 10){
    return acsmAddPatternNoOneLen(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 11){
    return acsmAddPatternWithOddEvenWithOneLen(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 12){
    return acsmAddPatternWithOddEvenWithOneLen(p, pat, n, nocase, offset, depth, negative, id, iid);
  }else if(p->acMode == 13){
    return acsmAddPatternWithOddEven(p, pat, n, nocase, offset, depth, negative, id, iid);
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

    acsm->acsmTransTable = (trans_node_t**)AC_MALLOC(acsm, sizeof(trans_node_t*) * acsm->acsmMaxStates, ACSM2_MEMORY_TYPE__TRANSTABLE);
    acsm->acsmMatchList = (ACSM_PATTERN2 **)AC_MALLOC(acsm, sizeof(ACSM_PATTERN2*) * acsm->acsmMaxStates, ACSM2_MEMORY_TYPE__MATCHLIST);

    acsm->acsmNumStates = 0;
    for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next){
        AddPatternStates(acsm, plist);
    }
    acsm->acsmNumStates++;
    acsm->sizeofstate = 4;

    acsm->acsmFailState = (acstate_t*)AC_MALLOC(acsm, sizeof(acstate_t) * acsm->acsmNumStates, ACSM2_MEMORY_TYPE__FAILSTATE);
    acsm->acsmNextState = (acstate_t**)AC_MALLOC_DFA(acsm, acsm->acsmNumStates * sizeof(acstate_t*), acsm->sizeofstate);

    if ((acsm->acsmFSA == FSA_DFA) || (acsm->acsmFSA == FSA_NFA)){
        Build_NFA(acsm);
    }

    if (acsm->acsmFSA == FSA_DFA){
        Convert_NFA_To_DFA(acsm);
    }

    if (acsm->acsmFormat == ACF_FULLQ) {
        if (Conv_List_To_Full(acsm))
            return -1;

        AC_FREE(acsm, acsm->acsmFailState, sizeof(acstate_t) * acsm->acsmNumStates, ACSM2_MEMORY_TYPE__FAILSTATE);
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
    acsm->lastPosition = 0;

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
      countNextStateAccess++;

      sindex = T[0];
      countCharacterAccess++;
      countCompareCharater++;
      acsm->countCharacterComparison++;

      if (ps[1]){
        if (MatchList[state]){
          // unsigned long long int maxPattern = 0;
          for(ACSM_PATTERN2* mlist = MatchList[state]; mlist != NULL; mlist = mlist->next){
              countSubPatternAccess++;
              acsm->callbackAcMatch(acsm, Tstart, T, state);
              
              // for(unsigned int i = 0; i < mlist->n; i++){
              //   printf("%02x", mlist->patrn[i]);
              // }
              // printf("\n");
              // printf("Pattern Id: %u\n", mlist->iid);
              // patternIdList[mlist->iid]++;
              // if(mlist->n > maxPattern){
              //   maxPattern = mlist->n;
              // }
          }
          // countDirtyCharater += maxPattern;
        }
      }
      countStateAccess++;

      state = ps[2 + sindex];
      countStateAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      for(ACSM_PATTERN2* mlist = MatchList[state]; mlist != NULL; mlist = mlist->next){
          countSubPatternAccess++;
          acsm->callbackAcMatch(acsm, Tstart, T, state);
          // patternIdList[mlist->iid]++;
          // countDirtyCharater += mlist->n;
      }
    }

    return 0;
}

static inline int acsmSearchWithOddEven( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;
    // TextEnd = Tend;
    acsm->textEnd = Tend;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;
    acsm->lastState = *current_state; //

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T += 2){
      ps = NextState[state];
      countNextStateAccess++;

      sindex = T[0];
      countCharacterAccess++;
      countCompareCharater++;
      acsm->countCharacterComparison++;

      if (ps[1]){
        acsm->countFullMatch++;
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }
      countStateAccess++;

      acsm->lastState = state; //
      state = ps[2 + sindex];
      countStateAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

static inline int acsmSearchWithOddEvenSnortOneLen( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    int nindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;
    // TextEnd = Tend;
    acsm->textEnd = Tend;

    unsigned int* oneLenTable = acsm->oneLenTable;
    unsigned char* Tn = T + 1;

    acsm->subText = T;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;
    acsm->lastState = *current_state; //

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T += 2, Tn += 2){
      ps = NextState[state];
      countNextStateAccess++;

      sindex = T[0];
      foundCounter += oneLenTable[sindex];
      countCharacterAccess++;
      countCompareCharater++;

      if(Tn < Tend){
        nindex = Tn[0];
        foundCounter += oneLenTable[nindex];
      }

      if (ps[1]){
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }
      countStateAccess++;

      acsm->lastState = state; //
      state = ps[2 + sindex];
      countStateAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

// 子執行緒函數
void* oneLenThread(void* data) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(2, &mask);
  if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
    fprintf(stderr, "[INFO] Thread affinity failed. \n");
    exit(0);
  }

  OneLenThreadData* threadData = (OneLenThreadData*)data;
  ACSM_STRUCT2 *acsm = threadData->acsm;
  unsigned char *T = threadData->text;
  int n = threadData->length;
  unsigned char * Tend = T + n;
  unsigned int* oneLenTable = acsm->oneLenTable;

  unsigned int matchFounder = 0;
  int nindex;
  for(; T < Tend; T++){
    nindex = T[0];
    matchFounder += oneLenTable[nindex];
  }

  // threadData->matchFounder = matchFounder;
  pthread_exit(NULL); // 離開子執行緒
}

int createOnLenThread(pthread_t* thread, OneLenThreadData* threadData){
  threadData->matchFounder = 0;
  pthread_create(thread, NULL, oneLenThread, threadData);
}

int searchOneLenWithOddEven(ACSM_STRUCT2 *acsm, unsigned char *T, int n){
  unsigned char * Tend = T + n;
  unsigned int* oneLenTable = acsm->oneLenTable;
  int nindex;
  unsigned int matchFounder = 0;

  for(; T < Tend; T++){
    nindex = T[0];
    matchFounder += oneLenTable[nindex];
  }

  return matchFounder;
}

static inline int acsmSearchWithOddEvenSnortOneLenThread(ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;
    // TextEnd = Tend;
    acsm->textEnd = Tend;

    acsm->subText = T;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;
    acsm->lastState = *current_state; //

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T += 2){
      ps = NextState[state];
      countNextStateAccess++;

      sindex = T[0];
      countCharacterAccess++;
      countCompareCharater++;

      if (ps[1]){
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }
      countStateAccess++;

      acsm->lastState = state; //
      state = ps[2 + sindex];
      countStateAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

static inline int acsmSearchWithOddEvenSnort(ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;
    // TextEnd = Tend;
    acsm->textEnd = Tend;

    unsigned char* Tn = T + 1;

    acsm->subText = T;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;
    acsm->lastState = *current_state; //

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T += 2, Tn += 2){
      ps = NextState[state];
      countNextStateAccess++;

      sindex = T[0];
      countCharacterAccess++;
      countCompareCharater++;

      if (ps[1]){
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }
      countStateAccess++;

      acsm->lastState = state; //
      state = ps[2 + sindex];
      countStateAccess++;
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

// unsigned int subState = 0;
// unsigned int subDepth = 0;

static inline int acsmSearchWithTwiceOddEven(ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2 **MatchList = acsm->acsmMatchList;

    unsigned char * Tend = T + n;
    unsigned char * Tstart = T;

    if (current_state == NULL){
        return 0;
    }

    acstate_t state = *current_state;
    unsigned int stateDepth = acsm->acsmDepth[state];

    acstate_t *ps;
    acstate_t **NextState = acsm->acsmNextState;
    for (; T < Tend; T += 2){
      ps = NextState[state];
      sindex = T[0];

      if (ps[1]){
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }

      state = ps[2 + sindex];
      // if(acsm->acsmDepth[state] < stateDepth){
      //   unsigned int delta = stateDepth - acsm->acsmDepth[state];
      //   if(delta >= subDepth){
      //     subDepth = 0;
      //   }else{
      //     subDepth -= delta;
      //   }
      // }
      // stateDepth = acsm->acsmDepth[state];
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

static inline int acsmSearchWithSmallPattern( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    int char2;
    int char3;
    int char12;
    int char23;
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
      char2 = T[1];
      char3 = T[2];

      if (ps[1]){
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }

      char12 = (sindex << 8) + char2;
      char23 = (char2 << 8) + char3;
      foundCounter += lengthOneTable[sindex];
      foundCounter += lengthOneTable[char2];
      foundCounter += lengthTwoTable[char12];
      foundCounter += lengthTwoTable[char23];
      // searchSmallPattern(sindex, char2, char3);
      state = ps[2 + sindex];
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

static inline int acsmSearchWithOddEvenInline( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    int sindex;
    ACSM_PATTERN2** matchGroup = acsm->acsmMatchList;
    ACSM_PATTERN2* matchEntry;

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
        matchEntry = matchGroup[state];
        for(; matchEntry != NULL; matchEntry = matchEntry->next){
          unsigned char* origin_patrn = matchEntry->origin_patrn;

          unsigned char* start = T - matchEntry->start;
          unsigned int indexEnd = matchEntry->indexEnd;

          int isFull = 1;
          for(int i = indexEnd; i >= 0; i -= 2){
            if(origin_patrn[i] != start[i]){
              isFull = 0;
              break;
            }
          }

          if(isFull){
            foundCounter++;
          }
        }
        // acsm->callbackAcMatch(acsm, Tstart, T, state);
      }
      state = ps[2 + sindex];
    }

    *current_state = state;

    if (matchGroup[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

static inline int acsmSearchWithLastCharFilterInline( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
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
        unsigned char* filter = filterGroup[state];
        unsigned char* offset = offsetGroup[state];
        unsigned short size = sizeGroup[state];

        for(unsigned int i = 0; i < size; i++){
          unsigned char* lastChar = T - offset[i];
          if(filter[i] != lastChar[0]){
            continue;
          }

          ACSM_PATTERN2* pattern = subPatternGroup[state][i];
          unsigned char* origin_patrn = pattern->origin_patrn;
          unsigned char* start = T - pattern->start;
          unsigned int indexEnd = pattern->indexEnd;

          int isFull = 1;
          for(int i = indexEnd - 2; i >= 0; i -= 2){
            if(origin_patrn[i] != start[i]){
              isFull = 0;
              break;
            }
          }

          if(isFull){
            foundCounter++;
          }
        }
        // acsm->callbackAcMatch(acsm, Tstart, T, state);
      }

      state = ps[2 + sindex];
    }

    *current_state = state;

    if (MatchList[state]){
      unsigned char* filter = filterGroup[state];
      unsigned char* offset = offsetGroup[state];
      unsigned short size = sizeGroup[state];

      for(unsigned int i = 0; i < size; i++){
        unsigned char* lastChar = T - offset[i];
        if(filter[i] != lastChar[0]){
          continue;
        }

        ACSM_PATTERN2* pattern = subPatternGroup[state][i];
        unsigned char* origin_patrn = pattern->origin_patrn;
        unsigned char* start = T - pattern->start;
        unsigned int indexEnd = pattern->indexEnd;

        int isFull = 1;
        for(int i = indexEnd - 2; i >= 0; i -= 2){
          if(origin_patrn[i] != start[i]){
            isFull = 0;
            break;
          }
        }

        if(isFull){
          foundCounter++;
        }
      }
    }

    return 0;
}

static inline int acsmSearchWithOddEvenFilter(ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
    FilterFlag* filterFlagGroup = acsm->otherData;
    FilterFlag* currentFlag = filterFlagGroup;

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
    unsigned int countVisitChar = 0;
    unsigned int countSkipChar = 0;
    for (; T < Tend; T++, currentFlag++){
      if(currentFlag->x == 0){
        countSkipChar++;
        continue;
      }
      countVisitChar++;

      ps = NextState[state];
      sindex = T[0];

      if (ps[1]){
        if (MatchList[state]){
          unsigned long long int maxPattern = 0;
          for(ACSM_PATTERN2* mlist = MatchList[state]; mlist != NULL; mlist = mlist->next){
              acsm->callbackAcMatch(acsm, Tstart, T, state);
              // patternIdList[mlist->iid]++;
              if(mlist->n > maxPattern){
                maxPattern = mlist->n;
              }
          }
          // countMatchCharater += maxPattern;
        }
      }
      state = ps[2 + sindex];
    }

    *current_state = state;

    if (MatchList[state]){
      for(ACSM_PATTERN2* mlist = MatchList[state]; mlist != NULL; mlist = mlist->next){
          acsm->callbackAcMatch(acsm, Tstart, T, state);
          // patternIdList[mlist->iid]++;
          // countMatchCharater += mlist->n;
      }
    }

    // printf("[INFO] Visit Char: %u \n", countVisitChar);
    // printf("[INFO] Skip Char: %u \n", countSkipChar);
    return 0;
}

static inline int acsmSearchWithOddEvenHash( ACSM_STRUCT2 *acsm, unsigned long long int offset, unsigned char *T, int n, int (*Match)(void * id, int index, void *data), void *data, int *current_state ){
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
      sindex = (T[0] << 4) + T[1];

      if (ps[1]){
        acsm->callbackAcMatch(acsm, Tstart, T, state);
      }

      state = ps[2 + sindex];
    }

    *current_state = state;

    if (MatchList[state]){
      acsm->callbackAcMatch(acsm, Tstart, T, state);
    }

    return 0;
}

int acsmSearch2(ACSM_STRUCT2 * acsm, unsigned long long int offset, unsigned char *Tx, int n, int (*Match)(void * id, int index, void *data), void *data, int* current_state ){
  if(acsm->acMode == 1){
    int result = acsmSearchWithOddEven(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 2){
    int result = acsmSearchWithTwiceOddEven(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 0){
    int result = acsmSearchOrigin(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 3){
    int result = acsmSearchWithSmallPattern(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 4){
    int result = acsmSearchWithOddEvenInline(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 5){
    int result = acsmSearchWithLastCharFilterInline(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 6){
    int result = acsmSearchWithOddEvenFilter(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 7){
    int result = acsmSearchWithOddEvenHash(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 8){
    int result;
    if(acsm->oneLenFalg){
      result = acsmSearchWithOddEvenSnortOneLen(acsm, offset, Tx, n, Match, data, current_state);
    }else{
      result = acsmSearchWithOddEvenSnort(acsm, offset, Tx, n, Match, data, current_state);
    }
    return result;
  }else if(acsm->acMode == 9){
    int result;
    // if(acsm->oneLenFalg){
    //   // result = acsmSearchWithOddEvenSnortOneLenThread(acsm, offset, Tx, n, Match, data, current_state);
    //   result = acsmSearchWithOddEvenSnort(acsm, offset, Tx, n, Match, data, current_state);
    // }else{
    //   result = acsmSearchWithOddEvenSnort(acsm, offset, Tx, n, Match, data, current_state);
    // }
    result = acsmSearchWithOddEvenSnort(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 10){
    int result = acsmSearchOrigin(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 11){
    int result = acsmSearchWithOddEvenSnortOneLen(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 12){
    int result = acsmSearchWithOddEvenSnortOneLen(acsm, offset, Tx, n, Match, data, current_state);
    return result;
  }else if(acsm->acMode == 13){
    int result = acsmSearchWithOddEven(acsm, offset, Tx, n, Match, data, current_state);
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
    
    printf("| Pattern Memory     : %.2fKbytes\n", (float)p->acsm2_pattern_memory/1024 );
    printf("| MatchList Memory   : %.2fKbytes\n", (float)p->acsm2_matchlist_memory/1024 );
    printf("| Memory             : %.2fKbytes\n", (float)p->acsm2_total_memory/1024 );
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
  acsm->acMode = 0;
  acsm->callbackAcMatch = NULL;
}

void setCallbackAcMatch(ACSM_STRUCT2 * acsm, void (*callback)(ACSM_STRUCT2*, unsigned char*, unsigned char*, unsigned int)){
  acsm->callbackAcMatch = callback;
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

void acsmCalePatternDepth(ACSM_STRUCT2* acsm){
  for(unsigned int state = 0; state < acsm->acsmNumStates; state++){
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

        // printf("[INFO] pattern:%u, match_depth: %u \n", MatchList->iid + 1, MatchList->matchDepth);
        // printf("       falg:%u, same: %u \n", MatchList->flag, MatchList->same);
        // printf("       state:%u \n", state);
      }
      // printf("\n");
    }
  }
}

void fullMatchTwiceOddEven(ACSM_STRUCT2 *acsm, unsigned char * Tstart, unsigned char* T, unsigned int state){
  // printf("[INFO] Found pattern at position: %u.\n", (unsigned int)(T - Tstart));

  unsigned int stateDepth = acsm->acsmDepth[state];
  unsigned int length = stateDepth + stateDepth;
  unsigned char* start = T - 1 - length;

  ACSM_PATTERN2* currentStateMatchList = acsm->acsmMatchList[state];
  for(; currentStateMatchList != NULL; currentStateMatchList = currentStateMatchList->next){
    *(currentStateMatchList->otherMark) = currentStateMatchList->matchDepth;
  }

  unsigned int subState = 0;
  unsigned int subDepth = -1;
  unsigned int sindex = 0;
  // b aho_corasick.c:1328 if position == 32296.
  // unsigned int position = (unsigned int)(T - Tstart);

  acstate_t** NextState = acsm->acsmNextState;
  for(; start < T; start += 2){
    acstate_t* ps = NextState[subState];
    sindex = start[0];

    if(ps[1]){
      ACSM_PATTERN2* matchList = acsm->acsmMatchList[subState];
      for(; matchList != NULL; matchList = matchList->next){
        if(*(matchList->selfMark) == subDepth){
          foundCounter++;
          // printf("[INFO] Twice found pattern at position: %u.\n", (unsigned int)(T - Tstart));
        }
      }
    }

    subState = ps[2 + sindex];
    subDepth++;
  }

  if(acsm->acsmMatchList[subState]){
    ACSM_PATTERN2* matchList = acsm->acsmMatchList[subState];
    for(; matchList != NULL; matchList = matchList->next){
      if(*(matchList->selfMark) == subDepth){
        foundCounter++;
        // printf("[INFO] Twice found pattern at position: %u.\n", (unsigned int)(T - Tstart));
      }
    }
  }

  currentStateMatchList = acsm->acsmMatchList[state];
  for(; currentStateMatchList != NULL; currentStateMatchList = currentStateMatchList->next){
    *(currentStateMatchList->otherMark) = 0;
  }
}