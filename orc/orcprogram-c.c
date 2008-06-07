
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orcprogram.h>


void orc_c_init (void);

void
orc_program_assemble_c (OrcProgram *program)
{
  int i;
  int j;
  OrcInstruction *insn;
  OrcOpcode *opcode;
  OrcRule *rule;

  printf("\n");
  printf("void\n");
  printf("test (OrcExecutor *ex)\n");
  printf("{\n");
  printf("  int i;\n");

  for(i=0;i<program->n_vars;i++){
    OrcVariable *var = program->vars + i;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        printf("  int16_t var%d = %d;\n", i, var->s16);
        break;
      case ORC_VAR_TYPE_TEMP:
        printf("  int16_t var%d;\n", i);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        printf("  int16_t *var%d = ex->var%d;\n", i, i);
        break;
      case ORC_VAR_TYPE_PARAM:
        printf("  int16_t var%d = ex->var%d;\n", i, i);
        break;
      default:
        break;
    }

  }

  printf("\n");
  printf("  for (i = 0; i < n; i++) {\n");

  for(j=0;j<program->n_insns;j++){
    insn = program->insns + j;
    opcode = insn->opcode;

    printf("    // %d: %s\n", j, insn->opcode->name);

#if 0
    for(k=opcode->n_dest;k<opcode->n_src + opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_SRC:
          x86_emit_load_src (program, args[k]);
          break;
        case ORC_VAR_TYPE_CONST:
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
#endif

    rule = insn->rule;
    if (rule) {
      rule->emit (program, rule->emit_user, insn);
    } else {
      printf("No rule for: %s\n", opcode->name);
    }

#if 0
    for(k=0;k<opcode->n_dest;k++){
      switch (args[k]->vartype) {
        case ORC_VAR_TYPE_DEST:
          x86_emit_store_dest (program, args[k]);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
#endif
  }

  printf("  }\n");
  printf("}\n");
  printf("\n");
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
c_rule_add_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  printf ("    %s = %s + %s;\n", dest, src1, src2);
}

static void
c_rule_sub_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  printf ("    %s = %s - %s;\n", dest, src1, src2);
}

static void
c_rule_mul_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  printf ("    %s = %s * %s;\n", dest, src1, src2);
}

static void
c_rule_lshift_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  printf ("    %s = %s << %s;\n", dest, src1, src2);
}

static void
c_rule_rshift_s16 (OrcProgram *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  printf ("    %s = %s >> %s;\n", dest, src1, src2);
}


void
orc_c_init (void)
{
  orc_rule_register ("add_s16", ORC_RULE_C, c_rule_add_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("sub_s16", ORC_RULE_C, c_rule_sub_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("mul_s16", ORC_RULE_C, c_rule_mul_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("lshift_s16", ORC_RULE_C, c_rule_lshift_s16, NULL,
      ORC_RULE_REG_REG);
  orc_rule_register ("rshift_s16", ORC_RULE_C, c_rule_rshift_s16, NULL,
      ORC_RULE_REG_REG);
}

