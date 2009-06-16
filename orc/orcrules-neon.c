
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcarm.h>
#include <orc/orcdebug.h>

#include "neon.h"

const char *orc_neon_reg_name (int reg)
{
  static const char *vec_regs[] = {
    "d0", "d1", "d2", "d3",
    "d4", "d5", "d6", "d7",
    "d8", "d9", "d10", "d11",
    "d12", "d13", "d14", "d15",
    "d16", "d17", "d18", "d19",
    "d20", "d21", "d22", "d23",
    "d24", "d25", "d26", "d27",
    "d28", "d29", "d30", "d31",
  };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  return vec_regs[reg&0x1f];
}

const char *orc_neon_reg_name_quad (int reg)
{
  static const char *vec_regs[] = {
    "q0", "ERROR", "q1", "ERROR",
    "q2", "ERROR", "q3", "ERROR",
    "q4", "ERROR", "q5", "ERROR",
    "q6", "ERROR", "q7", "ERROR",
    "q8", "ERROR", "q9", "ERROR",
    "q10", "ERROR", "q11", "ERROR",
    "q12", "ERROR", "q13", "ERROR",
    "q14", "ERROR", "q15", "ERROR",
  };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  return vec_regs[reg&0x1f];
}

static void
orc_neon_emit_binary (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name (src1),
      orc_neon_reg_name (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

#define NEON_BINARY(code,a,b,c) \
  ((code) | \
   (((a)&0xf)<<12) | \
   ((((a)>>4)&0x1)<<22) | \
   (((b)&0xf)<<16) | \
   ((((b)>>4)&0x1)<<7) | \
   (((c)&0xf)<<0) | \
   ((((c)>>4)&0x1)<<5))

static void
orc_neon_emit_binary_long (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name_quad (dest), orc_neon_reg_name (src1),
      orc_neon_reg_name (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

#if 0
static void
orc_neon_emit_binary_narrow (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1, int src2)
{
  ORC_ASSERT((code & 0x004ff0af) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name_quad (src1),
      orc_neon_reg_name_quad (src2));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<16;
  code |= ((src1>>4)&0x1)<<7;
  code |= (src2&0xf)<<0;
  code |= ((src2>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}
#endif

static void
orc_neon_emit_unary (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_unary_long (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name_quad (dest), orc_neon_reg_name (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

static void
orc_neon_emit_unary_narrow (OrcCompiler *p, const char *name, unsigned int code,
    int dest, int src1)
{
  ORC_ASSERT((code & 0x0040f02f) == 0);

  ORC_ASM_CODE(p,"  %s %s, %s\n", name,
      orc_neon_reg_name (dest), orc_neon_reg_name_quad (src1));
  code |= (dest&0xf)<<12;
  code |= ((dest>>4)&0x1)<<22;
  code |= (src1&0xf)<<0;
  code |= ((src1>>4)&0x1)<<5;
  orc_arm_emit (p, code);
}

void
orc_neon_emit_mov (OrcCompiler *compiler, int dest, int src)
{
  orc_neon_emit_binary (compiler, "vorr", 0xf2200110,
      dest, src, src);
}

void
orc_neon_loadb (OrcCompiler *compiler, OrcVariable *var, int update)
{
  uint32_t code;
  int i;

  if (var->is_aligned && compiler->loop_shift == 3) {
    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_neon_reg_name (var->alloc),
        orc_arm_reg_name (var->ptr_register),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (var->ptr_register&0xf) << 16;
    code |= (var->alloc&0xf) << 12;
    code |= ((var->alloc>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else if (compiler->loop_shift == 3) {
    orc_arm_emit_sub (compiler, var->ptr_register, var->ptr_register,
        var->ptr_offset);

    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_neon_reg_name (var->alloc),
        orc_arm_reg_name (var->ptr_register),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (var->ptr_register&0xf) << 16;
    code |= (var->alloc&0xf) << 12;
    code |= ((var->alloc>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);

    update = 0;
    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_neon_reg_name (var->alloc + 1),
        orc_arm_reg_name (var->ptr_register),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (var->ptr_register&0xf) << 16;
    code |= ((var->alloc+1)&0xf) << 12;
    code |= (((var->alloc+1)>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);

    ORC_ASM_CODE(compiler,"  vtbl.8 %s, {%s,%s}, %s\n",
        orc_neon_reg_name (var->alloc),
        orc_neon_reg_name (var->alloc),
        orc_neon_reg_name (var->alloc + 1),
        orc_neon_reg_name (var->mask_alloc));
    code = NEON_BINARY(0xf3b00900, var->alloc, var->alloc, var->mask_alloc);
    //code |= (var->alloc&0xf) << 16;
    //code |= ((var->alloc>>4)&0x1) << 7;
    //code |= (var->alloc&0xf) << 12;
    //code |= ((var->alloc>>4)&0x1) << 22;
    orc_arm_emit (compiler, code);

    orc_arm_emit_add (compiler, var->ptr_register, var->ptr_register,
        var->ptr_offset);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.8 %s[%d], [%s]%s\n",
          orc_neon_reg_name (var->alloc), i,
          orc_arm_reg_name (var->ptr_register),
          update ? "!" : "");
      code = NEON_BINARY(0xf4a0000d, var->alloc, var->ptr_register, 0);
      //code |= (src1&0xf) << 16;
      //code |= (var->alloc&0xf) << 12;
      //code |= ((var->alloc>>4)&0x1) << 22;
      code |= i << 5;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_loadw (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 2) {
    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_neon_reg_name (dest),
        orc_arm_reg_name (src1),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (src1&0xf) << 16;
    code |= (dest&0xf) << 12;
    code |= ((dest>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.16 %s[%d], [%s]%s\n",
          orc_neon_reg_name (dest), i,
          orc_arm_reg_name (src1),
          update ? "!" : "");
      code = 0xf4a0040d;
      code |= (src1&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= i << 6;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_loadl (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 1) {
    ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
        orc_neon_reg_name (dest),
        orc_arm_reg_name (src1),
        update ? "!" : "");
    code = 0xf42007cd;
    code |= (src1&0xf) << 16;
    code |= (dest&0xf) << 12;
    code |= ((dest>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vld1.32 %s[%d], [%s]%s\n",
          orc_neon_reg_name (dest), i,
          orc_arm_reg_name (src1),
          update ? "!" : "");
      code = 0xf4a0080d;
      code |= (src1&0xf) << 16;
      code |= (dest&0xf) << 12;
      code |= ((dest>>4)&0x1) << 22;
      code |= i<<7;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_loadq (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned)
{
  uint32_t code;

  ORC_ASM_CODE(compiler,"  vld1.64 %s, [%s]%s\n",
      orc_neon_reg_name (dest),
      orc_arm_reg_name (src1),
      update ? "!" : "");
  code = 0xf42007cd;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= ((dest>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

void
orc_neon_emit_neg (OrcCompiler *compiler, int dest)
{
  orc_neon_emit_unary(compiler, "vneg.s8", 0xf3b10380, dest, dest);
}

void
orc_neon_storeb (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 3) {
    ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
        orc_neon_reg_name (src1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf40007cd;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.8 %s[%d], [%s]%s\n",
          orc_neon_reg_name (src1), i,
          orc_arm_reg_name (dest),
          update ? "!" : "");
      code = 0xf480000d;
      code |= (dest&0xf) << 16;
      code |= (src1&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= i<<5;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_storew (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 2) {
    ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
        orc_neon_reg_name (src1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf40007cd;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.16 %s[%d], [%s]%s\n",
          orc_neon_reg_name (src1), i,
          orc_arm_reg_name (dest),
          update ? "!" : "");
      code = 0xf480040d;
      code |= (dest&0xf) << 16;
      code |= (src1&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= i<<6;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_storel (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  uint32_t code;
  int i;

  if (is_aligned && compiler->loop_shift == 2) {
    ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
        orc_neon_reg_name (src1),
        orc_arm_reg_name (dest),
        update ? "!" : "");
    code = 0xf40007cd;
    code |= (dest&0xf) << 16;
    code |= (src1&0xf) << 12;
    code |= ((src1>>4)&0x1) << 22;
    code |= (!update) << 1;
    orc_arm_emit (compiler, code);
  } else {
    for(i=0;i<(1<<compiler->loop_shift);i++){
      ORC_ASM_CODE(compiler,"  vst1.32 %s[%d], [%s]%s\n",
          orc_neon_reg_name (src1), i,
          orc_arm_reg_name (dest),
          update ? "!" : "");
      code = 0xf480080d;
      code |= (dest&0xf) << 16;
      code |= (src1&0xf) << 12;
      code |= ((src1>>4)&0x1) << 22;
      code |= i<<7;
      code |= (!update) << 1;
      orc_arm_emit (compiler, code);
    }
  }
}

void
orc_neon_storeq (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned)
{
  uint32_t code;

  ORC_ASM_CODE(compiler,"  vst1.64 %s, [%s]%s\n",
      orc_neon_reg_name (src1),
      orc_arm_reg_name (dest),
      update ? "!" : "");
  code = 0xf40007cd;
  code |= (dest&0xf) << 16;
  code |= (src1&0xf) << 12;
  code |= ((src1>>4)&0x1) << 22;
  code |= (!update) << 1;
  orc_arm_emit (compiler, code);
}

static int
orc_neon_get_const_shift (unsigned int value)
{
  int shift = 0;

  while((value & 0xff) != value) {
    shift++;
    value >>= 1;
  }
  return shift;
}

void
orc_neon_emit_loadib (OrcCompiler *compiler, int reg, int value)
{
  uint32_t code;

  if (value == 0) {
    orc_neon_emit_binary (compiler, "veor", 0xf3000110, reg, reg, reg);
    return;
  }

  value &= 0xff;
  ORC_ASM_CODE(compiler,"  vmov.i8 %s, #%d\n",
      orc_neon_reg_name (reg), value);
  code = 0xf2800e10;
  code |= (reg&0xf) << 12;
  code |= ((reg>>4)&0x1) << 22;
  code |= (value&0xf) << 0;
  code |= (value&0x70) << 12;
  code |= (value&0x80) << 17;
  orc_arm_emit (compiler, code);
}

void
orc_neon_emit_loadiw (OrcCompiler *compiler, int reg, int value)
{
  uint32_t code;
  int shift;
  int neg = FALSE;

  if (value == 0) {
    orc_neon_emit_binary (compiler, "veor", 0xf3000110, reg, reg, reg);
    return;
  }

  if (value < 0) {
    neg = TRUE;
    value = -value;
  }
  shift = orc_neon_get_const_shift (value);
  if ((value & (0xff<<shift)) == value) {
    value >>= shift;
    if (neg) {
      ORC_ASM_CODE(compiler,"  vmvn.i16 %s, #%d\n",
          orc_neon_reg_name (reg), value);
      code = 0xf2800830;
    } else {
      ORC_ASM_CODE(compiler,"  vmov.i16 %s, #%d\n",
          orc_neon_reg_name (reg), value);
      code = 0xf2800810;
    }
    code |= (reg&0xf) << 12;
    code |= ((reg>>4)&0x1) << 22;
    code |= (value&0xf) << 0;
    code |= (value&0x70) << 12;
    code |= (value&0x80) << 17;
    orc_arm_emit (compiler, code);

    if (shift > 0) {
      ORC_ASM_CODE(compiler,"  vshl.i16 %s, %s, #%d\n",
          orc_neon_reg_name (reg), orc_neon_reg_name (reg), shift);
      code = 0xf2900510;
      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (reg&0xf) << 0;
      code |= ((reg>>4)&0x1) << 5;
      code |= (shift&0xf) << 16;
      orc_arm_emit (compiler, code);
    }

    return;
  }

  ORC_COMPILER_ERROR(compiler, "unimplemented load of constant %d", value);
}

void
orc_neon_emit_loadil (OrcCompiler *compiler, int reg, int value)
{
  uint32_t code;
  int shift;
  int neg = FALSE;

  if (value == 0) {
    orc_neon_emit_binary (compiler, "veor", 0xf3000110, reg, reg, reg);
    return;
  }

  if (value < 0) {
    neg = TRUE;
    value = -value;
  }
  shift = orc_neon_get_const_shift (value);
  if ((value & (0xff<<shift)) == value) {
    value >>= shift;
    if (neg) {
      ORC_ASM_CODE(compiler,"  vmvn.i32 %s, #%d\n",
          orc_neon_reg_name (reg), value);
      code = 0xf2800030;
    } else {
      ORC_ASM_CODE(compiler,"  vmov.i32 %s, #%d\n",
          orc_neon_reg_name (reg), value);
      code = 0xf2800010;
    }
    code |= (reg&0xf) << 12;
    code |= ((reg>>4)&0x1) << 22;
    code |= (value&0xf) << 0;
    code |= (value&0x70) << 12;
    code |= (value&0x80) << 17;
    orc_arm_emit (compiler, code);

    if (shift > 0) {
      ORC_ASM_CODE(compiler,"  vshl.i32 %s, %s, #%d\n",
          orc_neon_reg_name (reg), orc_neon_reg_name (reg), shift);
      code = 0xf2a00510;
      code |= (reg&0xf) << 12;
      code |= ((reg>>4)&0x1) << 22;
      code |= (reg&0xf) << 0;
      code |= ((reg>>4)&0x1) << 5;
      code |= (shift&0xf) << 16;
      orc_arm_emit (compiler, code);
    }

    return;
  }

  ORC_COMPILER_ERROR(compiler, "unimplemented load of constant %d", value);
}

void
orc_neon_emit_loadpb (OrcCompiler *compiler, int dest, int param)
{
  uint32_t code;

  orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
      compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

  ORC_ASM_CODE(compiler,"  vld1.8 %s[], [%s]\n",
      orc_neon_reg_name (dest), orc_arm_reg_name (compiler->gp_tmpreg));
  code = 0xf4a00c0f;
  code |= (compiler->gp_tmpreg&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= ((dest>>4)&0x1) << 22;
  orc_arm_emit (compiler, code);
}

void
orc_neon_emit_loadpw (OrcCompiler *compiler, int dest, int param)
{
  uint32_t code;

  orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
      compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

  ORC_ASM_CODE(compiler,"  vld1.16 %s[], [%s]\n",
      orc_neon_reg_name (dest), orc_arm_reg_name (compiler->gp_tmpreg));
  code = 0xf4a00c4f;
  code |= (compiler->gp_tmpreg&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= ((dest>>4)&0x1) << 22;
  orc_arm_emit (compiler, code);
}

void
orc_neon_emit_loadpl (OrcCompiler *compiler, int dest, int param)
{
  uint32_t code;

  orc_arm_emit_add_imm (compiler, compiler->gp_tmpreg,
      compiler->exec_reg, ORC_STRUCT_OFFSET(OrcExecutor, params[param]));

  ORC_ASM_CODE(compiler,"  vld1.32 %s[], [%s]\n",
      orc_neon_reg_name (dest), orc_arm_reg_name (compiler->gp_tmpreg));
  code = 0xf4a00c8f;
  code |= (compiler->gp_tmpreg&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= ((dest>>4)&0x1) << 22;
  orc_arm_emit (compiler, code);
}

#define UNARY(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_neon_emit_unary (p, insn_name, code, \
      p->vars[insn->dest_args[0]].alloc, \
      p->vars[insn->src_args[0]].alloc); \
}

#define UNARY_LONG(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_neon_emit_unary_long (p, insn_name, code, \
      p->vars[insn->dest_args[0]].alloc, \
      p->vars[insn->src_args[0]].alloc); \
}

#define UNARY_NARROW(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_neon_emit_unary_narrow (p, insn_name, code, \
      p->vars[insn->dest_args[0]].alloc, \
      p->vars[insn->src_args[0]].alloc); \
}

#define BINARY(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_neon_emit_binary (p, insn_name, code, \
      p->vars[insn->dest_args[0]].alloc, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->src_args[1]].alloc); \
}

#define BINARY_LONG(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_neon_emit_binary_long (p, insn_name, code, \
      p->vars[insn->dest_args[0]].alloc, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->src_args[1]].alloc); \
}

#define BINARY_NARROW(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_neon_emit_binary_narrow (p, insn_name, code, \
      p->vars[insn->dest_args[0]].alloc, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->src_args[1]].alloc); \
}

#define MOVE(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) { \
    uint32_t x = code; \
    ORC_ASM_CODE(p,"  " insn_name " %s, %s\n", \
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc)); \
    x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<16; \
    x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<7; \
    x |= (p->vars[insn->src_args[0]].alloc&0xf)<<12; \
    x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<22; \
    x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
    x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5; \
    orc_arm_emit (p, x); \
  } \
}


typedef struct {
  uint32_t code;
  char *name;
  int negate;
  int bits;
} ShiftInfo;
ShiftInfo immshift_info[] = {
  { 0xf2880510, "vshl.i8", FALSE, 8 }, /* shlb */
  { 0xf2880010, "vshr.s8", TRUE, 8 }, /* shrsb */
  { 0xf3880010, "vshr.u8", TRUE, 8 }, /* shrub */
  { 0xf2900510, "vshl.i16", FALSE, 16 },
  { 0xf2900010, "vshr.s16", TRUE, 16 },
  { 0xf3900010, "vshr.u16", TRUE, 16 },
  { 0xf2a00510, "vshl.i32", FALSE, 32 },
  { 0xf2a00010, "vshr.s32", TRUE, 32 },
  { 0xf3a00010, "vshr.u32", TRUE, 32 }
};
ShiftInfo regshift_info[] = {
  { 0xf3000400, "vshl.u8", FALSE }, /* shlb */
  { 0xf2000400, "vshl.s8", TRUE }, /* shrsb */
  { 0xf3000400, "vshl.u8", TRUE }, /* shrub */
  { 0xf3100400, "vshl.u16", FALSE },
  { 0xf2100400, "vshl.s16", TRUE },
  { 0xf3100400, "vshl.u16", TRUE },
  { 0xf3200400, "vshl.u32", FALSE },
  { 0xf2200400, "vshl.s32", TRUE },
  { 0xf3200400, "vshl.u32", TRUE }
};

static void
orc_neon_rule_shift (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int type = (int)user;
  uint32_t code;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    int shift = p->vars[insn->src_args[1]].value;
    if (shift < 0) {
      ORC_COMPILER_ERROR(p, "shift negative");
      return;
    }
    if (shift >= immshift_info[type].bits) {
      ORC_COMPILER_ERROR(p, "shift too large");
      return;
    }
    code = immshift_info[type].code;
    ORC_ASM_CODE(p,"  %s %s, %s, #%d\n",
        immshift_info[type].name,
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        p->vars[insn->src_args[1]].value);
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    if (immshift_info[type].negate) {
      shift = immshift_info[type].bits - shift;
    }
    code |= shift<<16;
    orc_arm_emit (p, code);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    orc_neon_emit_loadpb (p, p->tmpreg, insn->src_args[1]);

    if (regshift_info[type].negate) {
      orc_neon_emit_neg (p, p->tmpreg);
    }

    code = regshift_info[type].code;
    ORC_ASM_CODE(p,"  %s %s, %s, %s\n",
        regshift_info[type].name,
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        orc_neon_reg_name (p->tmpreg));
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= (p->tmpreg&0xf)<<16;
    code |= ((p->tmpreg>>4)&0x1)<<7;
    orc_arm_emit (p, code);
  } else {
    ORC_PROGRAM_ERROR(p,"shift rule only works with constants and params");
  }
}

#if 0
static void
orc_neon_rule_shrsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t code;
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    code = 0xf2900010;
    ORC_ASM_CODE(p,"  vshr.s16 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        p->vars[insn->src_args[1]].value);
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= ((16 - p->vars[insn->src_args[1]].value)&0xf)<<16;
    orc_arm_emit (p, code);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    code = 0xf2100400;
    ORC_ASM_CODE(p,"  vshl.s16 %s, %s, %s\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= (p->vars[insn->src_args[1]].alloc&0xf)<<16;
    code |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<7;
    orc_arm_emit (p, code);
  } else {
    ORC_PROGRAM_ERROR(p,"shift rule only works with constants and params");
  }
}

static void
orc_neon_rule_shrsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t code;
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    code = 0xf2900010;
    ORC_ASM_CODE(p,"  vshr.s32 %s, %s, #%d\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        p->vars[insn->src_args[1]].value);
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= ((16 - p->vars[insn->src_args[1]].value)&0xf)<<16;
    orc_arm_emit (p, code);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    code = 0xf2100400;
    ORC_ASM_CODE(p,"  vshl.s32 %s, %s, %s\n",
        orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
        orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
    code |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12;
    code |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22;
    code |= (p->vars[insn->src_args[0]].alloc&0xf)<<0;
    code |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5;
    code |= (p->vars[insn->src_args[1]].alloc&0xf)<<16;
    code |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<7;
    orc_arm_emit (p, code);
  } else {
    ORC_PROGRAM_ERROR(p,"shift rule only works with constants and params");
  }
}
#endif


static void
orc_neon_rule_andn (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* this is special because the operand order is reversed */
  orc_neon_emit_binary (p, "vbic", 0xf2100110,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->src_args[0]].alloc);
}


UNARY(absb,"vabs.s8",0xf3b10300)
BINARY(addb,"vadd.i8",0xf2000800)
BINARY(addssb,"vqadd.s8",0xf2000010)
BINARY(addusb,"vqadd.u8",0xf3000010)
BINARY(andb,"vand",0xf2000110)
//BINARY(andnb,"vbic",0xf2100110)
BINARY(avgsb,"vrhadd.s8",0xf2000100)
BINARY(avgub,"vrhadd.u8",0xf3000100)
BINARY(cmpeqb,"vceq.i8",0xf3000810)
BINARY(cmpgtsb,"vcgt.s8",0xf2000300)
MOVE(copyb,"vmov",0xf2200110)
BINARY(maxsb,"vmax.s8",0xf2000600)
BINARY(maxub,"vmax.u8",0xf3000600)
BINARY(minsb,"vmin.s8",0xf2000610)
BINARY(minub,"vmin.u8",0xf3000610)
BINARY(mullb,"vmul.i8",0xf2000910)
BINARY(orb,"vorr",0xf2200110)
//LSHIFT(shlb,"vshl.i8",0xf2880510)
//RSHIFT(shrsb,"vshr.s8",0xf2880010,8)
//RSHIFT(shrub,"vshr.u8",0xf3880010,8)
BINARY(subb,"vsub.i8",0xf3000800)
BINARY(subssb,"vqsub.s8",0xf2000210)
BINARY(subusb,"vqsub.u8",0xf3000210)
BINARY(xorb,"veor",0xf3000110)

UNARY(absw,"vabs.s16",0xf3b50300)
BINARY(addw,"vadd.i16",0xf2100800)
BINARY(addssw,"vqadd.s16",0xf2100010)
BINARY(addusw,"vqadd.u16",0xf3100010)
BINARY(andw,"vand",0xf2000110)
//BINARY(andnw,"vbic",0xf2100110)
BINARY(avgsw,"vrhadd.s16",0xf2100100)
BINARY(avguw,"vrhadd.u16",0xf3100100)
BINARY(cmpeqw,"vceq.i16",0xf3100810)
BINARY(cmpgtsw,"vcgt.s16",0xf2100300)
MOVE(copyw,"vmov",0xf2200110)
BINARY(maxsw,"vmax.s16",0xf2100600)
BINARY(maxuw,"vmax.u16",0xf3100600)
BINARY(minsw,"vmin.s16",0xf2100610)
BINARY(minuw,"vmin.u16",0xf3100610)
BINARY(mullw,"vmul.i16",0xf2100910)
BINARY(orw,"vorr",0xf2200110)
//LSHIFT(shlw,"vshl.i16",0xf2900510)
//RSHIFT(shrsw,"vshr.s16",0xf2900010,16)
//RSHIFT(shruw,"vshr.u16",0xf3900010,16)
BINARY(subw,"vsub.i16",0xf3100800)
BINARY(subssw,"vqsub.s16",0xf2100210)
BINARY(subusw,"vqsub.u16",0xf3100210)
BINARY(xorw,"veor",0xf3000110)

UNARY(absl,"vabs.s32",0xf3b90300)
BINARY(addl,"vadd.i32",0xf2200800)
BINARY(addssl,"vqadd.s32",0xf2200010)
BINARY(addusl,"vqadd.u32",0xf3200010)
BINARY(andl,"vand",0xf2000110)
//BINARY(andnl,"vbic",0xf2100110)
BINARY(avgsl,"vrhadd.s32",0xf2200100)
BINARY(avgul,"vrhadd.u32",0xf3200100)
BINARY(cmpeql,"vceq.i32",0xf3200810)
BINARY(cmpgtsl,"vcgt.s32",0xf2200300)
MOVE(copyl,"vmov",0xf2200110)
BINARY(maxsl,"vmax.s32",0xf2200600)
BINARY(maxul,"vmax.u32",0xf3200600)
BINARY(minsl,"vmin.s32",0xf2200610)
BINARY(minul,"vmin.u32",0xf3200610)
BINARY(mulll,"vmul.i32",0xf2200910)
BINARY(orl,"vorr",0xf2200110)
//LSHIFT(shll,"vshl.i32",0xf2a00510)
//RSHIFT(shrsl,"vshr.s32",0xf2a00010,32)
//RSHIFT(shrul,"vshr.u32",0xf3a00010,32)
BINARY(subl,"vsub.i32",0xf3200800)
BINARY(subssl,"vqsub.s32",0xf2200210)
BINARY(subusl,"vqsub.u32",0xf3200210)
BINARY(xorl,"veor",0xf3000110)

UNARY_LONG(convsbw,"vmovl.s8",0xf2880a10)
UNARY_LONG(convubw,"vmovl.u8",0xf3880a10)
UNARY_LONG(convswl,"vmovl.s16",0xf2900a10)
UNARY_LONG(convuwl,"vmovl.u16",0xf3900a10)
UNARY_NARROW(convwb,"vmovn.i16",0xf3b20200)
UNARY_NARROW(convssswb,"vqmovn.s16",0xf3b20280)
UNARY_NARROW(convsuswb,"vqmovun.s16",0xf3b20240)
UNARY_NARROW(convuuswb,"vqmovn.u16",0xf3b202c0)
UNARY_NARROW(convlw,"vmovn.i32",0xf3b60200)
UNARY_NARROW(convssslw,"vqmovn.s32",0xf3b60280)
UNARY_NARROW(convsuslw,"vqmovun.s32",0xf3b60240)
UNARY_NARROW(convuuslw,"vqmovn.u32",0xf3b602c0)

BINARY_LONG(mulsbw,"vmull.s8",0xf2800c00)
BINARY_LONG(mulubw,"vmull.u8",0xf3800c00)
BINARY_LONG(mulswl,"vmull.s16",0xf2900c00)
BINARY_LONG(muluwl,"vmull.u16",0xf3900c00)

UNARY(swapw,"vrev16.i8",0xf3b00100)
UNARY(swapl,"vrev32.i8",0xf3b00080)

UNARY_NARROW(select0lw,"vmovn.i32",0xf3b60200)
UNARY_NARROW(select0wb,"vmovn.i16",0xf3b20200)

//UNARY(mergebw,"vzip.8",0xf3b20180)
//UNARY(mergewl,"vzip.16",0xf3b60180)

static void
orc_neon_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_neon_emit_binary (p, "vadd.i16", 0xf2100800,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
}

static void
orc_neon_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_neon_emit_binary (p, "vadd.i32", 0xf2200800,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
}

static void
orc_neon_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_neon_emit_unary (p, "vrev16.i8", 0xf3b00100,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
  orc_neon_emit_unary_narrow (p, "vmovn.i16", 0xf3b20200,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
}

static void
orc_neon_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_neon_emit_unary (p, "vrev32.i16", 0xf3b40080,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
  orc_neon_emit_unary_narrow (p, "vmovn.i32", 0xf3b60200,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc);
}

static void
orc_neon_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
    orc_neon_emit_mov (p, p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[0]].alloc);
  }

  if (p->vars[insn->src_args[1]].last_use != p->insn_index) {
    orc_neon_emit_mov (p, p->tmpreg, p->vars[insn->src_args[1]].alloc);
    orc_neon_emit_unary (p, "vzip.8", 0xf3b20180,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg);
  } else {
    orc_neon_emit_unary (p, "vzip.8", 0xf3b20180,
        p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
  }
}

static void
orc_neon_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->dest_args[0]].alloc != p->vars[insn->src_args[0]].alloc) {
    orc_neon_emit_mov (p, p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[0]].alloc);
  }

  if (p->vars[insn->src_args[1]].last_use != p->insn_index) {
    orc_neon_emit_mov (p, p->tmpreg, p->vars[insn->src_args[1]].alloc);
    orc_neon_emit_unary (p, "vzip.16", 0xf3b60180,
        p->vars[insn->dest_args[0]].alloc,
        p->tmpreg);
  } else {
    orc_neon_emit_unary (p, "vzip.16", 0xf3b60180,
        p->vars[insn->dest_args[0]].alloc,
        p->vars[insn->src_args[1]].alloc);
  }
}

static void
orc_neon_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t x;
  
  x = 0xf3800700;
  ORC_ASM_CODE(p,"  vabdl.u8 %s, %s, %s\n",
      orc_neon_reg_name_quad (p->tmpreg),
      orc_neon_reg_name (p->vars[insn->src_args[0]].alloc),
      orc_neon_reg_name (p->vars[insn->src_args[1]].alloc));
  x |= (p->tmpreg&0xf)<<12;
  x |= ((p->tmpreg>>4)&0x1)<<22;
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<16;
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<7;
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0;
  x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5;
  orc_arm_emit (p, x);

  orc_neon_emit_unary (p, "vpadal.u16", 0xf3b40680,
      p->vars[insn->dest_args[0]].alloc,
      p->tmpreg);
}

void
orc_compiler_neon_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

#define REG(x) \
    orc_rule_register (rule_set, #x , orc_neon_rule_ ## x, NULL)

  REG(absb);
  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  //REG(andnb);
  REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(copyb);
  REG(maxsb);
  REG(maxub);
  REG(minsb);
  REG(minub);
  REG(mullb);
  REG(orb);
  //REG(shlb);
  //REG(shrsb);
  //REG(shrub);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(absw);
  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  //REG(andnw);
  REG(avgsw);
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(copyw);
  REG(maxsw);
  REG(maxuw);
  REG(minsw);
  REG(minuw);
  REG(mullw);
  REG(orw);
  //REG(shlw);
  //REG(shrsw);
  //REG(shruw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(absl);
  REG(addl);
  REG(addssl);
  REG(addusl);
  REG(andl);
  //REG(andnl);
  REG(avgsl);
  REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(copyl);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  REG(orl);
  //REG(shll);
  //REG(shrsl);
  //REG(shrul);
  REG(subl);
  REG(subssl);
  REG(subusl);
  REG(xorl);

  REG(convsbw);
  REG(convubw);
  REG(convswl);
  REG(convuwl);
  REG(convlw);
  REG(convssslw);
  REG(convsuslw);
  REG(convuuslw);
  REG(convwb);
  REG(convssswb);
  REG(convsuswb);
  REG(convuuswb);

  REG(mulsbw);
  REG(mulubw);
  REG(mulswl);
  REG(muluwl);

  REG(accw);
  REG(accl);
  REG(accsadubl);
  REG(swapw);
  REG(swapl);
  REG(select0wb);
  REG(select1wb);
  REG(select0lw);
  REG(select1lw);
  REG(mergebw);
  REG(mergewl);

  orc_rule_register (rule_set, "shlb", orc_neon_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shrsb", orc_neon_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrub", orc_neon_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shlw", orc_neon_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrsw", orc_neon_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shruw", orc_neon_rule_shift, (void *)5);
  orc_rule_register (rule_set, "shll", orc_neon_rule_shift, (void *)6);
  orc_rule_register (rule_set, "shrsl", orc_neon_rule_shift, (void *)7);
  orc_rule_register (rule_set, "shrul", orc_neon_rule_shift, (void *)8);

  orc_rule_register (rule_set, "andnb", orc_neon_rule_andn, NULL);
  orc_rule_register (rule_set, "andnw", orc_neon_rule_andn, NULL);
  orc_rule_register (rule_set, "andnl", orc_neon_rule_andn, NULL);
}

