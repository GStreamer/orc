
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orc/orc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

static const char *c_get_type_name (int size);
static void c_get_name (char *name, OrcCompiler *p, int var);
static void c_get_name_int (char *name, OrcCompiler *p, int var);

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
orc_target_c_get_asm_preamble (void)
{
  return "\n"
    "/* begin Orc C target preamble */\n"
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
    "#define ORC_AS_FLOAT(x) (((union { int i; float f; } *)(&x))->f)\n"
    "typedef union { int32_t i; float f; } orc_union32;\n"
    "typedef union { int64_t i; double f; } orc_union64;\n"
    "/* end Orc C target preamble */\n\n";
}

unsigned int
orc_compiler_c_get_default_flags (void)
{
  return 0;
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
  "t13", "t14", "t15", "t16",
};

static void
get_varname (char *s, OrcCompiler *compiler, int var)
{
  if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
    sprintf(s, "ex->arrays[%d]", var);
  } else {
    if (var < 48) {
      strcpy (s, varnames[var]);
    } else {
      sprintf(s, "t%d", var-32);
    }
  }
}

static void
get_varname_stride (char *s, OrcCompiler *compiler, int var)
{
  if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
    sprintf(s, "ex->params[%d]", var);
  } else {
    sprintf(s, "%s_stride", varnames[var]);
  }
}

void
orc_compiler_c_assemble (OrcCompiler *compiler)
{
  int i;
  int j;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;
  int prefix = 0;

  if (!(compiler->target_flags & ORC_TARGET_C_BARE)) {
    ORC_ASM_CODE(compiler,"void\n");
    ORC_ASM_CODE(compiler,"%s (OrcExecutor *ex)\n", compiler->program->name);
    ORC_ASM_CODE(compiler,"{\n");
  }

  ORC_ASM_CODE(compiler,"%*s  int i;\n", prefix, "");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  int j;\n");
  }
  if (compiler->program->constant_n == 0) {
    if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
      ORC_ASM_CODE(compiler,"  int n = ex->n;\n");
    }
  } else {
    ORC_ASM_CODE(compiler,"  int n = %d;\n", compiler->program->constant_n);
  }
  if (compiler->program->is_2d) {
    if (compiler->program->constant_m == 0) {
      if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
        ORC_ASM_CODE(compiler,"  int m = ex->params[ORC_VAR_A1];\n");
      }
    } else {
      ORC_ASM_CODE(compiler,"  int m = %d;\n", compiler->program->constant_m);
    }
  }

  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    char varname[20];
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_CONST:
        if (var->size >= 4) {
          if (var->value == 0x80000000) {
            ORC_ASM_CODE(compiler,"  const %s var%d = { 0x80000000 };\n",
                c_get_type_name(var->size), i);
          } else {
            ORC_ASM_CODE(compiler,"  const %s var%d = { %d };\n",
                c_get_type_name(var->size), i, var->value);
          }
        } else {
          if (var->value == 0x80000000) {
            ORC_ASM_CODE(compiler,"  const %s var%d = 0x80000000;\n",
                c_get_type_name(var->size), i);
          } else {
            ORC_ASM_CODE(compiler,"  const %s var%d = %d;\n",
                c_get_type_name(var->size), i, var->value);
          }
        }
        break;
      case ORC_VAR_TYPE_TEMP:
        ORC_ASM_CODE(compiler,"  %s var%d;\n", c_get_type_name(var->size), i);
        break;
      case ORC_VAR_TYPE_SRC:
        ORC_ASM_CODE(compiler,"  %s var%d;\n", c_get_type_name(var->size), i);
        ORC_ASM_CODE(compiler,"  const %s *%s ptr%d;\n",
            c_get_type_name (var->size),
            (compiler->target_flags & ORC_TARGET_C_C99) ? "restrict " : "",
            i);
        break;
      case ORC_VAR_TYPE_DEST:
        ORC_ASM_CODE(compiler,"  %s var%d;\n", c_get_type_name(var->size), i);
        ORC_ASM_CODE(compiler,"  %s *%s ptr%d;\n",
            c_get_type_name (var->size),
            (compiler->target_flags & ORC_TARGET_C_C99) ? "restrict " : "",
            i);
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        if (var->size >= 4) {
          ORC_ASM_CODE(compiler,"  %s var%d =  { 0 };\n",
              c_get_type_name (var->size),
              i);
        } else {
          ORC_ASM_CODE(compiler,"  %s var%d = 0;\n",
              c_get_type_name (var->size),
              i);
        }
        break;
      case ORC_VAR_TYPE_PARAM:
        c_get_name (varname, compiler, i);
        if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
          if (var->size >= 4) {
            ORC_ASM_CODE(compiler,"  const %s %s = *(%s *)(ex->params + %d);\n",
                c_get_type_name (var->size), varname,
                c_get_type_name (var->size), i);
          } else {
            ORC_ASM_CODE(compiler,"  const %s %s = ex->params[%d];\n",
                c_get_type_name (var->size), varname, i);
          }
        } else {
          ORC_ASM_CODE(compiler,"  const %s %s = %s;\n",
              c_get_type_name (var->size), varname, varnames[i]);
        }
        break;
      default:
        ORC_COMPILER_ERROR(compiler, "bad vartype");
        break;
    }
  }

  ORC_ASM_CODE(compiler,"\n");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  for (j = 0; j < m; j++) {\n");
    prefix = 2;

    for(i=0;i<ORC_N_VARIABLES;i++){
      OrcVariable *var = compiler->vars + i;
      if (var->name == NULL) continue;
      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
          {
            char s1[20], s2[20];
            get_varname(s1, compiler, i);
            get_varname_stride(s2, compiler, i);
            switch (var->sampling_type) {
              case ORC_SAMPLE_REGULAR:
                ORC_ASM_CODE(compiler,
                    "    ptr%d = ORC_PTR_OFFSET(%s, %s * j);\n",
                    i, s1, s2);
                break;
              case ORC_SAMPLE_TRANSPOSED:
                ORC_ASM_CODE(compiler,
                    "    ptr%d = ORC_PTR_OFFSET(%s, %d * j);\n",
                    i, s1, var->size);
                break;
              case ORC_SAMPLE_NEAREST:
              case ORC_SAMPLE_BILINEAR:
              case ORC_SAMPLE_FOUR_TAP:
                ORC_ASM_CODE(compiler,
                    "    ptr%d = ORC_PTR_OFFSET(%s, %s * j);\n",
                    i, s1, s2);
                break;
              default:
                ORC_COMPILER_ERROR(compiler, "eeek");
            }
          }
          break;
        case ORC_VAR_TYPE_DEST:
          {
            char s1[20], s2[20];
            get_varname(s1, compiler, i),
            get_varname_stride(s2, compiler, i),
            ORC_ASM_CODE(compiler,
                "    ptr%d = ORC_PTR_OFFSET(%s, %s * j);\n",
                i, s1, s2);
          }
          break;
        default:
          break;
      }
    }
  } else {
    for(i=0;i<ORC_N_VARIABLES;i++){
      OrcVariable *var = compiler->vars + i;
      char s[20];
      if (var->name == NULL) continue;
      get_varname(s, compiler, i);
      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
          ORC_ASM_CODE(compiler,"  ptr%d = (%s *)%s;\n", i,
              c_get_type_name (var->size), s);
          break;
        case ORC_VAR_TYPE_DEST:
          ORC_ASM_CODE(compiler,"  ptr%d = (%s *)%s;\n", i,
              c_get_type_name (var->size), s);
          break;
        default:
          break;
      }
    }
  }

  ORC_ASM_CODE(compiler,"\n");
  ORC_ASM_CODE(compiler,"%*s  for (i = 0; i < n; i++) {\n", prefix, "");

  /* Load from source (and maybe destination) arrays */
  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    char s[20];
    if (var->name == NULL) continue;
    c_get_name(s, compiler, i);
    if (var->vartype == ORC_VAR_TYPE_SRC) {
      ORC_ASM_CODE (compiler, "%*s    %s = *ptr%d;\n", prefix, "", s, i);
      ORC_ASM_CODE (compiler, "%*s    ptr%d++;\n", prefix, "", i);
    }
    if (var->vartype == ORC_VAR_TYPE_DEST && var->load_dest) {
      ORC_ASM_CODE (compiler, "%*s    %s = *ptr%d;\n", prefix, "", s, i);
    }
  }
  /* Emit instructions */
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
  /* Store to destination arrays */
  for(i=0;i<ORC_N_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    char s[20];
    if (var->name == NULL) continue;
    c_get_name(s, compiler, i);
    if (var->vartype == ORC_VAR_TYPE_DEST) {
      ORC_ASM_CODE (compiler, "%*s    *ptr%d = %s;\n", prefix, "", i, s);
      ORC_ASM_CODE (compiler, "%*s    ptr%d++;\n", prefix, "", i);
    }
  }
  ORC_ASM_CODE(compiler,"%*s  }\n", prefix, "");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  }\n");
  }

  for(i=0;i<ORC_N_VARIABLES;i++){
    char varname[20];
    OrcVariable *var = compiler->vars + i;
    c_get_name_int (varname, compiler, i);
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        if (var->size == 2) {
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"  ex->accumulators[%d] = (%s & 0xffff);\n",
                i - ORC_VAR_A1, varname);
          } else {
            ORC_ASM_CODE(compiler,"  *%s = (%s & 0xffff);\n",
                varnames[i], varname);
          }
        } else {
          if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC)) {
            ORC_ASM_CODE(compiler,"  ex->accumulators[%d] = %s;\n",
                i - ORC_VAR_A1, varname);
          } else {
            ORC_ASM_CODE(compiler,"  *%s = %s;\n",
                varnames[i], varname);
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
    case ORC_VAR_TYPE_SRC:
    case ORC_VAR_TYPE_DEST:
      sprintf(name, "var%d", var);
      break;
    default:
      ORC_COMPILER_ERROR(p, "bad vartype");
      sprintf(name, "ERROR");
      break;
  }
}

static void
c_get_name_int (char *name, OrcCompiler *p, int var)
{
  switch (p->vars[var].vartype) {
    case ORC_VAR_TYPE_CONST:
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_ACCUMULATOR:
    case ORC_VAR_TYPE_SRC:
    case ORC_VAR_TYPE_DEST:
      if (p->vars[var].size >= 4) {
        sprintf(name, "var%d.i", var);
      } else {
        sprintf(name, "var%d", var);
      }
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
    case ORC_VAR_TYPE_SRC:
    case ORC_VAR_TYPE_DEST:
      sprintf(name, "var%d.f", var);
      break;
    default:
      ORC_COMPILER_ERROR(p, "bad vartype");
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
      return "orc_union32";
    case 8:
      return "orc_union64";
    default:
      return "ERROR";
  }
}


#define UNARY(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[20], src1[20]; \
\
  c_get_name_int (dest, p, insn->dest_args[0]); \
  c_get_name_int (src1, p, insn->src_args[0]); \
 \
  ORC_ASM_CODE(p,"    %s = " op ";\n", dest, src1); \
}

#define BINARY(name,op) \
static void \
c_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  char dest[20], src1[20], src2[20]; \
\
  c_get_name_int (dest, p, insn->dest_args[0]); \
  c_get_name_int (src1, p, insn->src_args[0]); \
  c_get_name_int (src2, p, insn->src_args[1]); \
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
  c_get_name_int (dest, p, insn->dest_args[0]); \
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
  c_get_name_int (dest, p, insn->dest_args[0]); \
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
  c_get_name_int (src1, p, insn->src_args[0]); \
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

#include "opcodes.h"

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

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src1, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = %s + %s;\n", dest, dest, src1);
}

static void
c_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src1, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = %s + %s;\n", dest, dest, src1);
}

static void
c_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src1, p, insn->src_args[0]);
  c_get_name_int (src2, p, insn->src_args[1]);

  ORC_ASM_CODE(p,
      "    %s = %s + ORC_ABS((int32_t)(uint8_t)%s - (int32_t)(uint8_t)%s);\n",
      dest, dest, src1, src2);
}

static void
c_rule_splitlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest1[20], dest2[20], src[20];

  c_get_name_int (dest1, p, insn->dest_args[0]);
  c_get_name_int (dest2, p, insn->dest_args[1]);
  c_get_name_int (src, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = (%s >> 16) & 0xffff;\n", dest1, src);
  ORC_ASM_CODE(p,"    %s = %s & 0xffff;\n", dest2, src);
}

static void
c_rule_splitwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest1[20], dest2[20], src[20];

  c_get_name_int (dest1, p, insn->dest_args[0]);
  c_get_name_int (dest2, p, insn->dest_args[1]);
  c_get_name_int (src, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = (%s >> 8) & 0xff;\n", dest1, src);
  ORC_ASM_CODE(p,"    %s = %s & 0xff;\n", dest2, src);
}

static OrcTarget c_target = {
  "c",
  FALSE,
  ORC_GP_REG_BASE,
  orc_compiler_c_get_default_flags,
  orc_compiler_c_init,
  orc_compiler_c_assemble,
  { { 0 } },
  0,
  orc_target_c_get_asm_preamble,
};


void
orc_c_init (void)
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

#include "opcodes.h"

  orc_rule_register (rule_set, "accw", c_rule_accw, NULL);
  orc_rule_register (rule_set, "accl", c_rule_accl, NULL);
  orc_rule_register (rule_set, "accsadubl", c_rule_accsadubl, NULL);
  orc_rule_register (rule_set, "splitlw", c_rule_splitlw, NULL);
  orc_rule_register (rule_set, "splitwb", c_rule_splitwb, NULL);
}

