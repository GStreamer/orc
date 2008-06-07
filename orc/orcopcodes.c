
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


static OrcOpcode *opcode_list;
static int n_opcodes;
static int n_opcodes_alloc;



void
orc_opcode_register (const char *name, int n_dest, int n_src,
    OrcOpcodeEmulateFunc emulate, void *user)
{
  OrcOpcode *opcode;

  if (n_opcodes == n_opcodes_alloc) {
    n_opcodes_alloc += 100;
    opcode_list = realloc(opcode_list, sizeof(OrcOpcode) * n_opcodes_alloc);
  }

  opcode = opcode_list + n_opcodes;

  opcode->name = strdup (name);
  opcode->n_src = n_src;
  opcode->n_dest = n_dest;
  opcode->emulate = emulate;
  opcode->emulate_user = user;

  n_opcodes++;
}

OrcOpcode *
orc_opcode_find_by_name (const char *name)
{
  int i;

  for(i=0;i<n_opcodes;i++){
    if (!strcmp (name, opcode_list[i].name)) {
      return opcode_list + i;
    }
  }

  return NULL;
}

static void
move_s16 (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = ex->args[1]->s16;
}

static void
add_s16 (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 + ex->args[2]->s16);
}

static void
sub_s16 (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 - ex->args[2]->s16);
}

static void
mul_s16 (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 * ex->args[2]->s16);
}

static void
lshift_s16 (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 << ex->args[2]->s16);
}

static void
rshift_s16 (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 >> ex->args[2]->s16);
}

void
orc_opcode_init (void)
{
  orc_opcode_register("_loadi_s16", 1, 1, move_s16, NULL);

  orc_opcode_register("add_s16", 1, 2, add_s16, NULL);
  orc_opcode_register("sub_s16", 1, 2, sub_s16, NULL);
  orc_opcode_register("mul_s16", 1, 2, mul_s16, NULL);
  orc_opcode_register("lshift_s16", 1, 2, lshift_s16, NULL);
  orc_opcode_register("rshift_s16", 1, 2, rshift_s16, NULL);
}


