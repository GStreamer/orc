
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


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
  orc_test_init();

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("/* %s %d,%d,%d */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
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
  const char *s;

  p = orc_test_get_program_for_opcode (opcode);
  if (!p) return;

  result = orc_program_compile_for_target (p, orc_target_get_by_name("c"));
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    s = orc_program_get_asm_code (p);
    if (s != NULL) {
      printf("%s\n", s);
    } else {
      printf("no code\n");
    }
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
  const char *s;

  p = orc_test_get_program_for_opcode_const (opcode);
  if (!p) return;

  result = orc_program_compile_for_target (p, orc_target_get_by_name("c"));
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    s = orc_program_get_asm_code (p);
    if (s != NULL) {
      printf("%s\n", s);
    } else {
      printf("no code\n");
    }
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
  const char *s;

  p = orc_test_get_program_for_opcode_param (opcode);
  if (!p) return;

  result = orc_program_compile_for_target (p, orc_target_get_by_name("c"));
  if (!ORC_COMPILE_RESULT_IS_SUCCESSFUL(result)) {
    s = orc_program_get_asm_code (p);
    if (s != NULL) {
      printf("%s\n", s);
    } else {
      printf("no code\n");
    }
    error = TRUE;
    return;
  }

  orc_program_free (p);
}




