
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


void
orc_rule_register (const char *opcode_name, unsigned int mode,
    OrcRuleEmitFunc emit, void *emit_user)
{
  OrcOpcode *opcode;

  opcode = orc_opcode_find_by_name (opcode_name);
  if (opcode == NULL) {
    ORC_ERROR("failed to find opcode \"%s\"", opcode_name);
    return;
  }

  opcode->rules[mode].emit = emit;
  opcode->rules[mode].emit_user = emit_user;
}

