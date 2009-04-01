
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/x86.h>

#define SIZE 65536

int ssse3 = TRUE;
int sse41 = FALSE;

static void
sse_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src),
      x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  x86_emit_modrm_reg (p, src, dest);
}

static void
sse_emit_660f38 (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src),
      x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x38;
  *p->codeptr++ = code;
  x86_emit_modrm_reg (p, src, dest);
}

/* sse rules */

void
sse_emit_loadil (OrcCompiler *p, int reg, int value)
{
  if (value == 0) {
    ORC_ASM_CODE(p,"  pxor %%%s, %%%s\n", x86_get_regname_sse(reg),
        x86_get_regname_sse(reg));
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0xef;
    x86_emit_modrm_reg (p, reg, reg);
  } else {
    x86_emit_mov_imm_reg (p, 4, value, X86_ECX);

    ORC_ASM_CODE(p,"  movd %%ecx, %%%s\n", x86_get_regname_sse(reg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x6e;
    x86_emit_modrm_reg (p, X86_ECX, reg);

    ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
        x86_get_regname_sse(reg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x70;
    x86_emit_modrm_reg (p, reg, reg);
    *p->codeptr++ = 0x00;
  }
}

void
sse_emit_loadib (OrcCompiler *p, int reg, int value)
{
  value &= 0xff;
  value |= (value<<8);
  value |= (value<<16);
  sse_emit_loadil (p, reg, value);
}

void
sse_emit_loadiw (OrcCompiler *p, int reg, int value)
{
  value &= 0xffff;
  value |= (value<<16);
  sse_emit_loadil (p, reg, value);
}

void
sse_emit_loadpb (OrcCompiler *p, int reg, int param)
{
  ORC_ASM_CODE(p,"  movd %d(%%%s), %%%s\n",
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      x86_get_regname_ptr(x86_exec_ptr),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6e;
  x86_emit_modrm_memoffset (p, reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]), x86_exec_ptr);

  sse_emit_660f (p, "punpcklbw", 0x60, reg, reg);

  ORC_ASM_CODE(p,"  pshuflw $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0xf2;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, reg, reg);
  *p->codeptr++ = 0x00;

  ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, reg, reg);
  *p->codeptr++ = 0x00;
}

void
sse_emit_loadpw (OrcCompiler *p, int reg, int param)
{
  ORC_ASM_CODE(p,"  movd %d(%%%s), %%%s\n",
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      x86_get_regname_ptr(x86_exec_ptr),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6e;
  x86_emit_modrm_memoffset (p, reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]), x86_exec_ptr);

  ORC_ASM_CODE(p,"  pshuflw $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0xf2;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, reg, reg);
  *p->codeptr++ = 0x00;

  ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, reg, reg);
  *p->codeptr++ = 0x00;
}

void
sse_emit_loadpl (OrcCompiler *p, int reg, int param)
{
  ORC_ASM_CODE(p,"  movd %d(%%%s), %%%s\n",
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      x86_get_regname_ptr(x86_exec_ptr),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6e;
  x86_emit_modrm_memoffset (p, reg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]), x86_exec_ptr);

  ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(reg),
      x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, reg, reg);
  *p->codeptr++ = 0x00;
}

static void
sse_rule_copyx (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "movdqa", 0x6f,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_emit_66_rex_0f (OrcCompiler *p, OrcInstruction *insn, int code,
    const char *insn_name, int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      x86_get_regname_sse(src), x86_get_regname_sse(dest));

  *p->codeptr++ = 0x66;
  x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  if (code & 0xff00) {
    *p->codeptr++ = code >> 8;
    *p->codeptr++ = code & 0xff;
  } else {
    *p->codeptr++ = code;
  }
  x86_emit_modrm_reg (p, src, dest);
}

#define UNARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_66_rex_0f (p, insn, code, insn_name, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

#define BINARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  sse_emit_66_rex_0f (p, insn, code, insn_name, \
      p->vars[insn->src_args[1]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}


UNARY(absb,"pabsb",0x381c)
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
//UNARY(signb,"psignb",0x3808)
BINARY(subb,"psubb",0xf8)
BINARY(subssb,"psubsb",0xe8)
BINARY(subusb,"psubusb",0xd8)
BINARY(xorb,"pxor",0xef)

UNARY(absw,"pabsw",0x381d)
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
//UNARY(signw,"psignw",0x3809)
BINARY(subw,"psubw",0xf9)
BINARY(subssw,"psubsw",0xe9)
BINARY(subusw,"psubusw",0xd9)
BINARY(xorw,"pxor",0xef)

UNARY(absl,"pabsd",0x381e)
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
//BINARY(mulhsl,"pmulhd",0xe5)
//BINARY(mulhul,"pmulhud",0xe4)
BINARY(orl,"por",0xeb)
//UNARY(signl,"psignd",0x380a)
BINARY(subl,"psubd",0xfa)
//BINARY(subssl,"psubsd",0xe9)
//BINARY(subusl,"psubusd",0xd9)
BINARY(xorl,"pxor",0xef)


static void
sse_rule_signX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int imm_vals[] = { 0x01010101, 0x00010001, 0x00000001 };
  const char * names[] = { "psignb", "psignw", "psignd" };
  int codes[] = { 0x08, 0x09, 0x0a };

  if (src == dest) {
    sse_emit_660f (p, "movdqa", 0x6f, src, p->tmpreg);
    src = p->tmpreg;
  }

  x86_emit_mov_imm_reg (p, 4, imm_vals[((int)user)], X86_ECX);

  ORC_ASM_CODE(p,"  movd %%ecx, %%%s\n", x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6e;
  x86_emit_modrm_reg (p, X86_ECX, dest);

  ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(dest),
      x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, dest, dest);
  *p->codeptr++ = 0x00;

  sse_emit_660f38 (p, names[((int)user)], codes[((int)user)], src, dest);
}

static void
sse_rule_shift (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int type = (int)user;
  int imm_code1[] = { 0x71, 0x71, 0x71, 0x72, 0x72, 0x72 };
  int imm_code2[] = { 6, 2, 4, 6, 2, 4 };
  int reg_code[] = { 0xf1, 0xd1, 0xe1, 0xf2, 0xd2, 0xe2 };
  const char *code[] = { "psllw", "psrlw", "psraw", "pslld", "psrld", "psrad" };

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    ORC_ASM_CODE(p,"  %s $%d, %%%s\n", code[type],
        p->vars[insn->src_args[1]].value,
        x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = imm_code1[type];
    x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, imm_code2[type]);
    *p->codeptr++ = p->vars[insn->src_args[1]].value;
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    /* FIXME this is a gross hack to reload the register with a
     * 64-bit version of the parameter. */
    ORC_ASM_CODE(p,"  movd %d(%%%s), %%%s\n",
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]),
        x86_get_regname_ptr(x86_exec_ptr),
        x86_get_regname_sse(p->tmpreg));
    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = 0x6e;
    x86_emit_modrm_memoffset (p,
        p->tmpreg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]),
        x86_exec_ptr);

    ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", code[type],
        x86_get_regname_sse(p->tmpreg),
        x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));

    *p->codeptr++ = 0x66;
    *p->codeptr++ = 0x0f;
    *p->codeptr++ = reg_code[type];
    x86_emit_modrm_reg (p, p->tmpreg,
        p->vars[insn->dest_args[0]].alloc);
  } else {
    ORC_PROGRAM_ERROR(p,"rule only works with constants or params");
  }
}

static void
sse_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "punpcklbw", 0x60,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);

  ORC_ASM_CODE(p,"  psraw $8, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 4);
  *p->codeptr++ = 8;
}

static void
sse_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* FIXME should do this by unpacking with a zero reg */

  sse_emit_660f (p, "punpcklbw", 0x60,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);

  ORC_ASM_CODE(p,"  psrlw $8, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 2);
  *p->codeptr++ = 8;

}

static void
sse_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "packsswb", 0x63,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "packuswb", 0x67,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  ORC_ASM_CODE(p,"  psllw $8, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 6);
  *p->codeptr++ = 8;

  ORC_ASM_CODE(p,"  psrlw $8, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 2);
  *p->codeptr++ = 8;

  sse_emit_660f (p, "packuswb", 0x67,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "punpcklwd", 0x61,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);

  ORC_ASM_CODE(p,"  psrad $16, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x72;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 4);
  *p->codeptr++ = 16;
}

static void
sse_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* FIXME should do this by unpacking with a zero reg */

  sse_emit_660f (p, "punpcklwd", 0x61,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);

  ORC_ASM_CODE(p,"  psrld $16, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x72;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 2);
  *p->codeptr++ = 16;

}

static void
sse_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  ORC_ASM_CODE(p,"  pslld $16, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x72;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 6);
  *p->codeptr++ = 16;

  ORC_ASM_CODE(p,"  psrad $16, %%%s\n",
      x86_get_regname_sse(p->vars[insn->dest_args[0]].alloc));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x72;
  x86_emit_modrm_reg (p, p->vars[insn->dest_args[0]].alloc, 4);
  *p->codeptr++ = 16;

  sse_emit_660f (p, "packssdw", 0x6b, dest, dest);
}

static void
sse_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f (p, "packssdw", 0x6b,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convsuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  sse_emit_660f38 (p, "packusdw", 0x2b,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  sse_emit_660f (p, "pmulhw", 0xe5, src, tmp);
  sse_emit_660f (p, "pmullw", 0xd5, src, dest);
  sse_emit_660f (p, "punpcklwd", 0x61, tmp, dest);
}

static void
sse_rule_maxuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  x86_emit_mov_imm_reg (p, 4, 0x80008000, X86_ECX);

  ORC_ASM_CODE(p,"  movd %%ecx, %%%s\n", x86_get_regname_sse(tmp));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6e;
  x86_emit_modrm_reg (p, X86_ECX, tmp);

  ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", x86_get_regname_sse(tmp),
      x86_get_regname_sse(tmp));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  x86_emit_modrm_reg (p, tmp, tmp);
  *p->codeptr++ = 0x00;

  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  sse_emit_660f (p, "pmaxsw", 0xee, src, dest);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_minuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_loadib (p, tmp, 0x80);

  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  sse_emit_660f (p, "pminsw", 0xea, src, dest);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_avgsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_loadib (p, tmp, 0x80);

  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  sse_emit_660f (p, "pavgb", 0xe0, src, dest);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_avgsw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_loadiw (p, tmp, 0x8000);

  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  sse_emit_660f (p, "pavgw", 0xe3, src, dest);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_maxsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  sse_emit_660f (p, "pcmpgtb", 0x64, src, tmp);
  sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_minsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_660f (p, "movdqa", 0x6f, src, tmp);
  sse_emit_660f (p, "pcmpgtb", 0x64, dest, tmp);
  sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_maxsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  sse_emit_660f (p, "pcmpgtd", 0x66, src, tmp);
  sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_minsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_660f (p, "movdqa", 0x6f, src, tmp);
  sse_emit_660f (p, "pcmpgtd", 0x66, dest, tmp);
  sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_maxul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_loadil (p, tmp, 0x80000000);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);

  sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  sse_emit_660f (p, "pcmpgtd", 0x66, src, tmp);
  sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  sse_emit_660f (p, "por", 0xeb, tmp, dest);

  sse_emit_loadil (p, tmp, 0x80000000);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_minul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  sse_emit_loadil (p, tmp, 0x80000000);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);

  sse_emit_660f (p, "movdqa", 0x6f, src, tmp);
  sse_emit_660f (p, "pcmpgtd", 0x66, dest, tmp);
  sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  sse_emit_660f (p, "por", 0xeb, tmp, dest);

  sse_emit_loadil (p, tmp, 0x80000000);
  sse_emit_660f (p, "pxor", 0xef, tmp, src);
  sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}


void
orc_compiler_sse_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

#define REG(x) \
  orc_rule_register (rule_set, #x , sse_rule_ ## x, NULL)

  /* SSE 2 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target);

  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(andnb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(maxub);
  REG(minub);
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
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(maxsw);
  REG(minsw);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);
  REG(orw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(addl);
  REG(andl);
  REG(andnl);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(orl);
  REG(subl);
  REG(xorl);

  orc_rule_register (rule_set, "copyb", sse_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyw", sse_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyl", sse_rule_copyx, NULL);

  orc_rule_register (rule_set, "shlw", sse_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shruw", sse_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrsw", sse_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shll", sse_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrul", sse_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shrsl", sse_rule_shift, (void *)5);

  orc_rule_register (rule_set, "convsbw", sse_rule_convsbw, NULL);
  orc_rule_register (rule_set, "convubw", sse_rule_convubw, NULL);
  orc_rule_register (rule_set, "convssswb", sse_rule_convssswb, NULL);
  orc_rule_register (rule_set, "convsuswb", sse_rule_convsuswb, NULL);
  orc_rule_register (rule_set, "convwb", sse_rule_convwb, NULL);

  orc_rule_register (rule_set, "convswl", sse_rule_convswl, NULL);
  orc_rule_register (rule_set, "convuwl", sse_rule_convuwl, NULL);
  orc_rule_register (rule_set, "convssslw", sse_rule_convssslw, NULL);

  orc_rule_register (rule_set, "mulswl", sse_rule_mulswl, NULL);

  /* slow rules */
  orc_rule_register (rule_set, "maxuw", sse_rule_maxuw_slow, NULL);
  orc_rule_register (rule_set, "minuw", sse_rule_minuw_slow, NULL);
  orc_rule_register (rule_set, "avgsb", sse_rule_avgsb_slow, NULL);
  orc_rule_register (rule_set, "avgsw", sse_rule_avgsw_slow, NULL);
  orc_rule_register (rule_set, "maxsb", sse_rule_maxsb_slow, NULL);
  orc_rule_register (rule_set, "minsb", sse_rule_minsb_slow, NULL);
  orc_rule_register (rule_set, "maxsl", sse_rule_maxsl_slow, NULL);
  orc_rule_register (rule_set, "minsl", sse_rule_minsl_slow, NULL);
  orc_rule_register (rule_set, "maxul", sse_rule_maxul_slow, NULL);
  orc_rule_register (rule_set, "minul", sse_rule_minul_slow, NULL);
  orc_rule_register (rule_set, "convlw", sse_rule_convlw, NULL);

  /* SSE 3 -- no rules */

  /* SSSE 3 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target);

  orc_rule_register (rule_set, "signb", sse_rule_signX, (void *)0);
  orc_rule_register (rule_set, "signw", sse_rule_signX, (void *)1);
  orc_rule_register (rule_set, "signl", sse_rule_signX, (void *)2);
  REG(absb);
  REG(absw);
  REG(absl);

if (0) {
  /* SSE 4.1 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target);

  REG(maxsb);
  REG(minsb);
  REG(maxuw);
  REG(minuw);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  orc_rule_register (rule_set, "convsuslw", sse_rule_convsuslw, NULL);
}

  /* SSE 4.2 -- no rules */

  /* SSE 4a -- no rules */

  /* SSE 5 -- no rules */
}

