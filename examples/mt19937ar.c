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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <orc/orc.h>

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */


/* mag01[x] = x * MATRIX_A  for x=0,1 */
static const orc_uint32 mag01[2]={0x0UL, MATRIX_A};

void
mt19937_ref (orc_uint32 *d, orc_uint32 *mt)
{
  orc_uint32 y;
  int kk;

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

  for(kk=0;kk<N;kk++){
    y = mt[kk];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    d[kk] = y;
  }
}


OrcProgram *p1, *p2;

void
create_programs (void)
{

  /* s1 is mt, s2 is mt+1, s3 is mt+M */
  p1 = orc_program_new_dss (4, 4, 4);
  orc_program_add_source (p1, 4, "s3");

  orc_program_add_temporary (p1, 4, "t1");
  orc_program_add_temporary (p1, 4, "t2");
  orc_program_add_temporary (p1, 4, "y");

  orc_program_add_constant (p1, 4, UPPER_MASK, "c1");
  orc_program_add_constant (p1, 4, LOWER_MASK, "c2");
  orc_program_add_constant (p1, 4, 1, "c3");
  orc_program_add_constant (p1, 4, MATRIX_A, "c6");

  orc_program_append_str (p1, "andl", "t1", "s1", "c1");
  orc_program_append_str (p1, "andl", "t2", "s2", "c2");
  orc_program_append_str (p1, "orl", "y", "t1", "t2");

  orc_program_append_str (p1, "shrul", "t1", "y", "c3");
  orc_program_append_str (p1, "xorl", "t2", "s3", "t1");
  orc_program_append_str (p1, "andl", "y", "y", "c3");
  orc_program_append_str (p1, "cmpeql", "y", "y", "c3");
  orc_program_append_str (p1, "andl", "y", "y", "c6");
  orc_program_append_str (p1, "xorl", "d1", "t2", "y");
  
  orc_program_compile (p1);


  p2 = orc_program_new ();
  orc_program_add_destination (p2, 4, "d1");
  orc_program_add_temporary (p2, 4, "y");
  orc_program_add_temporary (p2, 4, "t1");

  orc_program_add_constant (p2, 4, 11, "c11");
  orc_program_add_constant (p2, 4, 7, "c7");
  orc_program_add_constant (p2, 4, 0x9d2c5680, "x1");
  orc_program_add_constant (p2, 4, 15, "c15");
  orc_program_add_constant (p2, 4, 0xefc60000, "x2");
  orc_program_add_constant (p2, 4, 15, "c18");

  orc_program_append_str (p2, "shrul", "t1", "d1", "c11");
  orc_program_append_str (p2, "xorl", "y", "d1", "t1");
  orc_program_append_str (p2, "shll", "t1", "y", "c7");
  orc_program_append_str (p2, "andl", "t1", "t1", "x1");
  orc_program_append_str (p2, "xorl", "y", "y", "t1");
  orc_program_append_str (p2, "shll", "t1", "y", "c15");
  orc_program_append_str (p2, "andl", "t1", "t1", "x2");
  orc_program_append_str (p2, "xorl", "y", "y", "t1");
  orc_program_append_str (p2, "shrul", "t1", "y", "c18");
  orc_program_append_str (p2, "xorl", "d1", "y", "t1");

  orc_program_compile (p2);

}


int
main (int argc, char *argv[])
{
  orc_init();

  create_programs ();

  return 0;
}

