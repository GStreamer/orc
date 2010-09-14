
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


int error = FALSE;

void test_opcode_src (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_test_init();
  orc_init();

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("opcode_%-20s ", opcode_set->opcodes[i].name);
    test_opcode_src (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode_src (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  int flags = 0;

  p = orc_test_get_program_for_opcode (opcode);

  printf("%g\n", orc_test_performance_full (p, flags, NULL));

  orc_program_free (p);
}

