
#include "config.h"

#include <glib.h>
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

void test1(void);
void test2(void);
void test3(void);
void test4(void);

int
main (int argc, char *argv[])
{
  orc_init ();

  test4();

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

    orc_executor_run (ex);
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


uint8_t bsrc1[N+4];
uint8_t bsrc2[N];
uint8_t bdest_test[N];
uint8_t bdest_ref[N];
uint8_t bdest[N];

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

