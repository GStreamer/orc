
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
  ORC_ASM_CODE(p,"  " opcode " %s, %s, %s\n", \
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc)); \
  powerpc_emit_VX(p, code , \
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[1]].alloc)); \
}

#define RULE_SHIFT(name, opcode, code) \
static void \
powerpc_rule_ ## name (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  if (p->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_CONST && \
      p->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_PARAM) { \
    ORC_COMPILER_ERROR(p,"rule only works with constants or params"); \
  } \
  ORC_ASM_CODE(p,"  " opcode " %s, %s, %s\n", \
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc), \
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc)); \
  powerpc_emit_VX(p, code , \
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[0]].alloc), \
      powerpc_regnum (p->vars[insn->src_args[1]].alloc)); \
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
  ORC_ASM_CODE(p,"  vandc %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000444,
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum (p->vars[insn->src_args[1]].alloc),
      powerpc_regnum (p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_copyX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vor %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000484,
      powerpc_regnum (p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum (p->vars[insn->src_args[0]].alloc),
      powerpc_regnum (p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_mullb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000308,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vsldoi %s, %s, %s, 1\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000002c | (1<<6),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc));
}

static void
powerpc_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000308,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleub %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000208,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000348,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vsldoi %s, %s, %s, 2\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000002c | (2<<6),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc));
}

static void
powerpc_rule_mulhsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000348,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulhuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleuh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000248,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}


#ifdef alternate
static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vxor %s, %s, %s\n",
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VX(p, 0x100004c4,
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0),
      powerpc_regnum(POWERPC_V0));

  ORC_ASM_CODE(p,"  vmladduhm %s, %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc),
      powerpc_get_regname(POWERPC_V0));
  powerpc_emit_VA(p, 4, 
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc),
      powerpc_regnum(POWERPC_V0), 34);

}
#endif

static void
powerpc_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vupkhsb %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000020e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      0,
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vupkhsh %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000024e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      0,
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);

  ORC_ASM_CODE(p,"  vmrghb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(reg),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000000c,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(reg),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);

  ORC_ASM_CODE(p,"  vmrghh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(reg),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000004c,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(reg),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkshss %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000018e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkswss %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x100001ce,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkshus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000010e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convsuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkswus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000014e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convuuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuhus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000008e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convuuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuwus %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x100000ce,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuhum %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000000e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vpkuwum %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x1000004e,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesb %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000308,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleub %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000208,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmulesh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000348,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_muluwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vmuleuh %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000248,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));
}

static void
powerpc_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vadduhm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000040,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  ORC_ASM_CODE(p,"  vadduwm %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc));
  powerpc_emit_VX(p, 0x10000080,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc));
}

static void
powerpc_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmpreg2 = POWERPC_V31;

  ORC_ASM_CODE(p,"  vmaxub %s, %s, %s\n",
      powerpc_get_regname(p->tmpreg),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000002,
      powerpc_regnum(p->tmpreg),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vminub %s, %s, %s\n",
      powerpc_get_regname(tmpreg2),
      powerpc_get_regname(p->vars[insn->src_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->src_args[1]].alloc));
  powerpc_emit_VX(p, 0x10000202,
      powerpc_regnum(tmpreg2),
      powerpc_regnum(p->vars[insn->src_args[0]].alloc),
      powerpc_regnum(p->vars[insn->src_args[1]].alloc));

  ORC_ASM_CODE(p,"  vsububm %s, %s, %s\n",
      powerpc_get_regname(p->tmpreg),
      powerpc_get_regname(p->tmpreg),
      powerpc_get_regname(tmpreg2));
  powerpc_emit_VX(p, 0x10000400,
      powerpc_regnum(p->tmpreg),
      powerpc_regnum(p->tmpreg),
      powerpc_regnum(tmpreg2));

  ORC_ASM_CODE(p,"  vsum4ubs %s, %s, %s\n",
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->vars[insn->dest_args[0]].alloc),
      powerpc_get_regname(p->tmpreg));
  powerpc_emit_VX(p, 0x10000608,
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->vars[insn->dest_args[0]].alloc),
      powerpc_regnum(p->tmpreg));
}

static void
powerpc_rule_signb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, 1);
  powerpc_emit_VX_2(p, "vminsb", 0x10000302,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc,
      reg);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, -1);
  powerpc_emit_VX_2(p, "vmaxsb", 0x10000102,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      reg);
}

static void
powerpc_rule_signw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, 1);
  powerpc_emit_VX_2(p, "vminsh", 0x10000342,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc,
      reg);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, -1);
  powerpc_emit_VX_2(p, "vmaxsh", 0x10000142,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      reg);
}

static void
powerpc_rule_signl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, 1);
  powerpc_emit_VX_2(p, "vminsw", 0x10000382,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->src_args[0]].alloc,
      reg);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, -1);
  powerpc_emit_VX_2(p, "vmaxsw", 0x10000182,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc,
      reg);
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

