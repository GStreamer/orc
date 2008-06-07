
#include <stdlib.h>
#include <stdint.h>
#include <orc/orcprogram.h>

void
test (OrcExecutor *ex)
{
  int i;
  int n = ex->n;
  int16_t *var0 = ex->arrays[0];
  int16_t *var1 = ex->arrays[1];
  int16_t *var2 = ex->arrays[2];
  int16_t var3;
  int16_t var6;

  for (i = 0; i < n; i++) {
    var0[i] = (var1[i] + var2[i] + 1) >> 1;
  }
}

