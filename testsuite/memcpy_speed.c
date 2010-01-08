
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-test/orcprofile.h>

#define ALIGN(ptr,n) ((void *)((unsigned long)(ptr) & (~(unsigned long)(n-1))))

int
main(int argc, char *argv[])
{
  char *s, *d;
  uint32_t *src, *dest;
  OrcProfile prof;
  double ave, std;
  int i,j;
  double cpufreq;

  orc_init ();

  cpufreq = 1000e6;

  s = malloc(1024*1024*64+1024);
  d = malloc(1024*1024*64+1024);
  src = ALIGN(s,128);
  dest = ALIGN(d,128);

  printf("orc_memset:\n");
  for(i=10;i<26;i++){
    orc_profile_init (&prof);
    for(j=0;j<10;j++){
      orc_profile_start(&prof);
      orc_memset (dest, 0, 1<<i);
      orc_profile_stop(&prof);
    }

    orc_profile_get_ave_std (&prof, &ave, &std);

    printf("%d: %10.4g %10.4g %10.4g %10.4g\n", i, ave, std,
        ave/(1<<i), cpufreq/(ave/(1<<i)));
  }

  printf("orc_memcpy:\n");
  for(i=10;i<26;i++){
    orc_profile_init (&prof);
    for(j=0;j<10;j++){
      orc_profile_start(&prof);
      orc_memcpy (dest, src, 1<<i);
      orc_profile_stop(&prof);
    }

    orc_profile_get_ave_std (&prof, &ave, &std);

    printf("%d: %10.4g %10.4g %10.4g %10.4g\n", i, ave, std,
        ave/(1<<i), cpufreq/(ave/(1<<i)));
  }

  return 0;
}

