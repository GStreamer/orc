
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

/**
 * SECTION:orcrule
 * @title: OrcRule
 * @short_description: Creating rules for code generation
 */


void
orc_rule_register (OrcRuleSet *rule_set,
    const char *opcode_name,
    OrcRuleEmitFunc emit, void *emit_user)
{
  int i;
  OrcOpcodeSet *opcode_set;

  opcode_set = orc_opcode_set_get_nth (rule_set->opcode_major);

  i = orc_opcode_set_find_by_name (opcode_set, opcode_name);
  if (i == -1) {
    ORC_ERROR("failed to find opcode \"%s\"", opcode_name);
    return;
  }

  rule_set->rules[i].emit = emit;
  rule_set->rules[i].emit_user = emit_user;
}

OrcRuleSet *
orc_rule_set_new (OrcOpcodeSet *opcode_set, OrcTarget *target,
    unsigned int required_flags)
{
  OrcRuleSet *rule_set;

  rule_set = target->rule_sets + target->n_rule_sets;
  target->n_rule_sets++;

  memset (rule_set, 0, sizeof(OrcRuleSet));

  rule_set->opcode_major = opcode_set->opcode_major;
  rule_set->required_target_flags = required_flags;

  rule_set->rules = malloc (sizeof(OrcRule) * opcode_set->n_opcodes);
  memset (rule_set->rules, 0, sizeof(OrcRule) * opcode_set->n_opcodes);

  return rule_set;
}
