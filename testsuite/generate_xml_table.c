
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


int error = FALSE;

char * test_opcode (OrcStaticOpcode *opcode, OrcTarget *target,
    unsigned int target_flags);

int
main (int argc, char *argv[])
{
  int i;
  int j;
  OrcOpcodeSet *opcode_set;
  OrcTarget *targets[10];
  unsigned int target_flags[10];
  int n_targets;

  orc_init();
  orc_test_init();

  targets[0] = orc_target_get_by_name("sse");
  target_flags[0] = orc_target_get_default_flags(targets[0]);

  targets[1] = orc_target_get_by_name("mmx");
  target_flags[1] = orc_target_get_default_flags(targets[1]);

  targets[2] = orc_target_get_by_name("altivec");
  target_flags[2] = orc_target_get_default_flags(targets[2]);

  targets[3] = orc_target_get_by_name("arm");
  target_flags[3] = orc_target_get_default_flags(targets[3]);

  targets[4] = orc_target_get_by_name("neon");
  target_flags[4] = orc_target_get_default_flags(targets[4]);

  targets[5] = orc_target_get_by_name("c64x-c");
  target_flags[5] = orc_target_get_default_flags(targets[5]);

  n_targets=6;
  
  printf(
"<table frame=\"all\" id=\"table-basictypes\" xreflabel=\"Table of Opcode Rule Coverage\">\n"
"<title>Table of Opcode Rule Coverage</title>\n"
"<tgroup cols=\"%d\" align=\"left\" colsep=\"1\" rowsep=\"1\">\n"
"<thead>\n"
"<row>\n"
"<entry>opcode name</entry>\n", n_targets+1);
    for(j=0;j<n_targets;j++){
      printf("<entry>%s</entry>\n",
        orc_target_get_name(targets[j]));
    }
  printf(
"</row>\n"
"</thead>\n"
"<tbody valign=\"top\">\n");

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("<row>\n");
    printf("<entry>%s</entry>\n", opcode_set->opcodes[i].name);
    for(j=0;j<n_targets;j++){
      printf("<entry>%s</entry>\n",
        test_opcode (opcode_set->opcodes + i, targets[j], target_flags[j]));
    }
    printf("</row>\n");
  }
  printf(
"</tbody>\n"
"</tgroup>\n"
"</table>\n");

  return 0;
}

char *
test_opcode (OrcStaticOpcode *opcode, OrcTarget *target,
    unsigned int target_flags)
{
  OrcProgram *p;
  OrcCompileResult ret;

  p = orc_test_get_program_for_opcode (opcode);
  if (p) {
    ret = orc_program_compile_full (p, target, target_flags);
    orc_program_free (p);

    if (ORC_COMPILE_RESULT_IS_SUCCESSFUL(ret)) {
      return "yes";
    }
  }

  p = orc_test_get_program_for_opcode_const (opcode);
  if (p) {
    ret = orc_program_compile_full (p, target, target_flags);
    orc_program_free (p);

    if (ORC_COMPILE_RESULT_IS_SUCCESSFUL(ret)) {
      return "yes";
    }
  }

  return "no";
}

