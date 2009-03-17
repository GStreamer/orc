
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>


int error = FALSE;

void test_opcode (const char *name);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_init();

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("/* %s %d,%d,%d %p */\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1],
        opcode_set->opcodes[i].emulate);
    test_opcode (opcode_set->opcodes[i].name);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode (const char *name)
{
  OrcProgram *p;
  char s[40];
  int ret;

  p = orc_program_new_dss (2,2,2);

  sprintf(s, "test_%s", name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, name, "d1", "s1", "s2");

  ret = orc_program_compile (p);
  if (!ret) {
    error = TRUE;
    goto out;
  }

  printf("%s", orc_program_get_asm_code (p));

out:
  orc_program_free (p);
}


