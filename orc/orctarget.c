#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <orc/orctarget.h>
#include <orc/orcinternal.h>

static OrcTarget *targets[ORC_N_TARGETS];
static int n_targets;

static OrcTarget *default_target;

void
orc_target_register (OrcTarget *target)
{
  targets[n_targets] = target;
  n_targets++;

  if (target->executable) {
    default_target = target;
  }
}

OrcTarget *
orc_target_get_by_name (const char *name)
{
  int i;

  if (name == NULL) return orc_target_get_default();

  for(i=0;i<n_targets;i++){
    if (strcmp (name, targets[i]->name) == 0) {
      return targets[i];
    }
  }

  return NULL;
}

OrcTarget *
orc_target_get_default (void)
{
  const char *const envvar = _orc_getenv ("ORC_BACKEND");

  if (envvar != NULL) {
    OrcTarget *const target = orc_target_get_by_name (envvar);

    if (target != NULL)
      return target;
  }

  return default_target;
}

const char *
orc_target_get_name (OrcTarget *target)
{
  if (target == NULL) return NULL;
  return target->name;
}

unsigned int
orc_target_get_default_flags (OrcTarget *target)
{
  if (target == NULL) return 0;
  return target->get_default_flags();
}

const char *
orc_target_get_preamble (OrcTarget *target)
{
  if (target->get_asm_preamble == NULL) return "";

  return target->get_asm_preamble ();
}

const char *
orc_target_get_asm_preamble (const char *target)
{
  OrcTarget *t;

  t = orc_target_get_by_name (target);
  if (t == NULL) return "";

  return orc_target_get_preamble (t);
}

const char *
orc_target_get_flag_name (OrcTarget *target, int shift)
{
  if (target->get_flag_name == NULL) return "";

  return target->get_flag_name (shift);
}

/* FIXME the order of parameters should not be this,
 * first the target + flags, then the opcode
 */
OrcRule *
orc_target_get_rule (OrcTarget *target, OrcStaticOpcode *opcode,
    unsigned int target_flags)
{
  OrcOpcodeSet *opcode_set;
  OrcRule *rule;
  int i;
  int j;


  /* FIXME yes, we are iterating twice */
  opcode_set = orc_opcode_set_find_by_opcode (opcode);
  j = orc_opcode_set_find_by_name (opcode_set, opcode->name);

  for (i = target->n_rule_sets - 1; i >= 0; i--) {
    if (target->rule_sets[i].opcode_major != opcode_set->opcode_major) continue;
    if (target->rule_sets[i].required_target_flags & (~target_flags)) continue;

    /* A rule set has the same number of rules as the opcode_set has opcodes */
    rule = target->rule_sets[i].rules + j;
    if (rule->emit) return rule;
  }

  return NULL;
}
