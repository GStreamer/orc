
#ifndef _ORC_RANDOM_H_
#define _ORC_RANDOM_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

typedef struct _OrcRandom OrcRandom;
struct _OrcRandom {
  unsigned int x;
};

void orc_random_init (OrcRandom *context, int seed);
void orc_random_bits (OrcRandom *context, void *data, int n_bytes);
unsigned int orc_random (OrcRandom *context);

ORC_END_DECLS

#endif

