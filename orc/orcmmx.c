
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcmmx.h>

/**
 * SECTION:orcmmx
 * @title: MMX
 * @short_description: code generation for MMX
 */


const char *
orc_x86_get_regname_mmx(int i)
{
  static const char *x86_regs[] = {
    "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
    "mm8", "mm9", "mm10", "mm11", "mm12", "mm13", "mm14", "mm15"
  };

  if (i>=X86_MM0 && i<X86_MM0 + 16) return x86_regs[i - X86_MM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

void
orc_mmx_emit_f20f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_mmx(src),
      orc_x86_get_regname_mmx(dest));
  *p->codeptr++ = 0xf2;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_mmx_emit_f30f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_mmx(src),
      orc_x86_get_regname_mmx(dest));
  *p->codeptr++ = 0xf3;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_mmx_emit_0f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_mmx(src),
      orc_x86_get_regname_mmx(dest));
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_mmx_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_mmx(src),
      orc_x86_get_regname_mmx(dest));
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  if (code & 0xff00) {
    *p->codeptr++ = code >> 8;
  }
  *p->codeptr++ = code & 0xff;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_mmx_emit_pshufw (OrcCompiler *p, int shuf, int src, int dest)
{
  ORC_ASM_CODE(p,"  pshufw $0x%04x, %%%s, %%%s\n", shuf,
      orc_x86_get_regname_mmx(src),
      orc_x86_get_regname_mmx(dest));
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  orc_x86_emit_modrm_reg (p, src, dest);
  *p->codeptr++ = shuf;
}

void
orc_mmx_emit_pinsrw_memoffset (OrcCompiler *p, int imm, int offset,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  pinsrw $%d, %d(%%%s), %%%s\n", imm, offset,
      orc_x86_get_regname_ptr(p, src),
      orc_x86_get_regname_mmx(dest));
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0xc4;
  orc_x86_emit_modrm_memoffset (p, offset, src, dest);
  *p->codeptr++ = imm;

}

void
orc_mmx_emit_pextrw_memoffset (OrcCompiler *p, int imm, int src,
    int offset, int dest)
{
  ORC_ASM_CODE(p,"  pextrw $%d, %%%s, %d(%%%s)\n", imm,
      orc_x86_get_regname_ptr(p, src),
      offset, orc_x86_get_regname_mmx(dest));
  orc_x86_emit_rex (p, 0, src, 0, dest);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0xc4;
  orc_x86_emit_modrm_memoffset (p, offset, dest, src);
  *p->codeptr++ = imm;
}

void
orc_mmx_emit_shiftimm (OrcCompiler *p, const char *insn_name, int code,
    int modrm_code, int shift, int reg)
{
  ORC_ASM_CODE(p,"  %s $%d, %%%s\n", insn_name, shift,
      orc_x86_get_regname_mmx(reg));
  orc_x86_emit_rex (p, 0, 0, 0, reg);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, reg, modrm_code);
  *p->codeptr++ = shift;
}

void
orc_x86_emit_mov_memindex_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %d(%%%s,%%%s,%d), %%%s\n", offset,
          orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %d(%%%s,%%%s,%d), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
  orc_x86_emit_modrm_memindex (compiler, reg2, offset, reg1, regindex, shift);
}

void
orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6f;
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
  orc_x86_emit_modrm_memoffset (compiler, offset, reg1, reg2);
}

void
orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %%%s, %d(%%%s)\n", orc_x86_get_regname_mmx(reg1), offset,
          orc_x86_get_regname_ptr(compiler, reg2));
      orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %%%s, %d(%%%s)\n", orc_x86_get_regname_mmx(reg1), offset,
          orc_x86_get_regname_ptr(compiler, reg2));
      orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7f;
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }

  orc_x86_emit_modrm_memoffset (compiler, offset, reg2, reg1);
}

void orc_x86_emit_mov_mmx_reg_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movq %%%s, %%%s\n", orc_x86_get_regname_mmx(reg1),
        orc_x86_get_regname_mmx(reg2));

  orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x6f;
  orc_x86_emit_modrm_reg (compiler, reg1, reg2);
}

void orc_x86_emit_mov_reg_mmx (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movd %%%s, %%%s\n", orc_x86_get_regname(reg1),
      orc_x86_get_regname_mmx(reg2));
  orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x6e;
  orc_x86_emit_modrm_reg (compiler, reg1, reg2);
}

void orc_x86_emit_mov_mmx_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movd %%%s, %%%s\n", orc_x86_get_regname_mmx(reg1),
      orc_x86_get_regname(reg2));
  orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x7e;
  orc_x86_emit_modrm_reg (compiler, reg2, reg1);
}

