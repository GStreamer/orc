
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcarm.h>
#include <orc/orcdebug.h>

#include <orc/neon.h>

#if 0
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
#endif

const char *orc_neon_reg_name_s (int reg)
{
  static const char *vec_regs[] = {
    "s0", "s2", "s4", "s6",
    "s8", "s10", "s12", "s14",
    "s16", "s18", "s20", "s22",
    "s24", "s26", "s28", "s30",
    "ERROR", "ERROR", "ERROR", "ERROR",
    "ERROR", "ERROR", "ERROR", "ERROR",
    "ERROR", "ERROR", "ERROR", "ERROR",
    "ERROR", "ERROR", "ERROR", "ERROR"
  };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+32) {
    return "ERROR";
  }

  return vec_regs[reg&0x1f];
}

#define UNARY(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s\n", \
      orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      orc_neon_reg_name (p->vars[insn->src_args[0]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5; \
  orc_arm_emit (p, x); \
}

#define UNARY_S(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s\n", \
      orc_neon_reg_name_s (p->vars[insn->dest_args[0]].alloc), \
      orc_neon_reg_name_s (p->vars[insn->src_args[0]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5; \
  orc_arm_emit (p, x); \
}

#define BINARY(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s, %s\n", \
      orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      orc_neon_reg_name (p->vars[insn->src_args[0]].alloc), \
      orc_neon_reg_name (p->vars[insn->src_args[1]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<16; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<7; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5; \
  orc_arm_emit (p, x); \
}

#define BINARY_S(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s, %s\n", \
      orc_neon_reg_name_s (p->vars[insn->dest_args[0]].alloc), \
      orc_neon_reg_name_s (p->vars[insn->src_args[0]].alloc), \
      orc_neon_reg_name_s (p->vars[insn->src_args[1]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<16; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<7; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<5; \
  orc_arm_emit (p, x); \
}

#define BINARY_R(opcode,insn_name,code) \
static void \
orc_neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s, %s\n", \
      orc_neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      orc_neon_reg_name (p->vars[insn->src_args[0]].alloc), \
      orc_neon_reg_name (p->vars[insn->src_args[1]].alloc)); \
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<16; \
  x |= ((p->vars[insn->src_args[1]].alloc>>4)&0x1)<<7; \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12; \
  x |= ((p->vars[insn->dest_args[0]].alloc>>4)&0x1)<<22; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  x |= ((p->vars[insn->src_args[0]].alloc>>4)&0x1)<<5; \
  orc_arm_emit (p, x); \
}


BINARY(addf,"vadd.f32",0xf2000d00)
BINARY(subf,"vsub.f32",0xf2200d00)
BINARY(mulf,"vmul.f32",0xf3000d10)
BINARY_S(divf,"vdiv.f32",0xee800a00)
UNARY(invf,"vrecpe.f32",0xf3bb0500)
UNARY_S(sqrtf,"vsqrt.f32",0xeeb10ac0)
BINARY(maxf,"vmax.f32",0xf2000f00)
BINARY(minf,"vmin.f32",0xf2200f00)
UNARY(invsqrtf,"vrsqrte.f32",0xf3bb0580)
BINARY(cmpeqf,"vceq.f32",0xf2000e00)
BINARY_R(cmpltf,"vclt.f32",0xf3200e00)
BINARY_R(cmplef,"vcle.f32",0xf3000e00)
UNARY(convfl,"vcvt.s32.f32",0xf3bb0700)
UNARY(convlf,"vcvt.f32.s32",0xf3bb0600)

BINARY(addg,"vadd.f64",0xee300b00)
BINARY(subg,"vsub.f64",0xee300b40)
BINARY(mulg,"vmul.f64",0xee200b00)
BINARY(divg,"vdiv.f64",0xee800b00)
UNARY(sqrtg,"vsqrt.f64",0xeeb10bc0)
//BINARY(cmpeqg,"fcmped",0xf2000e00)
//BINARY(cmpltg,"fcmpltd",0xf6400e00)
//BINARY(cmpleg,"fcmpled",0xf3000e00)
//UNARY(convgl,"vcvt.s32.f64",0xf3bb0700)
//UNARY(convlg,"vcvt.f64.s32",0xf3bb0600)

void
orc_float_neon_register_rules (void)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("float"),
      orc_target_get_by_name("neon"), 0);

#define REG(x) \
    orc_rule_register (rule_set, #x , orc_neon_rule_ ## x, NULL)

  REG(addf);
  REG(subf);
  REG(mulf);
  REG(divf);
  REG(invf);
  REG(sqrtf);
  REG(maxf);
  REG(minf);
  REG(invsqrtf);
  REG(cmpeqf);
  REG(cmpltf);
  REG(cmplef);
  REG(convfl);
  REG(convlf);

  REG(addg);
  REG(subg);
  REG(mulg);
  REG(divg);
  REG(sqrtg);
  //REG(cmpeqg);
  //REG(cmpltg);
  //REG(cmpleg);
  //REG(convgl);
  //REG(convlg);
}

