
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ORC_ENABLE_UNSTABLE_API

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-test/orcprofile.h>


#define ALIGN(ptr,n) ((void *)((orc_intptr)(ptr) & (~(orc_intptr)(n-1))))

int hot_src = TRUE;
int hot_dest = TRUE;
int flush_cache = FALSE;


void
touch (unsigned char *ptr, int n)
{
  static int sum;
  int i;
  for(i=0;i<n;i++){
    sum += ptr[i];
  }
}

int
main(int argc, char *argv[])
{
  char *s, *d;
  orc_uint8 *src, *dest;
  OrcProfile prof;
  OrcProfile prof_libc;
  double ave, std;
  double ave_libc, std_libc;
  double null;
  int i,j;
  double cpufreq;
  int unalign;
  OrcProgram *p;
  int level1, level2, level3;
  int max;
  //const uint8_t zero = 0;

  orc_init ();

  //cpufreq = 2333e6;
  cpufreq = 1;

  if (argc > 1) {
    unalign = strtoul (argv[1], NULL, 0);
  } else {
    unalign = 0;
  }

  s = malloc(1024*1024*64+1024);
  d = malloc(1024*1024*64+1024);
  src = ORC_PTR_OFFSET(ALIGN(s,128),unalign);
  dest = ALIGN(d,128);

  orc_profile_init (&prof);
  for(j=0;j<10;j++){
    orc_profile_start(&prof);
    orc_profile_stop(&prof);
  }
  orc_profile_get_ave_std (&prof, &null, &std);
  
  {
    OrcCompileResult result;

    p = orc_program_new ();
    orc_program_set_name (p, "orc_memcpy");
    //orc_program_set_name (p, "orc_memset");
    orc_program_add_destination (p, 1, "d1");
    orc_program_add_source (p, 1, "s1");
    //orc_program_add_parameter (p, 1, "p1");

    orc_program_append (p, "copyb", ORC_VAR_D1, ORC_VAR_S1, ORC_VAR_D1);

    result = orc_program_compile (p);

    if (ORC_COMPILE_RESULT_IS_FATAL (result)) {
      fprintf (stderr, "Failed to compile orc_memcpy\n");
      return -1;
    }
  }

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif
  orc_get_data_cache_sizes (&level1, &level2, &level3);
  if (level3 > 0) {
    max = (log(level3)/M_LN2 - 6.0) * 10 + 20;
  } else if (level2 > 0) {
    max = (log(level2)/M_LN2 - 6.0) * 10 + 20;
  } else {
    max = 140;
  }

  for(i=0;i<max;i++){
    double x = i*0.1 + 6.0;
    int size = pow(2.0, x);

    if (flush_cache) {
      touch (src, (1<<18));
    }
    if (hot_src) {
      touch (src, size);
    }
    if (hot_dest) {
      touch (dest, size);
    }

    orc_profile_init (&prof);
    for(j=0;j<10;j++){
      OrcExecutor _ex, *ex = &_ex;
      void (*func) (OrcExecutor *);

      orc_profile_start(&prof);
      //orc_memcpy (dest, src, size);
      ex->program = p;
      ex->n = size;
      ex->arrays[ORC_VAR_D1] = dest;
      ex->arrays[ORC_VAR_S1] = (void *)src;

      func = p->code_exec;
      func (ex);

      orc_profile_stop(&prof);
      if (flush_cache) {
        touch (src, (1<<18));
      }
      if (hot_src) {
        touch (src, size);
      }
      if (hot_dest) {
        touch (dest, size);
      }
    }

    orc_profile_init (&prof_libc);
    for(j=0;j<10;j++){
      orc_profile_start(&prof_libc);
      memcpy (dest, src, size);
      orc_profile_stop(&prof_libc);
      if (flush_cache) {
        touch (src, (1<<18));
      }
      if (hot_src) {
        touch (src, size);
      }
      if (hot_dest) {
        touch (dest, size);
      }
    }

    orc_profile_get_ave_std (&prof, &ave, &std);
    orc_profile_get_ave_std (&prof_libc, &ave_libc, &std_libc);

    ave -= null;
    ave_libc -= null;

    //printf("%d: %10.4g %10.4g %10.4g %10.4g (libc %10.4g)\n", i, ave, std,
    //    ave/(1<<i), cpufreq/(ave/(1<<i)),
    //    cpufreq/(ave_libc/(1<<i)));
    printf("%g %10.4g %10.4g\n", x,
        cpufreq/(ave/size), cpufreq/(ave_libc/size));
    //printf("%g %10.4g %10.4g\n", x,
    //    32*(ave/(size)), 32*(ave_libc/(size)));
    fflush (stdout);
  }

  return 0;
}

