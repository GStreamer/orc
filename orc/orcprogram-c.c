
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


void orc_c_init (void);

void
orc_program_c_init (OrcProgram *program)
{

}

void
orc_program_assemble_c (OrcProgram *program)
{
  int i;
  int j;
  OrcInstruction *insn;
  OrcOpcode *opcode;
  OrcRule *rule;

  ORC_ASM_CODE(program,"\n");
  ORC_ASM_CODE(program,"void\n");
  ORC_ASM_CODE(program,"test (OrcExecutor *ex)\n");
  ORC_ASM_CODE(program,"{\n");
  ORC_ASM_CODE(program,"  int i;\n");

  for(i=0;i<program->n_vars;i++){
    OrcVariable *var = program->vars + i;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        ORC_ASM_CODE(program,"  int16_t var%d = %d;\n", i,
            (int16_t)var->value);
        break;
      case ORC_VAR_TYPE_TEMP:
        ORC_ASM_CODE(program,"  int16_t var%d;\n", i);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        ORC_ASM_CODE(program,"  int16_t *var%d = ex->var%d;\n", i, i);
        break;
      case ORC_VAR_TYPE_PARAM:
        ORC_ASM_CODE(program,"  int16_t var%d = ex->var%d;\n", i, i);
        break;
      default:
        break;
    }

  }

  ORC_ASM_CODE(program,"\n");
  ORC_ASM_CODE(program,"  for (i = 0; i < n; i++) {\n");

  for(j=0;j<program->n_insns;j++){
    insn = program->insns + j;
    opcode = insn->opcode;

    ORC_ASM_CODE(program,"    /* %d: %s */\n", j, insn->opcode->name);

    rule = insn->rule;
    if (rule) {
      rule->emit (program, rule->emit_user, insn);
    } else {
      ORC_ASM_CODE(program,"#error No rule for: %s\n", opcode->name);
    }
  }

  ORC_ASM_CODE(program,"  }\n");
  ORC_ASM_CODE(program,"}\n");
  ORC_ASM_CODE(program,"\n");
}


/* rules */

static void
c_get_name (char *name, OrcProgram *p, int var)
{
  switch (p->vars[var].vartype) {
    case ORC_VAR_TYPE_CONST:
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
      sprintf(name, "var%d", var);
      break;
    case ORC_VAR_TYPE_SRC:
    case ORC_VAR_TYPE_DEST:
      sprintf(name, "var%d[i]", var);
      break;
    default:
      sprintf(name, "ERROR");
      break;
  }

}

static void
c_rule_copyw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);

  ORC_ASM_CODE(p,"    %s = %s;\n", dest, src1);
}

static void
c_rule_addw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s + %s;\n", dest, src1, src2);
}

static void
c_rule_subw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s - %s;\n", dest, src1, src2);
}

static void
c_rule_mullw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s * %s;\n", dest, src1, src2);
}

static void
c_rule_shlw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s << %s;\n", dest, src1, src2);
}

static void
c_rule_shrsw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s >> %s;\n", dest, src1, src2);
}


void
orc_c_init (void)
{
  orc_rule_register ("copyw", ORC_TARGET_C, c_rule_copyw, NULL);
  orc_rule_register ("addw", ORC_TARGET_C, c_rule_addw, NULL);
  orc_rule_register ("subw", ORC_TARGET_C, c_rule_subw, NULL);
  orc_rule_register ("mullw", ORC_TARGET_C, c_rule_mullw, NULL);
  orc_rule_register ("shlw", ORC_TARGET_C, c_rule_shlw, NULL);
  orc_rule_register ("shrsw", ORC_TARGET_C, c_rule_shrsw, NULL);
}

