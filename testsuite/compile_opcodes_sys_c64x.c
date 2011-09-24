
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


int error = FALSE;

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
    printf("/* %s %d,%d,%d */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode (opcode_set->opcodes + i);
    exit (0);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s const %d,%d,%d */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_const (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s param %d,%d,%d */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_param (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  OrcTestResult ret;

  p = orc_test_get_program_for_opcode (opcode);
  if (!p) return;

  ret = orc_test_gcc_compile_c64x (p);
  if (ret == ORC_TEST_FAILED) {
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
  OrcTestResult ret;

  p = orc_test_get_program_for_opcode_const (opcode);
  if (!p) return;

  ret = orc_test_gcc_compile_c64x (p);
  if (ret == ORC_TEST_FAILED) {
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
  OrcTestResult ret;

  p = orc_test_get_program_for_opcode_param (opcode);
  if (!p) return;

  ret = orc_test_gcc_compile_c64x (p);
  if (ret == ORC_TEST_FAILED) {
    printf("%s", orc_program_get_asm_code (p));
    error = TRUE;
    return;
  }

  orc_program_free (p);
}

