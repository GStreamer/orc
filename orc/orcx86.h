
#ifndef _ORC_X86_H_
#define _ORC_X86_H_

#include <orc/orcprogram.h>

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

const char * orc_x86_get_regname(int i);
int orc_x86_get_regnum(int i);
const char * orc_x86_get_regname_16(int i);
const char * orc_x86_get_regname_64(int i);
const char * orc_x86_get_regname_ptr(OrcCompiler *compiler, int i);

void orc_x86_emit_push (OrcCompiler *compiler, int size, int reg);
void orc_x86_emit_pop (OrcCompiler *compiler, int size, int reg);
void orc_x86_emit_mov_memoffset_reg (OrcCompiler *compiler, int size, int offset, int reg1, int reg2);
void orc_x86_emit_mov_reg_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2);
void orc_x86_emit_mov_imm_reg (OrcCompiler *compiler, int size, int value, int reg1);
void orc_x86_emit_mov_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_test_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_sar_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
void orc_x86_emit_dec_memoffset (OrcCompiler *compiler, int size, int offset, int reg);
void orc_x86_emit_add_imm_memoffset (OrcCompiler *compiler, int size, int value, int offset, int reg);
void orc_x86_emit_add_reg_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg);
void orc_x86_emit_and_imm_memoffset (OrcCompiler *compiler, int size, int value, int offset, int reg);
void orc_x86_emit_add_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
void orc_x86_emit_and_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
void orc_x86_emit_add_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_sub_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
void orc_x86_emit_imul_memoffset_reg (OrcCompiler *compiler, int size,
    int offset, int reg, int destreg);
void orc_x86_emit_sub_memoffset_reg (OrcCompiler *compiler, int size,
    int offset, int reg, int destreg);
void orc_x86_emit_cmp_reg_memoffset (OrcCompiler *compiler, int size, int reg1,
    int offset, int reg);
void orc_x86_emit_cmp_imm_memoffset (OrcCompiler *compiler, int size, int value,
    int offset, int reg);
void orc_x86_emit_test_imm_memoffset (OrcCompiler *compiler, int size, int value,
    int offset, int reg);
void orc_x86_emit_emms (OrcCompiler *compiler);
void orc_x86_emit_ret (OrcCompiler *compiler);
void orc_x86_emit_jle (OrcCompiler *compiler, int label);
void orc_x86_emit_je (OrcCompiler *compiler, int label);
void orc_x86_emit_jne (OrcCompiler *compiler, int label);
void orc_x86_emit_jmp (OrcCompiler *compiler, int label);
void orc_x86_emit_label (OrcCompiler *compiler, int label);
void orc_x86_emit_align (OrcCompiler *compiler);
void orc_x86_emit_prologue (OrcCompiler *compiler);
void orc_x86_emit_epilogue (OrcCompiler *compiler);

void orc_x86_emit_rex (OrcCompiler *compiler, int size, int reg1, int reg2, int reg3);
void orc_x86_emit_modrm_memoffset (OrcCompiler *compiler, int reg1, int offset, int reg2);
void orc_x86_emit_modrm_reg (OrcCompiler *compiler, int reg1, int reg2);

void orc_x86_do_fixups (OrcCompiler *compiler);


#endif

