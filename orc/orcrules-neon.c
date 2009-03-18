
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/arm.h>


const char *neon_reg_name (int reg)
{
  static const char *vec_regs[] = {
    "d0", "d1", "d2", "d3",
    "d4", "d5", "d6", "d7",
    "d8", "d9", "d10", "d11",
    "d12", "d13", "d14", "d15" };

  if (reg < ORC_VEC_REG_BASE || reg >= ORC_VEC_REG_BASE+16) {
    return "ERROR";
  }

  return vec_regs[reg&0xf];
}

void
neon_loadw (OrcCompiler *compiler, int dest, int src1, int offset)
{
  uint32_t code;

  code = 0xed900b00;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (offset&0xf0) << 4;
  code |= offset&0x0f;

  ORC_ASM_CODE(compiler,"  vldr.64 %s, [%s, #%d]\n",
      neon_reg_name (dest),
      arm_reg_name (src1), offset);
  arm_emit (compiler, code);
}

void
neon_storew (OrcCompiler *compiler, int dest, int offset, int src1)
{
  uint32_t code;

  code = 0xed800b00;
  code |= (dest&0xf) << 16;
  code |= (src1&0xf) << 12;
  code |= (offset&0xf0) << 4;
  code |= offset&0x0f;

  ORC_ASM_CODE(compiler,"  vstr.64 %s, [%s, #%d]\n",
      neon_reg_name (src1),
      arm_reg_name (dest), offset);
  arm_emit (compiler, code);
}

#define UNARY(opcode,insn_name,code) \
static void \
neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s\n", \
      neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      neon_reg_name (p->vars[insn->src_args[0]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  arm_emit (p, x); \
}

#define BINARY(opcode,insn_name,code) \
static void \
neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s, %s\n", \
      neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      neon_reg_name (p->vars[insn->src_args[0]].alloc), \
      neon_reg_name (p->vars[insn->src_args[1]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<16; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<12; \
  x |= (p->vars[insn->src_args[1]].alloc&0xf)<<0; \
  arm_emit (p, x); \
}

#define MOVE(opcode,insn_name,code) \
static void \
neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s\n", \
      neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      neon_reg_name (p->vars[insn->src_args[0]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<16; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<12; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  arm_emit (p, x); \
}

#define SHIFT(opcode,insn_name,code) \
static void \
neon_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  uint32_t x = code; \
  ORC_ASM_CODE(p,"  " insn_name " %s, %s, #2\n", \
      neon_reg_name (p->vars[insn->dest_args[0]].alloc), \
      neon_reg_name (p->vars[insn->src_args[0]].alloc)); \
  x |= (p->vars[insn->dest_args[0]].alloc&0xf)<<12; \
  x |= (p->vars[insn->src_args[0]].alloc&0xf)<<0; \
  arm_emit (p, x); \
}



UNARY(absb,"vabs.s8",0xf3b10300)
BINARY(addb,"vadd.i8",0xf2000800)
BINARY(addssb,"vqadd.s8",0xf2000010)
BINARY(addusb,"vqadd.u8",0xf3000010)
BINARY(andb,"vand",0xf2000110)
BINARY(cmpeqb,"vceq.i8",0xf3000810)
BINARY(cmpgtsb,"vcgt.s8",0xf2000300)
MOVE(copyb,"vmov",0xf2200110)
BINARY(maxsb,"vmax.s8",0xf2000600)
BINARY(maxub,"vmax.u8",0xf3000600)
BINARY(minsb,"vmin.s8",0xf2000610)
BINARY(minub,"vmin.u8",0xf3000610)
BINARY(mullb,"vmul.i8",0xf2000910)
BINARY(orb,"vorn",0xf2300110)
SHIFT(shlb,"vshl.i8",0xf2890510)
SHIFT(shrsb,"vshr.s8",0xf28f0010)
SHIFT(shrub,"vshr.u8",0xf38f0010)
BINARY(subb,"vsub.i8",0xf3000800)
BINARY(subssb,"vqsub.s8",0xf2000210)
BINARY(subusb,"vqsub.u8",0xf3000210)
BINARY(xorb,"veor",0xf3000110)

UNARY(absw,"vabs.s16",0xf3b50300)
BINARY(addw,"vadd.i16",0xf2100800)
BINARY(addssw,"vqadd.s16",0xf2100010)
BINARY(addusw,"vqadd.u16",0xf3100010)
BINARY(andw,"vand",0xf2000110)
BINARY(cmpeqw,"vceq.i16",0xf3100810)
BINARY(cmpgtsw,"vcgt.s16",0xf2100300)
MOVE(copyw,"vmov",0xf2200110)
BINARY(maxsw,"vmax.s16",0xf2100600)
BINARY(maxuw,"vmax.u16",0xf3100600)
BINARY(minsw,"vmin.s16",0xf2100610)
BINARY(minuw,"vmin.u16",0xf3100610)
BINARY(mullw,"vmul.i16",0xf2100910)
BINARY(orw,"vorn",0xf2300110)
SHIFT(shlw,"vshl.i16",0xf2910510)
SHIFT(shrsw,"vshr.s16",0xf29f0010)
SHIFT(shruw,"vshr.u16",0xf39f0010)
BINARY(subw,"vsub.i16",0xf3100800)
BINARY(subssw,"vqsub.s16",0xf2100210)
BINARY(subusw,"vqsub.u16",0xf3100210)
BINARY(xorw,"veor",0xf3000110)

UNARY(absl,"vabs.s32",0xf3b90300)
BINARY(addl,"vadd.i32",0xf2200800)
BINARY(addssl,"vqadd.s32",0xf2200010)
BINARY(addusl,"vqadd.u32",0xf3200010)
BINARY(andl,"vand",0xf2000110)
BINARY(cmpeql,"vceq.i32",0xf3200810)
BINARY(cmpgtsl,"vcgt.s32",0xf2200300)
MOVE(copyl,"vmov",0xf2200110)
BINARY(maxsl,"vmax.s32",0xf2200600)
BINARY(maxul,"vmax.u32",0xf3200600)
BINARY(minsl,"vmin.s32",0xf2200610)
BINARY(minul,"vmin.u32",0xf3200610)
BINARY(mulll,"vmul.i32",0xf2200910)
BINARY(orl,"vorn",0xf2300110)
SHIFT(shll,"vshl.i32",0xf2a10510)
SHIFT(shrsl,"vshr.s32",0xf2bf0010)
SHIFT(shrul,"vshr.u32",0xf3bf0010)
BINARY(subl,"vsub.i32",0xf3200800)
BINARY(subssl,"vqsub.s32",0xf2200210)
BINARY(subusl,"vqsub.u32",0xf3200210)
BINARY(xorl,"veor",0xf3000110)



void
orc_compiler_neon_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target);

#define REG(x) \
    orc_rule_register (rule_set, #x , neon_rule_ ## x, NULL)

  REG(absb);
  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(copyb);
  REG(maxsb);
  REG(maxub);
  REG(minsb);
  REG(minub);
  REG(mullb);
  REG(orb);
  REG(shlb);
  REG(shrsb);
  REG(shrub);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(absw);
  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(copyw);
  REG(maxsw);
  REG(maxuw);
  REG(minsw);
  REG(minuw);
  REG(mullw);
  REG(orw);
  REG(shlw);
  REG(shrsw);
  REG(shruw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(absl);
  REG(addl);
  REG(addssl);
  REG(addusl);
  REG(andl);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(copyl);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  REG(orl);
  REG(shll);
  REG(shrsl);
  REG(shrul);
  REG(subl);
  REG(subssl);
  REG(subusl);
  REG(xorl);

}

