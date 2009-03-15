
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>


int error = FALSE;

void test_opcode (const char *name);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcode *opcode_list;
  int n_opcodes;

  orc_init();

  n_opcodes = orc_opcode_get_list (&opcode_list);

  //printf("%s", orc_target_get_asm_preamble (ORC_TARGET_C));

  for(i=0;i<n_opcodes;i++){
    printf("/* %s %d %d %p */\n",
        opcode_list[i].name,
        opcode_list[i].n_src,
        opcode_list[i].n_dest,
        opcode_list[i].emulate
        );
    test_opcode (opcode_list[i].name);
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
  }

  printf("%s", orc_program_get_asm_code (p));

  orc_program_free (p);
}


