#include "config.h"

#include <string.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>
#include <orc/orcutils-private.h>

/**
 * SECTION:orcopcode
 * @title: OrcOpcode
 * @short_description: Operations
 */


static OrcOpcodeSet *opcode_sets;
static int n_opcode_sets;

#if 0
int
orc_opcode_get_list (OrcOpcode **list)
{
  (*list) = opcode_list;
  return n_opcodes;
}
#endif

#if 0
void
orc_opcode_register (const char *name, int n_dest, int n_src,
    OrcOpcodeEmulateFunc emulate, void *user)
{
  OrcOpcode *opcode;

  if (n_opcodes == n_opcodes_alloc) {
    n_opcodes_alloc += 100;
    opcode_list = orc_realloc(opcode_list, sizeof(OrcOpcode) * n_opcodes_alloc);
  }

  opcode = opcode_list + n_opcodes;

  opcode->name = strdup (name);
  opcode->n_src = n_src;
  opcode->n_dest = n_dest;
  opcode->emulate = emulate;
  opcode->emulate_user = user;

  n_opcodes++;
}
#endif

int
orc_opcode_register_static (OrcStaticOpcode *sopcode, char *prefix)
{
  int n;
  int major;

  n = 0;
  while (sopcode[n].name[0]) {
    n++;
  }

  major = n_opcode_sets;

  n_opcode_sets++;
  opcode_sets = orc_realloc (opcode_sets, sizeof(OrcOpcodeSet)*n_opcode_sets);

  memset (opcode_sets + major, 0, sizeof(OrcOpcodeSet));
  strncpy(opcode_sets[major].prefix, prefix, sizeof(opcode_sets[major].prefix)-1);
  opcode_sets[major].n_opcodes = n;
  opcode_sets[major].opcodes = sopcode;
  opcode_sets[major].opcode_major = major;

  return major;
}

OrcOpcodeSet *
orc_opcode_set_get (const char *name)
{
  int i;

  for(i=0;i<n_opcode_sets;i++){
    if (strcmp (opcode_sets[i].prefix, name) == 0) {
      return opcode_sets + i;
    }
  }

  return NULL;
}

OrcOpcodeSet *
orc_opcode_set_get_nth (int opcode_major)
{
  return opcode_sets + opcode_major;
}

OrcOpcodeSet *
orc_opcode_set_find_by_opcode (OrcStaticOpcode * opcode)
{
  int k;
  int j;

  /* Pointer arithmetic to find a pointer inside an array ...
  */
  for (k = 0; k < n_opcode_sets; k++) {
    j = opcode - opcode_sets[k].opcodes;

    if (j < 0 || j >= opcode_sets[k].n_opcodes) continue;
    if (opcode_sets[k].opcodes + j != opcode) continue;

    return &opcode_sets[k];
  }

  return NULL;
}

/* FIXME This is wrongly named, you are finding the index of the opcode on a specific opcode set
 */
int
orc_opcode_set_find_by_name (OrcOpcodeSet *opcode_set, const char *name)
{
  int j;

  for(j=0;j<opcode_set->n_opcodes;j++){
    if (strcmp (name, opcode_set->opcodes[j].name) == 0) {
      return j;
    }
  }

  return -1;
}

OrcStaticOpcode *
orc_opcode_find_by_name (const char *name)
{
  int i;
  int j;

  for(i=0;i<n_opcode_sets;i++){
    j = orc_opcode_set_find_by_name (opcode_sets + i, name);
    if (j >= 0) {
      return &opcode_sets[i].opcodes[j];
    }
  }

  return NULL;
}

void
emulate_null (OrcOpcodeExecutor *ex, int offset, int n)
{
  /* This is a placeholder for adding new opcodes */
  ORC_ERROR("emulate_null() called.  This is a bug.");
}

void
orc_opcode_init (void)
{
  orc_opcode_sys_init ();
}
