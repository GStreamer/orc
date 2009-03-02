
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

  orc_program_append_code(program,"\n");
  orc_program_append_code(program,"void\n");
  orc_program_append_code(program,"test (OrcExecutor *ex)\n");
  orc_program_append_code(program,"{\n");
  orc_program_append_code(program,"  int i;\n");

  for(i=0;i<program->n_vars;i++){
    OrcVariable *var = program->vars + i;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        orc_program_append_code(program,"  int16_t var%d = %d;\n", i, var->s16);
        break;
      case ORC_VAR_TYPE_TEMP:
        orc_program_append_code(program,"  int16_t var%d;\n", i);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_program_append_code(program,"  int16_t *var%d = ex->var%d;\n", i, i);
        break;
      case ORC_VAR_TYPE_PARAM:
        orc_program_append_code(program,"  int16_t var%d = ex->var%d;\n", i, i);
        break;
      default:
        break;
    }

  }

  orc_program_append_code(program,"\n");
  orc_program_append_code(program,"  for (i = 0; i < n; i++) {\n");

  for(j=0;j<program->n_insns;j++){
    insn = program->insns + j;
    opcode = insn->opcode;

    orc_program_append_code(program,"    /* %d: %s */\n", j, insn->opcode->name);

    rule = insn->rule;
    if (rule) {
      rule->emit (program, rule->emit_user, insn);
    } else {
      orc_program_append_code(program,"#error No rule for: %s\n", opcode->name);
    }
  }

  orc_program_append_code(program,"  }\n");
  orc_program_append_code(program,"}\n");
  orc_program_append_code(program,"\n");
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
c_rule_copy_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);

  orc_program_append_code(p,"    %s = %s;\n", dest, src1);
}

static void
c_rule_add_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  orc_program_append_code(p,"    %s = %s + %s;\n", dest, src1, src2);
}

static void
c_rule_sub_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  orc_program_append_code(p,"    %s = %s - %s;\n", dest, src1, src2);
}

static void
c_rule_mul_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  orc_program_append_code(p,"    %s = %s * %s;\n", dest, src1, src2);
}

static void
c_rule_lshift_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  orc_program_append_code(p,"    %s = %s << %s;\n", dest, src1, src2);
}

static void
c_rule_rshift_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  orc_program_append_code(p,"    %s = %s >> %s;\n", dest, src1, src2);
}


void
orc_c_init (void)
{
  orc_rule_register ("copy_s16", ORC_TARGET_C, c_rule_copy_s16, NULL);
  orc_rule_register ("add_s16", ORC_TARGET_C, c_rule_add_s16, NULL);
  orc_rule_register ("sub_s16", ORC_TARGET_C, c_rule_sub_s16, NULL);
  orc_rule_register ("mul_s16", ORC_TARGET_C, c_rule_mul_s16, NULL);
  orc_rule_register ("lshift_s16", ORC_TARGET_C, c_rule_lshift_s16, NULL);
  orc_rule_register ("rshift_s16", ORC_TARGET_C, c_rule_rshift_s16, NULL);
}

