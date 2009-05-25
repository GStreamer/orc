
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/x86.h>


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

