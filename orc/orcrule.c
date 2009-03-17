
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


void
orc_rule_register (OrcRuleSet *rule_set,
    const char *opcode_name,
    OrcRuleEmitFunc emit, void *emit_user)
{
  int i;

  i = orc_opcode_set_find_by_name (rule_set->opcode_set, opcode_name);
  if (i == -1) {
    ORC_ERROR("failed to find opcode \"%s\"", opcode_name);
    return;
  }

  rule_set->rules[i].emit = emit;
  rule_set->rules[i].emit_user = emit_user;
}

