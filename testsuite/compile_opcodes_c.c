
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


int error = FALSE;

void test_opcode (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_init();
  orc_test_init();

  opcode_set = orc_opcode_set_get ("sys");

  printf("%s", orc_target_get_asm_preamble ("c"));

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s %d,%d,%d %p */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1],
        opcode_set->opcodes[i].emulate);
    test_opcode (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  OrcCompileResult result;

  p = orc_test_get_program_for_opcode (opcode);
  if (!p) return;

  result = orc_program_compile_for_target (p, orc_target_get_by_name("c"));
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    error = TRUE;
    return;
  }

  printf("%s", orc_program_get_asm_code (p));

  orc_program_free (p);
}


