
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
orc_target_c_get_typedefs (void)
{
  return
    "#ifndef _ORC_INTEGER_TYPEDEFS_\n"
    "#define _ORC_INTEGER_TYPEDEFS_\n"
    "#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L\n"
    "#include <stdint.h>\n"
    "typedef int8_t orc_int8;\n"
    "typedef int16_t orc_int16;\n"
    "typedef int32_t orc_int32;\n"
    "typedef int64_t orc_int64;\n"
    "typedef uint8_t orc_uint8;\n"
    "typedef uint16_t orc_uint16;\n"
    "typedef uint32_t orc_uint32;\n"
    "typedef uint64_t orc_uint64;\n"
    "#elif defined(_MSC_VER)\n"
    "typedef signed __int8 orc_int8;\n"
    "typedef signed __int16 orc_int16;\n"
    "typedef signed __int32 orc_int32;\n"
    "typedef signed __int64 orc_int64;\n"
    "typedef unsigned __int8 orc_uint8;\n"
    "typedef unsigned __int16 orc_uint16;\n"
    "typedef unsigned __int32 orc_uint32;\n"
    "typedef unsigned __int64 orc_uint64;\n"
    "#else\n"
    "#include <limits.h>\n"
    "typedef signed char orc_int8;\n"
    "typedef short orc_int16;\n"
    "typedef int orc_int32;\n"
    "typedef unsigned char orc_uint8;\n"
    "typedef unsigned short orc_uint16;\n"
    "typedef unsigned int orc_uint32;\n"
    "#if INT_MAX == LONG_MAX\n"
    "typedef long long orc_int64;\n"
    "typedef unsigned long long orc_uint64;\n"
    "#else\n"
    "typedef long orc_int64;\n"
    "typedef unsigned long orc_uint64;\n"
    "#endif\n"
    "#endif\n"
    "typedef union { orc_int32 i; float f; } orc_union32;\n"
    "typedef union { orc_int64 i; double f; } orc_union64;\n"
    "#endif\n";
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
    "#define ORC_MIN_NORMAL (1.1754944909521339405e-38)\n"
    "#define ORC_DENORMAL(x) (((x) > -ORC_MIN_NORMAL && (x) < ORC_MIN_NORMAL) ? ((x)<0 ? (-0.0f) : (0.0f)) : (x))\n"
    "#define ORC_MINF(a,b) (isnan(a) ? a : isnan(b) ? b : ((a)<(b)) ? (a) : (b))\n"
    "#define ORC_MAXF(a,b) (isnan(a) ? a : isnan(b) ? b : ((a)>(b)) ? (a) : (b))\n"
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
  if (compiler->target_flags & ORC_TARGET_C_NOEXEC) {
    if (var < 48) {
      strcpy (s, varnames[var]);
    } else {
      sprintf(s, "t%d", var-32);
    }
  } else if (compiler->target_flags & ORC_TARGET_C_OPCODE) {
    if (var < ORC_VAR_S1) {
      sprintf(s, "ex->dest_ptrs[%d]", var-ORC_VAR_D1);
    } else {
      sprintf(s, "ex->src_ptrs[%d]", var-ORC_VAR_S1);
    }
  } else {
    sprintf(s, "ex->arrays[%d]", var);
  }
}

static void
get_varname_stride (char *s, OrcCompiler *compiler, int var)
{
  if (compiler->target_flags & ORC_TARGET_C_NOEXEC) {
    sprintf(s, "%s_stride", varnames[var]);
  } else {
    sprintf(s, "ex->params[%d]", var);
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

#if 0
  if (!(compiler->target_flags & ORC_TARGET_C_OPCODE)) {
    ORC_ASM_CODE(compiler,"%*s  int offset = 0;\n", prefix, "");
  }
#endif
  ORC_ASM_CODE(compiler,"%*s  int i;\n", prefix, "");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  int j;\n");
  }
  if (compiler->program->constant_n == 0) {
    if (!(compiler->target_flags & ORC_TARGET_C_NOEXEC) &&
        !(compiler->target_flags & ORC_TARGET_C_OPCODE)) {
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

  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
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
        if (!(var->last_use == -1 && var->first_use == 0)) {
          ORC_ASM_CODE(compiler,"  %s var%d;\n", c_get_type_name(var->size), i);
        }
        break;
      case ORC_VAR_TYPE_SRC:
        ORC_ASM_CODE(compiler,"  const %s *%s ptr%d;\n",
            c_get_type_name (var->size),
            (compiler->target_flags & ORC_TARGET_C_C99) ? "restrict " : "",
            i);
        break;
      case ORC_VAR_TYPE_DEST:
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
        if (compiler->target_flags & ORC_TARGET_C_NOEXEC) {
          ORC_ASM_CODE(compiler,"  const %s %s = %s;\n",
              var->is_float_param ? "float" : "int",
              varname, varnames[i]);
        } else if (compiler->target_flags & ORC_TARGET_C_OPCODE) {
          if (var->is_float_param) {
            ORC_ASM_CODE(compiler,"  const float %s = ((orc_union32 *)(ex->src_ptrs[%d]))->f;\n",
                varname, i - ORC_VAR_P1 + compiler->program->n_src_vars);
          } else {
            ORC_ASM_CODE(compiler,"  const int %s = ((orc_union32 *)(ex->src_ptrs[%d]))->i;\n",
                varname, i - ORC_VAR_P1 + compiler->program->n_src_vars);
          }
        } else {
          if (var->is_float_param) {
            ORC_ASM_CODE(compiler,"  const float %s = ((orc_union32 *)(ex->params+%d))->f;\n",
                varname, i);
          } else {
            ORC_ASM_CODE(compiler,"  const int %s = ex->params[%d];\n",
                varname, i);
          }
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

    for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
      OrcVariable *var = compiler->vars + i;
      if (var->name == NULL) continue;
      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
          {
            char s1[20], s2[20];
            get_varname(s1, compiler, i);
            get_varname_stride(s2, compiler, i);
            ORC_ASM_CODE(compiler,
                "    ptr%d = ORC_PTR_OFFSET(%s, %s * j);\n",
                i, s1, s2);
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
    for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
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

#if 0
  /* Load from source (and maybe destination) arrays */
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
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
#endif
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
      ORC_COMPILER_ERROR(compiler, "No rule for: %s on target %s", opcode->name,
          compiler->target->name);
      compiler->error = TRUE;
    }
  }
#if 0
  /* update pointers */
  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    OrcVariable *var = compiler->vars + i;
    if (var->name == NULL) continue;
    if (var->vartype == ORC_VAR_TYPE_DEST || var->vartype == ORC_VAR_TYPE_SRC) {
      ORC_ASM_CODE (compiler, "%*s    ptr%d++;\n", prefix, "", i);
    }
  }
#endif
  ORC_ASM_CODE(compiler,"%*s  }\n", prefix, "");
  if (compiler->program->is_2d) {
    ORC_ASM_CODE(compiler,"  }\n");
  }

  for(i=0;i<ORC_N_COMPILER_VARIABLES;i++){
    char varname[20];
    OrcVariable *var = compiler->vars + i;
    c_get_name_int (varname, compiler, i);
    if (var->name == NULL) continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        if (var->size == 2) {
          if (compiler->target_flags & ORC_TARGET_C_NOEXEC) {
            ORC_ASM_CODE(compiler,"  *%s = (%s & 0xffff);\n",
                varnames[i], varname);
          } else if (compiler->target_flags & ORC_TARGET_C_OPCODE) {
            ORC_ASM_CODE(compiler,"  ((orc_union32 *)ex->dest_ptrs[%d])->i = "
                "(%s + ((orc_union32 *)ex->dest_ptrs[%d])->i) & 0xffff;\n",
                i - ORC_VAR_A1, varname, i - ORC_VAR_A1);
          } else {
            ORC_ASM_CODE(compiler,"  ex->accumulators[%d] = (%s & 0xffff);\n",
                i - ORC_VAR_A1, varname);
          }
        } else {
          if (compiler->target_flags & ORC_TARGET_C_NOEXEC) {
            ORC_ASM_CODE(compiler,"  *%s = %s;\n",
                varnames[i], varname);
          } else if (compiler->target_flags & ORC_TARGET_C_OPCODE) {
            ORC_ASM_CODE(compiler,"  ((orc_union32 *)ex->dest_ptrs[%d])->i += %s;\n",
                i - ORC_VAR_A1, varname);
          } else {
            ORC_ASM_CODE(compiler,"  ex->accumulators[%d] = %s;\n",
                i - ORC_VAR_A1, varname);
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
    case ORC_VAR_TYPE_PARAM:
      sprintf(name, "var%d", var);
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
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_ACCUMULATOR:
    case ORC_VAR_TYPE_SRC:
    case ORC_VAR_TYPE_DEST:
      sprintf(name, "var%d.f", var);
      break;
    case ORC_VAR_TYPE_PARAM:
      sprintf(name, "var%d", var);
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
      return "orc_int8";
    case 2:
      return "orc_int16";
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
#define BINARY_SQ(a,b) BINARY(a,b)
#define BINARY_UQ(a,b) BINARY(a,b)
#define UNARY_SB(a,b) UNARY(a,b)
#define UNARY_UB(a,b) UNARY(a,b)
#define UNARY_SW(a,b) UNARY(a,b)
#define UNARY_UW(a,b) UNARY(a,b)
#define UNARY_SL(a,b) UNARY(a,b)
#define UNARY_UL(a,b) UNARY(a,b)
#define UNARY_SQ(a,b) UNARY(a,b)
#define UNARY_UQ(a,b) UNARY(a,b)
#define BINARY_BW(a,b) BINARY(a,b)
#define BINARY_WL(a,b) BINARY(a,b)
#define BINARY_LW(a,b) BINARY(a,b)
#define BINARY_WB(a,b) BINARY(a,b)
#define UNARY_BW(a,b) UNARY(a,b)
#define UNARY_WL(a,b) UNARY(a,b)
#define UNARY_LQ(a,b) UNARY(a,b)
#define UNARY_QL(a,b) UNARY(a,b)
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
#undef BINARY_SQ
#undef BINARY_UQ
#undef BINARY_F
#undef UNARY_SB
#undef UNARY_UB
#undef UNARY_SW
#undef UNARY_UW
#undef UNARY_SL
#undef UNARY_UL
#undef UNARY_SQ
#undef UNARY_UQ
#undef UNARY_F
#undef BINARY_BW
#undef BINARY_WL
#undef BINARY_LW
#undef BINARY_WB
#undef UNARY_BW
#undef UNARY_WL
#undef UNARY_LQ
#undef UNARY_QL
#undef UNARY_LW
#undef UNARY_WB
#undef UNARY_FL
#undef UNARY_LF
#undef BINARY_FL

static void
c_rule_loadX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->target_flags & ORC_TARGET_C_OPCODE &&
      !(insn->flags & ORC_INSN_FLAG_ADDED)) {
    ORC_ASM_CODE(p,"    var%d = ptr%d[offset + i];\n", insn->dest_args[0],
        insn->src_args[0]);
  } else {
    ORC_ASM_CODE(p,"    var%d = ptr%d[i];\n", insn->dest_args[0],
        insn->src_args[0]);
  }
}

static void
c_rule_loadoffX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->target_flags & ORC_TARGET_C_OPCODE &&
      !(insn->flags & ORC_INSN_FLAG_ADDED)) {
    ORC_ASM_CODE(p,"    var%d = ptr%d[offset + i+var%d];\n", insn->dest_args[0],
        insn->src_args[0], insn->src_args[1]);
  } else {
    ORC_ASM_CODE(p,"    var%d = ptr%d[i+var%d];\n", insn->dest_args[0],
        insn->src_args[0], insn->src_args[1]);
  }
}

static void
c_rule_loadupdb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->target_flags & ORC_TARGET_C_OPCODE &&
      !(insn->flags & ORC_INSN_FLAG_ADDED)) {
    ORC_ASM_CODE(p,"    var%d = ptr%d[(offset + i)>>1];\n", insn->dest_args[0],
        insn->src_args[0]);
  } else {
    ORC_ASM_CODE(p,"    var%d = ptr%d[i>>1];\n", insn->dest_args[0],
        insn->src_args[0]);
  }
}

static void
c_rule_loadupib (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->target_flags & ORC_TARGET_C_OPCODE &&
      !(insn->flags & ORC_INSN_FLAG_ADDED)) {
    ORC_ASM_CODE(p,"    var%d = ((offset + i)&1) ? ((orc_uint8)ptr%d[(offset + i)>>1] + (orc_uint8)ptr%d[((offset + i)>>1)+1] + 1)>>1 : ptr%d[(offset + i)>>1];\n",
        insn->dest_args[0], insn->src_args[0], insn->src_args[0],
        insn->src_args[0]);
  } else {
    ORC_ASM_CODE(p,"    var%d = (i&1) ? ((orc_uint8)ptr%d[i>>1] + (orc_uint8)ptr%d[(i>>1)+1] + 1)>>1 : ptr%d[i>>1];\n",
        insn->dest_args[0], insn->src_args[0], insn->src_args[0],
        insn->src_args[0]);
  }
}

static void
c_rule_storeX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->target_flags & ORC_TARGET_C_OPCODE &&
      !(insn->flags & ORC_INSN_FLAG_ADDED)) {
    ORC_ASM_CODE(p,"    ptr%d[offset + i] = var%d;\n", insn->dest_args[0],
        insn->src_args[0]);
  } else {
    ORC_ASM_CODE(p,"    ptr%d[i] = var%d;\n", insn->dest_args[0],
        insn->src_args[0]);
  }
}

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
      "    %s = %s + ORC_ABS((orc_int32)(orc_uint8)%s - (orc_int32)(orc_uint8)%s);\n",
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

static void
c_rule_splatbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src, p, insn->src_args[0]);

  ORC_ASM_CODE(p,"    %s = ((%s&0xff) << 8) | (%s&0xff);\n", dest, src, src);
}

static void
c_rule_splatbl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src, p, insn->src_args[0]);

  ORC_ASM_CODE(p,
      "    %s = ((%s&0xff) << 24) | ((%s&0xff)<<16) | ((%s&0xff) << 8) | (%s&0xff);\n",
      dest, src, src, src, src);
}

static void
c_rule_splatw0q (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src, p, insn->src_args[0]);

  ORC_ASM_CODE(p,
      "    %s = ((((orc_uint64)%s)>>48) << 48) | "
      "((((orc_uint64)%s)>>48)<<32) | "
      "((((orc_uint64)%s)>>48) << 16) | "
      "((((orc_uint64)%s)>>48));\n",
      dest, src, src, src, src);
}

static void
c_rule_div255w (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src, p, insn->src_args[0]);

  ORC_ASM_CODE(p,
      "    %s = ((uint16_t)(((orc_uint16)(%s+128)) + (((orc_uint16)(%s+128))>>8)))>>8;\n",
      dest, src, src);
}

static void
c_rule_divluw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  char dest[20], src1[20], src2[20];

  c_get_name_int (dest, p, insn->dest_args[0]);
  c_get_name_int (src1, p, insn->src_args[0]);
  c_get_name_int (src2, p, insn->src_args[1]);

  ORC_ASM_CODE(p,
      "    %s = ((%s&0xff) == 0) ? 255 : ORC_CLAMP_UB(((uint16_t)%s)/((uint16_t)%s&0xff));\n",
      dest, src2, src1, src2);
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
#define BINARY_SQ(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_UQ(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_F(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_SQ(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_UQ(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_F(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_BW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_WL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_LW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_WB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_BW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_WL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_LQ(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_QL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_LW(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_WB(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);

#define UNARY_FL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define BINARY_FL(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);
#define UNARY_LF(a,b) orc_rule_register (rule_set, #a , c_rule_ ## a, NULL);

#include "opcodes.h"

  orc_rule_register (rule_set, "loadb", c_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadw", c_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadl", c_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadq", c_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadoffb", c_rule_loadoffX, NULL);
  orc_rule_register (rule_set, "loadoffw", c_rule_loadoffX, NULL);
  orc_rule_register (rule_set, "loadoffl", c_rule_loadoffX, NULL);
  orc_rule_register (rule_set, "loadupdb", c_rule_loadupdb, NULL);
  orc_rule_register (rule_set, "loadupib", c_rule_loadupib, NULL);
  orc_rule_register (rule_set, "storeb", c_rule_storeX, NULL);
  orc_rule_register (rule_set, "storew", c_rule_storeX, NULL);
  orc_rule_register (rule_set, "storel", c_rule_storeX, NULL);
  orc_rule_register (rule_set, "storeq", c_rule_storeX, NULL);

  orc_rule_register (rule_set, "accw", c_rule_accw, NULL);
  orc_rule_register (rule_set, "accl", c_rule_accl, NULL);
  orc_rule_register (rule_set, "accsadubl", c_rule_accsadubl, NULL);
  orc_rule_register (rule_set, "splitlw", c_rule_splitlw, NULL);
  orc_rule_register (rule_set, "splitwb", c_rule_splitwb, NULL);
  orc_rule_register (rule_set, "splatbw", c_rule_splatbw, NULL);
  orc_rule_register (rule_set, "splatbl", c_rule_splatbl, NULL);
  orc_rule_register (rule_set, "splatw0q", c_rule_splatw0q, NULL);
  orc_rule_register (rule_set, "div255w", c_rule_div255w, NULL);
  orc_rule_register (rule_set, "divluw", c_rule_divluw, NULL);
}

