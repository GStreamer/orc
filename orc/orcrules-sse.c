
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>

#define SIZE 65536

/* sse rules */

void
sse_emit_loadiw (OrcProgram *p, int reg, int value)
{
  if (value == 0) {
    printf("  pxor %%%s, %%%s\n", x86_get_regname_sse(reg),
        x86_get_regname_sse(reg));
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xef;
    x86_emit_modrm_reg (p, reg, reg);
  } else if (value == -1) {
    printf("  pcmpeqw %%%s, %%%s\n", x86_get_regname_sse(reg),
        x86_get_regname_sse(reg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x75;
    x86_emit_modrm_reg (p, reg, reg);

  } else if (value == 1) {
    printf("  pcmpeqw %%%s, %%%s\n", x86_get_regname_sse(reg),
        x86_get_regname_sse(reg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x75;
    x86_emit_modrm_reg (p, reg, reg);

    printf("  psrlw $15, %%%s\n", x86_get_regname_sse(reg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x71;
    x86_emit_modrm_reg (p, reg, 2);
    *p->codeptr++ = 15;
  } else {
    value &= 0xffff;
    value |= (value<<16);

    x86_emit_mov_imm_reg (p, 4, value, X86_ECX);

    printf("  movd %%ecx, %%%s\n", x86_get_regname_sse(reg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x6e;
    x86_emit_modrm_reg (p, X86_ECX, reg);

    printf("  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
        x86_get_regname_sse(reg));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x70;
    x86_emit_modrm_reg (p, reg, reg);
    *p->codeptr++ = 0x00;
  }
}

static void
sse_rule_copyw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  movdqa %%%s, %%%s\n",
      x86_get_regname_sse(p->vars[insn->args[1]].alloc),
      x86_get_regname_sse(p->vars[insn->args[0]].alloc));

  *p->codeptr++ = 0x66;
  x86_emit_rex (p, 0, p->vars[insn->args[1]].alloc, 0,
      p->vars[insn->args[0]].alloc);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6f;
  x86_emit_modrm_reg (p, p->vars[insn->args[1]].alloc,
      p->vars[insn->args[0]].alloc);
}

static void
sse_emit_66_rex_0f (OrcProgram *p, OrcInstruction *insn, int code,
    const char *insn_name)
{
  printf("  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(p->vars[insn->args[2]].alloc),
      x86_get_regname_sse(p->vars[insn->args[0]].alloc));

  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  if (code & 0xff00) {
    *p->codeptr++ = code >> 8;
    *p->codeptr++ = code & 0xff;
  } else {
    *p->codeptr++ = code;
  }
  x86_emit_modrm_reg (p, p->vars[insn->args[2]].alloc,
      p->vars[insn->args[0]].alloc);
}

#if 0
static void
sse_rule_addw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  sse_emit_66_rex_0f (p, insn, 0xfd, "paddw");
}

static void
sse_rule_subw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  sse_emit_66_rex_0f (p, insn, 0xf9, "psubw");
}

static void
sse_rule_mullw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  sse_emit_66_rex_0f (p, insn, 0xd5, "pmullw");
}
#endif

#define BINARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcProgram *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_66_rex_0f (p, insn, code, insn_name); \
}

BINARY(addb,"paddb",0xfc)
BINARY(addssb,"paddsb",0xec)
BINARY(addusb,"paddusb",0xdc)
BINARY(andb,"pand",0xdb)
BINARY(andnb,"pandn",0xdf)
BINARY(avgub,"pavgb",0xe0)
BINARY(cmpeqb,"pcmpeqb",0x74)
BINARY(cmpgtsb,"pcmpgtb",0x64)
BINARY(maxsb,"pmaxsb",0x383c)
BINARY(maxub,"pmaxub",0xde)
BINARY(minsb,"pminsb",0x3838)
BINARY(minub,"pminub",0xda)
//BINARY(mullb,"pmullb",0xd5)
//BINARY(mulhsb,"pmulhb",0xe5)
//BINARY(mulhub,"pmulhub",0xe4)
BINARY(orb,"por",0xeb)
BINARY(subb,"psubb",0xf8)
BINARY(subssb,"psubsb",0xe8)
BINARY(subusb,"psubusb",0xd8)
BINARY(xorb,"pxor",0xef)

BINARY(addw,"paddw",0xfd)
BINARY(addssw,"paddsw",0xed)
BINARY(addusw,"paddusw",0xdd)
BINARY(andw,"pand",0xdb)
BINARY(andnw,"pandn",0xdf)
BINARY(avguw,"pavgw",0xe3)
BINARY(cmpeqw,"pcmpeqw",0x75)
BINARY(cmpgtsw,"pcmpgtw",0x65)
BINARY(maxsw,"pmaxsw",0xee)
BINARY(maxuw,"pmaxuw",0x383e)
BINARY(minsw,"pminsw",0xea)
BINARY(minuw,"pminuw",0x383a)
BINARY(mullw,"pmullw",0xd5)
BINARY(mulhsw,"pmulhw",0xe5)
BINARY(mulhuw,"pmulhuw",0xe4)
BINARY(orw,"por",0xeb)
BINARY(subw,"psubw",0xf9)
BINARY(subssw,"psubsw",0xe9)
BINARY(subusw,"psubusw",0xd9)
BINARY(xorw,"pxor",0xef)

BINARY(addl,"paddd",0xfe)
//BINARY(addssl,"paddsd",0xed)
//BINARY(addusl,"paddusd",0xdd)
BINARY(andl,"pand",0xdb)
BINARY(andnl,"pandn",0xdf)
//BINARY(avgul,"pavgd",0xe3)
BINARY(cmpeql,"pcmpeqd",0x76)
BINARY(cmpgtsl,"pcmpgtd",0x66)
BINARY(maxsl,"pmaxsd",0x383d)
BINARY(maxul,"pmaxud",0x383f)
BINARY(minsl,"pminsd",0x3839)
BINARY(minul,"pminud",0x383b)
BINARY(mulll,"pmulld",0x3840)
BINARY(mulhsl,"pmulhd",0xe5)
BINARY(mulhul,"pmulhud",0xe4)
BINARY(orl,"por",0xeb)
BINARY(subl,"psubd",0xfa)
//BINARY(subssl,"psubsd",0xe9)
//BINARY(subusl,"psubusd",0xd9)
BINARY(xorl,"pxor",0xef)


static void
sse_rule_shlw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->args[2]].vartype == ORC_VAR_TYPE_CONST) {
    printf("  psllw $%d, %%%s\n",
        p->vars[insn->args[2]].value,
        x86_get_regname_sse(p->vars[insn->args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x71;
    x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 6);
    *p->codeptr++ = p->vars[insn->args[2]].value;
  } else {
    /* FIXME this doesn't work quite right */
    printf("  psllw %%%s, %%%s\n",
        x86_get_regname_sse(p->vars[insn->args[2]].alloc),
        x86_get_regname_sse(p->vars[insn->args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xf1;
    x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc,
        p->vars[insn->args[2]].alloc);
  }
}

static void
sse_rule_shrsw (OrcProgram *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->args[2]].vartype == ORC_VAR_TYPE_CONST) {
    printf("  psraw $%d, %%%s\n",
        p->vars[insn->args[2]].value,
        x86_get_regname_sse(p->vars[insn->args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x71;
    x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc, 4);
    *p->codeptr++ = p->vars[insn->args[2]].value;
  } else {
    /* FIXME this doesn't work quite right */
    printf("  psraw %%%s, %%%s\n",
        x86_get_regname_sse(p->vars[insn->args[2]].alloc),
        x86_get_regname_sse(p->vars[insn->args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xe1;
    x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc,
        p->vars[insn->args[2]].alloc);
  }
}

static void
sse_rule_convsuswb (OrcProgram *p, void *user, OrcInstruction *insn)
{
  printf("  packuswb %%%s, %%%s\n",
      x86_get_regname_sse(p->vars[insn->args[1]].alloc),
      x86_get_regname_sse(p->vars[insn->args[0]].alloc));

  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x67;
  x86_emit_modrm_reg (p, p->vars[insn->args[0]].alloc,
      p->vars[insn->args[1]].alloc);
}

void
orc_program_sse_register_rules (void)
{
#define REG(x) \
  orc_rule_register ( #x , ORC_TARGET_SSE, sse_rule_ ## x, NULL)

  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(andnb);
  //REG(avgsb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(maxsb);
  REG(maxub);
  REG(minsb);
  REG(minub);
  //REG(mullb);
  //REG(mulhsb);
  //REG(mulhub);
  REG(orb);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

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
  REG(maxuw);
  REG(minsw);
  REG(minuw);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);
  REG(orw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(addl);
  //REG(addssl);
  //REG(addusl);
  REG(andl);
  REG(andnl);
  //REG(avgsl);
  //REG(avgul);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  REG(mulhsl);
  REG(mulhul);
  REG(orl);
  REG(subl);
  //REG(subssl);
  //REG(subusl);
  REG(xorl);

  orc_rule_register ("copyw", ORC_TARGET_SSE, sse_rule_copyw, NULL);
  orc_rule_register ("shlw", ORC_TARGET_SSE, sse_rule_shlw, NULL);
  orc_rule_register ("shrsw", ORC_TARGET_SSE, sse_rule_shrsw, NULL);
  orc_rule_register ("convsuswb", ORC_TARGET_SSE, sse_rule_convsuswb, NULL);
}

