
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

static const char *c_get_type_name (int size);

void orc_c_init (void);

void
orc_compiler_c_init (OrcCompiler *compiler)
{
  int i;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
    compiler->valid_regs[i] = 1;
  }
  compiler->loop_shift = 0;
}

const char *
orc_target_get_asm_preamble (int target)
{
  return "\n"
    "/* begin Orc C target preamble */\n"
    "#include <stdint.h>\n"
    "typedef struct _OrcProgram OrcProgram;\n"
    "typedef struct _OrcExecutor OrcExecutor;\n"
    "#define ORC_N_VARIABLES 20\n"
    "#define ORC_N_REGISTERS 20\n"
    "#define ORC_OPCODE_N_ARGS 4\n"
    "struct _OrcExecutor {\n"
    "  OrcProgram *program;\n"
    "  int n;\n"
    "  int counter1;\n"
    "  int counter2;\n"
    "  int counter3;\n"
    "  void *arrays[ORC_N_VARIABLES];\n"
    "  int params[ORC_N_VARIABLES];\n"
    "  //OrcVariable vars[ORC_N_VARIABLES];\n"
    "  //OrcVariable *args[ORC_OPCODE_N_ARGS];\n"
    "};\n"
    "#define ORC_CLAMP(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))\n"
    "#define ORC_ABS(a) ((a)<0 ? (-a) : (a))\n"
    "#define ORC_MIN(a,b) ((a)<(b) ? (a) : (b))\n"
    "#define ORC_MAX(a,b) ((a)>(b) ? (a) : (b))\n"
    "#define ORC_SB_MAX 127\n"
    "#define ORC_SB_MIN (-1-ORC_SB_MAX)\n"
    "#define ORC_UB_MAX 255\n"
    "#define ORC_UB_MIN 0\n"
    "#define ORC_SW_MAX 32767\n"
    "#define ORC_SW_MIN (-1-ORC_SW_MAX)\n"
    "#define ORC_UW_MAX 65535\n"
    "#define ORC_UW_MIN 0\n"
    "#define ORC_SL_MAX 2147483647\n"
    "#define ORC_SL_MIN (-1-ORC_SL_MAX)\n"
    "#define ORC_UL_MAX 4294967295U\n"
    "#define ORC_UL_MIN 0\n"
    "#define ORC_CLAMP_SB(x) ORC_CLAMP(x,ORC_SB_MIN,ORC_SB_MAX)\n"
    "#define ORC_CLAMP_UB(x) ORC_CLAMP(x,ORC_UB_MIN,ORC_SB_MAX)\n"
    "#define ORC_CLAMP_SW(x) ORC_CLAMP(x,ORC_SW_MIN,ORC_SB_MAX)\n"
    "#define ORC_CLAMP_UW(x) ORC_CLAMP(x,ORC_UW_MIN,ORC_SB_MAX)\n"
    "#define ORC_CLAMP_SL(x) ORC_CLAMP(x,ORC_SL_MIN,ORC_SB_MAX)\n"
    "#define ORC_CLAMP_UL(x) ORC_CLAMP(x,ORC_UL_MIN,ORC_SB_MAX)\n"
    "/* end Orc C target preamble */\n\n";
}

void
orc_compiler_assemble_c (OrcCompiler *compiler)
{
  int i;
  int j;
  OrcInstruction *insn;
  OrcOpcode *opcode;
  OrcRule *rule;

  ORC_ASM_CODE(compiler,"void\n");
  ORC_ASM_CODE(compiler,"%s (OrcExecutor *ex)\n", compiler->program->name);
  ORC_ASM_CODE(compiler,"{\n");
  ORC_ASM_CODE(compiler,"  int i;\n");

  for(i=0;i<compiler->n_vars;i++){
    OrcVariable *var = compiler->vars + i;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        ORC_ASM_CODE(compiler,"  %s var%d = %d;\n",
            c_get_type_name(var->size), i, var->value);
        break;
      case ORC_VAR_TYPE_TEMP:
        ORC_ASM_CODE(compiler,"  %s var%d;\n", c_get_type_name(var->size), i);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        ORC_ASM_CODE(compiler,"  %s *var%d = ex->arrays[%d];\n",
            c_get_type_name (var->size), i, i);
        break;
      case ORC_VAR_TYPE_PARAM:
        ORC_ASM_CODE(compiler,"  %s var%d = ex->arrays[%d];\n",
            c_get_type_name (var->size), i, i);
        break;
      default:
        break;
    }

  }

  ORC_ASM_CODE(compiler,"\n");
  ORC_ASM_CODE(compiler,"  for (i = 0; i < ex->n; i++) {\n");

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    ORC_ASM_CODE(compiler,"    /* %d: %s */\n", j, insn->opcode->name);

    rule = insn->rule;
    if (rule) {
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      ORC_PROGRAM_ERROR(compiler,"No rule for: %s\n", opcode->name);
      compiler->error = TRUE;
    }
  }

  ORC_ASM_CODE(compiler,"  }\n");
  ORC_ASM_CODE(compiler,"}\n");
  ORC_ASM_CODE(compiler,"\n");
}


/* rules */

static void
c_get_name (char *name, OrcCompiler *p, int var)
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

static const char *
c_get_type_name (int size)
{
  switch (size) {
    case 1:
      return "int8_t";
    case 2:
      return "int16_t";
    case 4:
      return "int32_t";
    default:
      return "ERROR";
  }
}


#if 0
static void
c_rule_copyw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);

  ORC_ASM_CODE(p,"    %s = %s;\n", dest, src1);
}
#endif

#define BINARY(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[20], src1[20], src2[20]; \
\
  c_get_name (dest, p, insn->args[0]); \
  c_get_name (src1, p, insn->args[1]); \
  c_get_name (src2, p, insn->args[2]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1, src2); \
}

#define BINARY_SB(a,b) BINARY(a,b)
#define BINARY_UB(a,b) BINARY(a,b)
#define BINARY_SW(a,b) BINARY(a,b)
#define BINARY_UW(a,b) BINARY(a,b)
#define BINARY_SL(a,b) BINARY(a,b)
#define BINARY_UL(a,b) BINARY(a,b)
#define UNARY_SB(a,b) BINARY(a,b)
#define UNARY_UB(a,b) BINARY(a,b)
#define UNARY_SW(a,b) BINARY(a,b)
#define UNARY_UW(a,b) BINARY(a,b)
#define UNARY_SL(a,b) BINARY(a,b)
#define UNARY_UL(a,b) BINARY(a,b)
#define BINARY_BW(a,b) BINARY(a,b)
#define BINARY_WL(a,b) BINARY(a,b)
#define BINARY_LW(a,b) BINARY(a,b)
#define BINARY_WB(a,b) BINARY(a,b)

#if 0
//UNNARY(absw, src1 + src2)
BINARY(addw, src1 + src2)
BINARY(addssw, CLAMP(src1 + src2, ORC_SB_MIN,ORC_SB_MAX))
BINARY(addusw, CLAMP(src1 + src2, 0,ORC_UB_MAX))
BINARY(subw, src1 - src2)
BINARY(mullw, src1 * src2)
BINARY(mulhw, (src1 * src2)>>16)
BINARY(shlw, src1 << src2)
BINARY(shrsw, src1 >> src2)
#endif

#include "opcodes.h"

#undef BINARY_SB
#undef BINARY_UB
#undef BINARY_SW
#undef BINARY_UW
#undef BINARY_SL
#undef BINARY_UL
#undef UNARY_SB
#undef UNARY_UB
#undef UNARY_SW
#undef UNARY_UW
#undef UNARY_SL
#undef UNARY_UL
#undef BINARY_BW
#undef BINARY_WL
#undef BINARY_LW
#undef BINARY_WB


#if 0
static void
c_rule_addw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s + %s;\n", dest, src1, src2);
}

static void
c_rule_subw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s - %s;\n", dest, src1, src2);
}

static void
c_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s * %s;\n", dest, src1, src2);
}

static void
c_rule_shlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s << %s;\n", dest, src1, src2);
}

static void
c_rule_shrsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->args[0]);
  c_get_name (src1, p, insn->args[1]);
  c_get_name (src2, p, insn->args[2]);

  ORC_ASM_CODE(p,"    %s = %s >> %s;\n", dest, src1, src2);
}
#endif


void
orc_c_init (void)
{
#define BINARY_SB(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_UB(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_SW(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_UW(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_SL(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_UL(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define UNARY_SB(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define UNARY_UB(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define UNARY_SW(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define UNARY_UW(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define UNARY_SL(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define UNARY_UL(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_BW(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_WL(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_LW(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);
#define BINARY_WB(a,b) orc_rule_register ( #a , ORC_TARGET_C, c_rule_ ## a, NULL);

#include "opcodes.h"

#if 0
  orc_rule_register ("copyw", ORC_TARGET_C, c_rule_copyw, NULL);
  orc_rule_register ("addw", ORC_TARGET_C, c_rule_addw, NULL);
  orc_rule_register ("subw", ORC_TARGET_C, c_rule_subw, NULL);
  orc_rule_register ("mulw", ORC_TARGET_C, c_rule_mullw, NULL);
  //orc_rule_register ("mulhw", ORC_TARGET_C, c_rule_mulhw, NULL);
  orc_rule_register ("shlw", ORC_TARGET_C, c_rule_shlw, NULL);
  orc_rule_register ("shrsw", ORC_TARGET_C, c_rule_shrsw, NULL);
#endif
}

