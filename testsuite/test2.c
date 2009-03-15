
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

  for(i=0;i<n_opcodes;i++){
    printf("%s %d %d %p\n",
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

uint8_t d1[100];
uint8_t s1[100];
uint8_t s2[100];

void
test_opcode (const char *name)
{
  OrcProgram *p;
  char s[40];
  int ret;
  OrcExecutor *ex;

  p = orc_program_new_dss (2,2,2);

  sprintf(s, "test_%s", name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, name, "d1", "s1", "s2");

  ret = orc_program_compile_for_target (p, ORC_TARGET_C);
  if (!ret) {
    error = TRUE;
  }

  ex = orc_executor_new (p);
  orc_executor_set_n (ex, 10);
  orc_executor_set_array_str (ex, "d1", d1);
  orc_executor_set_array_str (ex, "s1", s1);
  orc_executor_set_array_str (ex, "s2", s2);

  orc_executor_emulate (ex);

  orc_executor_free (ex);

  orc_program_free (p);
}


