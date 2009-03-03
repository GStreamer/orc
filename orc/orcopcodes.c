
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
copyw (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = ex->args[1]->s16;
}

static void
addw (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 + ex->args[2]->s16);
}

static void
subw (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 - ex->args[2]->s16);
}

static void
mullw (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 * ex->args[2]->s16);
}

static void
shlw (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(ex->args[1]->s16 << ex->args[2]->s16);
}

static void
shrsw (OrcExecutor *ex, void *user)
{
  ex->args[0]->s16 = (int16_t)(((int16_t)ex->args[1]->s16) >> ex->args[2]->s16);
}

static void
convsuswb (OrcExecutor *ex, void *user)
{
  ex->args[0]->u8 = (int16_t)(ex->args[1]->s16);
}

void
orc_opcode_init (void)
{
  orc_opcode_register("copyw", 1, 2, copyw, NULL);
  orc_opcode_register("addw", 1, 2, addw, NULL);
  orc_opcode_register("subw", 1, 2, subw, NULL);
  orc_opcode_register("mullw", 1, 2, mullw, NULL);
  orc_opcode_register("shlw", 1, 2, shlw, NULL);
  orc_opcode_register("shrsw", 1, 2, shrsw, NULL);
  orc_opcode_register("convsuswb", 1, 1, convsuswb, NULL);
}


