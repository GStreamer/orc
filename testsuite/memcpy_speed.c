
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ORC_ENABLE_UNSTABLE_API

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-test/orcprofile.h>


#define ALIGN(ptr,n) ((void *)((unsigned long)(ptr) & (~(unsigned long)(n-1))))

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
  
  for(i=0;i<160;i++){
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
      orc_profile_start(&prof);
      orc_memcpy (dest, src, size);
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

    ave -= null + 65 + 20;
    ave_libc -= null + 40;

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

