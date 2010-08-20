#include <stdio.h>
#include "example2orc.h"

#define N 10

short a[N*2];
short b[N];
short c[N*2];

int
main (int argc, char *argv[])
{
  int i;
  double volume = 0.5;

  /* Create some data in the source arrays */
  for(i=0;i<N;i++){
    a[2*i] = 10*i;
    a[2*i+1] = 100*i;
    b[i] = (i&1) ? 10000 : -10000;
  }

  /* Call a function that uses Orc */
  audio_add_mono_to_stereo_scaled_s16 (c, a, b, volume*4096, N);

  /* Print the results */
  for(i=0;i<N;i++){
    printf("%d: %d,%d %d -> %d,%d\n", i, a[2*i], a[2*i+1], b[i],
        c[2*i], c[2*i+1]);
  }

  return 0;
}

