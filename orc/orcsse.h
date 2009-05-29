
#ifndef _ORC_SSE_H_
#define _ORC_SSE_H_

#include <orc/orcx86.h>

enum {
  X86_XMM0 = ORC_VEC_REG_BASE,
  X86_XMM1,
  X86_XMM2,
  X86_XMM3,
  X86_XMM4,
  X86_XMM5,
  X86_XMM6,
  X86_XMM7,
  X86_XMM8,
  X86_XMM9,
  X86_XMM10,
  X86_XMM11,
  X86_XMM12,
  X86_XMM13,
  X86_XMM14,
  X86_XMM15
};

#define ORC_SSE_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

const char * orc_x86_get_regname_sse(int i);
void orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);
void orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);
void orc_x86_emit_mov_sse_reg_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_reg_sse (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_sse_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_sse_emit_loadib (OrcCompiler *p, int reg, int value);
void orc_sse_emit_loadiw (OrcCompiler *p, int reg, int value);
void orc_sse_emit_loadil (OrcCompiler *p, int reg, int value);
void orc_sse_emit_loadpb (OrcCompiler *p, int reg, int value);
void orc_sse_emit_loadpw (OrcCompiler *p, int reg, int value);
void orc_sse_emit_loadpl (OrcCompiler *p, int reg, int value);
void orc_sse_emit_loadpq (OrcCompiler *p, int reg, int value);

void orc_sse_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_sse_emit_f20f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_sse_emit_f30f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_sse_emit_0f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_sse_emit_pshufd (OrcCompiler *p, int shuf, int src, int dest);
void orc_sse_emit_pshuflw (OrcCompiler *p, int shuf, int src, int dest);
void orc_sse_emit_shiftimm (OrcCompiler *p, const char *insn_name,
    int code, int modrm_code, int shift, int reg);

unsigned int orc_sse_get_cpu_flags (void);

#endif

