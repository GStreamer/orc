
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-test/orcprofile.h>


#define ALIGN(ptr,n) ((void *)((unsigned long)(ptr) & (~(unsigned long)(n-1))))

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
  //const uint8_t zero = 0;

  orc_init ();

  //cpufreq = 2333e6;
  cpufreq = 1;

  s = malloc(1024*1024*64+1024);
  d = malloc(1024*1024*64+1024);
  src = ALIGN(s,128);
  dest = ALIGN(d,128);

  orc_profile_init (&prof);
  for(j=0;j<10;j++){
    orc_profile_start(&prof);
    orc_profile_stop(&prof);
  }
  orc_profile_get_ave_std (&prof, &null, &std);

#if 0
 // printf("orc_memset:\n");
  for(i=10;i<26;i++){
    orc_profile_init (&prof);
    orc_profile_init (&prof_libc);
    for(j=0;j<10;j++){
      orc_profile_start(&prof);
      orc_memset (dest, 0, 1<<i);
      orc_profile_stop(&prof);
      orc_profile_start(&prof_libc);
      memset (dest, 0, 1<<i);
      orc_profile_stop(&prof_libc);
    }

    orc_profile_get_ave_std (&prof, &ave, &std);
    orc_profile_get_ave_std (&prof_libc, &ave_libc, &std_libc);

#if 0
    printf("%d: %10.4g %10.4g %10.4g %10.4g (libc %10.4g)\n", i, ave, std,
        ave/(1<<i), cpufreq/(ave/(1<<i)),
        cpufreq/(ave_libc/(1<<i)));
    printf("%d %10.4g %10.4g %10.4g\n", i,
        cpufreq/(ave/(1<<i)), cpufreq/(ave_libc/(1<<i)));
#endif
  }
#endif

  //printf("orc_memcpy:\n");
  //for(i=10;i<26;i++){
  for(i=0;i<200;i++){
    double x = i*0.1 + 6.0;
    int size = pow(2.0, x);

    orc_profile_init (&prof);
    for(j=0;j<10;j++){
      orc_profile_start(&prof);
      orc_memcpy (dest, src, size);
      orc_profile_stop(&prof);
    }

    orc_profile_init (&prof_libc);
    for(j=0;j<10;j++){
      orc_profile_start(&prof_libc);
      memcpy (dest, src, size);
      orc_profile_stop(&prof_libc);
    }

    orc_profile_get_ave_std (&prof, &ave, &std);
    orc_profile_get_ave_std (&prof_libc, &ave_libc, &std_libc);

    ave -= null + 60;
    ave_libc -= null + 35;

    //printf("%d: %10.4g %10.4g %10.4g %10.4g (libc %10.4g)\n", i, ave, std,
    //    ave/(1<<i), cpufreq/(ave/(1<<i)),
    //    cpufreq/(ave_libc/(1<<i)));
    printf("%g %10.4g %10.4g\n", x,
        cpufreq/(ave/(size)), cpufreq/(ave_libc/(size)));
    //printf("%g %10.4g %10.4g\n", x,
    //    32*(ave/(size)), 32*(ave_libc/(size)));
    fflush (stdout);
  }

  return 0;
}

