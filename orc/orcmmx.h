
#ifndef _ORC_MMX_H_
#define _ORC_MMX_H_

#include <orc/orcx86.h>

typedef enum {
  ORC_TARGET_MMX_MMX = (1<<0),
  ORC_TARGET_MMX_MMXEXT = (1<<1),
  ORC_TARGET_MMX_3DNOW = (1<<2),
  ORC_TARGET_MMX_3DNOWEXT = (1<<3),
  ORC_TARGET_MMX_SSSE3 = (1<<4),
  ORC_TARGET_MMX_FRAME_POINTER = (1<<7),
  ORC_TARGET_MMX_SHORT_JUMPS = (1<<8),
  ORC_TARGET_MMX_64BIT = (1<<9)
} OrcTargetMMXFlags;

typedef enum {
  X86_MM0 = ORC_VEC_REG_BASE,
  X86_MM1,
  X86_MM2,
  X86_MM3,
  X86_MM4,
  X86_MM5,
  X86_MM6,
  X86_MM7
} OrcMMXRegister;

#define ORC_MMX_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

const char * orc_x86_get_regname_mmx(int i);
void orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);
void orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);
void orc_x86_emit_mov_mmx_reg_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_reg_mmx (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_mmx_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_mmx_emit_loadib (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadiw (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadil (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpb (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpw (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpl (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpq (OrcCompiler *p, int reg, int value);

void orc_mmx_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_f20f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_f30f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_0f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_pshufw (OrcCompiler *p, int shuf, int src, int dest);
void orc_mmx_emit_shiftimm (OrcCompiler *p, const char *insn_name,
    int code, int modrm_code, int shift, int reg);
unsigned int orc_mmx_get_cpu_flags (void);

#endif

