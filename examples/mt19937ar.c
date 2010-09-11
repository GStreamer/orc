/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include <stdio.h>
#include <orc/orc.h>
#include <stdlib.h>
#include <string.h>
#include "mt19937arorc.h"

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(orc_uint32 init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* Orc version */

typedef struct _OrcRandomContext OrcRandomContext;
struct _OrcRandomContext {
  orc_uint32 d[N];
  orc_uint32 mt[N+1];
  int mti;
};

OrcRandomContext *
orc_random_context_new (void)
{
  OrcRandomContext *context;
  context = malloc(sizeof(OrcRandomContext));
  memset (context, 0, sizeof(OrcRandomContext));
  context->mti = N+1;
  return context;
}

void
orc_random_init_genrand(OrcRandomContext *context, orc_uint32 s)
{
  orc_uint32 *mt = context->mt;
  int mti;

  mt[0] = s;
  for (mti=1; mti<N; mti++) {
    mt[mti] = 
      (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
  }
  context->mti = mti;
}

void
orc_random_init_by_array (OrcRandomContext *context, orc_uint32 *init_key,
    int key_length)
{
  int i, j, k;
  orc_uint32 *mt = context->mt;

  orc_random_init_genrand (context, 19650218UL);
  i=1; j=0;
  k = (N>key_length ? N : key_length);
  for (; k; k--) {
    mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
      + init_key[j] + j; /* non linear */
    i++; j++;
    if (i>=N) { mt[0] = mt[N-1]; i=1; }
    if (j>=key_length) j=0;
  }
  for (k=N-1; k; k--) {
    mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
      - i; /* non linear */
    i++;
    if (i>=N) { mt[0] = mt[N-1]; i=1; }
  }

  mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

#if 0
/* These are the functions that were converted to Orc code. */
static void
mix (orc_uint32 *mt, orc_uint32 *mt2, int n)
{
  orc_uint32 y;
  int kk;

  for (kk=0;kk<n;kk++) {
    orc_uint32 t1;
    orc_uint32 t2;

    t1 = mt[kk]&UPPER_MASK;
    t2 = mt[kk+1]&LOWER_MASK;
    y = t1 | t2;

    t1 = y&1;
    t1 = (t1) ? MATRIX_A : 0;
    y = y >> 1;
    y ^= t1;

    mt[kk] = mt2[kk] ^ y;
  }
}

static void
temper (orc_uint32 *d, orc_uint32 *mt, int n)
{
  orc_uint32 y;
  int i;

  for(i=0;i<N;i++){
    y = mt[i];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    d[i] = y;
  }
}
#endif

static void
update_context (OrcRandomContext *context)
{
  orc_uint32 *d = context->d;
  orc_uint32 *mt = context->mt;

#if 0
  mix (mt, mt + M, N-M);
  mt[N] = mt[0];
  mix (mt + N - M, mt, M);
  temper (d, mt, N);
#endif

  mt19937ar_mix (mt, mt + 1, mt + M, N-M);
  mt[N] = mt[0];
  mt19937ar_mix (mt + N - M, mt + N - M + 1, mt, M);
  mt19937ar_temper (d, mt, N);

#if 0
  /* too many temp variables, compiles incorrectly */
  mt19937ar_mix_temper (d, mt, mt + 1, mt + M, N-M);
  mt[N] = mt[0];
  mt19937ar_mix_temper (d + N - M, mt + N - M, mt + N - M + 1, mt, M);
#endif
}

orc_uint32
orc_random_genrand_int32 (OrcRandomContext *context)
{
  if (context->mti >= N) { /* generate N words at one time */
    if (context->mti == N+1)   /* if init_genrand() has not been called, */
      orc_random_init_genrand(context, 5489UL); /* a default initial seed is used */

    update_context (context);

    context->mti = 0;
  }

  return context->d[context->mti++];
}


int main(void)
{
  int i;
  orc_uint32 init[4]={0x123, 0x234, 0x345, 0x456};
  int length=4;
  orc_uint32 ref, test;
  OrcRandomContext *context;
  int error = 0;

  init_by_array(init, length);

  context = orc_random_context_new ();
  orc_random_init_by_array (context, init, length);

  printf("1000 outputs of genrand_int32()\n");
  for (i=0; i<1000; i++) {
    ref = genrand_int32();
    test = orc_random_genrand_int32(context);
    printf("%08x %08x %c\n", ref, test, (ref == test) ? ' ' : '*');
    if (ref != test) error = 1;
  }
  if (error) {
    printf("FAIL\n");
  }

  return 0;
}

