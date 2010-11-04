
#ifndef _ORC_SSE_H_
#define _ORC_SSE_H_

#include <orc/orcx86.h>
#include <orc/orcx86insn.h>

typedef enum {
  ORC_TARGET_SSE_SSE2 = (1<<0),
  ORC_TARGET_SSE_SSE3 = (1<<1),
  ORC_TARGET_SSE_SSSE3 = (1<<2),
  ORC_TARGET_SSE_SSE4_1 = (1<<3),
  ORC_TARGET_SSE_SSE4_2 = (1<<4),
  ORC_TARGET_SSE_SSE4A = (1<<5),
  ORC_TARGET_SSE_SSE5 = (1<<6),
  ORC_TARGET_SSE_FRAME_POINTER = (1<<7),
  ORC_TARGET_SSE_SHORT_JUMPS = (1<<8),
  ORC_TARGET_SSE_64BIT = (1<<9)
}OrcTargetSSEFlags;

typedef enum {
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
}OrcSSERegister;

#define ORC_SSE_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

const char * orc_x86_get_regname_sse(int i);
void orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);
void orc_x86_emit_movhps_memoffset_sse (OrcCompiler *compiler, int offset,
    int reg1, int reg2);
void orc_x86_emit_mov_memindex_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned);
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


void orc_sse_set_mxcsr (OrcCompiler *compiler);
void orc_sse_restore_mxcsr (OrcCompiler *compiler);

void orc_sse_load_constant (OrcCompiler *compiler, int reg, int size,
    orc_uint64 value);

void orc_sse_emit_sysinsn (OrcCompiler *p, int opcode, int src, int dest,
    int imm);
void orc_sse_emit_sysinsn_load_memoffset (OrcCompiler *p, int index, int offset,
    int src, int dest, int imm);
void orc_sse_emit_sysinsn_store_memoffset (OrcCompiler *p, int index, int src,
    int offset, int dest, int imm);
void orc_sse_emit_sysinsn_load_memindex (OrcCompiler *p, int index, int imm,
    int offset, int src, int src_index, int shift, int dest);
void orc_sse_emit_sysinsn_load_register (OrcCompiler *p, int index, int imm,
    int src, int dest);
void orc_sse_emit_sysinsn_imm_reg (OrcCompiler *p, int index, int imm,
    int dest);
void orc_sse_emit_sysinsn_imm_memoffset (OrcCompiler *p, int index, int imm,
    int offset, int dest);
void orc_sse_emit_sysinsn_reg_memoffset (OrcCompiler *p, int index, int src,
    int offset, int dest);
void orc_sse_emit_sysinsn_memoffset_reg (OrcCompiler *p, int index, int offset,
    int src, int dest);
void orc_sse_emit_sysinsn_branch (OrcCompiler *p, int index, int label);
void orc_sse_emit_sysinsn_label (OrcCompiler *p, int index, int label);
void orc_sse_emit_sysinsn_none (OrcCompiler *p, int index);

unsigned int orc_sse_get_cpu_flags (void);


#endif

