
#include <orc-pixel/orcpixel.h>
#include <orc/orc.h>
#include <orc/orcdebug.h>
#include <orc/orcsse.h>

#include <stdlib.h>



static void
sse_rule_compin (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int tmpreg2 = X86_XMM7;

  orc_sse_emit_660f (p, "punpcklbw", 0x60,
      p->vars[insn->src_args[0]].alloc,
      p->tmpreg);
  ORC_ASM_CODE(p,"  psrlw $8, %%%s\n",
      orc_x86_get_regname_sse(p->tmpreg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  orc_x86_emit_modrm_reg (p, p->tmpreg, 2);
  *p->codeptr++ = 8;
  
  orc_sse_emit_660f (p, "punpcklbw", 0x60,
      p->vars[insn->src_args[1]].alloc,
      tmpreg2);
  if (p->loop_shift > 0) {
    orc_sse_emit_660f (p, "punpcklwd", 0x61,
        tmpreg2, tmpreg2);
    orc_sse_emit_660f (p, "punpckldq", 0x62,
        tmpreg2, tmpreg2);
  } else {
    orc_sse_emit_f20f (p, "pshuflw $00,", 0x70,
        tmpreg2, tmpreg2);
    *p->codeptr++ = 0x00;
  }
  ORC_ASM_CODE(p,"  psrlw $8, %%%s\n",
      orc_x86_get_regname_sse(tmpreg2));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  orc_x86_emit_modrm_reg (p, tmpreg2, 2);
  *p->codeptr++ = 8;

  orc_sse_emit_660f (p, "pmullw", 0xd5,
      p->tmpreg,
      tmpreg2);

  ORC_ASM_CODE(p,"  movl $0x00800080, %%ecx\n");
  orc_x86_emit_rex(p, 4, X86_ECX, 0, 0);
  *p->codeptr++ = 0xb8 + orc_x86_get_regnum(X86_ECX);
  *p->codeptr++ = 0x80;
  *p->codeptr++ = 0x00;
  *p->codeptr++ = 0x80;
  *p->codeptr++ = 0x00;

  ORC_ASM_CODE(p,"  movd %%ecx, %%%s\n", orc_x86_get_regname_sse(p->tmpreg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x6e;
  orc_x86_emit_modrm_reg (p, X86_ECX, p->tmpreg);

  ORC_ASM_CODE(p,"  pshufd $0, %%%s, %%%s\n", orc_x86_get_regname_sse(p->tmpreg),
      orc_x86_get_regname_sse(p->tmpreg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  orc_x86_emit_modrm_reg (p, p->tmpreg, p->tmpreg);
  *p->codeptr++ = 0x00;

  orc_sse_emit_660f (p, "paddw", 0xfd,
      p->tmpreg,
      tmpreg2);
  orc_sse_emit_660f (p, "movdqa", 0x6f,
      tmpreg2,
      p->tmpreg);

  ORC_ASM_CODE(p,"  psrlw $8, %%%s\n",
      orc_x86_get_regname_sse(p->tmpreg));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  orc_x86_emit_modrm_reg (p, p->tmpreg, 2);
  *p->codeptr++ = 8;

  orc_sse_emit_660f (p, "paddw", 0xfd,
      p->tmpreg,
      tmpreg2);

  ORC_ASM_CODE(p,"  psrlw $8, %%%s\n",
      orc_x86_get_regname_sse(tmpreg2));
  *p->codeptr++ = 0x66;
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x71;
  orc_x86_emit_modrm_reg (p, tmpreg2, 2);
  *p->codeptr++ = 8;

  orc_sse_emit_660f (p, "packuswb", 0x67,
      tmpreg2,
      tmpreg2);

  orc_sse_emit_660f (p, "movdqa", 0x6f,
      tmpreg2,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_compadd (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "paddusb", 0xdc,
      p->vars[insn->src_args[1]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}


void
orc_pixel_sse_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("pixel"),
      orc_target_get_by_name ("sse"), ORC_TARGET_SSE_SSE2);

  orc_rule_register (rule_set, "compin", sse_rule_compin, NULL);
  orc_rule_register (rule_set, "compadd", sse_rule_compadd, NULL);
}



#if 0
#define COMPOSITE_OVER(d,s,m) ((d) + (s) - ORC_MULDIV_255((d),(m)))
#define COMPOSITE_ADD(d,s) ORC_CLAMP((d) + (s), 0, 255)
#define COMPOSITE_IN(s,m) ORC_MULDIV_255((s),(m))

#define ORC_DIVIDE_255(x) ((((x)+128) + (((x)+128)>>8))>>8)
#define ORC_MULDIV_255(a,b) ORC_DIVIDE_255((a)*(b))


#define ORC_ARGB_A(x) (((x)>>24)&0xff)
#define ORC_ARGB_R(x) (((x)>>16)&0xff)
#define ORC_ARGB_G(x) (((x)>>8)&0xff)
#define ORC_ARGB_B(x) (((x)>>0)&0xff)
#define ORC_ARGB(a,r,g,b) ((((b)&0xff)<<0)|(((g)&0xff)<<8)|(((r)&0xff)<<16)|(((a)&0xff)<<24))


static void
compin (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src = ex->src_values[0];
  unsigned int mask = ex->src_values[1];

  ex->dest_values[0] = ORC_ARGB(
      COMPOSITE_IN(ORC_ARGB_A(src), mask),
      COMPOSITE_IN(ORC_ARGB_R(src), mask),
      COMPOSITE_IN(ORC_ARGB_G(src), mask),
      COMPOSITE_IN(ORC_ARGB_B(src), mask));
}

static void
compover (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src1 = ex->src_values[0];
  unsigned int src2 = ex->src_values[1];
  unsigned int a;

  a = ORC_ARGB_A(src2);
  ex->dest_values[0] = ORC_ARGB(
      COMPOSITE_OVER(ORC_ARGB_A(src1), ORC_ARGB_A(src2), a),
      COMPOSITE_OVER(ORC_ARGB_R(src1), ORC_ARGB_R(src2), a),
      COMPOSITE_OVER(ORC_ARGB_G(src1), ORC_ARGB_G(src2), a),
      COMPOSITE_OVER(ORC_ARGB_B(src1), ORC_ARGB_B(src2), a));
}

static void
compovera (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src1 = ex->src_values[0];
  unsigned int src2 = ex->src_values[1];

  ex->dest_values[0] = COMPOSITE_OVER(src1, src2, src2);
}

static void
compadd (OrcOpcodeExecutor *ex, void *user)
{
  unsigned int src1 = ex->src_values[0];
  unsigned int src2 = ex->src_values[1];

  ex->dest_values[0] = ORC_ARGB(
      COMPOSITE_ADD(ORC_ARGB_A(src1), ORC_ARGB_A(src2)),
      COMPOSITE_ADD(ORC_ARGB_R(src1), ORC_ARGB_R(src2)),
      COMPOSITE_ADD(ORC_ARGB_G(src1), ORC_ARGB_G(src2)),
      COMPOSITE_ADD(ORC_ARGB_B(src1), ORC_ARGB_B(src2)));
}

static OrcStaticOpcode opcodes[] = {
  { "compin", compin, NULL, 0, { 4 }, { 4, 1 } },
  { "compover", compover, NULL, 0, { 4 }, { 4, 4 } },
  { "compovera", compovera, NULL, 0, { 1 }, { 1, 1 } },
  { "compadd", compadd, NULL, 0, { 4 }, { 4, 4 } },

  { "" }
};

#endif
