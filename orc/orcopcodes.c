
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

void
orc_opcode_register_static (OrcStaticOpcode *sopcode)
{
  while (sopcode->name[0]) {
    OrcOpcode *opcode;
    int i;

    if (n_opcodes == n_opcodes_alloc) {
      n_opcodes_alloc += 100;
      opcode_list = realloc(opcode_list, sizeof(OrcOpcode) * n_opcodes_alloc);
    }

    opcode = opcode_list + n_opcodes;

    memset (opcode, 0, sizeof(OrcOpcode));

    opcode->name = sopcode->name;
    for(i=0;i<ORC_STATIC_OPCODE_N_DEST;i++){
      opcode->dest_size[i] = sopcode->dest_size[i];
      if (sopcode->dest_size[i]) opcode->n_dest = i + 1;
    }
    for(i=0;i<ORC_STATIC_OPCODE_N_SRC;i++){
      opcode->src_size[i] = sopcode->src_size[i];
      if (sopcode->src_size[i]) opcode->n_src = i + 1;
    }
    opcode->emulate = sopcode->emulate;
    opcode->emulate_user = sopcode->emulate_user;

    n_opcodes++;
    sopcode++;
  }
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
  ex->args[0]->value = ex->args[1]->value;
}

static void
addw (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int16_t)(ex->args[1]->value + ex->args[2]->value);
}

static void
subw (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int16_t)(ex->args[1]->value - ex->args[2]->value);
}

static void
mullw (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int16_t)(ex->args[1]->value * ex->args[2]->value);
}

static void
shlw (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int16_t)(ex->args[1]->value << ex->args[2]->value);
}

static void
shrsw (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int16_t)(((int16_t)ex->args[1]->value) >> ex->args[2]->value);
}

static void
convsuswb (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int16_t)(ex->args[1]->value);
}

static void
convbw (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (uint8_t)(ex->args[1]->value);
}

static void
addb (OrcExecutor *ex, void *user)
{
  ex->args[0]->value = (int8_t)(ex->args[1]->value + ex->args[2]->value);
}

static OrcStaticOpcode opcodes[] = {
  { "convbw", convbw, NULL, { 2 }, { 1 } },
  { "copyw", copyw, NULL, { 2 }, { 2, 2 } },
  { "addw", addw, NULL, { 2 }, { 2, 2 } },
  { "subw", subw, NULL, { 2 }, { 2, 2 } },
  { "mullw", mullw, NULL, { 2 }, { 2, 2 } },
  { "shlw", shlw, NULL, { 2 }, { 2, 2 } },
  { "shrsw", shrsw, NULL, { 2 }, { 2, 2 } },
  { "convsuswb", convsuswb, NULL, { 1 }, { 2 } },

  { "addb", addb, NULL, { 1 }, { 1, 1 } },
  { "" }
};

void
orc_opcode_init (void)
{
  orc_opcode_register_static (opcodes);
}


