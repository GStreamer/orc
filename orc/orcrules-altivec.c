
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcpowerpc.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>



/* rules */

static void
powerpc_rule_loadpX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];

  if (src->vartype == ORC_VAR_TYPE_PARAM) {
    int greg = compiler->gp_tmpreg;

    powerpc_emit_addi (compiler,
        greg, POWERPC_R3,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[0]]));
    ORC_ASM_CODE(compiler,"  lvewx %s, 0, %s\n",
        powerpc_get_regname (dest->alloc),
        powerpc_get_regname (greg));
    powerpc_emit_X (compiler, 0x7c00008e, powerpc_regnum(dest->alloc),
        0, powerpc_regnum(greg));

    ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n",
        powerpc_get_regname (POWERPC_V0),
        powerpc_get_regname (greg));
    powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
        0, powerpc_regnum(greg));

    ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n",
        powerpc_get_regname (dest->alloc),
        powerpc_get_regname (dest->alloc),
        powerpc_get_regname (dest->alloc),
        powerpc_get_regname (POWERPC_V0));
    powerpc_emit_VA (compiler, 4,
        powerpc_regnum(dest->alloc),
        powerpc_regnum(dest->alloc),
        powerpc_regnum(dest->alloc),
        powerpc_regnum(POWERPC_V0), 43);
    switch (src->size) {
      case 1:
        ORC_ASM_CODE(compiler,"  vspltb %s, %s, 3\n",
            powerpc_get_regname (dest->alloc),
            powerpc_get_regname (dest->alloc));
        powerpc_emit_VX (compiler, 0x1000020c,
            powerpc_regnum(dest->alloc), 3, powerpc_regnum(dest->alloc));
        break;
      case 2:
        ORC_ASM_CODE(compiler,"  vsplth %s, %s, 1\n",
            powerpc_get_regname (dest->alloc),
            powerpc_get_regname (dest->alloc));
        powerpc_emit_VX (compiler, 0x1000024c,
            powerpc_regnum(dest->alloc), 1, powerpc_regnum(dest->alloc));
        break;
      case 4:
        ORC_ASM_CODE(compiler,"  vspltw %s, %s, 0\n",
            powerpc_get_regname (dest->alloc),
            powerpc_get_regname (dest->alloc));
        powerpc_emit_VX (compiler, 0x1000028c,
            powerpc_regnum(dest->alloc), 0, powerpc_regnum(dest->alloc));
        break;
    }
  } else {
    int value = src->value;

    switch (src->size) {
      case 1:
        if (value < 16 && value >= -16) {
          ORC_ASM_CODE(compiler,"  vspltisb %s, %d\n",
              powerpc_get_regname(dest->alloc), value&0x1f);
          powerpc_emit_VX(compiler, 0x1000030c,
              powerpc_regnum(dest->alloc), value & 0x1f, 0);
        } else {
          ORC_COMPILER_ERROR(compiler,"can't load constant");
        }
        break;
      case 2:
        if (value < 16 && value >= -16) {
          ORC_ASM_CODE(compiler,"  vspltish %s, %d\n",
              powerpc_get_regname(dest->alloc), value&0x1f);
          powerpc_emit_VX(compiler, 0x1000034c,
              powerpc_regnum(dest->alloc), value & 0x1f, 0);
        } else {
          ORC_COMPILER_ERROR(compiler,"can't load constant");
        }
        break;
      case 4:
        if (value < 16 && value >= -16) {
          ORC_ASM_CODE(compiler,"  vspltisw %s, %d\n",
              powerpc_get_regname(dest->alloc), value&0x1f);
          powerpc_emit_VX(compiler, 0x1000038c,
              powerpc_regnum(dest->alloc), value & 0x1f, 0);
        } else {
          ORC_COMPILER_ERROR(compiler,"can't load constant");
        }
        break;
    }
  }

}

static void
powerpc_rule_loadX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = src->size << compiler->loop_shift;

  switch (size) {
    case 1:
      ORC_ASM_CODE(compiler,"  lvebx %s, 0, %s\n",
          powerpc_get_regname (dest->alloc),
          powerpc_get_regname (src->ptr_register));
      powerpc_emit_X (compiler, 0x7c00000e, powerpc_regnum(dest->alloc),
          0, powerpc_regnum(src->ptr_register));
      break;
    case 2:
      ORC_ASM_CODE(compiler,"  lvehx %s, 0, %s\n",
          powerpc_get_regname (dest->alloc),
          powerpc_get_regname (src->ptr_register));
      powerpc_emit_X (compiler, 0x7c00004e, powerpc_regnum(dest->alloc),
          0, powerpc_regnum(src->ptr_register));
      break;
    case 4:
      ORC_ASM_CODE(compiler,"  lvewx %s, 0, %s\n",
          powerpc_get_regname (dest->alloc),
          powerpc_get_regname (src->ptr_register));
      powerpc_emit_X (compiler, 0x7c00008e, powerpc_regnum(dest->alloc),
          0, powerpc_regnum(src->ptr_register));
      break;
  }
  ORC_ASM_CODE(compiler,"  lvsl %s, 0, %s\n",
      powerpc_get_regname (POWERPC_V0),
      powerpc_get_regname (src->ptr_register));
  powerpc_emit_X (compiler, 0x7c00000c, powerpc_regnum(POWERPC_V0),
      0, powerpc_regnum(src->ptr_register));
  ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n",
  powerpc_get_regname (dest->alloc),
  powerpc_get_regname (dest->alloc),
      powerpc_get_regname (dest->alloc),
      powerpc_get_regname (POWERPC_V0));
  powerpc_emit_VA (compiler, 4,
      powerpc_regnum(dest->alloc),
      powerpc_regnum(dest->alloc),
      powerpc_regnum(dest->alloc),
      powerpc_regnum(POWERPC_V0), 43);
}

static void
powerpc_rule_storeX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int size = dest->size << compiler->loop_shift;

  ORC_ASM_CODE(compiler,"  lvsr %s, 0, %s\n",
      powerpc_get_regname (POWERPC_V0),
      powerpc_get_regname (dest->ptr_register));
  powerpc_emit_X (compiler, 0x7c00004c, powerpc_regnum(POWERPC_V0),
      0, powerpc_regnum(dest->ptr_register));
  ORC_ASM_CODE(compiler,"  vperm %s, %s, %s, %s\n",
      powerpc_get_regname (src->alloc),
      powerpc_get_regname (src->alloc),
      powerpc_get_regname (src->alloc),
      powerpc_get_regname (POWERPC_V0));
  powerpc_emit_VA (compiler, 4,
      powerpc_regnum(src->alloc),
      powerpc_regnum(src->alloc),
      powerpc_regnum(src->alloc),
      powerpc_regnum(POWERPC_V0), 43);

  switch (size) {
    case 1:
      ORC_ASM_CODE(compiler,"  stvebx %s, 0, %s\n",
          powerpc_get_regname (src->alloc),
          powerpc_get_regname (dest->ptr_register));
      powerpc_emit_X (compiler, 0x7c00010e,
          powerpc_regnum(src->alloc),
          0, powerpc_regnum(dest->ptr_register));
      break;
    case 2:
      ORC_ASM_CODE(compiler,"  stvehx %s, 0, %s\n",
          powerpc_get_regname (src->alloc),
          powerpc_get_regname (dest->ptr_register));
      powerpc_emit_X (compiler, 0x7c00014e,
          powerpc_regnum(src->alloc),
          0, powerpc_regnum(dest->ptr_register));
      break;
    case 4:
      ORC_ASM_CODE(compiler,"  stvewx %s, 0, %s\n",
          powerpc_get_regname (src->alloc),
          powerpc_get_regname (dest->ptr_register));
      powerpc_emit_X (compiler, 0x7c00018e,
          powerpc_regnum(src->alloc),
          0, powerpc_regnum(dest->ptr_register));
      break;
  }
}



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
  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) { \
    ORC_ASM_CODE(p,"  vspltisb %s, %d\n", \
        powerpc_get_regname(p->tmpreg), p->vars[insn->src_args[1]].value); \
    powerpc_emit_VX(p, 0x1000030c, \
        powerpc_regnum(p->tmpreg), p->vars[insn->src_args[1]].value, 0); \
    powerpc_emit_VX_2 (p, opcode, code , dest, src1, p->tmpreg);\
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) { \
    ORC_COMPILER_ERROR(p,"rule only works with constants"); \
    powerpc_emit_VX_2 (p, opcode, code , dest, src1, src2);\
  } else { \
    ORC_COMPILER_ERROR(p,"rule only works with constants or params"); \
  } \
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

  powerpc_emit_vandc (p, dest, src2, src1);
}

static void
powerpc_rule_copyX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vor (p, dest, src1, src1);
}

static void
powerpc_rule_mullb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmulesb (p, dest, src1, src2);
  powerpc_emit_vsldoi (p, dest, dest, dest, 1);
}

static void
powerpc_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmulesb (p, dest, src1, src2);
}

static void
powerpc_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmuleub (p, dest, src1, src2);
}

static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmulesh (p, dest, src1, src2);
  powerpc_emit_vsldoi (p, dest, dest, dest, 2);
}

static void
powerpc_rule_mulhsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmulesh (p, dest, src1, src2);
}

static void
powerpc_rule_mulhuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmuleuh (p, dest, src1, src2);
}

#ifdef alternate
static void
powerpc_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int tmp = POWERPC_V0;

  powerpc_emit_vxor (p, tmp, tmp, tmp);
  powerpc_emit_vmladduhm (p, dest, src1, src2, POWERPC_V0);
}
#endif

static void
powerpc_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vupkhsb (p, dest, src1);
}

static void
powerpc_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vupkhsh (p, dest, src1);
}

static void
powerpc_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmrghb (p, dest, reg, src1);
}

static void
powerpc_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg = powerpc_get_constant (p, ORC_CONST_ZERO, 0);
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmrghh (p, dest, reg, src1);
}

static void
powerpc_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkshss (p, dest, src1, src1);
}

static void
powerpc_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkswss (p, dest, src1, src1);
}

static void
powerpc_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkshus (p, dest, src1, src1);
}

static void
powerpc_rule_convsuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkswus (p, dest, src1, src1);
}

static void
powerpc_rule_convuuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkuhus (p, dest, src1, src1);
}

static void
powerpc_rule_convuuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkuwus (p, dest, src1, src1);
}

static void
powerpc_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkuhum (p, dest, src1, src1);
}

static void
powerpc_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkuwum (p, dest, src1, src1);
}

static void
powerpc_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmulesb (p, dest, src1, src2);
}

static void
powerpc_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmuleub (p, dest, src1, src2);
}

static void
powerpc_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmulesh (p, dest, src1, src2);
}

static void
powerpc_rule_muluwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmuleuh (p, dest, src1, src2);
}

#if 0
/* doesn't work */
static void
powerpc_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vadduhm (p, dest, dest, src1);
}
#endif

static void
powerpc_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vadduwm (p, dest, dest, src1);
}

#if 0
/* doesn't work */
static void
powerpc_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmp1 = p->tmpreg;
  int tmp2 = POWERPC_V31;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vmaxub (p, tmp1, src1, src2);
  powerpc_emit_vminub (p, tmp2, src1, src2);
  powerpc_emit_vsububm (p, tmp1, tmp1, tmp2);
  powerpc_emit_vsum4ubs (p, dest, dest, tmp1);
}
#endif

static void
powerpc_rule_signb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, 1);
  powerpc_emit_vminsb (p, dest, src1, reg);
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_B, -1);
  powerpc_emit_vmaxsb (p, dest, dest, reg);
}

static void
powerpc_rule_signw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, 1);
  powerpc_emit_vminsh(p, dest, src1, reg);
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_W, -1);
  powerpc_emit_vmaxsh(p, dest, dest, reg);
}

static void
powerpc_rule_signl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int reg;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, 1);
  powerpc_emit_vminsw (p, dest, src1, reg);
  reg = powerpc_get_constant (p, ORC_CONST_SPLAT_L, -1);
  powerpc_emit_vmaxsw (p, dest, dest, reg);
}

static void
powerpc_rule_select0wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int perm;

  perm = powerpc_get_constant_full (p, 0x00010405, 0x08090c0d,
		  0x10111415, 0x18191c1d);
  powerpc_emit_vperm (p, dest, src1, src1, perm);
}

static void
powerpc_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkuhum (p, dest, src1, src1);
}

static void
powerpc_rule_select0lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int perm;

  perm = powerpc_get_constant_full (p, 0x00010405, 0x08090c0d, 0x10111415, 0x18191c1d);
  powerpc_emit_vperm (p, dest, src1, src1, perm);
}

static void
powerpc_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  powerpc_emit_vpkuwum (p, dest, src1, src1);
}

static void
powerpc_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int perm;

  perm = powerpc_get_constant_full (p, 0x00100111, 0x02120313, 0x04140515, 0x06160717);
  powerpc_emit_vperm (p, dest, src1, src2, perm);
}

static void
powerpc_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int perm;

  perm = powerpc_get_constant_full (p, 0x00011011, 0x02031213, 0x04051415, 0x06071617);
  powerpc_emit_vperm (p, dest, src1, src2, perm);
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

  //REG(accw);
  REG(accl);
  //REG(accsadubl);

  REG(signb);
  REG(signw);
  REG(signl);

  if (0) REG(select0wb);
  if (0) REG(select1wb);
  if (0) REG(select0lw);
  if (0) REG(select1lw);
  if (0) REG(mergebw);
  if (0) REG(mergewl);

  orc_rule_register (rule_set, "loadpb", powerpc_rule_loadpX, NULL);
  orc_rule_register (rule_set, "loadpw", powerpc_rule_loadpX, NULL);
  orc_rule_register (rule_set, "loadpl", powerpc_rule_loadpX, NULL);
  orc_rule_register (rule_set, "loadb", powerpc_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadw", powerpc_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadl", powerpc_rule_loadX, NULL);
  orc_rule_register (rule_set, "storeb", powerpc_rule_storeX, NULL);
  orc_rule_register (rule_set, "storew", powerpc_rule_storeX, NULL);
  orc_rule_register (rule_set, "storel", powerpc_rule_storeX, NULL);

  orc_rule_register (rule_set, "andnb", powerpc_rule_andnX, NULL);
  orc_rule_register (rule_set, "andnw", powerpc_rule_andnX, NULL);
  orc_rule_register (rule_set, "andnl", powerpc_rule_andnX, NULL);

  orc_rule_register (rule_set, "copyb", powerpc_rule_copyX, NULL);
  orc_rule_register (rule_set, "copyw", powerpc_rule_copyX, NULL);
  orc_rule_register (rule_set, "copyl", powerpc_rule_copyX, NULL);
}

