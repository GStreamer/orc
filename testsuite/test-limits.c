
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <orc-test/orctest.h>


static int error = FALSE;
static const char *names = "0123456789abcdefX";

static void
test_simple (int max, int (*adder) (OrcProgram *, int, const char *))
{
  OrcProgram *p;
  int v;
  OrcCompileResult result;

  p = orc_program_new ();

  /* dummy program so compile doesn't barf */
  orc_program_add_destination (p, 2, "d1");
  orc_program_add_source (p, 2, "s1");
  orc_program_append_str (p, "addw", "d1", "d1", "s1");

  /* we've alreay added one of those */
  if (adder == orc_program_add_destination || adder == orc_program_add_source)
    max--;

  /* Check we can add up to the claimed max */
  for (v = 0; v < max; v++)
    (*adder) (p, 2, names + v);
  result = orc_program_compile (p);
  if (ORC_COMPILE_RESULT_IS_FATAL (result))
    error = TRUE;

  orc_program_reset (p);

  /* Check we can not add one more */
  (*adder) (p, 2, names + v);
  result = orc_program_compile (p);
  if (ORC_COMPILE_RESULT_IS_SUCCESSFUL (result))
    error = TRUE;

  orc_program_free (p);
}

static int
add_constant (OrcProgram *program, int size, const char *name)
{
  return orc_program_add_constant (program, size, 0, name);
}

int
main (int argc, char *argv[])
{
  orc_init();
  orc_test_init();

  test_simple (ORC_MAX_DEST_VARS, orc_program_add_destination);
  test_simple (ORC_MAX_SRC_VARS, orc_program_add_source);
  test_simple (ORC_MAX_TEMP_VARS, orc_program_add_temporary);
  test_simple (ORC_MAX_CONST_VARS, add_constant);
  test_simple (ORC_MAX_PARAM_VARS, orc_program_add_parameter);
  test_simple (ORC_MAX_ACCUM_VARS, orc_program_add_accumulator);

  if (error) return 1;
  return 0;
}

