/*
**
** $Id: mwm.c,v 1.3 2003/12/17 21:25:14 jh8 Exp $
**
** mwm.c   - A Modified Wu-Manber Style Multi-Pattern Matcher
**
** Copyright (C) 2002 Sourcefire,Inc
** Marc Norton
**
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
**
**
**
** A Fast Multi-Pattern Search Engine
** 
** This algorithm is based on the Wu & Manber '94 paper. This algorithm is not an 
** exact implementation in that it uses a standard one or 2 byte Bad Character shift table,
** whereas Wu & Manber used a 2 byte Bad Characeter shift table. This implementation
** also uses a fixed 2 byte prefix hash table, Wu & Manber used a variable size  
** 2 byte hash. Hash groups are defined as in Wu & Manber.  The Pattern groups are searched 
** using a reverse string compare as in Wu & Manber. 
**
** A pattern match tracking mechanism has been added to reduce the effect of DOS attacks, 
** and generally improve the worst case performanmce scenario.  This feature is 
** enabled/disabled via BITOP_TEST and requires the bitop.h file.
**
** 
** Initial Version - Marc Norton - April 2002 
**
**
   Algorithm:
  
     This is a simplified implementation of Wu & Manber's '92 and '94 papers.
  
     1) A multi-pattern Boyer-Moore style bad character or word shift is done until 
     a possible pattern is detected.  
  
     2) Hashing is used on the 2 character prefix of the current text to index into a 
     group of patterns with the same prefix. Patterns must be sorted. Wu & Manber used 
     the last 2 characters of the psuedo multi-pattern minimum length. Either works fine.
     
     3) A reverse string compare is performed against each pattern in the 
     group to see if any of the patterns match the current text prefix.
       
       
   Algorithm Steps
  
   Preprocess:     
  
     a) Sort The Patterns, forming a sorted list of patterns.
     b) Build a hash table of the patterns as follows: For each pattern find it's
        hash, in the hash index vector save this patterns index, now count how 
        many patterns following this one have the same hash as it, save this value 
        with the 1st pattern that has this hash.

     c) Build a Boyer-Moore style bad Character shift table, using the min shift from 
        all patterns for the shift value for each characters in the shift table. For the 
  purposes of the Bad Character Shift we assume all patterns are the same length as the
        shortest pattern.  This works quite well in practice.  Also build a Bad Word
        shift table.

   Search:

    a) Perform Boyer-Moore Bad Character or Bad Word Shift loop to find a possible 
       pattern match.

    b) Check if a hashed pattern group entry exists for this possible patterns prefix, 
       If no pattern hash exists for this suffix shift go to d)

    c) If a hash exists, test all of the patterns in the pattern group to see
       which ones if any match the text suffix.  Use a reverse string comparison
       to benefit from the increased likelihood of misses at the ends of the string.
       This tidbit is based on Boyer-Moore searching.  Uses bit flags to eliminate
       previously hit matches from contention.  This provides a better worst case
       scenario for this setwise pattern matching. 

    d) Use a one character shift and go back to a)

   Pattern Matcher Selection Heuristics:

      We can use A Boyer-Moore for small sets of patterns.  We can use one of 3 matchers
   for larger sets.  When the minimum pattern size within the pattern group is one
   character we use the version without a Boyer-Moore bad character or bad word 
   shift.  When we have groups with a minimum pattern size of 2 or more characters 
   we use the version with the bad character shift.  When we have groups with a minimum
   pattern size of 2 or more characters and the number of patterns small-medium we can use
   the version with the bad word shift.  More testing is needed to help determine the optimal
   switching points between these algorithms.
      
   Case vs NoCase:

     We convert all patterns and search texts to one case, find a match, and if case is
     important we retest just the pattern characters against the text in exact mode.

   Algorithm Strengths:

    Performance is limited by the minimum pattern length of the group. The bigger
    the minumum, the more the bad character shift helps, and the faster the search.  
    The minimum pattern length also determines whether a tree based search is faster or slower.
    
    Small groups of patterns 10-100 with a large minimum pattern size (3+ chars) get the best
    performanc.  Oh, if it were all that simple.
  
   Improvements or Variations:

       where to start ...much more to come...

   Notes:
 
   Observations:
   
    This algorithm is CPU cache sensitive, aren't they all.
     
    
   Ancestory:

   The structure of the API interface is based on the interface used by the samples 
   from Dan Gusfields book.

   Some References:

    Boyer-Moore   - The original and still one of the best.
    Horspool      - Boyer-Moore-Horspool.
    Sunday        - Quicksearch and the Tuned Boyer-Moore algorithm.
    Wu and Manber - A Fast Multi-Pattern Matcher '94   -- agrep, fgrep, sgrep
    Aho & Corasick- State machine approach, very slick as well.
    Dan Gusfield  - Algorithms on strings, trees, and sequences.
    Steven Graham - 
    Crochemere    -
  
  
  NOTES:
  
   4-2002    - Marc Norton
               Initial implementation 
  
   5/23/2002 - Marc Norton - 
               Added Boyer-Moore-Horspool locally, so it's inlined.
               We use a simple <5 pattern count to decide to use the
               BMH method over the MWM method.  This is not always right
               but close enough for now. This may get replaced with the standard
               Boyer-Moore in mString - the standard Boyer Moore may protect 
               better against some types of DOS'ing attempts.
   11/02    -  Fixed bug for multiple duplicate one byte patterns. 
   10/03    -  Changed ID to a void * for better 64 bit compatability.
      -  Added BITOP_TEST to make testing rotuines easier.
      -  Added Windows __inline command back into bitops.
      -  Added Standalone Windows support for UNIT64 for standalone testing
            -  modified mwm.c, mwm.h, bitop.h
   10/28/03 -  Fixed bug in mwmPrepHashedPatternGroups(), it was setting resetting psIID
               as it walked through the patterns counting the number of patterns in each group.
         This caused an almost random occurrence of an already tested rule's bitop
         flag to cause another rule to never get tested.
   12/08/03 -  Removed the usage of NumArray1, it was unneccessary, reverted back to NumArray 
               mwmPrepHashedPatternGroups:
               When counting one byte patterns in 'ningroup' added a check for psLen==1
               Broke out common search code into an inline routine ..group2()
*/

/*
**  INCLUDES
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

# include <time.h>
#include "snort_wu_manber.h"

// int FatalError( char *, ... );
int FatalError( char * s );


// 計算花費的記憶體。
// === CUSTOM CODE ===============================
static unsigned long long int costMemory = 0;

static void* CALLOC(size_t nitems, size_t size){
  costMemory += size * nitems;
  return calloc(nitems, size);
}

unsigned long long int memoryShiftTable = 0;

static void* CALLOC_SHIFT_TABLE(size_t nitems, size_t size){
  if(memoryShiftTable == 0){
    memoryShiftTable = 256 * sizeof(short);
  }
  memoryShiftTable += size * nitems;
  return calloc(nitems, size);
}

static void ADD_SHIFT_TABLE(size_t nitems, size_t size){
  memoryShiftTable += size * nitems;
}

unsigned long long int memoryHashTable = 0;

static void* CALLOC_HASH_TABLE(size_t nitems, size_t size){
  memoryHashTable += size * nitems;
  return calloc(nitems, size);
}
// ===============================================


/*
*   Count of how many byte have been scanned,
*   this gets reset each time a user requests this tidbit,
*   this counts across all pattern groups.
*/
static UINT64  iPatCount=0;

UINT64 mwmGetPatByteCount()
{
   return  iPatCount;
}

void mwmResetByteCount()
{
   iPatCount=0;
}

/*
** Translation Table 
*/
static unsigned char xlatcase[256];

/*
** NoCase Buffer -This must be protected by a mutex if multithreaded calls to
** the pattern matcher occur or else we could have the user pass one of these 
** for each pattern matcher instance from his stack or heap.
*/
// static unsigned char S[65536];
static unsigned char S[2100000];


/*
*
*/
static void init_xlatcase()
{
   int i;
   for(i=0;i<256;i++)
   {
     xlatcase[ i ] =  toupper(i);
   }
}


/*
*
*/
static INLINE void ConvCaseToUpper( unsigned char *s, int m )
{
     int  i;
     for( i=0; i < m; i++ )
     {
       s[i] = xlatcase[ s[i] ];
     }
}

/*
*
*/
static INLINE void ConvCaseToUpperEx( unsigned char * d, unsigned char *s, int m )
{
     int i;
     for( i=0; i < m; i++ )
     {
        d[i] = xlatcase[ s[i] ];
     }
}


/*
*
*  Boyer-Moore-Horsepool for small pattern groups
*    
*/
#undef COPY_PATTERNS
HBM_STRUCT * hbm_prepx(HBM_STRUCT *p, unsigned char * pat, int m)
{
     int     k;

     if( !m ) return 0;
     if( !p ) return 0;

#ifdef COPYPATTERN
     p->P = (unsigned char*)malloc( m + 1 )
     if( !p->P ) return 0;
     memcpy(p->P,pat,m);
#else
     p->P = pat;
#endif

     p->M = m;

     /* Compute normal Boyer-Moore Bad Character Shift */
     for(k = 0; k < 256; k++) p->bcShift[k] = m;
     for(k = 0; k < m; k++)   p->bcShift[pat[k]] = m - k - 1;

     return p;
}

/*
*
*/
HBM_STRUCT * hbm_prep(unsigned char * pat, int m)
{
     HBM_STRUCT    *p;

     p = (HBM_STRUCT*)malloc( sizeof(HBM_STRUCT) );
     if( !p ) return 0;

     return hbm_prepx( p, pat, m );
}

#ifdef XXX_NOT_USED
/*
*
*/
static void hbm_free( HBM_STRUCT *p )
{
    if(p)
    {
#ifdef COPYPATTERN
       if( p->P )free(p->P);
#endif
       free(p);
    }
}
#endif


/*
*   Boyer-Moore Horspool
*   Does NOT use Sentinel Byte(s)
*   Scan and Match Loops are unrolled and separated
*   Optimized for 1 byte patterns as well
*/
static INLINE unsigned char * hbm_match(HBM_STRUCT * px, unsigned char * text, int n)
{
  unsigned char *pat, *t, *et, *q;
  int            m1, k;
  short    *bcShift;

  m1     = px->M-1;
  pat    = px->P;
  bcShift= px->bcShift;

  t  = text + m1;  
  et = text + n; 

  /* Handle 1 Byte patterns - it's a faster loop */
  if( !m1 )
  {
    for( ;t<et; t++ ) 
      if( *t == *pat ) return t;
    return 0;
  }
 
  /* Handle MultiByte Patterns */
  while( t < et )
  {
    /* Scan Loop - Bad Character Shift */
    do 
    {
      t += bcShift[*t];
      if( t >= et )return 0;;

      t += (k=bcShift[*t]);      
      if( t >= et )return 0;

    } while( k );

    /* Unrolled Match Loop */
    k = m1;
    q = t - m1;
    while( k >= 4 )
    {
      if( pat[k] != q[k] )goto NoMatch;  k--;
      if( pat[k] != q[k] )goto NoMatch;  k--;
      if( pat[k] != q[k] )goto NoMatch;  k--;
      if( pat[k] != q[k] )goto NoMatch;  k--;
    }
    /* Finish Match Loop */
    while( k >= 0 )
    {
      if( pat[k] != q[k] )goto NoMatch;  k--;
    }
    /* If matched - return 1st char of pattern in text */
    return q;

NoMatch:
    
    /* Shift by 1, this replaces the good suffix shift */
    t++; 
  }

  return 0;
}





/*
**     mwmAlloc:: Allocate and Init Big hash Table Verions
**
**     maxpats - max number of patterns to support
**
*/
void * mwmNew()
{
   MWM_STRUCT * p = (MWM_STRUCT * )CALLOC( sizeof(MWM_STRUCT),1 );
   if( !p )
   { 
     return 0;
   }
   
   init_xlatcase();

   p->msSmallest = 32000;

   return (void*)p;
}


/*
**
*/
void mwmFree( void * pv )
{
   MWM_STRUCT * p = (MWM_STRUCT * )pv;
   if( p )
   {
     if( p->msPatArray ) free( p->msPatArray );
     if( p->msNumArray ) free( p->msNumArray );
     if( p->msHash     ) free( p->msHash );
     if( p->msShift2   ) free( p->msShift2 );
     free( p );
   }
}

/*
** mwmAddPatternEx::
**
** returns -1: max patterns exceeded
**          0: already present, uniqueness compiled in
**          1: added
*/
// 增加 Pattern 到 Wu Manber 的資料結構裡。
//    回傳 -1，已經新增 Pattern 到最大值。
//    回傳 0，已經新增過這條 Pattern。
//    回傳 1，已經新增 Pattern 成功。
int mwmAddPatternEx( void *pv, unsigned char * P, int m, 
       unsigned noCase,  unsigned offset, unsigned depth , void * id, int iid )
{
    // 建立資料結構。
    MWM_STRUCT *ps = (MWM_STRUCT*)pv;
    MWM_PATTERN_STRUCT *plist=0;
    MWM_PATTERN_STRUCT *p = (MWM_PATTERN_STRUCT*)calloc(sizeof(MWM_PATTERN_STRUCT),1);

    // 如果 Pattern 為空。
    if( !p ){
      return -1;
    }

    // 不確定。
#ifdef REQUIRE_UNIQUE_PATTERNS
    for( plist=ps->plist; plist!=NULL; plist=plist->next )
    {
       if( plist->psLen == m )
       {
           if( memcmp(P,plist->psPat,m) == 0 ) 
           {
               return 0;  /*already added */
           }
       }
    }
#endif

    // 走到 Pattern List 的尾端。
    if( ps->plist ){
       for( plist=ps->plist; plist->next!=NULL; plist=plist->next );
        plist->next = p;
    }else{
        ps->plist = p;
    }

    // 複製一份 Pattern，並且轉換成大寫。
    /* Allocate and store the Pattern  'P' with NO CASE info*/
    p->psPat =  (unsigned char*)malloc( m );
    if( !p->psPat ){
      return -1;
    }
    memcpy(p->psPat, P, m );
    ConvCaseToUpper( p->psPat, m );

    // 複製一份 Pattern，並且保持原狀。
    /* Allocate and store the Pattern  'P' with CASE info*/
    p->psPatCase =  (unsigned char*)malloc( m );
    if( !p->psPatCase ){
      return -1;
    }
    memcpy( p->psPatCase, P, m );

    // 設定一些其他的數值。
    p->psLen    = m;
    p->psID     = id; 
    p->psIID    = iid;
    p->psNoCase = noCase;
    p->psOffset = offset;
    p->psDepth  = depth;

    // 不確定。
    ps->msNoCase += noCase;
    
    // Pattern 數量增加一。
    ps->msNumPatterns++;

    // 找出 Pattern 的最大長度和最小長度。
    if( p->psLen < (unsigned)ps->msSmallest ) ps->msSmallest= p->psLen;
    if( p->psLen > (unsigned)ps->msLargest  ) ps->msLargest = p->psLen;
    
    // 統計全部 Pattern 總共多少長度、平均多少長度。
    ps->msTotal   += p->psLen;
    ps->msAvg      = ps->msTotal / ps->msNumPatterns;

    // 成功回傳 1。
    return 1;
}



#ifdef OLDSHIT
/*
** mwmAddPatternEx::
**
** returns -1: max patterns exceeded
**          0: already present, uniqueness compiled in
**          1: added
*/
int mwmAddPatternExOrig( MWM_STRUCT *ps, unsigned char * P, int m, 
       unsigned noCase,  unsigned offset, unsigned depth ,unsigned id, int iid )
{
    int kk;

    MWM_PATTERN_STRUCT *p;

    if( ps->msNumPatterns >= ps->msMaxPatterns )
    {
        return -1;
    }

#ifdef REQUIRE_UNIQUE_PATTERNS
    for(i=0;i<ps->msNumPatterns;i++)
    {
       if( ps->msPatArray[i].psLen == m )
       {
           if( memcmp(P,ps->msPatArray[i].psPat,m) == 0 ) 
           {
               return 0;  /*already added */
           }
       }
    }
#endif

    p = &ps->msPatArray[ ps->msNumPatterns ];

    /* Allocate and store the Pattern  'P' with NO CASE info*/
    p->psPat =  (unsigned char*)malloc( m );
    memcpy(p->psPat, P, m );

    ConvCaseToUpper( p->psPat, m );

    /* Allocate and store the Pattern  'P' with CASE info*/
    p->psPatCase =  (unsigned char*)malloc( m );
    memcpy( p->psPatCase, P, m );

    p->psLen    = m;
    p->psID     = id; 
    p->psIID    = iid;
    p->psNoCase = noCase;
    p->psOffset = offset;
    p->psDepth  = depth;

    ps->msNoCase += noCase;

    kk = ps->msNumPatterns;

    ps->msNumPatterns++;

    if( ps->msPatArray[kk].psLen < (unsigned)ps->msSmallest ) ps->msSmallest= ps->msPatArray[kk].psLen;
    if( ps->msPatArray[kk].psLen > (unsigned)ps->msLargest  ) ps->msLargest = ps->msPatArray[kk].psLen;
 
    ps->msTotal   += ps->msPatArray[kk].psLen;
    ps->msAvg      = ps->msTotal / ps->msNumPatterns;

    return 1;
}
#endif

  
/*
** Exact Pattern Matcher - don't use this...
*/
int mwmAddPattern( void * pv, unsigned char * P, int m, unsigned id )
{

    return mwmAddPatternEx( pv, P, m, 0, id, 0, pv, 0 );
}



/*
** bcompare:: 
**
** Perform a Binary comparsion of 2 byte sequences of possibly 
** differing lengths.
**
** returns -1 a < b
**         +1 a > b
**          0 a = b
*/
static int bcompare( unsigned char *a, int alen, unsigned char * b, int blen ) 
{
   // 記錄結果。
   int stat;
   
   if( alen == blen ){
    // 如果長度相同，直接進行比較。
    return memcmp(a,b,alen);
   }
   else if( alen < blen ){
      // 如果長度不同，比較 Pattern 最短的部分。
      if( (stat=memcmp(a,b,alen)) != 0 ){
        // 如果比較結果不同，回傳結果。
        return stat;
      }
     // 如果比較結果相同，回傳則短的 Pattern 比較小。
     return -1;
   }
   else{  
       // 如果長度不同，比較 Pattern 最短的部分。
      if( (stat=memcmp(a,b,blen)) != 0 ){
        // 如果比較結果不同，回傳結果。
        return stat;
      }
     // 如果比較結果相同，回傳則短的 Pattern 比較小。
     return +1;
   }
}

/*
** sortcmp::  qsort callback
*/
// 針對 Pattern 的字串做排序。
static int CDECL sortcmp( const void * e1, const void * e2 )
{ 
    // 倆倆比對的資料，轉換成對應的資料結構。
    MWM_PATTERN_STRUCT *r1= (MWM_PATTERN_STRUCT*)e1;
    MWM_PATTERN_STRUCT *r2= (MWM_PATTERN_STRUCT*)e2;
    // 提供字串和長度，進行比較。
    return bcompare( r1->psPat, r1->psLen, r2->psPat, r2->psLen ); 
}

/*
** HASH ROUTINE - used during pattern setup, but inline during searches
*/
static unsigned HASH16( unsigned char * T )
{
     return (unsigned short) (((*T)<<8) | *(T+1));
}

/*
** Build the hash table, and pattern groups
*/
static void mwmPrepHashedPatternGroups(MWM_STRUCT * ps)
{
  unsigned sindex,hindex,ningroup;
  int i;

  /*
  **  Allocate and Init 2+ byte pattern hash table 
  */
  // 建立 65535 長度的 Hash Table，這個 Table 是針對 2 個 Bytes 以上的 Pattern 用的。
   ps->msNumHashEntries = HASHTABLESIZE;
   ps->msHash = (HASH_TYPE*)CALLOC_HASH_TABLE(ps->msNumHashEntries, sizeof(HASH_TYPE));
   if( !ps->msHash ) 
   {
       FatalError("No memory in mwmPrephashedPatternGroups()\n"
                  "Try uncommenting the \"config detection: search-method\""
                  "in snort.conf\n");
   }

   // 將 Hash Table 每一個值都初始化為 -1。
   /* Init Hash table to default value */
   for(i=0;i<(int)ps->msNumHashEntries;i++)
   {
      ps->msHash[i] = (HASH_TYPE)-1;
   }
   
   // 針對 1 個 Bytes 的 Pattern 建立自己的 Hash Table。
   /* Initialize The One Byte Pattern Hash Table */
   for(i=0;i<256;i++)
   {   
      ps->msHash1[i] = (HASH_TYPE)-1;
   }

  /*
  ** Add the patterns to the hash table
  */
  // 將 Pattern 加入 Hash Table 中。
  for(i=0;i<ps->msNumPatterns;i++){ 
    // 如果 Pattern 的長度是 2 個 Bytes 以上。
    if( ps->msPatArray[i].psLen > 1 ){
       // 將只有大寫的 Pattern 取出，並且計算 16 bits 的 Hash，也就是 Pattern 前兩個字。
       hindex = HASH16(ps->msPatArray[i].psPat); 
       // Hash Table 裡記錄 Pattern 再第幾個位置。 
       sindex = ps->msHash[ hindex ] = i;
       // 並且計算有幾條 Pattern 相同 Hash，然後儲存在 msNumArray。
       // 因為 Pattern 有排序過才能這樣做。
       ningroup = 1;  
       while( (++i < ps->msNumPatterns) && (hindex==HASH16(ps->msPatArray[i].psPat)) ){
         ningroup++;
       }
       ps->msNumArray[ sindex ] = ningroup;
       i--;

    // 如果 Pattern 的長度是 1 個 Byte。
    }else if( ps->msPatArray[i].psLen == 1 ){
       // 將只有大寫的 Pattern 取出，並且計算 8 bits 的 Hash，也就是 Pattern 第一個字。
       hindex = ps->msPatArray[i].psPat[0]; 
       // Hash Table 裡記錄 Pattern 再第幾個位置。 
       sindex = ps->msHash1[ hindex ] = i;
       // 並且計算有幾條 Pattern 相同 Hash，然後儲存在 msNumArray。
       // 因為 Pattern 有排序過才能這樣做。
       ningroup = 1;  
       while((++i < ps->msNumPatterns) && (hindex == ps->msPatArray[i].psPat[0]) && (ps->msPatArray[i].psLen == 1))
         ningroup++;

       ps->msNumArray[ sindex ] = ningroup;
       i--;
    }
  }


}


/*
*  Standard Bad Character Multi-Pattern Skip Table
*/
static void mwmPrepBadCharTable(MWM_STRUCT * ps)
{
    unsigned  short i, k,  m, cindex, shift;
    unsigned  small_value=32000, large_value=0;
    
    // 找出最大 Pattern 長度和最小 Pattern 長度。
    /* Determine largest and smallest pattern sizes */
    for(i=0;i<ps->msNumPatterns;i++)
    {
      if( ps->msPatArray[i].psLen < small_value ) small_value = ps->msPatArray[i].psLen;
      if( ps->msPatArray[i].psLen > large_value ) large_value = ps->msPatArray[i].psLen;
    }

    // 如果最小 Pattern 長度大於 255，那麼 Shift 長度設定為 255。
    // 如果最小 Pattern 長度是 10，那麼 Shift 長度設定為 10。
    m = (unsigned short) small_value; 
    if( m > 255 ){
      m = 255;
    }
    ps->msShiftLen = m;

    // 設定 Shift Table 為最小 Pattern 長度。
    // 如果最小 Pattern 為 10，將 Shift Table 的長度初始化為 10。
    /* Initialze the default shift table. Max shift of 256 characters */
    for(i=0;i<256;i++){
       ps->msShift[i] = m;  
    }

    // 建立 Bas Character Shift Table。
    /*  Multi-Pattern BAD CHARACTER SHIFT */
    for(i=0;i<ps->msNumPatterns;i++)
    {
       for(k=0;k<m;k++)
       {
          // 從 Shift 值往前走，最大 255。
          shift = (unsigned short)(m - 1 - k);
          if( shift > 255 ){
            shift = 255;
          }

          // 從 Shift 值 255 開始往前取出字元。
          // 如果這個字元查到的 Shift 值是 20，假設目前 Shift 值是 10，則替換成最小的 10。
          // 如果這個字元查到的 Shift 值是 5，假設目前 Shift 值是 10，則不替換。
          // 這裡是考慮最小可以 Shift 的距離。
          cindex = ps->msPatArray[ i ].psPat[ k ];
          if( shift < ps->msShift[ cindex ] ){
              ps->msShift[ cindex ] = shift;
          }
       }
    }
}

/*
** Prep and Build a Bad Word Shift table
*/
static void mbmPrepBadWordTable(MWM_STRUCT * ps)
{
    // 初始化。
    int i;
    unsigned  short  k,  m, cindex;
    unsigned  small_value=32000, large_value=0;
    unsigned  shift;

    // 建立資料結構。
    ps->msShift2 = (unsigned char *)CALLOC_SHIFT_TABLE(BWSHIFTABLESIZE, sizeof(char));
    if( !ps->msShift2 )
         return;

    // 找出 Pattern 的最大最小長度。
    /* Determine largest and smallest pattern sizes */
    for(i=0;i<ps->msNumPatterns;i++){
        if( ps->msPatArray[i].psLen < small_value ) small_value = ps->msPatArray[i].psLen;
        if( ps->msPatArray[i].psLen > large_value ) large_value = ps->msPatArray[i].psLen;
    }

    // 用最小 Pattern 長度當作 Shift 的預設值，最長 255。
    m = (unsigned short) small_value;  /* Maximum Boyer-Moore Shift */
    /* Limit the maximum size of the smallest pattern to 255 bytes */
    if( m > 255 ){ 
      m = 255; 
    }
    ps->msShiftLen = m;

    // 將 Shift Table 的初始值都設定成最小 Pattern 長度。
    /* Initialze the default shift table. */
    for(i=0;i<BWSHIFTABLESIZE;i++){
      ps->msShift2[i] = (unsigned)(m-1);  
    }

    // 用兩個字元的 OR Hash 當作 Shift 值。
    /* Multi-Pattern Bad Word Shift Table Values */
    for(i=0;i<ps->msNumPatterns;i++)
    {
        for(k=0;k<m-1;k++)
        {
         shift = (unsigned short)(m - 2 - k);

         if( shift > 255 ){
           shift = 255;
         }

         cindex = ( ps->msPatArray[i].psPat[ k ] | (ps->msPatArray[i].psPat[k+1]<<8) );

         if( shift < ps->msShift2[ cindex ] ){
             ps->msShift2[ cindex ] = shift;
         }
       }
    }
}




/*
**  Print some Pattern Stats
*/
void mwmShowStats( void * pv )
{
   MWM_STRUCT * ps = (MWM_STRUCT*)pv;

   int i;
   printf("Pattern Stats\n");
   printf("-------------\n");
   printf("Patterns   : %d\n"      , ps->msNumPatterns);
   printf("Average    : %d chars\n", ps->msAvg);
   printf("Smallest   : %d chars\n", ps->msSmallest);
   printf("Largest    : %d chars\n", ps->msLargest);
   printf("Total chars: %d\n"    , ps->msTotal);

   for(i=0;i<ps->msLargest+1;i++)
   {
     if( ps->msLengths[i] ) 
       printf("Len[%d] : %d patterns\n", i, ps->msLengths[i] );
   }

   printf("\n");

}

/*
**  Calc some pattern length stats
*/
// 統計字串每種長度有多少條 Pattern。
static void mwmAnalyzePattens( MWM_STRUCT * ps )
{
   int i;

   // 建立 Pattern Max + 1 長度的 Counter Array。
   ps->msLengths= (int*) malloc( sizeof(int) * (ps->msLargest+1) );
   
   // 如果有 Pattern。 
   if( ps->msLengths )
   {
     // 先將所有 Counter 初始化為 0。
     memset( ps->msLengths, 0, sizeof(int) * (ps->msLargest+1) );

     /// 每找到一種長度，該 Counter 增加 1。
     for(i=0;i<ps->msNumPatterns;i++)
     {
        ps->msLengths[ ps->msPatArray[i].psLen ]++;
     }
   }
}



/*
**  Selects the bad Word Algorithm over the bad Character algorithm
**  This should be called before mwmPrepPatterns
*/
void mwmLargeShifts( void *  pv, int flag)
{
   MWM_STRUCT * ps = (MWM_STRUCT*)pv;
   ps->msLargeShifts = flag;
}

/*
** mwmGetNpatterns::
*/
int mwmGetNumPatterns( void  * pv )
{
    MWM_STRUCT *p = (MWM_STRUCT*)pv;
    return p->msNumPatterns;
}


#ifdef BITOP_TEST    
/*
*  Assign the rule mask vi an API
*/
void mwmSetRuleMask( void  * pv, BITOP * rm )
{
   MWM_STRUCT * ps = (MWM_STRUCT*) pv;
   ps->RuleMask = rm;
}
#endif

/*
*
*  Finds matches within a groups of patterns, these patterns all have at least 2 characters
*  This version walks thru all of the patterns in the group and applies a reverse string comparison
*  to minimize the impact of sequences of patterns that keep repeating intital character strings
*  with minimal differences at the end of the strings.
*
*/

static 
INLINE
int mwmGroupMatch2( MWM_STRUCT * ps, 
                   int index,
                   unsigned char * Tx, 
                   unsigned char * T, 
                   unsigned char * Tc, 
                   int Tleft,
                   void * data,
                   int (*match)(void*,int,void*) )
{
     int k, nfound=0;
     MWM_PATTERN_STRUCT * patrn; 
     MWM_PATTERN_STRUCT * patrnEnd; 

     // 利用 Hash 值找出 Pattern Bucket 和 Number of Pattern。
     /* Process the Hash Group Patterns against the current Text Suffix */
     patrn    = &ps->msPatArray[index];       
     patrnEnd = patrn + ps->msNumArray[index];

     // 走訪每一個 Pattern。
     /*  Match Loop - Test each pattern in the group against the Text */
     for( ;patrn < patrnEnd; patrn++ )  
     {
       unsigned char *p, *q;

       // 檢查 Input Text 還剩下多少，如果小於 Pattern 長度則不可能 Match。
       /* Test if this Pattern is to big for Text, not a possible match */
       if( (unsigned)Tleft < patrn->psLen )
           continue;

#ifdef BITOP_TEST    
       /* Test if this rule has been hit already - ignore it if it has */
       if( ps->RuleMask )
       {
    if( boIsBitSet( ps->RuleMask, patrn->psIID ) )
            continue;
       }
#endif

       /* Setup the reverse string compare */
       k = patrn->psLen - HASHBYTES16 - 1; 
       q = patrn->psPat + HASHBYTES16;     
       p = T            + HASHBYTES16;     

       /* Compare strings backward, unrolling does not help in perf tests. */
       while( k >= 0 && (q[k] == p[k]) ) k--;

       /* We have a content match - call the match routine for further processing */
       if( k < 0 ) 
       {
         if( Tc && !patrn->psNoCase )
         {  
           /* Validate a case sensitive match - than call match */
           if( !memcmp(patrn->psPatCase,&Tc[T-Tx],patrn->psLen) )
           {
             nfound++; 
             if( match( patrn->psID, (int)(T-Tx), data ) )
               return -(nfound+1);
           }

         }else{
           nfound++; 
           if( match( patrn->psID, (int)(T-Tx), data ) )
               return -(nfound+1);
         }
       }
     }

     return nfound;
}


/*
**  
**  No Bad Character Shifts
**  Handles pattern groups with one byte or larger patterns 
**  Uses 1 byte and 2 byte hash tables to group patterns
**
*/
static int mwmSearchExNoBC( MWM_STRUCT *ps, 
                unsigned char * Tx, int n, unsigned char * Tc,
                int(*match)( void * id, int index,void* data ),
                void * data )
{
    int                 Tleft, index, nfound, ng;
    unsigned char      *T, *Tend, *B;
    MWM_PATTERN_STRUCT *patrn, *patrnEnd;

    nfound = 0;

    Tleft = n;
    Tend  = Tx + n;

    /* Test if text is shorter than the shortest pattern */
    if( (unsigned)n < ps->msShiftLen )
        return 0;

    /*  Process each suffix of the Text, left to right, incrementing T so T = S[j] */
    for(T = Tx, B = Tx + ps->msShiftLen - 1; B < Tend; T++, B++, Tleft--)
    {
        /* Test for single char pattern matches */
        if( (index = ps->msHash1[*T]) != (HASH_TYPE)-1 )
        {
            patrn    = &ps->msPatArray[index];
            patrnEnd = patrn + ps->msNumArray[index];

            for( ;patrn < patrnEnd; patrn++ )  
            {
                if( Tc && !patrn->psNoCase )
                {    
                    if( patrn->psPatCase[0] == Tc[T-Tx] )
                  {  
                        nfound++;
                        if(match(patrn->psID,  (int)(T-Tx), data))  
                            return nfound;
                    }
                }
                else
                {
                    nfound++;
                    if(match(patrn->psID, (int)(T-Tx), data))  
                        return nfound;
                }
            }
        }     

        /* 
        ** Test for last char in Text, one byte pattern test 
        ** was done above, were done. 
        */
        if( Tleft == 1 )
            return nfound; 

        /* 
        ** Test if the 2 char prefix of this suffix shows up 
        ** in the hash table
        */
        if( (index = ps->msHash [ ( (*T)<<8 ) | *(T+1) ] ) == (HASH_TYPE)-1 )
            continue; 

        /* Match this group against the current suffix */
        ng = mwmGroupMatch2( ps, index,Tx, T, Tc, Tleft, data, match );
        if( ng < 0 )
        {
            ng = -ng;
            ng--;
            nfound += ng;

            return nfound;
        }
        else
        {
            nfound += ng;
        }
    }

    return nfound;
}


/*
**
**  Uses Bad Character Shifts
**  Handles pattern groups with 2 or more bytes per pattern
**  Uses 2 byte hash table to group patterns
**
*/
static 

int mwmSearchExBC( MWM_STRUCT *ps, 
                   unsigned char * Tx, int n, unsigned char * Tc,
                   int(*match)( void * id, int index, void * data ), 
                   void * data )
{
  int                 Tleft, index, nfound, tshift,ng;
  unsigned char      *T, *Tend, *B;
  /*MWM_PATTERN_STRUCT *patrn, *patrnEnd;*/

  // 計算找到的數量。
  nfound = 0;

  // Input Text 的長度。
  Tleft = n;
  // Input Text 的尾部。
  Tend  = Tx + n;

  // 如果 Input Text 比最小 Pattern 長度短一定找不到，直接回傳。
  /* Test if text is shorter than the shortest pattern */
  if( (unsigned)n < ps->msShiftLen ){
      return 0;
  }


  // 走訪 Input Text。
  /*  Process each suffix of the Text, left to right, incrementing T so T = S[j] */
  for( T = Tx, B = Tx + ps->msShiftLen - 1; B < Tend; T++, B++, Tleft-- )
  {   
       // 不停走訪 Input Text，直到 Shift Table 不等於 0。
       /* Multi-Pattern Bad Character Shift */
       while( (tshift=ps->msShift[*B]) > 0 ) 
       {
          B += tshift; T += tshift; Tleft -= tshift;
          if( B >= Tend ) return nfound;

          // 雖然這裡多 Shift 了一次，但只要遇到 Shift Value 等於 0 則不會 Shift。
          // 不知道意義為何，刪掉這一段動作是相同的。
          tshift=ps->msShift[*B];
          B += tshift; T += tshift; Tleft -= tshift;
          if( B >= Tend ) return nfound;
       }

     // 如果 Input Text 走到底了，直接回傳。
     /* Test for last char in Text, one byte pattern test was done above, were done. */
     if( Tleft == 1 ){   
        return nfound;
     } 

     // 如果 Hash Table 查不到，繼續往下找。
     /* Test if the 2 char prefix of this suffix shows up in the hash table */
     if( (index = ps->msHash [ ( (*T)<<8 ) | *(T+1) ] ) == (HASH_TYPE)-1 ){
         continue; 
     }

     /* Match this group against the current suffix */
     ng = mwmGroupMatch2( ps, index,Tx, T, Tc, Tleft, data, match );
     if( ng < 0 ){
         ng = -ng;
         ng--;
         nfound += ng;
         return nfound;
     }else{
        nfound += ng;
     }
  }

  return nfound;
}


/*
**
**  Uses Bad Word Shifts
**  Handles pattern groups with 2 or more bytes per pattern
**  Uses 2 byte hash table to group patterns
**
*/
static 
int mwmSearchExBW( MWM_STRUCT *ps, 
                unsigned char * Tx, int n, unsigned char * Tc,
                int(*match) ( void *  id, int index,void* data ),
                void * data )

{
  int                 Tleft, index, nfound, tshift, ng;
  unsigned char      *T, *Tend, *B;

  nfound = 0;

  Tleft = n;
  Tend  = Tx + n;

  /* Test if text is shorter than the shortest pattern */
  if( (unsigned)n < ps->msShiftLen )
      return 0;

  /*  Process each suffix of the Text, left to right, incrementing T so T = S[j] */
  for( T = Tx, B = Tx + ps->msShiftLen - 1; B < Tend; T++, B++, Tleft-- )
  {
     /* Multi-Pattern Bad Word Shift */
     tshift = ps->msShift2[ ((*B)<<8) | *(B-1) ];
     while( tshift ) 
     {
        B     += tshift;  T += tshift; Tleft -= tshift;
        if( B >= Tend ) return nfound;
        tshift = ps->msShift2[ ((*B)<<8) | *(B-1) ];
     }

     /* Test for last char in Text, we are done, one byte pattern test was done above. */
     if( Tleft == 1 ) return nfound; 


     /* Test if the 2 char prefix of this suffix shows up in the hash table */
     if( (index = ps->msHash [ ( (*T)<<8 ) | *(T+1) ] ) == (HASH_TYPE)-1 )
         continue; 


     /* Match this group against the current suffix */
     ng = mwmGroupMatch2( ps, index,Tx, T, Tc, Tleft, data, match );
     if( ng < 0 )
     {
         ng = -ng;
         ng--;
         nfound += ng;
         return nfound;
     }
     else
     {
        nfound += ng;
     }

  }

  return nfound;
}


/* Display function for testing */
static void show_bytes(unsigned n, unsigned char *p)
{
    int i;
    for(i=0;i<(int)n;i++)
    {
       if( p[i] >=32 && p[i]<=127 )printf("%c",p[i]);
       else printf("\\x%2.2X",p[i]);
    }
  
}

/*
**   Display patterns in this group
*/
void mwmGroupDetails( void * pv )
{
   MWM_STRUCT * ps = (MWM_STRUCT*)pv;
   int index,i, m, gmax=0, total=0,gavg=0,subgroups;
   static int k=0;
   MWM_PATTERN_STRUCT *patrn, *patrnEnd;

    printf("*** MWM-Pattern-Group: %d\n",k++); 

    subgroups=0;   
    for(i=0;i<65536;i++)
    {
       if( (index = ps->msHash [i]) == (HASH_TYPE)-1 )
           continue; 
     
       patrn    = &ps->msPatArray[index];       /* 1st pattern of hash group is here */
       patrnEnd = patrn + ps->msNumArray[index];/* never go here... */
    
       printf("  Sub-Pattern-Group: %d-%d\n",subgroups,i);
       
       subgroups++;
       
       for( m=0; patrn < patrnEnd; m++, patrn++ )  /* Test all patterns in the group */
       {
          printf("   Pattern[%d] : ",m);

    show_bytes(patrn->psLen,patrn->psPat);
    
    printf("\n");
       }  
       
       if( m > gmax ) gmax = m;
       
       total+=m;
       
       gavg = total / subgroups;
    }
    
    printf("Total Group Patterns    : %d\n",total);
    printf("  Number of Sub-Groups  : %d\n",subgroups);
    printf("  Sub-Group Max Patterns: %d\n",gmax);
    printf("  Sub-Group Avg Patterns: %d\n",gavg);

}   
   
/*
**
** mwmPrepPatterns::    Prepare the pattern group for searching
**
*/
int mwmPrepPatterns( void * pv ){

   MWM_STRUCT* ps = (MWM_STRUCT *) pv;
   int kk;
   MWM_PATTERN_STRUCT* plist;

   // 有多少條 Pattern 就建立多長的 Array，單位是 MWM_PATTERN_STRUCT。
   /* Build an array of pointers to the list of Pattern nodes */
   ps->msPatArray = (MWM_PATTERN_STRUCT*)CALLOC( sizeof(MWM_PATTERN_STRUCT), ps->msNumPatterns );
   if( !ps->msPatArray ) 
   {
        return -1; 
   }
   
   // 有多少條 Pattern 就建立多長的 Array，單位是 unsigned short。 
   ps->msNumArray = (unsigned short *)CALLOC( sizeof(short), ps->msNumPatterns  );
   if( !ps->msNumArray ) 
   { 
        return -1; 
   }


   // 複製 Link List Node 到 Array Node。
   /* Copy the list node info into the Array */
   for( kk=0, plist = ps->plist; plist!=NULL && kk < ps->msNumPatterns; plist=plist->next )
   {
        memcpy( &ps->msPatArray[kk++], plist, sizeof(MWM_PATTERN_STRUCT) );
   }
  
  // 統計字串每種長度有多少條 Pattern。
  mwmAnalyzePattens( ps );

  // 根據 sortcmp 方法規定的方式排序字串。
  // 如果長度相同，直接用 memcmp 方法比較。
  // 如果長度不同，在最短部分用 memcmp 方法比較。
  // 如果長度不同，在最短部分用 memcmp 結果相同，則較短的 Pattern 比較小。
  /* Sort the patterns */
  qsort( ps->msPatArray, ps->msNumPatterns, sizeof(MWM_PATTERN_STRUCT), sortcmp ); 

  // 建立 Wu Manber 的 Hash Table 和 Pattern Group。
  /* Build the Hash table, and pattern groups, per Wu & Manber */
  mwmPrepHashedPatternGroups(ps);

  /* Select the Pattern Matcher Class */
  if( ps->msNumPatterns < 5 ){
     // 如果 Pattern 數量小於 5，使用 Boyer Moore。
     ps->msMethod =  MTH_BM;
  }else{
     // 如果 Pattern 數量大於等於 5，使用 Wu-Manber。
     ps->msMethod =  MTH_MWM;
  }
 

  // 如果使用 Wu-Manber。
  /* Setup Wu-Manber */
  if( ps->msMethod == MTH_MWM )
  { 
      // 建立 Wu-Manber 的 Shift Char Table。
      /* Build the Bad Char Shift Table per Wu & Manber */
      mwmPrepBadCharTable(ps);
      ADD_SHIFT_TABLE(256, sizeof(short));

     // 如果 LargeShift 這個 Flag 沒有設定成 1，這裡不會動作。
     // 如果最小的 Pattern 長度是 10，那麼最小 Shift 距離也會是 10。
     /* Build the Bad Word Shift Table per Wu & Manber */
     if( (ps->msShiftLen > 1) && ps->msLargeShifts ){
       mbmPrepBadWordTable( ps );
     }

     // 如果最小 Pattern 長度是 1。
     /* Min patterns is 1 byte */
     if( ps->msShiftLen == 1 ){
        // printf("[INFO] Mehod 1: mwmSearchExNoBC\n");
        ps->search =  mwmSearchExNoBC;
      
     // 如果最小 Pattern 長度大於 1 並且不啟動 msLargeShifts。
     /* Min patterns is >1 byte */
     }else if( (ps->msShiftLen >  1) && !ps->msLargeShifts ){
        // printf("[INFO] Mehod 2: mwmSearchExBC\n");
      ps->search =  mwmSearchExBC;
     
     // 如果最小 Pattern 長度大於 1，並且啟動 msLargeShifts 和 msShift2。
     /* Min patterns is >1 byte - and we've been asked to use a 2 byte bad words shift instead. */
     }else if( (ps->msShiftLen >  1) && ps->msLargeShifts && ps->msShift2 ){
        // printf("[INFO] Mehod 3: mwmSearchExBW\n");
      ps->search =  mwmSearchExBW;
     
     // 如果最小 Pattern 長度大於 1。
     /* Min patterns is >1 byte */
     }else{
        // printf("[INFO] Mehod 4: mwmSearchExBC\n");
        ps->search =  mwmSearchExBC;
     }

// 不確定。
#ifdef XXXX      
    // if( ps->msDetails )   /* For testing - show this info */
    //    mwmGroupDetails( ps );
#endif

   }

   // 如果 Pattern 數量小於 5，那麼會使用 Boyer Moore。
   // 每一條 Pattern 都會建立自己的 Boyer Moore 資料結構。
   /* Initialize the Boyer-Moore Pattern data */
   if( ps->msMethod == MTH_BM )
   {
       int i;

       /* Allocate and initialize the BMH data for each pattern */
       for(i=0;i<ps->msNumPatterns;i++)
       {
           ps->msPatArray[ i ].psBmh = hbm_prep( ps->msPatArray[ i ].psPat, ps->msPatArray[ i ].psLen );
       }   
   }

   return 0;
}

/*
** Search a body of text or data for paterns 
*/
int mwmSearch( void * pv,
               unsigned char * T, int n,
  int(*match)( void * id,  int index, void * data ),
               void * data )
{
      MWM_STRUCT * ps = (MWM_STRUCT*)pv;
 
      iPatCount += n;


      // 將 Input Text 全部轉換為大寫。
      ConvCaseToUpperEx( S, T, n ); /* Copy and Convert to Upper Case */

      // 如果 Pattern 數量小於 5，會使用 Boyer Moore。
      if( ps->msMethod == MTH_BM )
      {
         /* Boyer-Moore  */

         int i,nfound=0;
         unsigned char * Tx;

         for( i=0; i<ps->msNumPatterns; i++ )
         {
            Tx = hbm_match( ps->msPatArray[i].psBmh, S, n );

            if( Tx )
            {
               /* If we are case sensitive, do a final exact match test */
               if( !ps->msPatArray[i].psNoCase )
               {
                 if( memcmp(ps->msPatArray[i].psPatCase,&T[Tx-S],ps->msPatArray[i].psLen) )
                     continue; /* no match, next pattern please */
               }

               nfound++;
  
               if( match(ps->msPatArray[i].psID, (int)(Tx-S),data) )
                  return nfound;
            }
         }

         return nfound;

      }
      // 如果 Pattern 長度大於等於 5，使用 Wu-Manber 的方法。
      /* MTH_MWM */
      else {
        // 如果最小 Pattern 長度是 1，執行 mwmSearchExNoBC 方法。
        // 如果最小 Pattern 長度大於 1 並且不啟動 msLargeShifts，執行 mwmSearchExBC 方法。
        // 如果最小 Pattern 長度大於 1 並且啟動 msLargeShifts，執行 mwmSearchExBW 方法。
        // 如果最小 Pattern 長度大於 1，執行 mwmSearchExBC 方法。
         /* Wu-Manber */
         // printf("Search Method: %p \n", ps->search);
         return ps->search( ps, S, n, T, match, data );
      }
}


/*
**
*/
void mwmFeatures(void)
{
   printf("%s\n",MWM_FEATURES);
}

#ifdef MWM_MAIN
int FatalError( char * s )
{
   printf("FatalError: %s\n",s);
   exit(0);
}
/*
    global array of pattern pointers feeds of of ID..see argv parseing...
*/
char * patArray[10000];

/*
** Routine to process matches
*/
static int match (  void* id, int index, void * data )
{
   //  printf(" pattern matched: index= %d, id=%d, %s \n",   index, id, patArray[(int)id]  );
   return 0;
}


/*
*/
typedef struct
{
  unsigned char * b;
  int blen;
}BINARY;

/*
*/
int gethex( int c )
{
   
   if( c >= 'A' && c <= 'F' ) return c -'A' + 10;
   if( c >= 'a' && c <= 'f' ) return c -'a' + 10;
   if( c >= '0' && c <= '9' ) return c -'0';

   return 0;
}
/*
*/
BINARY  * converthexbytes( unsigned char * s)
{
   int      val, k=0, m;
   BINARY * p;
   int      len = strlen(s);

   printf("--input hex: %s\n",s);   

   p = malloc( sizeof(BINARY) );

   p->b   = malloc( len / 2 );
   p->blen= len / 2;

   while( *s )
   {
      val   = gethex(*s);
      s++;
      val <<= 4;

      if( !*s ) break; // check that we have 2 digits for hex, else ignore the 1st

      val |= gethex(*s);
      s++;

      p->b[k++] = val;
   }

   if( k != p->blen )
   {
      printf("hex length mismatch\n");
   }

   printf("--output hex[%d]: ",p->blen); for(m=0;m<p->blen;m++) printf("%2.2x", p->b[m]);

   printf(" \n");

   return p;
}

/*
   Synthetic data
*/
BINARY * syndata( int nbytes, int irand, int repchar )
{
   BINARY * p =(BINARY*)malloc( sizeof(BINARY) );
   if( ! p ) return 0;
 
   p->b    = (unsigned char *)malloc( nbytes );
   if( ! p->b ) return 0;
   p->blen = nbytes;

   if( irand )
   {
     int i;

     srand( time(0) );

     for(i=0;i<nbytes;i++)
     {
        p->b[i] = (unsigned)( rand() & 0xff );
     }

   }
   else
   {
       memset(p->b,repchar,nbytes);
   }
   return p;
}
/*
*/
int randpat( unsigned char * s, int imin )
{
    int i,len;

    static int first=1;

    if( first )
    {
       first=0;
       srand(time(0));
    }

    while( 1 )
    {
       len = rand() & 0xf; //max of 15 bytes 
       if( len >= imin ) break;
    }
    

    for(i=0;i<len;i++)
    {
        s[i] = 'a' + ( rand() % 26 ); // a-z
    }

    s[len]=0;

    printf("--%s\n",s);
    return len;
}

// === CUSTOM CODE ===============================
#include "util_file.h"
#include "clock_count.h"

static void addPatternIntoAc(MWM_STRUCT* ps, unsigned int size, unsigned char** patterns, unsigned int* lengths){
  for(unsigned int i = 0; i < size; i++){
    // 將 pattern 加入 Aho Corasick，尚未編譯。
    int nocase = 0;
    int offset = 0;
    int depth = 0;
    int negative = 0;
    void* value = patterns[i];
    int id = i;
    mwmAddPatternEx( ps, patterns[i], lengths[i], nocase, 0, 0, (patterns + i) /* ID */, 3000 /* IID -internal id*/ );
    // acsmAddPattern2(acsm, patterns[i], lengths[i], nocase, offset, depth, negative, value, id);
  }
}

void showWuMemoryInfo(){
   printf("[INFO] Memory Pattern: %llu \n", costMemory);
   printf("[INFO] Memory Shift Table: %llu \n", memoryShiftTable);
   printf("[INFO] Memory Hash Table: %llu \n", memoryHashTable);
   printf("\n");
}
// ===============================================

/*
** Test driver 
*/

// int CDECL main ( int argc, char ** argv )
// // int main ( int argc, char ** argv )
// {  
//   // 儲存 Trace 的字串。
//    unsigned char *T, *text;
//    int            n,textlen; 
//    int            nmatches, i, bm = 0;
//    MWM_STRUCT    *ps;
//    int            npats=0, len, stat, nocase=0;
//    BINARY        *p;
//    int            irep=0,irand=0,isyn=0,nrep=1024,repchar=0;

//    // 如果參數不對。
//    if( argc < 5 )
//    {
//       printf("usage: %s [-rand|-rep -n bytes -c|ch repchar -pr npats minlen ] -t|th TextToSearch -nocase [-pr numpats] -p|ph pat1 -p|ph pat2 -p|ph pat3\n",argv[0]);
//       exit(1);
//    }

//    // 建立 Wu-Manber 的資料結構。
//    /* -- Allocate a Pattern Matching Structure - and Init it. */
//    ps = mwmNew();

//    for(i=1;i<argc;i++)
//    {
//        // 不在意大小寫。
//        if( strcmp(argv[i],"-nocase")==0 )
//        {
//           nocase = 1;
//        }

//        // 增加 Pattern。
//        if( strcmp(argv[i],"-p")==0 )
//        {
//           i++;
//           npats++;
//           patArray[npats] = argv[i];
//           len = strlen( argv[i] );

//           // mwmAddPatternEx( ps, (unsigned char*)argv[i], len, nocase, 0, 0, (void*)npats /* ID */, 3000 /* IID -internal id*/ );
//           mwmAddPatternEx( ps, (unsigned char*)argv[i], len, nocase, 0, 0, (patArray + npats) /* ID */, 3000 /* IID -internal id*/ );
//        }

//        // 增加 Hex Pattern。
//        if( strcmp(argv[i],"-ph")==0 )
//        {
//           i++;
//           npats++;
//           patArray[npats] = argv[i];
//           p = converthexbytes( argv[i] );

//           // mwmAddPatternEx( ps, p->b, p->blen, 1 /* nocase*/, 0, 0, (void*)npats /* ID */, 3000 /* IID -internal id*/ );
//           mwmAddPatternEx( ps, p->b, p->blen, 1 /* nocase*/, 0, 0, (patArray + npats) /* ID */, 3000 /* IID -internal id*/ );
//        }

//        // 增加隨機產生的 Pattern，指定 Pattern 數量和最小長度。
//        if( strcmp(argv[i],"-pr")==0 )
//        {
//           int m = atoi( argv[++i] );
//           int imin = atoi( argv[++i] );
//           int k;
//           npats = 0;
//           for(k=0;k<m;k++)
//           {
//              unsigned char px[256];
//              int           len = randpat( px, imin );
//              npats++;
//              patArray[npats] = (unsigned char *)malloc( len+1 );
//              sprintf(patArray[npats],"%s",px);
//              // mwmAddPatternEx( ps, px, len, 0 /* nocase*/, 0, 0, (void*)npats /* ID */, 3000 /* IID -internal id*/ );
//              mwmAddPatternEx( ps, px, len, 0 /* nocase*/, 0, 0, (patArray + npats) /* ID */, 3000 /* IID -internal id*/ );
//           }
//        }

//        // 不確定。
//        if( strcmp(argv[i],"-rand")==0 )
//        {
//           irand = 1;
//           isyn  = 1;
//        }

//        // 不確定。
//        if( strcmp(argv[i],"-rep")==0 )
//        {
//           irep  = 1;
//           isyn  = 1;
//        }

//        // 不確定。
//        if( strcmp(argv[i],"-n")==0 )
//        {
//           nrep = atoi( argv[++i] );
//           isyn  = 1;
//        }

//        // 不確定。
//        if( strcmp(argv[i],"-c")==0 )
//        {
//           repchar = argv[++i][0];
//           isyn  = 1;
//        }

//        // 不確定。
//        if( strcmp(argv[i],"-ch")==0 )
//        {
//           BINARY * px = converthexbytes( argv[++i] );
//           repchar = px->b[0];
//           isyn  = 1;
//        }

//        // 設定 Trace File 的字串。
//        if( strcmp(argv[i],"-t")==0 )
//        {
//           i++;
//           text = argv[i];
//           textlen=strlen(text);
//        }

//        // 設定 Trace File 的 Hex 字串。
//        if( strcmp(argv[i],"-th")==0 )
//        {
//            i++;
//            p = converthexbytes( argv[i] );
//            text = p->b;
//            textlen = p->blen;
//        }
//    }

//   // 自行增加 Pattern 字串和 Trace 字串。
//   // === CUSTOM CODE ===============================
//   unsigned int numberOfPattern = 0;
//   unsigned char** patternGroup = NULL;
//   unsigned int* patternLengthGroup = NULL;

//   // readPatternFile("../patterns/patterns_500", &numberOfPattern, &patternGroup, &patternLengthGroup);
//   readPatternFile("../patterns/patterns_500", &numberOfPattern, &patternGroup, &patternLengthGroup);
//   addPatternIntoAc(ps, numberOfPattern, patternGroup, patternLengthGroup);
  
//   unsigned long long int traceSize = 0;
//   char* trace = NULL; 
//   // readTraceFile("../traces/trace_7_90p", &traceSize, &trace);
//   readTraceFile("../traces/trace_7_90p", &traceSize, &trace);
//   text = trace;
//   textlen = traceSize;
//   // ===============================================


//    // 字面上意思是將字串合成，目前猜測是隨機產生字串。
//    /* generate synthetic text */
//    if( isyn ){
//        p = syndata( nrep, irand, repchar );
//        text = p->b;
//        textlen = p->blen;
//    }

//     // 是否用兩個字元的 Hash 計算 Shift Table，這是自己加上去的。
//     // mwmLargeShifts(ps, 1);
    
//     // 將 Pattern 建立 Wu-Manber 的資料結構。
//     /* --- Preprocess Patterns */
//     mwmPrepPatterns( ps );  

//     // //  ---- Do a multi-pattern search in the Text 
//     // stat = mwmSearch( (void*)ps, (unsigned char*)text, textlen, match, 0 ); 


//     // 計算搜尋花費的時間。
//     // === CUSTOM CODE ===============================
//     initClockTimer();
//     ClockCount.reset();
//     for(unsigned int i = 0; i < 100; i++){
//       ClockCount.begin();
//       unsigned int current_state = 0;
//       // 主要的搜尋方法。
//       stat += mwmSearch( (void*)ps, (unsigned char*)text, textlen, match, 0 ); 
//       ClockCount.end();
//     }
      
//     ClockCount.show();
//     // ===============================================

//    if( stat == 0 )
//    {
//        printf("no pattern matches\n");
//    }else{
//        printf("%d pattern matches in list\n",stat);
//    }

//    printf("[INFO] Memory Pattern: %llu \n", costMemory);
//    printf("[INFO] Memory Shift Table: %llu \n", memoryShiftTable);
//    printf("[INFO] Memory Hash Table: %llu \n", memoryHashTable);

//    return 0;
// }

#endif
