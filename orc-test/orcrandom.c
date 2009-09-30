
#include "config.h"

#include <orc-test/orctest.h>
#include <orc-test/orcrandom.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>



void
orc_random_init (OrcRandom *context, int seed)
{

  context->x = seed;

}



void
orc_random_bits (OrcRandom *context, void *data, int n_bytes)
{
  uint8_t *d = data;
  int i;
  for(i=0;i<n_bytes;i++){
    context->x = 1103515245*context->x + 12345;
    d[i] = context->x>>16;
  }
}

void
orc_random_floats (OrcRandom *context, float *data, int n)
{
  int i;
  for(i=0;i<n;i++){
    context->x = 1103515245*context->x + 12345;
    data[i] = (double)(context->x>>16) / 32768.0 - 1.0;
  }
}

unsigned int
orc_random (OrcRandom *context)
{
  context->x = 1103515245*context->x + 12345;
  return context->x;
}
