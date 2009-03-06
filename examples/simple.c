
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>

#define N 100

int16_t src1[N+4];
int16_t src2[N];
int16_t dest_test[N];
int16_t dest_ref[N];
int16_t dest[N];

uint8_t bsrc1[N+4];
uint8_t bsrc2[N];
uint8_t bdest_test[N];
uint8_t bdest_ref[N];
uint8_t bdest[N];

void test1(void);
void test2(void);
void test3(void);
void test4(void);
void test5(void);
void test6(void);
void test7(void);
void test8(void);

int
main (int argc, char *argv[])
{
  orc_init ();

  test8();

  exit(0);
}

void
test1(void)
{
  OrcProgram *p;
  OrcExecutor *ex;

  p = orc_program_new_dss (2, 2, 2);

  orc_program_append_str (p, "addw", "d1", "s1", "s2");

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N - 4);
  orc_executor_set_array_str (ex, "s1", src1);
  orc_executor_set_array_str (ex, "s2", src2);
  orc_executor_set_array_str (ex, "d1", dest);

  if (1) {
    int i;

    for(i=0;i<N;i++){
      src1[i] = rand()&0xf;
      src2[i] = rand()&0xf;
    }

    //orc_executor_run (ex);
    //orc_executor_emulate (ex);

    for(i=0;i<N;i++){
      printf("#  %4d %4d %4d %4d\n", src1[i], src2[i], dest[i],
          src1[i] + src2[i]);
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}


void
test2(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int s1, s2, s3, s4, d1;
  int t1, t2;
  int c1, c2, c3;
  int n;

  p = orc_program_new ();

  d1 = orc_program_add_destination (p, 2, "d1");
  //p->vars[d1].is_uncached = TRUE;
  s1 = orc_program_add_source (p, 2, "s1");
  s2 = orc_program_add_source (p, 2, "s2");
  s3 = orc_program_add_source (p, 2, "s3");
  s4 = orc_program_add_source (p, 2, "s4");
  c1 = orc_program_add_constant (p, 2, 3, "c1");
  c2 = orc_program_add_constant (p, 2, 4, "c2");
  c3 = orc_program_add_constant (p, 2, 3, "c3");
  t1 = orc_program_add_temporary (p, 2, "t1");
  t2 = orc_program_add_temporary (p, 2, "t2");

  orc_program_append (p, "addw", t1, s2, s3);
  orc_program_append (p, "addw", t2, s1, s4);
  orc_program_append (p, "mullw", t1, t1, c1);
  orc_program_append (p, "subw", t1, t1, t2);
  orc_program_append (p, "addw", t1, t1, c2);
  orc_program_append (p, "shrsw", d1, t1, c3);

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N);
  orc_executor_set_array (ex, s1, src1);
  orc_executor_set_array (ex, s2, src1 + 1);
  orc_executor_set_array (ex, s3, src1 + 2);
  orc_executor_set_array (ex, s4, src1 + 3);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n+3;i++){
      src1[i] = rand()&0xff;
    }
    for(i=0;i<n+4;i++){
      dest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array (ex, d1, dest_ref);
    orc_executor_emulate (ex);
#if 1
    for(i=0;i<n;i++){
      dest_ref[i] = (3*(src1[i+1]+src1[i+2])-(src1[i]+src1[i+3])+4)>>3;
    }
#endif
    
    orc_executor_set_array (ex, d1, dest_test + 1);
    orc_executor_run (ex);

    printf("# %d: %p %d %d %d\n",
        n, ex->arrays[0], ex->counter1, ex->counter2, ex->counter3);

    for(i=0;i<n+4;i++){
      printf("# %d: %4d %4d %4d %c\n", i, src1[i], dest_ref[i], dest_test[i+1],
          (dest_ref[i] == dest_test[i+1])?' ':'*');
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}


void
test3(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int s1, s2, d1;
  int t1, t2;
  int c1, c2;
  int n;

  p = orc_program_new ();

  d1 = orc_program_add_destination (p, 2, "d1");
  s1 = orc_program_add_source (p, 2, "s1");
  s2 = orc_program_add_source (p, 2, "s2");
  c1 = orc_program_add_constant (p, 2, -1, "c1");
  c2 = orc_program_add_constant (p, 2, 1, "c2");
  t1 = orc_program_add_temporary (p, 2, "t1");
  t2 = orc_program_add_temporary (p, 2, "t2");

  orc_program_append (p, "addw", t1, s1, s2);
  orc_program_append (p, "addw", t2, t1, c1);
  orc_program_append (p, "shrsw", d1, t2, c2);

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N);
  orc_executor_set_array (ex, s1, src1);
  orc_executor_set_array (ex, s2, src1 + 1);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n+1;i++){
      src1[i] = rand()&0xff;
    }
    for(i=0;i<n+4;i++){
      dest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array (ex, d1, dest_ref);
    orc_executor_emulate (ex);
#if 1
    for(i=0;i<n;i++){
      dest_ref[i] = (src1[i+1]+src1[i]-1)>>1;
    }
#endif
    
    orc_executor_set_array (ex, d1, dest_test);
    orc_executor_run (ex);

    for(i=0;i<n+4;i++){
      printf("# %d: %4d %4d %4d %c\n", i, src1[i], dest_ref[i], dest_test[i],
          (dest_ref[i] == dest_test[i])?' ':'*');
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}



void
test4(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int n;

  p = orc_program_new_dss (1, 1, 1);

  orc_program_append_str (p, "addb", "d1", "s1", "s2");

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N - 4);
  orc_executor_set_array_str (ex, "s1", bsrc1);
  orc_executor_set_array_str (ex, "s2", bsrc2);
  orc_executor_set_array_str (ex, "d1", bdest);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n;i++){
      bsrc1[i] = rand()&0xf;
      bsrc2[i] = rand()&0xf;
    }
    for(i=0;i<n+4;i++){
      bdest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array_str (ex, "d1", bdest_ref);
    orc_executor_emulate (ex);

    orc_executor_set_array_str (ex, "d1", bdest_test);
    orc_executor_run (ex);

    for(i=0;i<n+4;i++){
      printf("#  %4d %4d %4d %4d %c\n", bsrc1[i], bsrc2[i], bdest_ref[i],
          bdest_test[i], 
          (bdest_ref[i] == bdest_test[i])?' ':'*');
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}


void
test5(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int s1, s2, d1;
  int t1;
  int n;

  p = orc_program_new ();

  d1 = orc_program_add_destination (p, 2, "d1");
  s1 = orc_program_add_source (p, 2, "s1");
  s2 = orc_program_add_source (p, 2, "s2");
  t1 = orc_program_add_temporary (p, 2, "t1");

  orc_program_append (p, "absw", t1, s1, s2);
  orc_program_append (p, "addw", t1, s1, s2);
  orc_program_append (p, "addssw", t1, s1, s2);
  orc_program_append (p, "addusw", t1, s1, s2);
  orc_program_append (p, "andw", t1, s1, s2);
  orc_program_append (p, "andnw", t1, s1, s2);
  //orc_program_append (p, "avgsw", t1, s1, s2);
  orc_program_append (p, "avguw", t1, s1, s2);
  orc_program_append (p, "cmpeqw", t1, s1, s2);
  orc_program_append (p, "cmpgtsw", t1, s1, s2);
  orc_program_append (p, "maxsw", t1, s1, s2);
    //orc_program_append (p, "maxuw", t1, s1, s2);
  orc_program_append (p, "minsw", t1, s1, s2);
    //orc_program_append (p, "minuw", t1, s1, s2);
  orc_program_append (p, "mullw", t1, s1, s2);
  orc_program_append (p, "mulhsw", t1, s1, s2);
  orc_program_append (p, "mulhuw", t1, s1, s2);
  orc_program_append (p, "orw", t1, s1, s2);
  orc_program_append (p, "signw", t1, s1, s2);
  orc_program_append (p, "subw", t1, s1, s2);
  orc_program_append (p, "subssw", t1, s1, s2);
  orc_program_append (p, "subusw", t1, s1, s2);
  orc_program_append (p, "xorw", t1, s1, s2);

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N);
  orc_executor_set_array (ex, s1, src1);
  orc_executor_set_array (ex, s2, src1 + 1);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n+1;i++){
      src1[i] = rand()&0xff;
    }
    for(i=0;i<n+4;i++){
      dest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array (ex, d1, dest_ref);
    orc_executor_emulate (ex);
#if 1
    for(i=0;i<n;i++){
      dest_ref[i] = (src1[i+1]+src1[i]-1)>>1;
    }
#endif
    
    orc_executor_set_array (ex, d1, dest_test);
    //orc_executor_run (ex);

    for(i=0;i<n+4;i++){
      printf("# %d: %4d %4d %4d %c\n", i, src1[i], dest_ref[i], dest_test[i],
          (dest_ref[i] == dest_test[i])?' ':'*');
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}


void
test6(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int s1, s2, d1;
  int t1;
  int n;

  p = orc_program_new ();

  d1 = orc_program_add_destination (p, 1, "d1");
  s1 = orc_program_add_source (p, 1, "s1");
  s2 = orc_program_add_source (p, 1, "s2");
  t1 = orc_program_add_temporary (p, 1, "t1");

  orc_program_append (p, "absb", t1, s1, s2);
  orc_program_append (p, "addb", t1, s1, s2);
  orc_program_append (p, "addssb", t1, s1, s2);
  orc_program_append (p, "addusb", t1, s1, s2);
  orc_program_append (p, "andb", t1, s1, s2);
  orc_program_append (p, "andnb", t1, s1, s2);
  //orc_program_append (p, "avgsb", t1, s1, s2);
  orc_program_append (p, "avgub", t1, s1, s2);
  orc_program_append (p, "cmpeqb", t1, s1, s2);
  orc_program_append (p, "cmpgtsb", t1, s1, s2);
    //orc_program_append (p, "maxsb", t1, s1, s2);
  orc_program_append (p, "maxub", t1, s1, s2);
    //orc_program_append (p, "minsb", t1, s1, s2);
  orc_program_append (p, "minub", t1, s1, s2);
  //orc_program_append (p, "mullb", t1, s1, s2);
  //orc_program_append (p, "mulhsb", t1, s1, s2);
  //orc_program_append (p, "mulhub", t1, s1, s2);
  orc_program_append (p, "orb", t1, s1, s2);
  orc_program_append (p, "signb", t1, s1, s2);
  orc_program_append (p, "subb", t1, s1, s2);
  orc_program_append (p, "subssb", t1, s1, s2);
  orc_program_append (p, "subusb", t1, s1, s2);
  orc_program_append (p, "xorb", t1, s1, s2);

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N);
  orc_executor_set_array (ex, s1, src1);
  orc_executor_set_array (ex, s2, src1 + 1);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n+1;i++){
      src1[i] = rand()&0xff;
    }
    for(i=0;i<n+4;i++){
      dest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array (ex, d1, dest_ref);
    orc_executor_emulate (ex);
#if 1
    for(i=0;i<n;i++){
      dest_ref[i] = (src1[i+1]+src1[i]-1)>>1;
    }
#endif
    
    orc_executor_set_array (ex, d1, dest_test);
    //orc_executor_run (ex);

    for(i=0;i<n+4;i++){
      printf("# %d: %4d %4d %4d %c\n", i, src1[i], dest_ref[i], dest_test[i],
          (dest_ref[i] == dest_test[i])?' ':'*');
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}


void
test7(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int s1, s2, d1;
  int t1;
  int n;

  p = orc_program_new ();

  d1 = orc_program_add_destination (p, 1, "d1");
  s1 = orc_program_add_source (p, 1, "s1");
  s2 = orc_program_add_source (p, 1, "s2");
  t1 = orc_program_add_temporary (p, 1, "t1");

  orc_program_append (p, "absl", t1, s1, s2);
  orc_program_append (p, "addl", t1, s1, s2);
  //orc_program_append (p, "addssl", t1, s1, s2);
  //orc_program_append (p, "addusl", t1, s1, s2);
  orc_program_append (p, "andl", t1, s1, s2);
  orc_program_append (p, "andnl", t1, s1, s2);
  //orc_program_append (p, "avgsl", t1, s1, s2);
  //orc_program_append (p, "avgul", t1, s1, s2);
  orc_program_append (p, "cmpeql", t1, s1, s2);
  orc_program_append (p, "cmpgtsl", t1, s1, s2);
    //orc_program_append (p, "maxsl", t1, s1, s2);
    //orc_program_append (p, "maxul", t1, s1, s2);
    //orc_program_append (p, "minsl", t1, s1, s2);
    //orc_program_append (p, "minul", t1, s1, s2);
    //orc_program_append (p, "mulll", t1, s1, s2);
  //orc_program_append (p, "mulhsl", t1, s1, s2);
  //orc_program_append (p, "mulhul", t1, s1, s2);
  orc_program_append (p, "orl", t1, s1, s2);
  orc_program_append (p, "signl", t1, s1, s2);
  orc_program_append (p, "subl", t1, s1, s2);
  //orc_program_append (p, "subssl", t1, s1, s2);
  //orc_program_append (p, "subusl", t1, s1, s2);
  orc_program_append (p, "xorl", t1, s1, s2);

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N);
  orc_executor_set_array (ex, s1, src1);
  orc_executor_set_array (ex, s2, src1 + 1);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n+1;i++){
      src1[i] = rand()&0xff;
    }
    for(i=0;i<n+4;i++){
      dest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array (ex, d1, dest_ref);
    orc_executor_emulate (ex);
#if 1
    for(i=0;i<n;i++){
      dest_ref[i] = (src1[i+1]+src1[i]-1)>>1;
    }
#endif
    
    orc_executor_set_array (ex, d1, dest_test);
    //orc_executor_run (ex);

    for(i=0;i<n+4;i++){
      printf("# %d: %4d %4d %4d %c\n", i, src1[i], dest_ref[i], dest_test[i],
          (dest_ref[i] == dest_test[i])?' ':'*');
    }
  }

  orc_executor_free (ex);
  orc_program_free (p);
}



void
test8(void)
{
  OrcProgram *p;
  OrcExecutor *ex;
  int s1, d1;
  int n;

  p = orc_program_new ();

  d1 = orc_program_add_destination (p, 2, "d1");
  s1 = orc_program_add_source (p, 1, "s1");

  orc_program_append_ds (p, "convubw", d1, s1);

  orc_program_compile (p);

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, N);
  orc_executor_set_array (ex, s1, bsrc1);

  for(n=0;n<20;n++) {
    int i;

    for(i=0;i<n;i++){
      bsrc1[i] = rand()&0xff;
    }
    for(i=0;i<n+4;i++){
      dest[i] = 0;
    }

    orc_executor_set_n (ex, n);
    orc_executor_set_array (ex, d1, dest_ref);
    orc_executor_emulate (ex);
#if 0
    for(i=0;i<n;i++){
      dest_ref[i] = bsrc1[i];
    }
#endif
    
    orc_executor_set_array (ex, d1, dest_test);
    orc_executor_run (ex);

#if 1
    for(i=0;i<n+4;i++){
      printf("# %d: %4d %4d %4d %c\n", i, bsrc1[i], dest_ref[i], dest_test[i],
          (dest_ref[i] == dest_test[i])?' ':'*');
    }
#endif
  }

  orc_executor_free (ex);
  orc_program_free (p);
}


