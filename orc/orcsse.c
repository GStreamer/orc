
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcsse.h>

/**
 * SECTION:orcsse
 * @title: SSE
 * @short_description: code generation for SSE
 */


const char *
orc_x86_get_regname_sse(int i)
{
  static const char *x86_regs[] = {
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
  };

  if (i>=X86_XMM0 && i<X86_XMM0 + 16) return x86_regs[i - X86_XMM0];
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
orc_sse_emit_f20f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_sse(src),
      orc_x86_get_regname_sse(dest));
  *p->codeptr++ = 0xf2;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_sse_emit_f30f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_sse(src),
      orc_x86_get_regname_sse(dest));
  *p->codeptr++ = 0xf3;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_sse_emit_0f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_sse(src),
      orc_x86_get_regname_sse(dest));
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_sse_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest)
{
  ORC_ASM_CODE(p,"  %s %%%s, %%%s\n", insn_name,
      orc_x86_get_regname_sse(src),
      orc_x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  if (code & 0xff00) {
    *p->codeptr++ = code >> 8;
  }
  *p->codeptr++ = code & 0xff;
  orc_x86_emit_modrm_reg (p, src, dest);
}

void
orc_sse_emit_pshufd (OrcCompiler *p, int shuf, int src, int dest)
{
  ORC_ASM_CODE(p,"  pshufd $0x%04x, %%%s, %%%s\n", shuf,
      orc_x86_get_regname_sse(src),
      orc_x86_get_regname_sse(dest));
  *p->codeptr++ = 0x66;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  orc_x86_emit_modrm_reg (p, src, dest);
  *p->codeptr++ = shuf;
}

void
orc_sse_emit_pshuflw (OrcCompiler *p, int shuf, int src, int dest)
{
  ORC_ASM_CODE(p,"  pshuflw $0x%04x, %%%s, %%%s\n", shuf,
      orc_x86_get_regname_sse(src),
      orc_x86_get_regname_sse(dest));
  *p->codeptr++ = 0xf2;
  orc_x86_emit_rex (p, 0, dest, 0, src);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = 0x70;
  orc_x86_emit_modrm_reg (p, src, dest);
  *p->codeptr++ = shuf;
}

void
orc_sse_emit_shiftimm (OrcCompiler *p, const char *insn_name, int code,
    int modrm_code, int shift, int reg)
{
  ORC_ASM_CODE(p,"  %s $%d, %%%s\n", insn_name, shift,
      orc_x86_get_regname_sse(reg));
  *p->codeptr++ = 0x66;
  orc_x86_emit_rex (p, 0, 0, 0, reg);
  *p->codeptr++ = 0x0f;
  *p->codeptr++ = code;
  orc_x86_emit_modrm_reg (p, reg, modrm_code);
  *p->codeptr++ = shift;
}

void
orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_sse(reg2));
      *compiler->codeptr++ = 0x66;
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_sse(reg2));
      *compiler->codeptr++ = 0xf3;
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    case 16:
      if (is_aligned) {
        ORC_ASM_CODE(compiler,"  movdqa %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
            orc_x86_get_regname_sse(reg2));
        *compiler->codeptr++ = 0x66;
        orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
        *compiler->codeptr++ = 0x0f;
        *compiler->codeptr++ = 0x6f;
      } else {
        ORC_ASM_CODE(compiler,"  movdqu %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
            orc_x86_get_regname_sse(reg2));
        *compiler->codeptr++ = 0xf3;
        orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
        *compiler->codeptr++ = 0x0f;
        *compiler->codeptr++ = 0x6f;
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
  orc_x86_emit_modrm_memoffset (compiler, reg2, offset, reg1);
}

void
orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %%%s, %d(%%%s)\n", orc_x86_get_regname_sse(reg1), offset,
          orc_x86_get_regname_ptr(compiler, reg2));
      *compiler->codeptr++ = 0x66;
      orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %%%s, %d(%%%s)\n", orc_x86_get_regname_sse(reg1), offset,
          orc_x86_get_regname_ptr(compiler, reg2));
      *compiler->codeptr++ = 0x66;
      orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0xd6;
      break;
    case 16:
      if (aligned) {
        if (uncached) {
          ORC_ASM_CODE(compiler,"  movntdq %%%s, %d(%%%s)\n", orc_x86_get_regname_sse(reg1), offset,
              orc_x86_get_regname_ptr(compiler, reg2));
          *compiler->codeptr++ = 0x66;
          orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
          *compiler->codeptr++ = 0x0f;
          *compiler->codeptr++ = 0xe7;
        } else {
          ORC_ASM_CODE(compiler,"  movdqa %%%s, %d(%%%s)\n", orc_x86_get_regname_sse(reg1), offset,
              orc_x86_get_regname_ptr(compiler, reg2));
          *compiler->codeptr++ = 0x66;
          orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
          *compiler->codeptr++ = 0x0f;
          *compiler->codeptr++ = 0x7f;
        }
      } else {
        ORC_ASM_CODE(compiler,"  movdqu %%%s, %d(%%%s)\n", orc_x86_get_regname_sse(reg1), offset,
            orc_x86_get_regname_ptr(compiler, reg2));
        *compiler->codeptr++ = 0xf3;
        orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
        *compiler->codeptr++ = 0x0f;
        *compiler->codeptr++ = 0x7f;
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }

  orc_x86_emit_modrm_memoffset (compiler, reg1, offset, reg2);
}

void orc_x86_emit_mov_sse_reg_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movdqa %%%s, %%%s\n", orc_x86_get_regname_sse(reg1),
        orc_x86_get_regname_sse(reg2));

  *compiler->codeptr++ = 0x66;
  orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x6f;
  orc_x86_emit_modrm_reg (compiler, reg1, reg2);
}

void orc_x86_emit_mov_reg_sse (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movd %%%s, %%%s\n", orc_x86_get_regname(reg1),
      orc_x86_get_regname_sse(reg2));
  *compiler->codeptr++ = 0x66;
  orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x6e;
  orc_x86_emit_modrm_reg (compiler, reg1, reg2);
}

void orc_x86_emit_mov_sse_reg (OrcCompiler *compiler, int reg1, int reg2)
{
  ORC_ASM_CODE(compiler,"  movd %%%s, %%%s\n", orc_x86_get_regname_sse(reg1),
      orc_x86_get_regname(reg2));
  *compiler->codeptr++ = 0x66;
  orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
  *compiler->codeptr++ = 0x0f;
  *compiler->codeptr++ = 0x7e;
  orc_x86_emit_modrm_reg (compiler, reg2, reg1);
}

