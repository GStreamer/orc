
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcmmx.h>

#define SIZE 65536

/* mmx rules */

void
orc_mmx_emit_loadil (OrcCompiler *p, int reg, int value)
{
  if (value == 0) {
    orc_mmx_emit_pxor (p, reg, reg);
  } else {
    orc_x86_emit_mov_imm_reg (p, 4, value, p->gp_tmpreg);
    orc_x86_emit_mov_reg_mmx (p, p->gp_tmpreg, reg);
    orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(1,0,1,0), reg, reg);
  }
}

void
orc_mmx_emit_loadib (OrcCompiler *p, int reg, int value)
{
  value &= 0xff;
  value |= (value<<8);
  value |= (value<<16);
  orc_mmx_emit_loadil (p, reg, value);
}

void
orc_mmx_emit_loadiw (OrcCompiler *p, int reg, int value)
{
  value &= 0xffff;
  value |= (value<<16);
  orc_mmx_emit_loadil (p, reg, value);
}

void
orc_mmx_emit_loadpb (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_mmx (p, 4, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_mmx_emit_punpcklbw (p, reg, reg);

  orc_mmx_emit_pshufw (p, 0, reg, reg);
}

void
orc_mmx_emit_loadpw (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_mmx (p, 4, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_mmx_emit_pshufw (p, 0, reg, reg);
}

void
orc_mmx_emit_loadpl (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_mmx (p, 4, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(1,0,1,0), reg, reg);
}

void
orc_mmx_emit_loadpq (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_mmx (p, 8, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);
}

static void
mmx_rule_copyx (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_mmx_emit_movq (p, 
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

#define UNARY(opcode,insn_name,code) \
static void \
mmx_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_mmx_emit_660f (p, insn_name, code, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

#define BINARY(opcode,insn_name,code) \
static void \
mmx_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_mmx_emit_660f (p, insn_name, code, \
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
BINARY(maxub,"pmaxub",0xde)
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
BINARY(minsw,"pminsw",0xea)
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
//BINARY(mulhsl,"pmulhd",0xe5)
//BINARY(mulhul,"pmulhud",0xe4)
BINARY(orl,"por",0xeb)
//UNARY(signl,"psignd",0x380a)
BINARY(subl,"psubd",0xfa)
//BINARY(subssl,"psubsd",0xe9)
//BINARY(subusl,"psubusd",0xd9)
BINARY(xorl,"pxor",0xef)


static void
mmx_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_paddw (p, src, dest);
}

static void
mmx_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_paddd (p, src, dest);
}

static void
mmx_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = p->vars[insn->src_args[0]].alloc;
  int src2 = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_movq (p, src1, p->tmpreg);
  orc_mmx_emit_psadbw (p, src2, p->tmpreg);
  orc_mmx_emit_paddd (p, p->tmpreg, dest);
}

static void
mmx_rule_signX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int type = ORC_PTR_TO_INT(user);
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int imm_vals[] = { 0x01010101, 0x00010001, 0x00000001 };
  const char * names[] = { "psignb", "psignw", "psignd" };
  int codes[] = { 0x3808, 0x3809, 0x380a };

  if (src == dest) {
    orc_mmx_emit_movq (p, src, p->tmpreg);
    src = p->tmpreg;
  }

  orc_x86_emit_mov_imm_reg (p, 4, imm_vals[type], p->gp_tmpreg);

  orc_x86_emit_mov_reg_mmx (p, p->gp_tmpreg, dest);
  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(1,0,1,0), dest, dest);

  orc_mmx_emit_660f (p, names[type], codes[type], src, dest);
}

static void
mmx_rule_shift (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int type = ORC_PTR_TO_INT(user);
  int imm_code1[] = { 0x71, 0x71, 0x71, 0x72, 0x72, 0x72 };
  int imm_code2[] = { 6, 2, 4, 6, 2, 4 };
  int reg_code[] = { 0xf1, 0xd1, 0xe1, 0xf2, 0xd2, 0xe2 };
  const char *code[] = { "psllw", "psrlw", "psraw", "pslld", "psrld", "psrad" };

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_mmx_emit_shiftimm (p, code[type], imm_code1[type], imm_code2[type],
        p->vars[insn->src_args[1]].value,
        p->vars[insn->dest_args[0]].alloc);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    /* FIXME this is a gross hack to reload the register with a
     * 64-bit version of the parameter. */
    orc_x86_emit_mov_memoffset_mmx (p, 4, 
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]),
        p->exec_reg, p->tmpreg, FALSE);

    orc_mmx_emit_660f (p, code[type], reg_code[type],
        p->tmpreg,
        p->vars[insn->dest_args[0]].alloc);
  } else {
    ORC_COMPILER_ERROR(p,"rule only works with constants or params");
  }
}

static void
mmx_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_punpcklbw (p, src, dest);
  orc_mmx_emit_psraw (p, 8, dest);
}

static void
mmx_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME should do this by unpacking with a zero reg */
  orc_mmx_emit_punpcklbw (p, src, dest);
  orc_mmx_emit_psrlw (p, 8, dest);
}

static void
mmx_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_packsswb (p, src, dest);
}

static void
mmx_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_packuswb (p, src, dest);
}

static void
mmx_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_psllw (p, 8, dest);
  orc_mmx_emit_psrlw (p, 8, dest);
  orc_mmx_emit_packuswb (p, dest, dest);
}

static void
mmx_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_punpcklwd (p, src, dest);
  orc_mmx_emit_psrad (p, 16, dest);
}

static void
mmx_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME should do this by unpacking with a zero reg */
  orc_mmx_emit_punpcklwd (p, src, dest);
  orc_mmx_emit_psrld (p, 16, dest);
}

static void
mmx_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */
  if (dest != src) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pslld (p, 16, dest);
  orc_mmx_emit_psrad (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_packssdw (p, src, dest);
}

static void
mmx_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pmulhw (p, src, tmp);
  orc_mmx_emit_pmullw (p, src, dest);
  orc_mmx_emit_punpcklwd (p, tmp, dest);
}

static void
mmx_rule_select0lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */
  /* same as convlw */

  if (dest != src) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pslld (p, 16, dest);
  orc_mmx_emit_psrad (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_psrad (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_select0wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */
  /* same as convwb */

  if (dest != src) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_psllw (p, 8, dest);
  orc_mmx_emit_psraw (p, 8, dest);
  orc_mmx_emit_packsswb (p, dest, dest);
}

static void
mmx_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_psraw (p, 8, dest);
  orc_mmx_emit_packsswb (p, dest, dest);
}

static void
mmx_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_punpcklbw (p, src, dest);
}

static void
mmx_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_punpcklwd (p, src, dest);
}

static void
mmx_emit_load_mask (OrcCompiler *p, unsigned int mask1, unsigned int mask2)
{
  int tmp = p->tmpreg;
  int tmp2 = X86_MM7;

  orc_x86_emit_mov_imm_reg (p, 4, mask1, p->gp_tmpreg);
  orc_x86_emit_mov_reg_mmx (p, p->gp_tmpreg, tmp);
  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(1,0,1,0), tmp, tmp);
  orc_x86_emit_mov_imm_reg (p, 4, mask2, p->gp_tmpreg);
  orc_x86_emit_mov_reg_mmx (p, p->gp_tmpreg, tmp2);
  orc_mmx_emit_punpcklbw (p, tmp2, tmp2);
  orc_mmx_emit_punpcklwd (p, tmp2, tmp2);
  orc_mmx_emit_paddb (p, tmp2, tmp);
}

static void
mmx_rule_select0lw_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  mmx_emit_load_mask (p, 0x05040100, 0x08000800);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
mmx_rule_select1lw_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  mmx_emit_load_mask (p, 0x07060302, 0x08000800);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
mmx_rule_select0wb_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  mmx_emit_load_mask (p, 0x06040200, 0x08000800);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
mmx_rule_select1wb_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  mmx_emit_load_mask (p, 0x07050301, 0x08000800);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

/* slow rules */

static void
mmx_rule_swapw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  mmx_emit_load_mask (p, 0x02030001, 0x0c080400);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
mmx_rule_swapl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  mmx_emit_load_mask (p, 0x00010203, 0x0c080400);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
mmx_rule_maxuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_x86_emit_mov_imm_reg (p, 4, 0x80008000, p->gp_tmpreg);
  orc_x86_emit_mov_reg_mmx (p, p->gp_tmpreg, tmp);
  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(1,0,1,0), tmp, tmp);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_pmaxsw (p, src, dest);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
}

static void
mmx_rule_minuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_loadiw (p, tmp, 0x8000);

  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_pminsw (p, src, dest);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
}

static void
mmx_rule_avgsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_loadib (p, tmp, 0x80);

  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_pavgb (p, src, dest);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
}

static void
mmx_rule_avgsw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_loadiw (p, tmp, 0x8000);

  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_pavgw (p, src, dest);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
}

static void
mmx_rule_maxsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pcmpgtb (p, src, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src, tmp);
  orc_mmx_emit_por (p, tmp, dest);
}

static void
mmx_rule_minsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_pcmpgtb (p, dest, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src, tmp);
  orc_mmx_emit_por (p, tmp, dest);
}

static void
mmx_rule_maxsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pcmpgtd (p, src, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src, tmp);
  orc_mmx_emit_por (p, tmp, dest);
}

static void
mmx_rule_minsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_pcmpgtd (p, dest, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src, tmp);
  orc_mmx_emit_por (p, tmp, dest);
}

static void
mmx_rule_maxul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_loadil (p, tmp, 0x80000000);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pcmpgtd (p, src, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src, tmp);
  orc_mmx_emit_por (p, tmp, dest);

  orc_mmx_emit_loadil (p, tmp, 0x80000000);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
}

static void
mmx_rule_minul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_mmx_emit_loadil (p, tmp, 0x80000000);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_pcmpgtd (p, dest, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src, tmp);
  orc_mmx_emit_por (p, tmp, dest);

  orc_mmx_emit_loadil (p, tmp, 0x80000000);
  orc_mmx_emit_pxor (p, tmp, src);
  orc_mmx_emit_pxor (p, tmp, dest);
}


void
orc_compiler_mmx_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

#define REG(x) \
  orc_rule_register (rule_set, #x , mmx_rule_ ## x, NULL)

  /* MMX */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_MMX_MMX);

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

  REG(select0lw);
  REG(select1lw);
  REG(select0wb);
  REG(select1wb);
  REG(mergebw);
  REG(mergewl);

  orc_rule_register (rule_set, "copyb", mmx_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyw", mmx_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyl", mmx_rule_copyx, NULL);

  orc_rule_register (rule_set, "shlw", mmx_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shruw", mmx_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrsw", mmx_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shll", mmx_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrul", mmx_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shrsl", mmx_rule_shift, (void *)5);

  orc_rule_register (rule_set, "convsbw", mmx_rule_convsbw, NULL);
  orc_rule_register (rule_set, "convubw", mmx_rule_convubw, NULL);
  orc_rule_register (rule_set, "convssswb", mmx_rule_convssswb, NULL);
  orc_rule_register (rule_set, "convsuswb", mmx_rule_convsuswb, NULL);
  orc_rule_register (rule_set, "convwb", mmx_rule_convwb, NULL);

  orc_rule_register (rule_set, "convswl", mmx_rule_convswl, NULL);
  orc_rule_register (rule_set, "convuwl", mmx_rule_convuwl, NULL);
  orc_rule_register (rule_set, "convssslw", mmx_rule_convssslw, NULL);

  orc_rule_register (rule_set, "mulswl", mmx_rule_mulswl, NULL);

  orc_rule_register (rule_set, "accw", mmx_rule_accw, NULL);
  orc_rule_register (rule_set, "accl", mmx_rule_accl, NULL);
  orc_rule_register (rule_set, "accsadubl", mmx_rule_accsadubl, NULL);

  /* slow rules */
  orc_rule_register (rule_set, "maxuw", mmx_rule_maxuw_slow, NULL);
  orc_rule_register (rule_set, "minuw", mmx_rule_minuw_slow, NULL);
  orc_rule_register (rule_set, "avgsb", mmx_rule_avgsb_slow, NULL);
  orc_rule_register (rule_set, "avgsw", mmx_rule_avgsw_slow, NULL);
  orc_rule_register (rule_set, "maxsb", mmx_rule_maxsb_slow, NULL);
  orc_rule_register (rule_set, "minsb", mmx_rule_minsb_slow, NULL);
  orc_rule_register (rule_set, "maxsl", mmx_rule_maxsl_slow, NULL);
  orc_rule_register (rule_set, "minsl", mmx_rule_minsl_slow, NULL);
  orc_rule_register (rule_set, "maxul", mmx_rule_maxul_slow, NULL);
  orc_rule_register (rule_set, "minul", mmx_rule_minul_slow, NULL);
  orc_rule_register (rule_set, "convlw", mmx_rule_convlw, NULL);

  /* SSE 3 -- no rules */

  /* SSSE 3 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_MMX_SSSE3);

  orc_rule_register (rule_set, "signb", mmx_rule_signX, (void *)0);
  orc_rule_register (rule_set, "signw", mmx_rule_signX, (void *)1);
  orc_rule_register (rule_set, "signl", mmx_rule_signX, (void *)2);
  REG(absb);
  REG(absw);
  REG(absl);
  orc_rule_register (rule_set, "swapw", mmx_rule_swapw_slow, NULL);
  orc_rule_register (rule_set, "swapl", mmx_rule_swapl_slow, NULL);
  orc_rule_register (rule_set, "select0lw", mmx_rule_select0lw_sse3, NULL);
  orc_rule_register (rule_set, "select1lw", mmx_rule_select1lw_sse3, NULL);
  orc_rule_register (rule_set, "select0wb", mmx_rule_select0wb_sse3, NULL);
  orc_rule_register (rule_set, "select1wb", mmx_rule_select1wb_sse3, NULL);

  /* SSE 4.1 -- no rules */
#if 0
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_MMX_SSE4_1);

  REG(maxsb);
  REG(minsb);
  REG(maxuw);
  REG(minuw);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  orc_rule_register (rule_set, "convsuslw", mmx_rule_convsuslw, NULL);
#endif

  /* SSE 4.2 -- no rules */

  /* SSE 4a -- no rules */
}

