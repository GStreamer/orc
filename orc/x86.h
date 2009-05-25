
#ifndef _ORC_X86_H_
#define _ORC_X86_H_

#include <unistd.h>

void orc_x86_emit_push (OrcCompiler *compiler, int size, int reg);
void orc_x86_emit_pop (OrcCompiler *compiler, int size, int reg);
void orc_x86_emit_mov_memoffset_reg (OrcCompiler *compiler, int size, int offset, int reg1, int reg2);
void orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2);
void orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);
void orc_x86_emit_mov_reg_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2);
void orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2);
void orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);
void orc_x86_emit_mov_imm_reg (OrcCompiler *compiler, int size, int value, int reg1);
void orc_x86_emit_mov_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_mov_sse_reg_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_mmx_reg_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_reg_mmx (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_mmx_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_reg_sse (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_sse_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_test_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_sar_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
void orc_x86_emit_dec_memoffset (OrcCompiler *compiler, int size, int offset, int reg);
void orc_x86_emit_add_imm_memoffset (OrcCompiler *compiler, int size, int value, int offset, int reg);
void orc_x86_emit_add_reg_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg);
void orc_x86_emit_and_imm_memoffset (OrcCompiler *compiler, int size, int value, int offset, int reg);
void orc_x86_emit_add_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
void orc_x86_emit_and_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
void orc_x86_emit_sub_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_sub_memoffset_reg (OrcCompiler *compiler, int size,
    int offset, int reg, int destreg);
void orc_x86_emit_cmp_reg_memoffset (OrcCompiler *compiler, int size, int reg1,
    int offset, int reg);
void orc_x86_emit_cmp_imm_memoffset (OrcCompiler *compiler, int size, int value,
    int offset, int reg);
void orc_x86_emit_emms (OrcCompiler *compiler);
void orc_x86_emit_ret (OrcCompiler *compiler);
void orc_x86_emit_jle (OrcCompiler *compiler, int label);
void orc_x86_emit_je (OrcCompiler *compiler, int label);
void orc_x86_emit_jne (OrcCompiler *compiler, int label);
void orc_x86_emit_jmp (OrcCompiler *compiler, int label);
void orc_x86_emit_label (OrcCompiler *compiler, int label);
void orc_x86_emit_align (OrcCompiler *compiler);
void x86_do_fixups (OrcCompiler *compiler);
void orc_x86_emit_prologue (OrcCompiler *compiler);
void orc_x86_emit_epilogue (OrcCompiler *compiler);

void orc_x86_emit_rex (OrcCompiler *compiler, int size, int reg1, int reg2, int reg3);
void orc_x86_emit_modrm_memoffset (OrcCompiler *compiler, int reg1, int offset, int reg2);
void orc_x86_emit_modrm_reg (OrcCompiler *compiler, int reg1, int reg2);
void x86_test (OrcCompiler *compiler);

void orc_mmx_emit_loadiw (OrcCompiler *p, int reg, int value);

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

enum {
  X86_EAX = ORC_GP_REG_BASE,
  X86_ECX,
  X86_EDX,
  X86_EBX,
  X86_ESP,
  X86_EBP,
  X86_ESI,
  X86_EDI,
  X86_R8,
  X86_R9,
  X86_R10,
  X86_R11,
  X86_R12,
  X86_R13,
  X86_R14,
  X86_R15
};

enum {
  X86_MM0 = ORC_VEC_REG_BASE,
  X86_MM1,
  X86_MM2,
  X86_MM3,
  X86_MM4,
  X86_MM5,
  X86_MM6,
  X86_MM7
};

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

const char * orc_x86_get_regname(int i);
int orc_x86_get_regnum(int i);
const char * orc_x86_get_regname_16(int i);
const char * orc_x86_get_regname_64(int i);
const char * orc_x86_get_regname_ptr(OrcCompiler *compiler, int i);
const char * orc_x86_get_regname_mmx(int i);
const char * orc_x86_get_regname_sse(int i);

#endif

