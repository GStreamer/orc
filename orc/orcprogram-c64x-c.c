
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <orc/orc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

//static const char *c_get_type_name (int size);

void orc_c_init (void);

void
orc_compiler_c64x_c_init (OrcCompiler *compiler)
{
  int i;

  for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
    compiler->valid_regs[i] = 1;
  }
  compiler->loop_shift = 0;
}

const char *
orc_target_c64x_c_get_asm_preamble (void)
{
  return "\n"
    "/* begin Orc C target preamble */\n"
    "typedef signed char int8_t;\n"
    "typedef unsigned char uint8_t;\n"
    "typedef signed short int16_t;\n"
    "typedef unsigned short uint16_t;\n"
    "typedef signed int int32_t;\n"
    "typedef unsigned int uint32_t;\n"
    "typedef signed long long int64_t;\n"
    "typedef unsigned long long uint64_t;\n"
    "#define ORC_RESTRICT restrict\n"
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
    "#define ORC_ABS(a) ((a)<0 ? -(a) : (a))\n"
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
    "#define ORC_CLAMP_UB(x) ORC_CLAMP(x,ORC_UB_MIN,ORC_UB_MAX)\n"
    "#define ORC_CLAMP_SW(x) ORC_CLAMP(x,ORC_SW_MIN,ORC_SW_MAX)\n"
    "#define ORC_CLAMP_UW(x) ORC_CLAMP(x,ORC_UW_MIN,ORC_UW_MAX)\n"
    "#define ORC_CLAMP_SL(x) ORC_CLAMP(x,ORC_SL_MIN,ORC_SL_MAX)\n"
    "#define ORC_CLAMP_UL(x) ORC_CLAMP(x,ORC_UL_MIN,ORC_UL_MAX)\n"
    "#define ORC_SWAP_W(x) ((((x)&0xff)<<8) | (((x)&0xff00)>>8))\n"
    "#define ORC_SWAP_L(x) ((((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)&0xff0000)>>8) | (((x)&0xff000000)>>24))\n"
    "#define ORC_PTR_OFFSET(ptr,offset) ((void *)(((unsigned char *)(ptr)) + (offset)))\n"
    "/* end Orc C target preamble */\n\n";
}

unsigned int
orc_compiler_c64x_c_get_default_flags (void)
{
  return ORC_TARGET_C_NOEXEC;
}

static const char *varnames[] = {
  "d1", "d2", "d3", "d4",
  "s1", "s2", "s3", "s4",
  "s5", "s6", "s7", "s8",
  "a1", "a2", "a3", "d4",
  "c1", "c2", "c3", "c4",
  "c5", "c6", "c7", "c8",
  "p1", "p2", "p3", "p4",
  "p5", "p6", "p7", "p8",
  "t1", "t2", "t3", "t4",
  "t5", "t6", "t7", "t8",
  "t9", "t10", "t11", "t12",
  "t13", "t14", "t15", "t16"
};

void
output_prototype (OrcCompiler *compiler)
{
  OrcProgram *p = compiler->program;
  OrcVariable *var;
  int i;
  int need_comma;

  ORC_ASM_CODE(compiler, "%s (", p->name);
  need_comma = FALSE;
  for(i=0;i<4;i++){
    var = &p->vars[ORC_VAR_D1 + i];
    if (var->size) {
      if (need_comma) ORC_ASM_CODE(compiler, ", ");
      if (var->type_name) {
        ORC_ASM_CODE(compiler, "%s * %s", var->type_name,
            varnames[ORC_VAR_D1 + i]);
      } else {
        ORC_ASM_CODE(compiler, "uint%d_t * %s", var->size*8,
            varnames[ORC_VAR_D1 + i]);
      }
      if (p->is_2d) {
        ORC_ASM_CODE(compiler, ", int %s_stride", varnames[ORC_VAR_D1 + i]);
      }
      need_comma = TRUE;
    }
  }
  for(i=0;i<4;i++){
    var = &p->vars[ORC_VAR_A1 + i];
    if (var->size) {
      if (need_comma) ORC_ASM_CODE(compiler, ", ");
      if (var->type_name) {
        ORC_ASM_CODE(compiler, "%s * %s", var->type_name,
            varnames[ORC_VAR_A1 + i]);
      } else {
        ORC_ASM_CODE(compiler, "uint%d_t * %s", var->size*8,
            varnames[ORC_VAR_A1 + i]);
      }
      need_comma = TRUE;
    }
  }
  for(i=0;i<8;i++){
    var = &p->vars[ORC_VAR_S1 + i];
    if (var->size) {
      if (need_comma) ORC_ASM_CODE(compiler, ", ");
      if (var->type_name) {
        ORC_ASM_CODE(compiler, "%s * %s", var->type_name,
            varnames[ORC_VAR_S1 + i]);
      } else {
        ORC_ASM_CODE(compiler, "uint%d_t * %s", var->size*8,
            varnames[ORC_VAR_S1 + i]);
      }
      if (p->is_2d) {
        ORC_ASM_CODE(compiler, ", int %s_stride", varnames[ORC_VAR_S1 + i]);
      }
      need_comma = TRUE;
    }
  }
  for(i=0;i<8;i++){
    var = &p->vars[ORC_VAR_P1 + i];
    if (var->size) {
      if (need_comma) ORC_ASM_CODE(compiler, ", ");
      ORC_ASM_CODE(compiler, "int %s", varnames[ORC_VAR_P1 + i]);
      need_comma = TRUE;
    }
  }
  if (p->constant_n == 0) {
    if (need_comma) ORC_ASM_CODE(compiler, ", ");
    ORC_ASM_CODE(compiler, "int n");
    need_comma = TRUE;
  }
  if (p->is_2d && p->constant_m == 0) {
    if (need_comma) ORC_ASM_CODE(compiler, ", ");
    ORC_ASM_CODE(compiler, "int m");
  }
  ORC_ASM_CODE(compiler, ")");
}

void
orc_compiler_c64x_c_assemble (OrcCompiler *compiler)
{
  int i;
  int j;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;
  int prefix = 0;
  int loop_shift = 0;

compiler->target_flags |= ORC_TARGET_C_NOEXEC;

  if (!(compiler->target_flags & ORC_TARGET_C_BARE)) {
    if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
      ORC_ASM_CODE(compiler,"void\n");
      ORC_ASM_CODE(compiler,"%s (OrcExecutor *ex)\n", compiler->program->name);
    } else{
      ORC_ASM_CODE(compiler,"void\n");
      output_prototype (compiler);
      ORC_ASM_CODE(compiler,"\n");
    }
    ORC_ASM_CODE(compiler,"{\n");
  }

  ORC_ASM_CODE(compiler,"%*s  int i;\n", prefix, "");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  int j;\n");
  }

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        {
          int value = var->value;

          if (var->size == 1) {
            value = (value&0xff);
            value |= (value<<8);
            value |= (value<<16);
          }
          if (var->size == 2) {
            value = (value&0xffff);
            value |= (value<<16);
          }

          if (value == 0x80000000) {
            ORC_ASM_CODE(compiler,"  const int var%d = 0x80000000;\n", i);
          } else {
            ORC_ASM_CODE(compiler,"  const int var%d = %d;\n",
                i, value);
          }
        }
        break;
      case ORC_VAR_TYPE_TEMP:
        ORC_ASM_CODE(compiler,"  int var%d;\n", i);
        break;
      case ORC_VAR_TYPE_SRC:
        ORC_ASM_CODE(compiler,"  const unsigned char * restrict var%d;\n", i);
        break;
      case ORC_VAR_TYPE_DEST:
        ORC_ASM_CODE(compiler,"  unsigned char * restrict var%d;\n", i);
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        ORC_ASM_CODE(compiler,"  int var%d = 0;\n", i);
        break;
      case ORC_VAR_TYPE_PARAM:
        if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
          ORC_ASM_CODE(compiler,"  const int var%d = ex->params[%d];\n", i, i);
        } else {
          ORC_ASM_CODE(compiler,"  const int var%d = %s;\n", i, varnames[i]);
        }
        break;
      default:
        ORC_COMPILER_ERROR(compiler, "bad vartype");
        break;
    }
  }

  ORC_ASM_CODE(compiler,"\n");
  if (compiler->program->is_2d) {
    if (compiler->program->constant_m == 0) {
      if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
        ORC_ASM_CODE(compiler,"  for (j = 0; j < ex->params[ORC_VAR_A1]; j++) {\n");
      } else {
        ORC_ASM_CODE(compiler,"  for (j = 0; j < m; j++) {\n");
      }
    } else {
      ORC_ASM_CODE(compiler,"  for (j = 0; j < %d; j++) {\n",
          compiler->program->constant_m);
    }
    prefix = 2;

    for(i=0;i<ORC_N_VARIABLES;i++){
      OrcVariable *var = compiler->vars + i;
      if (var->name == NULL) continue;
      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"    var%d = ORC_PTR_OFFSET(ex->arrays[%d], ex->params[%d] * j);\n",
                i, i, i);
          } else {
            ORC_ASM_CODE(compiler,"    var%d = ORC_PTR_OFFSET(%s, %s_stride * j);\n",
                i, varnames[i], varnames[i]);
          }
          break;
        case ORC_VAR_TYPE_DEST:
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"    var%d = ORC_PTR_OFFSET(ex->arrays[%d], ex->params[%d] * j);\n",
                i, i, i);
          } else {
            ORC_ASM_CODE(compiler,"    var%d = ORC_PTR_OFFSET(%s, %s_stride * j);\n",
                i, varnames[i], varnames[i]);
          }
          break;
        default:
          break;
      }
    }
  } else {
    for(i=0;i<ORC_N_VARIABLES;i++){
      OrcVariable *var = compiler->vars + i;
      if (var->name == NULL) continue;
      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"  var%d = ex->arrays[%d];\n", i, i);
          } else {
            ORC_ASM_CODE(compiler,"  var%d = (void *)%s;\n", i, varnames[i]);
          }
          break;
        case ORC_VAR_TYPE_DEST:
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"  var%d = ex->arrays[%d];\n", i, i);
          } else {
            ORC_ASM_CODE(compiler,"  var%d = (void *)%s;\n", i, varnames[i]);
          }
          break;
        default:
          break;
      }
    }
  }

  ORC_ASM_CODE(compiler,"\n");
  if (compiler->program->constant_n == 0) {
    if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
      ORC_ASM_CODE(compiler,"%*s  for (i = 0; i < ex->n; i++) {\n", prefix, "");
    } else {
      ORC_ASM_CODE(compiler,"%*s  for (i = 0; i < n; i++) {\n", prefix, "");
    }
  } else {
    ORC_ASM_CODE(compiler,"%*s  for (i = 0; i < %d; i++) {\n",
        prefix, "",
        compiler->program->constant_n);
  }

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    ORC_ASM_CODE(compiler,"%*s    /* %d: %s */\n", prefix, "",
        j, insn->opcode->name);

    rule = insn->rule;
    if (rule) {
      ORC_ASM_CODE(compiler,"%*s", prefix, "");
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      ORC_COMPILER_ERROR(compiler, "No rule for: %s\n", opcode->name);
      compiler->error = TRUE;
    }
  }
  ORC_ASM_CODE(compiler,"\n");
  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        ORC_ASM_CODE(compiler,"%*s    var%d += %d;\n", prefix, "",
            i, var->size << loop_shift);
        break;
      default:
        break;
    }
  }

  ORC_ASM_CODE(compiler,"%*s  }\n", prefix, "");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  }\n");
  }

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        if (var->size == 2) {
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"  ex->accumulators[%d] = (var%d & 0xffff);\n",
                i - ORC_VAR_A1, i);
          } else {
            ORC_ASM_CODE(compiler,"  *%s = (var%d & 0xffff);\n",
                varnames[i], i);
          }
        } else {
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"  ex->accumulators[%d] = var%d;\n",
                i - ORC_VAR_A1, i);
          } else {
            ORC_ASM_CODE(compiler,"  *%s = var%d;\n",
                varnames[i], i);
          }
        }
        break;
      default:
        break;
    }
  }

  if (!(compiler->target_flags & ORC_TARGET_C_BARE)) {
    ORC_ASM_CODE(compiler,"}\n");
    ORC_ASM_CODE(compiler,"\n");
  }
}


/* rules */

static void
c_get_name (char *name, OrcCompiler *p, int var)
{
  switch (p->vars[var].vartype) {
    case ORC_VAR_TYPE_CONST:
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_ACCUMULATOR:
      sprintf(name, "var%d", var);
      break;
    case ORC_VAR_TYPE_SRC:
      sprintf(name, "_mem4_const(var%d+i)", var);
      break;
    case ORC_VAR_TYPE_DEST:
      sprintf(name, "_mem4(var%d+i)", var);
      break;
    default:
      ORC_COMPILER_ERROR(p, "bad vartype");
      sprintf(name, "ERROR");
      break;
  }
}

static void
c_get_name_float (char *name, OrcCompiler *p, int var)
{
  switch (p->vars[var].vartype) {
    case ORC_VAR_TYPE_CONST:
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_ACCUMULATOR:
      sprintf(name, "(*(float *)(&var%d))", var);
      break;
    case ORC_VAR_TYPE_SRC:
    case ORC_VAR_TYPE_DEST:
      sprintf(name, "((float *)var%d)[i]", var);
      break;
    default:
      ORC_COMPILER_ERROR(p, "bad vartype");
      sprintf(name, "ERROR");
      break;
  }
}

#if 0
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
    case 8:
      return "int64_t";
    default:
      return "ERROR";
  }
}
#endif


#define UNARY(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[20], src1[20]; \
\
  c_get_name (dest, p, insn->dest_args[0]); \
  c_get_name (src1, p, insn->src_args[0]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1); \
}

#define BINARY(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[20], src1[20], src2[20]; \
\
  c_get_name (dest, p, insn->dest_args[0]); \
  c_get_name (src1, p, insn->src_args[0]); \
  c_get_name (src2, p, insn->src_args[1]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1, src2); \
}

#define UNARYF(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[40], src1[40]; \
\
  c_get_name_float (dest, p, insn->dest_args[0]); \
  c_get_name_float (src1, p, insn->src_args[0]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1); \
}

#define BINARYF(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[40], src1[40], src2[40]; \
\
  c_get_name_float (dest, p, insn->dest_args[0]); \
  c_get_name_float (src1, p, insn->src_args[0]); \
  c_get_name_float (src2, p, insn->src_args[1]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1, src2); \
}

#define BINARYFL(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[40], src1[40], src2[40]; \
\
  c_get_name (dest, p, insn->dest_args[0]); \
  c_get_name_float (src1, p, insn->src_args[0]); \
  c_get_name_float (src2, p, insn->src_args[1]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1, src2); \
}

#define UNARYFL(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[40], src1[40]; \
\
  c_get_name (dest, p, insn->dest_args[0]); \
  c_get_name_float (src1, p, insn->src_args[0]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1); \
}

#define UNARYLF(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[40], src1[40]; \
\
  c_get_name_float (dest, p, insn->dest_args[0]); \
  c_get_name (src1, p, insn->src_args[0]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1); \
}

#define BINARY_SB(a,b) BINARY(a,b)
#define BINARY_UB(a,b) BINARY(a,b)
#define BINARY_SW(a,b) BINARY(a,b)
#define BINARY_UW(a,b) BINARY(a,b)
#define BINARY_SL(a,b) BINARY(a,b)
#define BINARY_UL(a,b) BINARY(a,b)
#define UNARY_SB(a,b) UNARY(a,b)
#define UNARY_UB(a,b) UNARY(a,b)
#define UNARY_SW(a,b) UNARY(a,b)
#define UNARY_UW(a,b) UNARY(a,b)
#define UNARY_SL(a,b) UNARY(a,b)
#define UNARY_UL(a,b) UNARY(a,b)
#define BINARY_BW(a,b) BINARY(a,b)
#define BINARY_WL(a,b) BINARY(a,b)
#define BINARY_LW(a,b) BINARY(a,b)
#define BINARY_WB(a,b) BINARY(a,b)
#define UNARY_BW(a,b) UNARY(a,b)
#define UNARY_WL(a,b) UNARY(a,b)
#define UNARY_LW(a,b) UNARY(a,b)
#define UNARY_WB(a,b) UNARY(a,b)

#define BINARY_F(a,b) BINARYF(a,b)
#define BINARY_FL(a,b) BINARYFL(a,b)
#define UNARY_F(a,b) UNARYF(a,b)
#define UNARY_FL(a,b) UNARYFL(a,b)
#define UNARY_LF(a,b) UNARYLF(a,b)

#include "opcodes-c64x-c.h"

#undef BINARY_SB
#undef BINARY_UB
#undef BINARY_SW
#undef BINARY_UW
#undef BINARY_SL
#undef BINARY_UL
#undef BINARY_F
#undef UNARY_SB
#undef UNARY_UB
#undef UNARY_SW
#undef UNARY_UW
#undef UNARY_SL
#undef UNARY_UL
#undef UNARY_F
#undef BINARY_BW
#undef BINARY_WL
#undef BINARY_LW
#undef BINARY_WB
#undef UNARY_BW
#undef UNARY_WL
#undef UNARY_LW
#undef UNARY_WB
#undef UNARY_FL
#undef UNARY_LF
#undef BINARY_FL

static void
c_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20];

  c_get_name (dest, p, insn->dest_args[0]);
  c_get_name (src1, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = %s + %s;\n", dest, dest, src1);
}

static void
c_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20];

  c_get_name (dest, p, insn->dest_args[0]);
  c_get_name (src1, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = %s + %s;\n", dest, dest, src1);
}

static void
c_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name (dest, p, insn->dest_args[0]);
  c_get_name (src1, p, insn->src_args[0]);
  c_get_name (src2, p, insn->src_args[1]);

  ORC_ASM_CODE(p,
      "    %s = %s + ORC_ABS((int32_t)(uint8_t)%s - (int32_t)(uint8_t)%s);\n",
      dest, dest, src1, src2);
}

static OrcTarget c_target = {
  "c64x-c",
  FALSE,
  ORC_GP_REG_BASE,
  orc_compiler_c64x_c_get_default_flags,
  orc_compiler_c64x_c_init,
  orc_compiler_c64x_c_assemble,
  { { 0 } },
  0,
  orc_target_c64x_c_get_asm_preamble,
};


void
orc_c64x_c_init (void)
{
  OrcRuleSet *rule_set;

  orc_target_register (&c_target);

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), &c_target, 0);

#define BINARY_SB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_UB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_SW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_UW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_SL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_UL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_F(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_F(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_BW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_WL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_LW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_WB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_BW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_WL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_LW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_WB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);

#define UNARY_FL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_FL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_LF(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);

#include "opcodes-c64x-c.h"

  orc_rule_register (rule_set, "accw", c_rule_accw, NULL);
  orc_rule_register (rule_set, "accl", c_rule_accl, NULL);
  orc_rule_register (rule_set, "accsadubl", c_rule_accsadubl, NULL);
}

