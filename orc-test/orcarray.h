
#ifndef _ORC_ARRAY_H_
#define _ORC_ARRAY_H_

#include <orc-test/orctest.h>
#include <orc-test/orcrandom.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>

typedef struct _OrcArray OrcArray;
struct _OrcArray {
  void *data;
  int stride;
  int element_size;
  int n,m;

  void *alloc_data;
  int alloc_len;
};

OrcArray *orc_array_new (int n, int m, int element_size);
void orc_array_free (OrcArray *array);

void orc_array_set_pattern (OrcArray *array, int value);
void orc_array_set_random (OrcArray *array, OrcRandomContext *context);

int orc_array_compare (OrcArray *array1, OrcArray *array2, int flags);
int orc_array_check_out_of_bounds (OrcArray *array);

#endif

