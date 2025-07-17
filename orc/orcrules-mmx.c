
#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>
#include <orc/orcmmx.h>

#define ORC_REG_SIZE 8

static void
mmx_rule_loadpX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int reg;
  int size = ORC_PTR_TO_INT(user);

  if (src->vartype == ORC_VAR_TYPE_PARAM) {
    reg = dest->alloc;

    if (size == 8 && src->size == 8) {
      orc_x86_emit_mov_memoffset_mmx (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[0]]),
          compiler->exec_reg, reg, FALSE);
      /* FIXME yes, I understand this is terrible */
      orc_mmx_emit_pinsrw_memoffset (compiler, 2,
          (int)ORC_STRUCT_OFFSET(OrcExecutor,
            params[insn->src_args[0] + (ORC_VAR_T1 - ORC_VAR_P1)]) + 0,
          compiler->exec_reg, reg);
      orc_mmx_emit_pinsrw_memoffset (compiler, 3,
          (int)ORC_STRUCT_OFFSET(OrcExecutor,
            params[insn->src_args[0] + (ORC_VAR_T1 - ORC_VAR_P1)]) + 2,
          compiler->exec_reg, reg);
    } else {
      orc_x86_emit_mov_memoffset_mmx (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[0]]),
          compiler->exec_reg, reg, FALSE);
      if (size < 8) {
        if (size == 1) {
          orc_mmx_emit_punpcklbw (compiler, reg, reg);
        }
        if (size <= 2) {
          orc_mmx_emit_pshufw (compiler, ORC_MMX_SHUF(0,0,0,0), reg, reg);
        } else {
          orc_mmx_emit_pshufw (compiler, ORC_MMX_SHUF(1,0,1,0), reg, reg);
        }
      }
    }
  } else if (src->vartype == ORC_VAR_TYPE_CONST) {
    orc_compiler_load_constant_from_size_and_value (compiler, dest->alloc,
        size, src->value.i);
  } else {
    ORC_ASSERT(0);
  }
}

static void
mmx_rule_loadX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;

  offset = compiler->offset * src->size;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  } 
  switch (src->size << compiler->loop_shift) {
    case 1:
      orc_x86_emit_mov_memoffset_reg (compiler, 1, offset, ptr_reg,
          compiler->gp_tmpreg);
      orc_mmx_emit_movd_load_register (compiler, compiler->gp_tmpreg,
          dest->alloc);
      break;
    case 2:
      orc_mmx_emit_pxor (compiler, dest->alloc, dest->alloc);
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset, ptr_reg, dest->alloc);
      break;
    case 4:
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    case 8:
      orc_x86_emit_mov_memoffset_mmx (compiler, 8, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    case 16:
      orc_x86_emit_mov_memoffset_mmx (compiler, 16, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    default:
      ORC_COMPILER_ERROR (compiler, "bad load size %d",
          src->size << compiler->loop_shift);
      break;
  }

  src->update_type = 2;
}

static void
mmx_rule_loadoffX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;

  if (compiler->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_CONST) {
    ORC_COMPILER_ERROR (compiler, "code generation rule for %s only works with constant offset",
        insn->opcode->name);
    return;
  }

  offset = (compiler->offset + compiler->vars[insn->src_args[1]].value.i) *
    src->size;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  } 
  switch (src->size << compiler->loop_shift) {
    case 1:
      orc_x86_emit_mov_memoffset_reg (compiler, 1, offset, ptr_reg,
          compiler->gp_tmpreg);
      orc_mmx_emit_movd_load_register (compiler, compiler->gp_tmpreg,
          dest->alloc);
      break;
    case 2:
      orc_mmx_emit_pxor (compiler, dest->alloc, dest->alloc);
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset, ptr_reg, dest->alloc);
      break;
    case 4:
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    case 8:
      orc_x86_emit_mov_memoffset_mmx (compiler, 8, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    case 16:
      orc_x86_emit_mov_memoffset_mmx (compiler, 16, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    default:
      ORC_COMPILER_ERROR (compiler,"bad load size %d",
          src->size << compiler->loop_shift);
      break;
  }

  src->update_type = 2;
}

static void
mmx_rule_loadupib (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;
  int tmp = orc_compiler_get_temp_reg (compiler);

  offset = (compiler->offset * src->size) >> 1;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  } 
  switch (src->size << compiler->loop_shift) {
    case 1:
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset, ptr_reg, dest->alloc);
      orc_mmx_emit_movq (compiler, dest->alloc, tmp);
      orc_mmx_emit_psrlw_imm (compiler, 8, tmp);
      break;
    case 2:
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset, ptr_reg, dest->alloc);
      orc_mmx_emit_movq (compiler, dest->alloc, tmp);
      orc_mmx_emit_psrlw_imm (compiler, 8, tmp);
      break;
    case 4:
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset, ptr_reg, dest->alloc);
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset + 1, ptr_reg, tmp);
      break;
    case 8:
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, offset, ptr_reg,
          dest->alloc, FALSE);
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, offset + 1, ptr_reg,
          tmp, FALSE);
      break;
    case 16:
      orc_x86_emit_mov_memoffset_mmx (compiler, 8, offset, ptr_reg,
          dest->alloc, FALSE);
      orc_x86_emit_mov_memoffset_mmx (compiler, 8, offset + 1, ptr_reg,
          tmp, FALSE);
      break;
    case 32:
      orc_x86_emit_mov_memoffset_mmx (compiler, 16, offset, ptr_reg,
          dest->alloc, FALSE);
      orc_x86_emit_mov_memoffset_mmx (compiler, 16, offset + 1, ptr_reg,
          tmp, FALSE);
      break;
    default:
      ORC_COMPILER_ERROR(compiler,"bad load size %d",
          src->size << compiler->loop_shift);
      break;
  }

  orc_mmx_emit_pavgb (compiler, dest->alloc, tmp);
  orc_mmx_emit_punpcklbw (compiler, tmp, dest->alloc);

  src->update_type = 1;
  orc_compiler_release_temp_reg (compiler, tmp);
}

static void
mmx_rule_loadupdb (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;

  offset = (compiler->offset * src->size) >> 1;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  } 
  switch (src->size << compiler->loop_shift) {
    case 1:
    case 2:
      orc_x86_emit_mov_memoffset_reg (compiler, 1, offset, ptr_reg,
          compiler->gp_tmpreg);
      orc_mmx_emit_movd_load_register (compiler, compiler->gp_tmpreg, dest->alloc);
      break;
    case 4:
      orc_mmx_emit_pinsrw_memoffset (compiler, 0, offset, ptr_reg, dest->alloc);
      break;
    case 8:
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    case 16:
      orc_x86_emit_mov_memoffset_mmx (compiler, 8, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    case 32:
      orc_x86_emit_mov_memoffset_mmx (compiler, 16, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
    default:
      ORC_COMPILER_ERROR(compiler,"bad load size %d",
          src->size << compiler->loop_shift);
      break;
  }
  switch (src->size) {
    case 1:
      orc_mmx_emit_punpcklbw (compiler, dest->alloc, dest->alloc);
      break;
    case 2:
      orc_mmx_emit_punpcklwd (compiler, dest->alloc, dest->alloc);
      break;
    case 4:
      orc_mmx_emit_punpckldq (compiler, dest->alloc, dest->alloc);
      break;
  }

  src->update_type = 1;
}

static void
mmx_rule_storeX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int offset;
  int ptr_reg;

  offset = compiler->offset * dest->size;
  if (dest->ptr_register == 0) {
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        dest->ptr_offset, compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg; 
  } else {
    ptr_reg = dest->ptr_register;
  } 
  switch (dest->size << compiler->loop_shift) {
    case 1:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == compiler->gp_tmpreg) {
        ORC_COMPILER_ERROR (compiler, "unimplemented corner case in %s",
            insn->opcode->name);
      }
      orc_mmx_emit_movd_store_register (compiler, src->alloc, compiler->gp_tmpreg);
      orc_x86_emit_mov_reg_memoffset (compiler, 1, compiler->gp_tmpreg,
          offset, ptr_reg);
      break;
    case 2:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == compiler->gp_tmpreg) {
        ORC_COMPILER_ERROR(compiler, "unimplemented corner case in %s",
            insn->opcode->name);
      } 
      orc_mmx_emit_movd_store_register (compiler, src->alloc, compiler->gp_tmpreg);
      orc_x86_emit_mov_reg_memoffset (compiler, 2, compiler->gp_tmpreg,
          offset, ptr_reg);
      break;
    case 4:
      orc_x86_emit_mov_mmx_memoffset (compiler, 4, src->alloc, offset, ptr_reg,
          dest->is_aligned, dest->is_uncached);
      break;
    case 8:
      orc_x86_emit_mov_mmx_memoffset (compiler, 8, src->alloc, offset, ptr_reg,
          dest->is_aligned, dest->is_uncached);
      break;
    case 16:
      orc_x86_emit_mov_mmx_memoffset (compiler, 16, src->alloc, offset, ptr_reg,
          dest->is_aligned, dest->is_uncached);
      break;
    default:
      ORC_COMPILER_ERROR (compiler, "bad size");
      break;
  }

  dest->update_type = 2;
}

static void
mmx_rule_ldresnearl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  int increment_var = insn->src_args[2];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int tmp = orc_compiler_get_temp_reg (compiler);
  int i;

  for(i=0;i<(1<<compiler->loop_shift);i++){
    if (i == 0) {
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, 0,
          src->ptr_register, dest->alloc, FALSE);
    } else {
      orc_x86_emit_mov_memindex_mmx (compiler, 4, 0,
          src->ptr_register, compiler->gp_tmpreg, 2, tmp, FALSE);
      /* orc_mmx_emit_punpckldq (compiler, tmp, dest->alloc); */
      orc_mmx_emit_psllq_imm (compiler, 8*4*i, tmp);
      orc_mmx_emit_por (compiler, tmp, dest->alloc);
    }

    if (compiler->vars[increment_var].vartype == ORC_VAR_TYPE_PARAM) {
      orc_x86_emit_add_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[increment_var]),
          compiler->exec_reg, src->ptr_offset);
    } else {
      orc_x86_emit_add_imm_reg (compiler, 4,
          compiler->vars[increment_var].value.i,
          src->ptr_offset, FALSE);
    }

    orc_x86_emit_mov_reg_reg (compiler, 4, src->ptr_offset, compiler->gp_tmpreg);
    orc_x86_emit_sar_imm_reg (compiler, 4, 16, compiler->gp_tmpreg);
  }

  orc_x86_emit_add_reg_reg_shift (compiler, compiler->is_64bit ? 8 : 4,
      compiler->gp_tmpreg,
      src->ptr_register, 2);
  orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, src->ptr_offset);

  src->update_type = 0;
  orc_compiler_release_temp_reg (compiler, tmp);
}

static void
mmx_rule_ldreslinl (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *src = compiler->vars + insn->src_args[0];
  int increment_var = insn->src_args[2];
  OrcVariable *dest = compiler->vars + insn->dest_args[0];
  int tmp = orc_compiler_get_temp_reg (compiler);
  int tmp2 = orc_compiler_get_temp_reg (compiler);
  int zero;
  int regsize = compiler->is_64bit ? 8 : 4;
  int i;

  zero = orc_compiler_get_constant (compiler, 1, 0);
  for(i=0;i<(1<<compiler->loop_shift);i++){
    orc_x86_emit_mov_memoffset_mmx (compiler, 4, 0,
        src->ptr_register, tmp, FALSE);
    orc_x86_emit_mov_memoffset_mmx (compiler, 4, 4,
        src->ptr_register, tmp2, FALSE);

    orc_mmx_emit_punpcklbw (compiler, zero, tmp);
    orc_mmx_emit_punpcklbw (compiler, zero, tmp2);
    orc_mmx_emit_psubw (compiler, tmp, tmp2);

    orc_mmx_emit_movd_load_register (compiler, src->ptr_offset, tmp);
    orc_mmx_emit_pshufw (compiler, ORC_MMX_SHUF(0,0,0,0), tmp, tmp);
    orc_mmx_emit_psrlw_imm (compiler, 8, tmp);
    orc_mmx_emit_pmullw (compiler, tmp2, tmp);
    orc_mmx_emit_psraw_imm (compiler, 8, tmp);
    orc_mmx_emit_pxor (compiler, tmp2, tmp2);
    orc_mmx_emit_packsswb (compiler, tmp2, tmp);

    if (i == 0) {
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, 0,
          src->ptr_register, dest->alloc, FALSE);
      orc_mmx_emit_paddb (compiler, tmp, dest->alloc);
    } else {
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, 0,
          src->ptr_register, tmp2, FALSE);
      orc_mmx_emit_paddb (compiler, tmp, tmp2);
      orc_mmx_emit_psllq_imm (compiler, 32, tmp2);
      orc_mmx_emit_por (compiler, tmp2, dest->alloc);
    }

    if (compiler->vars[increment_var].vartype == ORC_VAR_TYPE_PARAM) {
      orc_x86_emit_add_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET(OrcExecutor, params[increment_var]),
          compiler->exec_reg, src->ptr_offset);
    } else {
      orc_x86_emit_add_imm_reg (compiler, regsize,
          compiler->vars[increment_var].value.i,
          src->ptr_offset, FALSE);
    }

    orc_x86_emit_mov_reg_reg (compiler, 4, src->ptr_offset, compiler->gp_tmpreg);
    orc_x86_emit_sar_imm_reg (compiler, 4, 16, compiler->gp_tmpreg);

    orc_x86_emit_add_reg_reg_shift (compiler, regsize, compiler->gp_tmpreg,
        src->ptr_register, 2);
    orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, src->ptr_offset);
  }

  src->update_type = 0;
  orc_compiler_release_temp_reg (compiler, tmp);
  orc_compiler_release_temp_reg (compiler, tmp2);
}

static void
mmx_rule_copyx (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->src_args[0]].alloc == p->vars[insn->dest_args[0]].alloc) {
    return;
  }

  orc_mmx_emit_movq (p,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

#define UNARY(opcode,insn_name,code) \
static void \
mmx_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_mmx_emit_ ## insn_name (p, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

#define BINARY(opcode, insn_name, code) \
  static void mmx_rule_##opcode (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    if (p->vars[insn->src_args[0]].alloc \
        != p->vars[insn->dest_args[0]].alloc) { \
      orc_mmx_emit_movq (p, p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->dest_args[0]].alloc); \
    } \
    orc_mmx_emit_##insn_name (p, p->vars[insn->src_args[1]].alloc, \
        p->vars[insn->dest_args[0]].alloc); \
  }

UNARY(absb,pabsb,0x381c)
BINARY(addb,paddb,0xfc)
BINARY(addssb,paddsb,0xec)
BINARY(addusb,paddusb,0xdc)
BINARY(andb,pand,0xdb)
BINARY(andnb,pandn,0xdf)
BINARY(avgub,pavgb,0xe0)
BINARY(cmpeqb,pcmpeqb,0x74)
BINARY(cmpgtsb,pcmpgtb,0x64)
BINARY(maxub,pmaxub,0xde)
BINARY(minub,pminub,0xda)
/* BINARY(mullb,pmullb,0xd5) */
/* BINARY(mulhsb,pmulhb,0xe5) */
/* BINARY(mulhub,pmulhub,0xe4) */
BINARY(orb,por,0xeb)
/* UNARY(signb,psignb,0x3808) */
BINARY(subb,psubb,0xf8)
BINARY(subssb,psubsb,0xe8)
BINARY(subusb,psubusb,0xd8)
BINARY(xorb,pxor,0xef)

UNARY(absw,pabsw,0x381d)
BINARY(addw,paddw,0xfd)
BINARY(addssw,paddsw,0xed)
BINARY(addusw,paddusw,0xdd)
BINARY(andw,pand,0xdb)
BINARY(andnw,pandn,0xdf)
BINARY(avguw,pavgw,0xe3)
BINARY(cmpeqw,pcmpeqw,0x75)
BINARY(cmpgtsw,pcmpgtw,0x65)
BINARY(maxsw,pmaxsw,0xee)
BINARY(minsw,pminsw,0xea)
BINARY(mullw,pmullw,0xd5)
BINARY(mulhsw,pmulhw,0xe5)
BINARY(mulhuw,pmulhuw,0xe4)
BINARY(orw,por,0xeb)
/* UNARY(signw,psignw,0x3809) */
BINARY(subw,psubw,0xf9)
BINARY(subssw,psubsw,0xe9)
BINARY(subusw,psubusw,0xd9)
BINARY(xorw,pxor,0xef)

UNARY(absl,pabsd,0x381e)
BINARY(addl,paddd,0xfe)
/* BINARY(addssl,paddsd,0xed) */
/* BINARY(addusl,paddusd,0xdd) */
BINARY(andl,pand,0xdb)
BINARY(andnl,pandn,0xdf)
/* BINARY(avgul,pavgd,0xe3) */
BINARY(cmpeql,pcmpeqd,0x76)
BINARY(cmpgtsl,pcmpgtd,0x66)
/* BINARY(mulhsl,pmulhd,0xe5) */
/* BINARY(mulhul,pmulhud,0xe4) */
BINARY(orl,por,0xeb)
/* UNARY(signl,psignd,0x380a) */
BINARY(subl,psubd,0xfa)
/* BINARY(subssl,psubsd,0xe9) */
/* BINARY(subusl,psubusd,0xd9) */
BINARY(xorl,pxor,0xef)

BINARY(andq,pand,0xdb)
BINARY(andnq,pandn,0xdf)
BINARY(orq,por,0xeb)
BINARY(xorq,pxor,0xef)

static void
mmx_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_paddw (p, src, dest);
}

static void
mmx_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_paddd (p, src, dest);
}

static void
mmx_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src1 = p->vars[insn->src_args[0]].alloc;
  const int src2 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  if (p->loop_shift <= 2) {
    orc_mmx_emit_movq (p, src1, tmp);
    orc_mmx_emit_psllq_imm (p, 8*(8 - (1<<p->loop_shift)), tmp);
    orc_mmx_emit_movq (p, src2, tmp2);
    orc_mmx_emit_psllq_imm (p, 8*(8 - (1<<p->loop_shift)), tmp2);
    orc_mmx_emit_psadbw (p, tmp2, tmp);
  } else {
    orc_mmx_emit_movq (p, src1, tmp);
    orc_mmx_emit_psadbw (p, src2, tmp);
  }
  orc_mmx_emit_paddd (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
  orc_compiler_release_temp_reg (p, tmp2);
}

static void
mmx_rule_signw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = orc_compiler_get_constant (p, 2, 0x0001);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pminsw (p, tmp, dest);
  tmp = orc_compiler_get_constant (p, 2, 0xffff);
  orc_mmx_emit_pmaxsw (p, tmp, dest);
}

static void
mmx_rule_absb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pxor (p, tmp, tmp);
  orc_mmx_emit_pcmpgtb (p, src, tmp);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_psubb (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_absw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src == dest) {
    orc_mmx_emit_movq (p, src, tmp);
  } else {
    orc_mmx_emit_movq (p, src, tmp);
    orc_mmx_emit_movq (p, tmp, dest);
  }

  orc_mmx_emit_psraw_imm (p, 15, tmp);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_psubw (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_absl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src == dest) {
    orc_mmx_emit_movq (p, src, tmp);
  } else {
    orc_mmx_emit_movq (p, src, tmp);
    orc_mmx_emit_movq (p, tmp, dest);
  }

  orc_mmx_emit_psrad_imm (p, 31, tmp);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_psubd (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_shift (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int type = ORC_PTR_TO_INT (user);
  /* int imm_code1[] = { 0x71, 0x71, 0x71, 0x72, 0x72, 0x72, 0x73, 0x73 }; */
  /* int imm_code2[] = { 6, 2, 4, 6, 2, 4, 6, 2 }; */
  /* int reg_code[] = { 0xf1, 0xd1, 0xe1, 0xf2, 0xd2, 0xe2, 0xf3, 0xd3 }; */
  /* const char *code[] = { "psllw", "psrlw", "psraw", "pslld", "psrld", "psrad", "psllq", "psrlq" }; */
  const int opcodes[] = { ORC_MMX_psllw, ORC_MMX_psrlw, ORC_MMX_psraw,
    ORC_MMX_pslld, ORC_MMX_psrld, ORC_MMX_psrad, ORC_MMX_psllq,
    ORC_MMX_psrlq };
  const int opcodes_imm[] = { ORC_MMX_psllw_imm, ORC_MMX_psrlw_imm,
    ORC_MMX_psraw_imm, ORC_MMX_pslld_imm, ORC_MMX_psrld_imm,
    ORC_MMX_psrad_imm, ORC_MMX_psllq_imm, ORC_MMX_psrlq_imm };
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_mmx_emit_cpuinsn_imm (p, opcodes_imm[type], 0,
        p->vars[insn->src_args[1]].value.i, 0, dest);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    int tmp = orc_compiler_get_temp_reg (p);

    /* FIXME this is a gross hack to reload the register with a
     * 64-bit version of the parameter. */
    orc_x86_emit_mov_memoffset_mmx (p, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]),
        p->exec_reg, tmp, FALSE);

    orc_mmx_emit_cpuinsn_mmx (p, opcodes[type], tmp, dest);
    orc_compiler_release_temp_reg (p, tmp);
  } else {
    ORC_COMPILER_ERROR (p, "code generation rule for %s only works with "
        "constant or parameter shifts", insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
mmx_rule_shlb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_mmx_emit_psllw_imm (p, p->vars[insn->src_args[1]].value.i, dest);
    const int tmp = orc_compiler_get_constant (p, 1,
        0xff & (0xff << p->vars[insn->src_args[1]].value.i));
    orc_mmx_emit_pand (p, tmp, dest);
  } else {
    ORC_COMPILER_ERROR (p, "code generation rule for %s only works with "
        "constant shifts", insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
mmx_rule_shrsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_mmx_emit_movq (p, src, tmp);
    orc_mmx_emit_psllw_imm (p, 8, tmp);
    orc_mmx_emit_psraw_imm (p, p->vars[insn->src_args[1]].value.i, tmp);
    orc_mmx_emit_psrlw_imm (p, 8, tmp);

    if (src != dest) {
      orc_mmx_emit_movq (p, src, dest);
    }

    orc_mmx_emit_psraw_imm (p, 8 + p->vars[insn->src_args[1]].value.i, dest);
    orc_mmx_emit_psllw_imm (p, 8, dest);

    orc_mmx_emit_por (p, tmp, dest);
  } else {
    ORC_COMPILER_ERROR (p, "code generation rule for %s only works with "
        "constant shifts", insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_shrub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_mmx_emit_psrlw_imm (p, p->vars[insn->src_args[1]].value.i, dest);
    const int tmp = orc_compiler_get_constant (p, 1,
        (0xff >> p->vars[insn->src_args[1]].value.i));
    orc_mmx_emit_pand (p, tmp, dest);
  } else {
    ORC_COMPILER_ERROR (p, "code generation rule for %s only works with "
        "constant shifts", insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
mmx_rule_shrsq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(3,2,3,2), src, tmp);
    orc_mmx_emit_psrad_imm (p, 31, tmp);
    orc_mmx_emit_psllq_imm (p, 64-p->vars[insn->src_args[1]].value.i, tmp);

    if (src != dest) {
      orc_mmx_emit_movq (p, src, dest);
    }
    
    orc_mmx_emit_psrlq_imm (p, p->vars[insn->src_args[1]].value.i, dest);
    orc_mmx_emit_por (p, tmp, dest);
  } else {
    ORC_COMPILER_ERROR (p, "code generation rule for %s only works with "
        "constant shifts", insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  /* values of dest are shifted away so don't matter */
  orc_mmx_emit_punpcklbw (p, src, dest);
  orc_mmx_emit_psraw_imm (p, 8, dest);
}

static void
mmx_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_pxor (p, tmp, tmp);
  orc_mmx_emit_punpcklbw (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_packsswb (p, src, dest);
}

static void
mmx_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_packuswb (p, src, dest);
}

static void
mmx_rule_convuuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_movq (p, src, dest);
  orc_mmx_emit_psrlw_imm (p, 15, tmp);
  orc_mmx_emit_psllw_imm (p, 14, tmp);
  orc_mmx_emit_por (p, tmp, dest);
  orc_mmx_emit_psllw_imm (p, 1, tmp);
  orc_mmx_emit_pxor (p, tmp, dest);
  orc_mmx_emit_packuswb (p, dest, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_psllw_imm (p, 8, dest);
  orc_mmx_emit_psrlw_imm (p, 8, dest);
  orc_mmx_emit_packuswb (p, dest, dest);
}

static void
mmx_rule_convhwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_psrlw_imm (p, 8, dest);
  orc_mmx_emit_packuswb (p, dest, dest);
}

static void
mmx_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  /* values of dest are shifted away so don't matter */
  orc_mmx_emit_punpcklwd (p, src, dest);
  orc_mmx_emit_psrad_imm (p, 16, dest);
}

static void
mmx_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_pxor (p, tmp, tmp);
  orc_mmx_emit_punpcklwd (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_pslld_imm (p, 16, dest);
  orc_mmx_emit_psrad_imm (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_convhlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }
  orc_mmx_emit_psrad_imm (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_packssdw (p, src, dest);
}

static void
mmx_rule_convslq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_psrad_imm (p, 31, tmp);
  orc_mmx_emit_punpckldq (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_convulq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 4, 0);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_punpckldq (p, tmp, dest);
}

static void
mmx_rule_convql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  orc_mmx_emit_movq (p, src, dest);
}

static void
mmx_rule_splatw3q (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(3,3,3,3), dest, dest);
}

static void
mmx_rule_splatbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_punpcklbw (p, dest, dest);
}

static void
mmx_rule_splatbl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_punpcklbw (p, dest, dest);
  orc_mmx_emit_punpcklwd (p, dest, dest);
}

static void
mmx_rule_div255w (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmpc = orc_compiler_get_constant (p, 2, 0x8081);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pmulhuw(p, tmpc, dest);
  orc_mmx_emit_psrlw_imm (p, 7, dest);
}

#if 1
static void
mmx_rule_divluw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* About 5.2 cycles per array member on ginger */
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int a = orc_compiler_get_temp_reg (p);
  const int j = orc_compiler_get_temp_reg (p);
  const int j2 = orc_compiler_get_temp_reg (p);
  const int l = orc_compiler_get_temp_reg (p);
  const int divisor = orc_compiler_get_temp_reg (p);
  const int tmp = orc_compiler_get_constant (p, 2, 0x8000);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, src1, divisor);
  orc_mmx_emit_psllw_imm (p, 8, divisor);
  orc_mmx_emit_psrlw_imm (p, 1, divisor);

  /* FIXME use an orc_compiler_get_constant to be able to cache it */
  orc_compiler_load_constant_from_size_and_value (p, a, 2, 0x00ff);
  orc_mmx_emit_movq (p, tmp, j);
  orc_mmx_emit_psrlw_imm (p, 8, j);

  orc_mmx_emit_pxor (p, tmp, dest);

  for (int i = 0; i < 7; i++) {
    orc_mmx_emit_movq (p, divisor, l);
    orc_mmx_emit_pxor (p, tmp, l);
    orc_mmx_emit_pcmpgtw (p, dest, l);
    orc_mmx_emit_movq (p, l, j2);
    orc_mmx_emit_pandn (p, divisor, l);
    orc_mmx_emit_psubw (p, l, dest);
    orc_mmx_emit_psrlw_imm (p, 1, divisor);

     orc_mmx_emit_pand (p, j, j2);
     orc_mmx_emit_pxor (p, j2, a);
     orc_mmx_emit_psrlw_imm (p, 1, j);
  }

  orc_mmx_emit_movq (p, divisor, l);
  orc_mmx_emit_pxor (p, tmp, l);
  orc_mmx_emit_pcmpgtw (p, dest, l);
  orc_mmx_emit_pand (p, j, l);
  orc_mmx_emit_pxor (p, l, a);

  orc_mmx_emit_movq (p, a, dest);
  orc_compiler_release_temp_reg (p, a);
  orc_compiler_release_temp_reg (p, j);
  orc_compiler_release_temp_reg (p, j2);
  orc_compiler_release_temp_reg (p, l);
  orc_compiler_release_temp_reg (p, divisor);
}
#else
static void
mmx_rule_divluw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* About 8.4 cycles per array member on ginger */
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int b = orc_compiler_get_temp_reg (p);
  const int a = orc_compiler_get_temp_reg (p);
  const int k = orc_compiler_get_temp_reg (p);
  const int j = orc_compiler_get_temp_reg (p);
  const int tmp = orc_compiler_get_constant (p, 2, 0x00ff);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, dest, b);
  orc_mmx_emit_pand (p, tmp, src1);

  orc_mmx_emit_pxor (p, tmp, b);

  orc_mmx_emit_pxor (p, a, a);
  orc_mmx_emit_movq (p, tmp, j);
  orc_mmx_emit_psrlw_imm (p, 8, j);

  for (int i = 0; i < 8; i++) {
    orc_mmx_emit_por (p, j, a);
    orc_mmx_emit_movq (p, a, k);
    orc_mmx_emit_pmullw (p, src1, k);
    orc_mmx_emit_pxor (p, tmp, k);
    orc_mmx_emit_pcmpgtw (p, b, k);
    orc_mmx_emit_pand (p, j, k);
    orc_mmx_emit_pxor (p, k, a);
    orc_mmx_emit_psrlw_imm (p, 1, j);
  }

  orc_mmx_emit_movq (p, a, dest);
}
#endif

static void
mmx_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_punpcklbw (p, src1, tmp);
  orc_mmx_emit_psraw_imm (p, 8, tmp);
  orc_mmx_emit_punpcklbw (p, dest, dest);
  orc_mmx_emit_psraw_imm (p, 8, dest);
  orc_mmx_emit_pmullw (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_punpcklbw (p, src1, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, tmp);
  orc_mmx_emit_punpcklbw (p, dest, dest);
  orc_mmx_emit_psrlw_imm (p, 8, dest);
  orc_mmx_emit_pmullw (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_mullb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, dest, tmp);

  orc_mmx_emit_pmullw (p, src1, dest);
  orc_mmx_emit_psllw_imm (p, 8, dest);
  orc_mmx_emit_psrlw_imm (p, 8, dest);

  orc_mmx_emit_movq (p, src1, tmp2);
  orc_mmx_emit_psraw_imm (p, 8, tmp2);
  orc_mmx_emit_psraw_imm (p, 8, tmp);
  orc_mmx_emit_pmullw (p, tmp2, tmp);
  orc_mmx_emit_psllw_imm (p, 8, tmp);

  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
  orc_compiler_release_temp_reg (p, tmp2);
}

static void
mmx_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_movq (p, dest, tmp2);
  orc_mmx_emit_psllw_imm (p, 8, tmp);
  orc_mmx_emit_psraw_imm (p, 8, tmp);

  orc_mmx_emit_psllw_imm (p, 8, dest);
  orc_mmx_emit_psraw_imm (p, 8, dest);

  orc_mmx_emit_pmullw (p, tmp, dest);
  orc_mmx_emit_psrlw_imm (p, 8, dest);

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_psraw_imm (p, 8, tmp);
  orc_mmx_emit_psraw_imm (p, 8, tmp2);
  orc_mmx_emit_pmullw (p, tmp, tmp2);
  orc_mmx_emit_psrlw_imm (p, 8, tmp2);
  orc_mmx_emit_psllw_imm (p, 8, tmp2);
  orc_mmx_emit_por (p, tmp2, dest);
  orc_compiler_release_temp_reg (p, tmp);
  orc_compiler_release_temp_reg (p, tmp2);
}

static void
mmx_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_movq (p, dest, tmp2);
  orc_mmx_emit_psllw_imm (p, 8, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, tmp);

  orc_mmx_emit_psllw_imm (p, 8, dest);
  orc_mmx_emit_psrlw_imm (p, 8, dest);

  orc_mmx_emit_pmullw (p, tmp, dest);
  orc_mmx_emit_psrlw_imm (p, 8, dest);

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, tmp2);
  orc_mmx_emit_pmullw (p, tmp, tmp2);
  orc_mmx_emit_psrlw_imm (p, 8, tmp2);
  orc_mmx_emit_psllw_imm (p, 8, tmp2);
  orc_mmx_emit_por (p, tmp2, dest);
  orc_compiler_release_temp_reg (p, tmp);
  orc_compiler_release_temp_reg (p, tmp2);
}

static void
mmx_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pmulhw (p, src1, tmp);
  orc_mmx_emit_pmullw (p, src1, dest);
  orc_mmx_emit_punpcklwd (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_muluwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pmulhuw (p, src1, tmp);
  orc_mmx_emit_pmullw (p, src1, dest);
  orc_mmx_emit_punpcklwd (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_mulll_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int offset = ORC_STRUCT_OFFSET (OrcExecutor, arrays[ORC_VAR_T1]);

  orc_x86_emit_mov_mmx_memoffset (p, ORC_REG_SIZE, p->vars[insn->src_args[0]].alloc,
      offset, p->exec_reg, FALSE, FALSE);
  orc_x86_emit_mov_mmx_memoffset (p, ORC_REG_SIZE, p->vars[insn->src_args[1]].alloc,
      offset + ORC_REG_SIZE, p->exec_reg, FALSE, FALSE);

  for (int i = 0; i < (1 << p->insn_shift); i++) {
     orc_x86_emit_mov_memoffset_reg (p, 4, offset + 4 * i, p->exec_reg,
         p->gp_tmpreg);
     orc_x86_emit_imul_memoffset_reg (p, 4, offset + ORC_REG_SIZE + 4 * i, p->exec_reg,
         p->gp_tmpreg);
     orc_x86_emit_mov_reg_memoffset (p, 4, p->gp_tmpreg, offset + 4 * i,
         p->exec_reg);
  }

  orc_x86_emit_mov_memoffset_mmx (p, ORC_REG_SIZE, offset, p->exec_reg,
      p->vars[insn->dest_args[0]].alloc, FALSE);
}

static void
mmx_rule_select0lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
     orc_mmx_emit_movq (p, src, dest);
  }

  /* FIXME slow */
  /* same as convlw */

  orc_mmx_emit_pslld_imm (p, 16, dest);
  orc_mmx_emit_psrad_imm (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
     orc_mmx_emit_movq (p, src, dest);
  }

  /* FIXME slow */

  orc_mmx_emit_psrad_imm (p, 16, dest);
  orc_mmx_emit_packssdw (p, dest, dest);
}

static void
mmx_rule_select0ql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  /* values of dest are shifted away so don't matter */

  /* same as convql */
  orc_mmx_emit_movq (p, src, dest);
}

static void
mmx_rule_select1ql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  /* values of dest are shifted away so don't matter */

  orc_mmx_emit_psrlq_imm (p, 32, dest);
  orc_mmx_emit_movq (p, src, dest);
}

static void
mmx_rule_select0wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
     orc_mmx_emit_movq (p, src, dest);
  }

  /* FIXME slow */
  /* same as convwb */

  orc_mmx_emit_psllw_imm (p, 8, dest);
  orc_mmx_emit_psraw_imm (p, 8, dest);
  orc_mmx_emit_packsswb (p, dest, dest);
}

static void
mmx_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
     orc_mmx_emit_movq (p, src, dest);
  }

  /* FIXME slow */

  orc_mmx_emit_psraw_imm (p, 8, dest);
  orc_mmx_emit_packsswb (p, dest, dest);
}

static void
mmx_rule_splitql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int zero = orc_compiler_get_constant (p, 4, 0);

  /* values of dest are shifted away so don't matter */

  orc_mmx_emit_movq (p, src, dest2);
  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF (3, 2, 3, 2), src, dest1);
  orc_mmx_emit_punpckldq (p, zero, dest1);
  orc_mmx_emit_punpckldq (p, zero, dest2);
}

static void
mmx_rule_splitlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;

  /* values of dest are shifted away so don't matter */

  /* FIXME slow */
  if (dest1 != src)
    orc_mmx_emit_movq (p, src, dest1);
  if (dest2 != src)
    orc_mmx_emit_movq (p, src, dest2);

  orc_mmx_emit_psrad_imm (p, 16, dest1);
  orc_mmx_emit_packssdw (p, dest1, dest1);

  orc_mmx_emit_pslld_imm (p, 16, dest2);
  orc_mmx_emit_psrad_imm (p, 16, dest2);
  orc_mmx_emit_packssdw (p, dest2, dest2);
}

static void
mmx_rule_splitwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int tmp = orc_compiler_get_constant (p, 2, 0xff);

  /* values of dest are shifted away so don't matter */

  ORC_DEBUG ("got tmp %d", tmp);
  /* FIXME slow */

  if (dest1 != src)
    orc_mmx_emit_movq (p, src, dest1);
  if (dest2 != src)
    orc_mmx_emit_movq (p, src, dest2);

  orc_mmx_emit_psraw_imm (p, 8, dest1);
  orc_mmx_emit_packsswb (p, dest1, dest1);

  orc_mmx_emit_pand (p, tmp, dest2);
  orc_mmx_emit_packuswb (p, dest2, dest2);
}

static void
mmx_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_punpcklbw (p, src1, dest);
}

static void
mmx_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_punpcklwd (p, src1, dest);
}

static void
mmx_rule_mergelq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_punpckldq (p, src1, dest);
}

static void
mmx_rule_swapw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_psllw_imm (p, 8, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_swapl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_pslld_imm (p, 16, tmp);
  orc_mmx_emit_psrld_imm (p, 16, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_psllw_imm (p, 8, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_swapwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_pslld_imm (p, 16, tmp);
  orc_mmx_emit_psrld_imm (p, 16, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_swapq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_movq (p, src, tmp);
  orc_mmx_emit_psllq_imm (p, 32, tmp);
  orc_mmx_emit_psrlq_imm (p, 32, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pslld_imm (p, 16, tmp);
  orc_mmx_emit_psrld_imm (p, 16, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_psllw_imm (p, 8, tmp);
  orc_mmx_emit_psrlw_imm (p, 8, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_swaplq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src != dest) {
    orc_mmx_emit_movq (p, src, dest);
  }

  orc_mmx_emit_pshufw (p, ORC_MMX_SHUF(1,0,3,2), dest, dest);
}

/* slow rules */

static void
mmx_rule_maxuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 2, 0x8000);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
  orc_mmx_emit_pmaxsw (p, src1, dest);
  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
}

static void
mmx_rule_minuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 2, 0x8000);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
  orc_mmx_emit_pminsw (p, src1, dest);
  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
}

static void
mmx_rule_avgsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 1, 0x80);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
  orc_mmx_emit_pavgb (p, src1, dest);
  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
}

static void
mmx_rule_avgsw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 2, 0x8000);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
  orc_mmx_emit_pavgw (p, src1, dest);
  orc_mmx_emit_pxor (p, tmp, src1);
  orc_mmx_emit_pxor(p, tmp, dest);
}

static void
mmx_rule_maxsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }
  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pcmpgtb (p, src1, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src1, tmp);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_minsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_pcmpgtb (p, dest, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src1, tmp);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_maxsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pcmpgtd (p, src1, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src1, tmp);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_minsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_pcmpgtd (p, dest, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src1, tmp);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_maxul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmpc = orc_compiler_get_constant (p, 4, 0x80000000);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, tmpc, src1);
  orc_mmx_emit_pxor(p, tmpc, dest);

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pcmpgtd (p, src1, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src1, tmp);
  orc_mmx_emit_por (p, tmp, dest);

  orc_mmx_emit_pxor (p, tmpc, src1);
  orc_mmx_emit_pxor(p, tmpc, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_minul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmpc = orc_compiler_get_constant (p, 4, 0x80000000);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, tmpc, src1);
  orc_mmx_emit_pxor(p, tmpc, dest);

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_pcmpgtd (p, dest, tmp);
  orc_mmx_emit_pand (p, tmp, dest);
  orc_mmx_emit_pandn (p, src1, tmp);
  orc_mmx_emit_por (p, tmp, dest);

  orc_mmx_emit_pxor (p, tmpc, src1);
  orc_mmx_emit_pxor(p, tmpc, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_avgsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  /* (a+b+1) >> 1 = (a|b) - ((a^b)>>1) */

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pxor (p, src1, tmp);
  orc_mmx_emit_psrad_imm(p, 1, tmp);

  orc_mmx_emit_por (p, src1, dest);
  orc_mmx_emit_psubd(p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_avgul (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  /* (a+b+1) >> 1 = (a|b) - ((a^b)>>1) */

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_pxor (p, src1, tmp);
  orc_mmx_emit_psrld_imm(p, 1, tmp);

  orc_mmx_emit_por (p, src1, dest);
  orc_mmx_emit_psubd(p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
}

static void
mmx_rule_addssl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

#if 0
  int tmp2 = orc_compiler_get_temp_reg (p);
  int tmp3 = orc_compiler_get_temp_reg (p);

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_pand (p, dest, tmp);

  orc_mmx_emit_movq (p, src1, tmp2);
  orc_mmx_emit_pxor (p, dest, tmp2);
  orc_mmx_emit_psrad_imm (p, 1, tmp2);
  orc_mmx_emit_paddd (p, tmp2, tmp);

  orc_mmx_emit_psrad (p, 30, tmp);
  orc_mmx_emit_pslld (p, 30, tmp);
  orc_mmx_emit_movq (p, tmp, tmp2);
  orc_mmx_emit_pslld_imm (p, 1, tmp2);
  orc_mmx_emit_movq (p, tmp, tmp3);
  orc_mmx_emit_pxor (p, tmp2, tmp3);
  orc_mmx_emit_psrad_imm (p, 31, tmp3);

  orc_mmx_emit_psrad_imm (p, 31, tmp2);
  tmp = orc_compiler_get_constant (p, 4, 0x80000000);
  orc_mmx_emit_pxor (p, tmp, tmp2); /*  clamped value */
  orc_mmx_emit_pand (p, tmp3, tmp2);

  orc_mmx_emit_paddd (p, src1, dest);
  orc_mmx_emit_pandn (p, dest, tmp3); /*  tmp is mask: ~0 is for clamping */
  orc_mmx_emit_movq (p, tmp3, dest);

  orc_mmx_emit_por (p, tmp2, dest);
#endif

  const int s = orc_compiler_get_temp_reg (p);
  const int t = orc_compiler_get_temp_reg (p);

  /*
     From Tim Terriberry: (slightly faster than above)

     m=0xFFFFFFFF;
     s=_a;
     t=_a;
     s^=_b;
     _a+=_b;
     t^=_a;
     t^=m;
     m>>=1;
     s|=t;
     t=_b;
     s>>=31;
     t>>=31;
     _a&=s;
     t^=m;
     s=~s&t;
     _a|=s; 
  */

  orc_mmx_emit_movq (p, dest, s);
  orc_mmx_emit_movq (p, dest, t);
  orc_mmx_emit_pxor (p, src1, s);
  orc_mmx_emit_paddd (p, src1, dest);
  orc_mmx_emit_pxor (p, dest, t);
  int tmp = orc_compiler_get_constant (p, 4, 0xffffffff);
  orc_mmx_emit_pxor (p, tmp, t);
  orc_mmx_emit_por (p, t, s);
  orc_mmx_emit_movq (p, src1, t);
  orc_mmx_emit_psrad_imm (p, 31, s);
  orc_mmx_emit_psrad_imm (p, 31, t);
  orc_mmx_emit_pand (p, s, dest);
  tmp = orc_compiler_get_constant (p, 4, 0x7fffffff);
  orc_mmx_emit_pxor (p, tmp, t);
  orc_mmx_emit_pandn (p, t, s);
  orc_mmx_emit_por (p, s, dest);

  orc_compiler_release_temp_reg (p, s);
  orc_compiler_release_temp_reg (p, t);
}

static void
mmx_rule_subssl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = orc_compiler_get_temp_constant (p, 4, 0xffffffff);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int tmp3 = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_pxor (p, src1, tmp);
  orc_mmx_emit_movq (p, tmp, tmp2);
  orc_mmx_emit_por (p, dest, tmp);

  orc_mmx_emit_pxor (p, dest, tmp2);
  orc_mmx_emit_psrad_imm (p, 1, tmp2);
  orc_mmx_emit_psubd (p, tmp2, tmp);

  orc_mmx_emit_psrad_imm (p, 30, tmp);
  orc_mmx_emit_pslld_imm (p, 30, tmp);
  orc_mmx_emit_movq (p, tmp, tmp2);
  orc_mmx_emit_pslld_imm (p, 1, tmp2);
  orc_mmx_emit_movq (p, tmp, tmp3);
  orc_mmx_emit_pxor (p, tmp2, tmp3);
  orc_mmx_emit_psrad_imm (p, 31, tmp3); /*  tmp3 is mask: ~0 is for clamping */

  orc_mmx_emit_psrad_imm (p, 31, tmp2);
  tmp = orc_compiler_get_constant (p, 4, 0x80000000);
  orc_mmx_emit_pxor (p, tmp, tmp2); /*  clamped value */
  orc_mmx_emit_pand (p, tmp3, tmp2);

  orc_mmx_emit_psubd (p, src1, dest);
  orc_mmx_emit_pandn (p, dest, tmp3);
  orc_mmx_emit_movq (p, tmp3, dest);

  orc_mmx_emit_por (p, tmp2, dest);
  orc_compiler_release_temp_reg (p, tmp2);
  orc_compiler_release_temp_reg (p, tmp3);
}

static void
mmx_rule_addusl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

#if 0
  /* an alternate version.  slower. */
  /* Compute the bit that gets carried from bit 0 to bit 1 */
  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_pand (p, dest, tmp);
  orc_mmx_emit_pslld_imm (p, 31, tmp);
  orc_mmx_emit_psrld_imm (p, 31, tmp);

  /* Add in (src>>1) */
  orc_mmx_emit_movq (p, src1, tmp2);
  orc_mmx_emit_psrld_imm (p, 1, tmp2);
  orc_mmx_emit_paddd (p, tmp2, tmp);

  /* Add in (dest>>1) */
  orc_mmx_emit_movq (p, dest, tmp2);
  orc_mmx_emit_psrld_imm (p, 1, tmp2);
  orc_mmx_emit_paddd (p, tmp2, tmp);

  /* turn overflow bit into mask */
  orc_mmx_emit_psrad_imm (p, 31, tmp);

  /* compute the sum, then or over the mask */
  orc_mmx_emit_paddd (p, src1, dest);
  orc_mmx_emit_por (p, tmp, dest);
#endif

  orc_mmx_emit_movq (p, src1, tmp);
  orc_mmx_emit_pand (p, dest, tmp);

  orc_mmx_emit_movq (p, src1, tmp2);
  orc_mmx_emit_pxor (p, dest, tmp2);
  orc_mmx_emit_psrld_imm (p, 1, tmp2);
  orc_mmx_emit_paddd (p, tmp2, tmp);

  orc_mmx_emit_psrad_imm (p, 31, tmp);
  orc_mmx_emit_paddd (p, src1, dest);
  orc_mmx_emit_por (p, tmp, dest);
  orc_compiler_release_temp_reg (p, tmp);
  orc_compiler_release_temp_reg (p, tmp2);
}

static void
mmx_rule_subusl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  if (src0 != dest) {
    orc_mmx_emit_movq (p, src0, dest);
  }

  orc_mmx_emit_movq (p, src1, tmp2);
  orc_mmx_emit_psrld_imm (p, 1, tmp2);

  orc_mmx_emit_movq (p, dest, tmp);
  orc_mmx_emit_psrld_imm (p, 1, tmp);
  orc_mmx_emit_psubd (p, tmp, tmp2);

  /* turn overflow bit into mask */
  orc_mmx_emit_psrad_imm (p, 31, tmp2);

  /* compute the difference, then and over the mask */
  orc_mmx_emit_psubd (p, src1, dest);
  orc_mmx_emit_pand (p, tmp2, dest);
  orc_compiler_release_temp_reg (p, tmp);
  orc_compiler_release_temp_reg (p, tmp2);
}

void
orc_compiler_mmx_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

#define REG(x) \
  orc_rule_register (rule_set, #x , mmx_rule_ ## x, NULL)

  /* ORC_TARGET_MMX_MMX */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_MMX_MMX);

  orc_rule_register (rule_set, "loadb", mmx_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadw", mmx_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadl", mmx_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadq", mmx_rule_loadX, NULL);
  orc_rule_register (rule_set, "loadoffb", mmx_rule_loadoffX, NULL);
  orc_rule_register (rule_set, "loadoffw", mmx_rule_loadoffX, NULL);
  orc_rule_register (rule_set, "loadoffl", mmx_rule_loadoffX, NULL);
  orc_rule_register (rule_set, "loadupdb", mmx_rule_loadupdb, NULL);
  orc_rule_register (rule_set, "loadupib", mmx_rule_loadupib, NULL);
  orc_rule_register (rule_set, "loadpb", mmx_rule_loadpX, (void *)1);
  orc_rule_register (rule_set, "loadpw", mmx_rule_loadpX, (void *)2);
  orc_rule_register (rule_set, "loadpl", mmx_rule_loadpX, (void *)4);
  orc_rule_register (rule_set, "loadpq", mmx_rule_loadpX, (void *)8);
  orc_rule_register (rule_set, "ldresnearl", mmx_rule_ldresnearl, NULL);
  orc_rule_register (rule_set, "ldreslinl", mmx_rule_ldreslinl, NULL);

  orc_rule_register (rule_set, "storeb", mmx_rule_storeX, NULL);
  orc_rule_register (rule_set, "storew", mmx_rule_storeX, NULL);
  orc_rule_register (rule_set, "storel", mmx_rule_storeX, NULL);
  orc_rule_register (rule_set, "storeq", mmx_rule_storeX, NULL);

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

  REG(andq);
  REG(andnq);
  REG(orq);
  REG(xorq);

  REG(select0ql);
  REG(select1ql);
  REG(select0lw);
  REG(select1lw);
  REG(select0wb);
  REG(select1wb);
  REG(mergebw);
  REG(mergewl);
  REG(mergelq);

  orc_rule_register (rule_set, "copyb", mmx_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyw", mmx_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyl", mmx_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyq", mmx_rule_copyx, NULL);

  orc_rule_register (rule_set, "shlw", mmx_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shruw", mmx_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrsw", mmx_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shll", mmx_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrul", mmx_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shrsl", mmx_rule_shift, (void *)5);
  orc_rule_register (rule_set, "shlq", mmx_rule_shift, (void *)6);
  orc_rule_register (rule_set, "shruq", mmx_rule_shift, (void *)7);
  orc_rule_register (rule_set, "shrsq", mmx_rule_shrsq, NULL);

  orc_rule_register (rule_set, "convsbw", mmx_rule_convsbw, NULL);
  orc_rule_register (rule_set, "convubw", mmx_rule_convubw, NULL);
  orc_rule_register (rule_set, "convssswb", mmx_rule_convssswb, NULL);
  orc_rule_register (rule_set, "convsuswb", mmx_rule_convsuswb, NULL);
  orc_rule_register (rule_set, "convuuswb", mmx_rule_convuuswb, NULL);
  orc_rule_register (rule_set, "convwb", mmx_rule_convwb, NULL);

  orc_rule_register (rule_set, "convswl", mmx_rule_convswl, NULL);
  orc_rule_register (rule_set, "convuwl", mmx_rule_convuwl, NULL);
  orc_rule_register (rule_set, "convssslw", mmx_rule_convssslw, NULL);

  orc_rule_register (rule_set, "convql", mmx_rule_convql, NULL);
  orc_rule_register (rule_set, "convslq", mmx_rule_convslq, NULL);
  orc_rule_register (rule_set, "convulq", mmx_rule_convulq, NULL);
  /* orc_rule_register (rule_set, "convsssql", mmx_rule_convsssql, NULL); */

  orc_rule_register (rule_set, "mulsbw", mmx_rule_mulsbw, NULL);
  orc_rule_register (rule_set, "mulubw", mmx_rule_mulubw, NULL);
  orc_rule_register (rule_set, "mulswl", mmx_rule_mulswl, NULL);
  orc_rule_register (rule_set, "muluwl", mmx_rule_muluwl, NULL);

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
  orc_rule_register (rule_set, "signw", mmx_rule_signw_slow, NULL);
  orc_rule_register (rule_set, "absb", mmx_rule_absb_slow, NULL);
  orc_rule_register (rule_set, "absw", mmx_rule_absw_slow, NULL);
  orc_rule_register (rule_set, "absl", mmx_rule_absl_slow, NULL);
  orc_rule_register (rule_set, "swapw", mmx_rule_swapw, NULL);
  orc_rule_register (rule_set, "swapl", mmx_rule_swapl, NULL);
  orc_rule_register (rule_set, "swapwl", mmx_rule_swapwl, NULL);
  orc_rule_register (rule_set, "swapq", mmx_rule_swapq, NULL);
  orc_rule_register (rule_set, "swaplq", mmx_rule_swaplq, NULL);
  orc_rule_register (rule_set, "splitql", mmx_rule_splitql, NULL);
  orc_rule_register (rule_set, "splitlw", mmx_rule_splitlw, NULL);
  orc_rule_register (rule_set, "splitwb", mmx_rule_splitwb, NULL);
  orc_rule_register (rule_set, "avgsl", mmx_rule_avgsl, NULL);
  orc_rule_register (rule_set, "avgul", mmx_rule_avgul, NULL);
  orc_rule_register (rule_set, "shlb", mmx_rule_shlb, NULL);
  orc_rule_register (rule_set, "shrsb", mmx_rule_shrsb, NULL);
  orc_rule_register (rule_set, "shrub", mmx_rule_shrub, NULL);
  orc_rule_register (rule_set, "mulll", mmx_rule_mulll_slow, NULL);
  orc_rule_register (rule_set, "mullb", mmx_rule_mullb, NULL);
  orc_rule_register (rule_set, "mulhsb", mmx_rule_mulhsb, NULL);
  orc_rule_register (rule_set, "mulhub", mmx_rule_mulhub, NULL);
  orc_rule_register (rule_set, "addssl", mmx_rule_addssl_slow, NULL);
  orc_rule_register (rule_set, "subssl", mmx_rule_subssl_slow, NULL);
  orc_rule_register (rule_set, "addusl", mmx_rule_addusl_slow, NULL);
  orc_rule_register (rule_set, "subusl", mmx_rule_subusl_slow, NULL);
  orc_rule_register (rule_set, "convhwb", mmx_rule_convhwb, NULL);
  orc_rule_register (rule_set, "convhlw", mmx_rule_convhlw, NULL);
  orc_rule_register (rule_set, "splatw3q", mmx_rule_splatw3q, NULL);
  orc_rule_register (rule_set, "splatbw", mmx_rule_splatbw, NULL);
  orc_rule_register (rule_set, "splatbl", mmx_rule_splatbl, NULL);
  orc_rule_register (rule_set, "div255w", mmx_rule_div255w, NULL);
  orc_rule_register (rule_set, "divluw", mmx_rule_divluw, NULL);

  /* ORC_TARGET_MMX_SSSE3 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_MMX_SSSE3);

  REG(absb);
  REG(absw);
  REG(absl);
}
