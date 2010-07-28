
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
  OrcStaticOpcode *opcode;
  const char *d1;

  orc_init();
  orc_test_init();

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    opcode = opcode_set->opcodes + i;

    printf(".function emulate_%s\n", opcode->name);
    if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
      printf(".accumulator %d a1\n", opcode->dest_size[0]);
      d1 = "a1";
    } else {
      printf(".dest %d d1\n", opcode->dest_size[0]);
      d1 = "d1";
    }
    if (opcode->dest_size[1]) {
      printf(".dest %d d2\n", opcode->dest_size[1]);
    }
    printf(".source %d s1\n", opcode->src_size[0]);
    if (opcode->src_size[1]) {
      if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
        printf(".param %d s2\n", opcode->src_size[1]);
      } else {
        printf(".source %d s2\n", opcode->src_size[1]);
      }
    }
    printf("\n");
    if (opcode->src_size[1]) {
      printf("%s %s, s1, s2\n", opcode->name, d1);
    } else {
      if (opcode->dest_size[1]) {
        printf("%s %s, d2, s1\n", opcode->name, d1);
      } else {
        printf("%s %s, s1\n", opcode->name, d1);
      }
    }
    printf("\n");
    printf("\n");

    printf(".function emulate_n16_%s\n", opcode->name);
    printf(".n 16\n");
    if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
      printf(".accumulator %d a1\n", opcode->dest_size[0]);
      d1 = "a1";
    } else {
      printf(".dest %d d1\n", opcode->dest_size[0]);
      d1 = "d1";
    }
    if (opcode->dest_size[1]) {
      printf(".dest %d d2\n", opcode->dest_size[1]);
    }
    printf(".source %d s1\n", opcode->src_size[0]);
    if (opcode->src_size[1]) {
      if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
        printf(".param %d s2\n", opcode->src_size[1]);
      } else {
        printf(".source %d s2\n", opcode->src_size[1]);
      }
    }
    printf("\n");
    if (opcode->src_size[1]) {
      printf("%s %s, s1, s2\n", opcode->name, d1);
    } else {
      if (opcode->dest_size[1]) {
        printf("%s %s, d2, s1\n", opcode->name, d1);
      } else {
        printf("%s %s, s1\n", opcode->name, d1);
      }
    }
    printf("\n");
    printf("\n");
  }

  if (error) return 1;
  return 0;
}

