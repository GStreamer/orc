
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>
#include <orc/orcdebug.h>


int error = FALSE;

void test_opcode (OrcStaticOpcode *opcode);

orc_uint8 array1[100];
orc_uint8 array2[100];

int orc_sad_u8 (orc_uint8 *s1, orc_uint8 *s2, int n);

int
main (int argc, char *argv[])
{
  int i;
  int n;
  int sum;

  orc_init();

  for(n=0;n<20;n++){
    sum = 0;
    for(i=0;i<n;i++){
      array1[i] = rand();
      array2[i] = rand();
      sum += abs(array1[i] - array2[i]);
    }
    if (sum != orc_sad_u8 (array1, array2, n)) {
      for(i=0;i<n;i++){
        printf("%d: %d %d -> %d\n", i, array1[i], array2[i],
            abs(array1[i] - array2[i]));
      }

      printf("sum %d %d\n", sum, orc_sad_u8 (array1, array2, n));
      error = TRUE;
    }
  }

  if (error) return 1;
  return 0;
}


int
orc_sad_u8 (orc_uint8 *s1, orc_uint8 *s2, int n)
{
  static OrcProgram *p = NULL;
  OrcExecutor *ex;
  int sum;
  OrcCompileResult result;

  if (p == NULL) {
    p = orc_program_new ();
    orc_program_add_accumulator (p, 4, "a1");
    orc_program_add_source (p, 1, "s1");
    orc_program_add_source (p, 1, "s2");

    orc_program_append_str (p, "accsadubl", "a1", "s1", "s2");

    result = orc_program_compile (p);
    if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
      return 0;
    }

    //printf("%s\n", orc_program_get_asm_code (p));
  }

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, n);
  orc_executor_set_array_str (ex, "s1", s1);
  orc_executor_set_array_str (ex, "s2", s2);

  orc_executor_run (ex);

  //sum = orc_executor_get_accumulator (ex, "a1");
  sum = ex->accumulators[0];

  orc_executor_free (ex);

  return sum;
}


