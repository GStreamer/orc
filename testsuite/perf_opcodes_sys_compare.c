
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
    //printf("opcode_%-20s ", opcode_set->opcodes[i].name);
    test_opcode_src (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode_src (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int flags = 0;
  double perf_mmx, perf_sse;

  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  orc_program_add_source (p, opcode->src_size[0], "s1");
  if (opcode->src_size[1] != 0) {
    if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
      orc_program_add_constant (p, opcode->src_size[1], 1, "s2");
    } else {
      orc_program_add_source (p, opcode->src_size[1], "s2");
    }
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_s_%s ", opcode->name);
  orc_program_set_name (p, s);

  if (opcode->dest_size[1] != 0) {
    orc_program_append_dds_str (p, opcode->name, "d1", "d2", "s1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "s2");
  }

  perf_mmx = orc_test_performance_full (p, flags, "mmx");
  perf_sse = orc_test_performance_full (p, flags, "sse");

  printf("%g %g\n", perf_mmx, perf_sse);

  orc_program_free (p);
}

