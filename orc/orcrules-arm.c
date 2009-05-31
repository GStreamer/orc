
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcarm.h>



void
orc_arm_loadw (OrcCompiler *compiler, int dest, int src1, int offset)
{
  uint32_t code;

  code = 0xe1d000b0;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (offset&0xf0) << 4;
  code |= offset&0x0f;

  ORC_ASM_CODE(compiler,"  ldrh %s, [%s, #%d]\n",
      orc_arm_reg_name (dest),
      orc_arm_reg_name (src1), offset);
  orc_arm_emit (compiler, code);
}

void
orc_arm_storew (OrcCompiler *compiler, int dest, int offset, int src1)
{
  uint32_t code;

  code = 0xe1c000b0;
  code |= (dest&0xf) << 16;
  code |= (src1&0xf) << 12;
  code |= (offset&0xf0) << 4;
  code |= offset&0x0f;

  ORC_ASM_CODE(compiler,"  strh %s, [%s, #%d]\n",
      orc_arm_reg_name (src1),
      orc_arm_reg_name (dest), offset);
  orc_arm_emit (compiler, code);
}

static void
orc_arm_rule_addw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t code;

  code = 0xe0800000;
  code |= (p->vars[insn->src_args[0]].alloc&0xf) << 16;
  code |= (p->vars[insn->dest_args[0]].alloc&0xf) << 12;
  code |= (p->vars[insn->src_args[1]].alloc&0xf) << 0;

  ORC_ASM_CODE(p,"  add %s, %s, %s\n",
      orc_arm_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[1]].alloc));
  orc_arm_emit (p, code);
}

static void
orc_arm_rule_subw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t code;

  code = 0xe0400000;
  code |= (p->vars[insn->src_args[0]].alloc&0xf) << 16;
  code |= (p->vars[insn->dest_args[0]].alloc&0xf) << 12;
  code |= (p->vars[insn->src_args[1]].alloc&0xf) << 0;

  ORC_ASM_CODE(p,"  sub %s, %s, %s\n",
      orc_arm_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[1]].alloc));
  orc_arm_emit (p, code);
}

static void
orc_arm_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t code;

  code = 0xe0000090;
  code |= (p->vars[insn->dest_args[0]].alloc&0xf) << 16;
  code |= (p->vars[insn->src_args[0]].alloc&0xf) << 0;
  code |= (p->vars[insn->src_args[1]].alloc&0xf) << 8;

  ORC_ASM_CODE(p,"  mul %s, %s, %s\n",
      orc_arm_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[1]].alloc));
  orc_arm_emit (p, code);
}

static void
orc_arm_rule_shrsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  uint32_t code;

  code = 0xe1a00050;
  code |= (p->vars[insn->dest_args[0]].alloc&0xf) << 12;
  code |= (p->vars[insn->src_args[0]].alloc&0xf) << 0;
  code |= (p->vars[insn->src_args[1]].alloc&0xf) << 8;

  ORC_ASM_CODE(p,"  asr %s, %s, %s\n",
      orc_arm_reg_name (p->vars[insn->dest_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[0]].alloc),
      orc_arm_reg_name (p->vars[insn->src_args[1]].alloc));
  orc_arm_emit (p, code);
}


void
orc_compiler_orc_arm_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

#if 0
#define REG(x) \
  orc_rule_register (rule_set, #x , orc_arm_rule_ ## x, NULL)

  REG(absb);
  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(andnb);
  //REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  if (sse41) REG(maxsb);
  REG(maxub);
  if (sse41) REG(minsb);
  REG(minub);
  //REG(mullb);
  //REG(mulhsb);
  //REG(mulhub);
  REG(orb);
  REG(signb);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(absw);
  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  REG(andnw);
  //REG(avgsw);
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(maxsw);
  if (sse41) REG(maxuw);
  REG(minsw);
  if (sse41) REG(minuw);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);
  REG(orw);
  REG(signw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(absl);
  REG(addl);
  //REG(addssl);
  //REG(addusl);
  REG(andl);
  REG(andnl);
  //REG(avgsl);
  //REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  if (sse41) REG(maxsl);
  if (sse41) REG(maxul);
  if (sse41) REG(minsl);
  if (sse41) REG(minul);
  if (sse41) REG(mulll);
  REG(mulhsl);
  REG(mulhul);
  REG(orl);
  REG(signl);
  REG(subl);
  //REG(subssl);
  //REG(subusl);
  REG(xorl);

  orc_rule_register (rule_set, "copyb", orc_arm_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyw", orc_arm_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyl", orc_arm_rule_copyx, NULL);

  orc_rule_register (rule_set, "shlw", orc_arm_rule_shlw, NULL);
  orc_rule_register (rule_set, "shrsw", orc_arm_rule_shrsw, NULL);

  orc_rule_register (rule_set, "convsbw", orc_arm_rule_convsbw, NULL);
  orc_rule_register (rule_set, "convubw", orc_arm_rule_convubw, NULL);
  orc_rule_register (rule_set, "convsuswb", orc_arm_rule_convsuswb, NULL);
#endif
  orc_rule_register (rule_set, "addw", orc_arm_rule_addw, NULL);
  orc_rule_register (rule_set, "subw", orc_arm_rule_subw, NULL);
  orc_rule_register (rule_set, "mullw", orc_arm_rule_mullw, NULL);
  orc_rule_register (rule_set, "shrsw", orc_arm_rule_shrsw, NULL);
}

