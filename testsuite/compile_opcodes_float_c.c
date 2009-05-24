
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-float/orcfloat.h>


int error = FALSE;
int verbose = FALSE;

void test_opcode (OrcStaticOpcode *opcode);
void test_opcode_const (OrcStaticOpcode *opcode);
void test_opcode_param (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_init();
  orc_float_init();
  orc_test_init();

  opcode_set = orc_opcode_set_get ("float");

  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("/* %s %d,%d,%d %p */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1],
        opcode_set->opcodes[i].emulate);
    test_opcode (opcode_set->opcodes + i);
  }

  if (error) {
    printf("test failed\n");
    return 1;
  } else {
    printf("test passed\n");
    return 0;
  }
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
    printf("%s", orc_program_get_asm_code (p));
    error = TRUE;
    return;
  }

  orc_program_free (p);
}

void
test_opcode_const (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  OrcCompileResult result;

  p = orc_test_get_program_for_opcode_const (opcode);
  if (!p) return;

  result = orc_program_compile_for_target (p, orc_target_get_by_name("c"));
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    printf("%s", orc_program_get_asm_code (p));
    error = TRUE;
    return;
  }

  orc_program_free (p);
}

void
test_opcode_param (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  OrcCompileResult result;

  p = orc_test_get_program_for_opcode_param (opcode);
  if (!p) return;

  result = orc_program_compile_for_target (p, orc_target_get_by_name("c"));
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    printf("%s", orc_program_get_asm_code (p));
    error = TRUE;
    return;
  }

  orc_program_free (p);
}




