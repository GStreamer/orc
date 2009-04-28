
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc-float/orcfloat.h>


int error = FALSE;

void test_opcode_src (OrcStaticOpcode *opcode);
void test_opcode_const (OrcStaticOpcode *opcode);
void test_opcode_param (OrcStaticOpcode *opcode);
void test_opcode_inplace (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_float_init();
  orc_test_init();
  orc_init();

  opcode_set = orc_opcode_set_get ("float");

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s src %d,%d,%d */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_src (opcode_set->opcodes + i);
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
  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s inplace %d,%d,%d */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_inplace (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode_src (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;

  if (opcode->src_size[1] == 0) {
    p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  } else {
    p = orc_program_new_dss (opcode->dest_size[0], opcode->src_size[0],
        opcode->src_size[1]);
  }

  sprintf(s, "test_s_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, opcode->name, "d1", "s1", "s2");

  ret = orc_test_compare_output (p);
  if (!ret) {
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_const (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;

  if (opcode->src_size[1] == 0) {
    return;
  }
  p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  orc_program_add_constant (p, opcode->src_size[1], 1, "c1");

  sprintf(s, "test_c_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, opcode->name, "d1", "s1", "c1");

  ret = orc_test_compare_output (p);
  if (!ret) {
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_param (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;

  if (opcode->src_size[1] == 0) {
    return;
  }
  p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  orc_program_add_parameter (p, opcode->src_size[1], "p1");

  sprintf(s, "test_c_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, opcode->name, "d1", "s1", "p1");

  ret = orc_test_compare_output (p);
  if (!ret) {
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_inplace (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;

  if (opcode->dest_size[0] != opcode->src_size[0]) return;

  if (opcode->src_size[1] == 0) {
    p = orc_program_new_ds (opcode->dest_size[0], opcode->src_size[0]);
  } else {
    p = orc_program_new_dss (opcode->dest_size[0], opcode->src_size[0],
        opcode->src_size[1]);
  }

  sprintf(s, "test_inplace_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, opcode->name, "d1", "d1", "s2");

  ret = orc_test_compare_output (p);
  if (!ret) {
    error = TRUE;
  }

  orc_program_free (p);
}

