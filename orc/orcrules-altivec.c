
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcpowerpc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>



/* rules */

#define RULE(name, opcode, code) \
static void \
powerpc_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  int src1 = ORC_SRC_ARG (p, insn, 0); \
  int src2 = ORC_SRC_ARG (p, insn, 1); \
  int dest = ORC_DEST_ARG (p, insn, 0); \
  powerpc_emit_VX_2 (p, opcode, code , dest, src1, src2);\
}

#define RULE_SHIFT(name, opcode, code) \
static void \
powerpc_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  int src1 = ORC_SRC_ARG (p, insn, 0); \
  int src2 = ORC_SRC_ARG (p, insn, 1); \
  int dest = ORC_DEST_ARG (p, insn, 0); \
  if (p->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_CONST && \
      p->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_PARAM) { \
    ORC_COMPILER_ERROR(p,"rule only works with constants or params"); \
  } \
  powerpc_emit_VX_2 (p, opcode, code , dest, src1, src2);\
}

RULE(addb, "vaddubm", 0x10000000)
RULE(addssb, "vaddsbs", 0x10000300)
RULE(addusb, "vaddubs", 0x10000200)
RULE(andb, "vand", 0x10000404)
//RULE(andnb, "vandc", 0x10000444)
RULE(avgsb, "vavgsb", 0x10000502)
RULE(avgub, "vavgub", 0x10000402)
RULE(cmpeqb, "vcmpequb", 0x10000006)
RULE(cmpgtsb, "vcmpgtsb", 0x10000306)
RULE(maxsb, "vmaxsb", 0x10000102)
RULE(maxub, "vmaxub", 0x10000002)
RULE(minsb, "vminsb", 0x10000302)
RULE(minub, "vminub", 0x10000202)
RULE(orb, "vor", 0x10000484)
RULE_SHIFT(shlb, "vslb", 0x10000104)
RULE_SHIFT(shrsb, "vsrab", 0x10000304)
RULE_SHIFT(shrub, "vsrb", 0x10000204)
RULE(subb, "vsububm", 0x10000400)
RULE(subssb, "vsubsbs", 0x10000700)
RULE(subusb, "vsububs", 0x10000600)
RULE(xorb, "vxor", 0x100004c4)

RULE(addw, "vadduhm", 0x10000040)
RULE(addssw, "vaddshs", 0x10000340)
RULE(addusw, "vadduhs", 0x10000240)
RULE(andw, "vand", 0x10000404)
//RULE(andnw, "vandc", 0x10000444)
RULE(avgsw, "vavgsh", 0x10000542)
RULE(avguw, "vavguh", 0x10000442)
RULE(cmpeqw, "vcmpequh", 0x10000046)
RULE(cmpgtsw, "vcmpgtsh", 0x10000346)
RULE(maxsw, "vmaxsh", 0x10000142)
RULE(maxuw, "vmaxuh", 0x10000042)
RULE(minsw, "vminsh", 0x10000342)
RULE(minuw, "vminuh", 0x10000242)
RULE(orw, "vor", 0x10000484)
RULE_SHIFT(shlw, "vslh", 0x10000144)
RULE_SHIFT(shrsw, "vsrah", 0x10000344)
RULE_SHIFT(shruw, "vsrh", 0x10000244)
RULE(subw, "vsubuhm", 0x10000440)
RULE(subssw, "vsubshs", 0x10000740)
RULE(subusw, "vsubuhs", 0x10000640)
RULE(xorw, "vxor", 0x100004c4)

RULE(addl, "vadduwm", 0x10000080)
RULE(addssl, "vaddsws", 0x10000380)
RULE(addusl, "vadduws", 0x10000280)
RULE(andl, "vand", 0x10000404)
//RULE(andnl, "vandc", 0x10000444)
RULE(avgsl, "vavgsw", 0x10000582)
RULE(avgul, "vavguw", 0x10000482)
RULE(cmpeql, "vcmpequw", 0x10000086)
RULE(cmpgtsl, "vcmpgtsw", 0x10000386)
RULE(maxsl, "vmaxsw", 0x10000182)
RULE(maxul, "vmaxuw", 0x10000082)
RULE(minsl, "vminsw", 0x10000382)
RULE(minul, "vminuw", 0x10000282)
RULE(orl, "vor", 0x10000484)
RULE_SHIFT(shll, "vslw", 0x10000184)
RULE_SHIFT(shrsl, "vsraw", 0x10000384)
RULE_SHIFT(shrul, "vsrw", 0x10000284)
RULE(subl, "vsubuwm", 0x10000480)
RULE(subssl, "vsubsws", 0x10000780)
RULE(subusl, "vsubuws", 0x10000680)
RULE(xorl, "vxor", 0x100004c4)

static void
powerpc_rule_andnX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vandc", 0x10000444, dest, src2, src1);
}

static void
powerpc_rule_copyX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vor", 0x10000484, dest, src1, src1);
}

static void
powerpc_rule_mullb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmulesb", 0x10000308, dest, src1, src2);

  ORC_ASM_CODE(p,"  vsldoi %s, %s, %s, 1\n",
      powerpc_get_regname(dest),
      powerpc_get_regname(dest),
      powerpc_get_regname(dest));
  powerpc_emit_VX(p, 0x1000002c | (1<<6),
      powerpc_regnum(dest),
      powerpc_regnum(dest),
      powerpc_regnum(dest));
}

static void
powerpc_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmulesb", 0x10000308, dest, src1, src2);
}

static void
powerpc_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmuleub", 0x10000208, dest, src1, src2);
}

static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmulesh", 0x10000348, dest, src1, src2);

  ORC_ASM_CODE(p,"  vsldoi %s, %s, %s, 2\n",
      powerpc_get_regname(dest),
      powerpc_get_regname(dest),
      powerpc_get_regname(dest));
  powerpc_emit_VX(p, 0x1000002c | (2<<6),
      powerpc_regnum(dest),
      powerpc_regnum(dest),
      powerpc_regnum(dest));
}

static void
powerpc_rule_mulhsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmulesh", 0x10000348, dest, src1, src2);
}

static void
powerpc_rule_mulhuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmuleuh", 0x10000248, dest, src1, src2);
}

#ifdef alternate
static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int tmp = POWERPC_V0;

  powerpc_emit_VX_2 (p, "vxor", 0x100004c4, tmp, tmp, tmp);

  ORC_ASM_CODE(p,"  vmladduhm %s, %s, %s, %s\n",
      powerpc_get_regname(dest),
      powerpc_get_regname(src1),
      powerpc_get_regname(src2),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VA(p, 4, 
      powerpc_regnum(dest),
      powerpc_regnum(src1),
      powerpc_regnum(src2),
      powerpc_regnum(POWERPC_V0), 34);

}
#endif

static void
powerpc_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  ORC_ASM_CODE(p,"  vupkhsb %s, %s\n",
      powerpc_get_regname(dest),
      powerpc_get_regname(src1));
  powerpc_emit_VX (p, 0x1000020e,
      powerpc_regnum(dest),
      0,
      powerpc_regnum(src1));
}

static void
powerpc_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  ORC_ASM_CODE(p,"  vupkhsh %s, %s\n",
      powerpc_get_regname(dest),
      powerpc_get_regname(src1));
  powerpc_emit_VX(p, 0x1000024e,
      powerpc_regnum(dest),
      0,
      powerpc_regnum(src1));
}

static void
powerpc_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmrghb", 0x1000000c, dest, reg, src1);
}

static void
powerpc_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2(p, "vmrghh", 0x1000004c, dest, reg, src1);
}

static void
powerpc_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkshss", 0x1000018e, dest, src1, src1);
}

static void
powerpc_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkswss", 0x100001ce, dest, src1, src1);
}

static void
powerpc_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkshus", 0x1000010e, dest, src1, src1);
}

static void
powerpc_rule_convsuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkswus", 0x1000014e, dest, src1, src1);
}

static void
powerpc_rule_convuuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkuhus", 0x1000008e, dest, src1, src1);
}

static void
powerpc_rule_convuuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkuwus", 0x100000ce, dest, src1, src1);
}

static void
powerpc_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkuhum", 0x1000000e, dest, src1, src1);
}

static void
powerpc_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vpkuwum", 0x1000004e, dest, src1, src1);
}

static void
powerpc_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmulesb", 0x10000308, dest, src1, src2);
}

static void
powerpc_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmuleub", 0x10000208, dest, src1, src2);
}

static void
powerpc_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmulesh", 0x10000348, dest, src1, src2);
}

static void
powerpc_rule_muluwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmuleuh", 0x10000248, dest, src1, src2);
}

static void
powerpc_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vadduhm", 0x10000040, dest, dest, src1);
}

static void
powerpc_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vadduwm", 0x10000080, dest, dest, src1);
}

static void
powerpc_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmp1 = p->tmpreg;;
  int tmp2 = POWERPC_V31;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_VX_2 (p, "vmaxub", 0x10000002, tmp1, src1, src2);
  powerpc_emit_VX_2 (p, "vminub", 0x10000202, tmp2, src1, src2);
  powerpc_emit_VX_2 (p, "vsububm", 0x10000400, tmp1, tmp1, tmp2);
  powerpc_emit_VX_2 (p, "vsum4ubs", 0x10000608, dest, dest, tmp1);
}

static void
powerpc_rule_signb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, 1);
  powerpc_emit_VX_2(p, "vminsb", 0x10000302, dest, src1, reg);
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, -1);
  powerpc_emit_VX_2(p, "vmaxsb", 0x10000102, dest, dest, reg);
}

static void
powerpc_rule_signw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, 1);
  powerpc_emit_VX_2(p, "vminsh", 0x10000342, dest, src1, reg);
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, -1);
  powerpc_emit_VX_2(p, "vmaxsh", 0x10000142, dest, dest, reg);
}

static void
powerpc_rule_signl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, 1);
  powerpc_emit_VX_2(p, "vminsw", 0x10000382, dest, src1, reg);
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, -1);
  powerpc_emit_VX_2(p, "vmaxsw", 0x10000182, dest, dest, reg);
}

void
orc_compiler_powerpc_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

#define REG(name) \
  orc_rule_register (rule_set, #name , powerpc_rule_ ## name , NULL);

  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(maxsb);
  REG(maxub);
  REG(minsb);
  REG(minub);
  REG(orb);
  REG(shlb);
  REG(shrsb);
  REG(shrub);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  REG(avgsw);
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(maxsw);
  REG(maxuw);
  REG(minsw);
  REG(minuw);
  REG(orw);
  REG(shlw);
  REG(shrsw);
  REG(shruw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(addl);
  REG(addssl);
  REG(addusl);
  REG(andl);
  REG(avgsl);
  REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(orl);
  REG(shll);
  REG(shrsl);
  REG(shrul);
  REG(subl);
  REG(subssl);
  REG(subusl);
  REG(xorl);

  REG(mullb);
  REG(mulhsb);
  REG(mulhub);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);

  REG(convsbw);
  REG(convswl);
  REG(convubw);
  REG(convuwl);
  REG(convssswb);
  REG(convssslw);
  REG(convsuswb);
  REG(convsuslw);
  REG(convuuswb);
  REG(convuuslw);
  REG(convwb);
  REG(convlw);

  REG(mulsbw);
  REG(mulubw);
  REG(mulswl);
  REG(muluwl);

  REG(accw);
  REG(accl);
  REG(accsadubl);

  REG(signb);
  REG(signw);
  REG(signl);

  orc_rule_register (rule_set, "andnb", powerpc_rule_andnX, NULL);
  orc_rule_register (rule_set, "andnw", powerpc_rule_andnX, NULL);
  orc_rule_register (rule_set, "andnl", powerpc_rule_andnX, NULL);

  orc_rule_register (rule_set, "copyb", powerpc_rule_copyX, NULL);
  orc_rule_register (rule_set, "copyw", powerpc_rule_copyX, NULL);
  orc_rule_register (rule_set, "copyl", powerpc_rule_copyX, NULL);
}

