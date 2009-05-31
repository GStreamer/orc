
#ifndef _ORC_POWERPC_H_
#define _ORC_POWERPC_H_

#include <orc/orcprogram.h>

enum {
  POWERPC_R0 = ORC_GP_REG_BASE,
  POWERPC_R1,
  POWERPC_R2,
  POWERPC_R3,
  POWERPC_R4,
  POWERPC_R5,
  POWERPC_R6,
  POWERPC_R7,
  POWERPC_R8,
  POWERPC_R9,
  POWERPC_R10,
  POWERPC_R11,
  POWERPC_R12,
  POWERPC_R13,
  POWERPC_R14,
  POWERPC_R15,
  POWERPC_R16,
  POWERPC_R17,
  POWERPC_R18,
  POWERPC_R19,
  POWERPC_R20,
  POWERPC_R21,
  POWERPC_R22,
  POWERPC_R23,
  POWERPC_R24,
  POWERPC_R25,
  POWERPC_R26,
  POWERPC_R27,
  POWERPC_R28,
  POWERPC_R29,
  POWERPC_R30,
  POWERPC_R31,
  POWERPC_V0 = ORC_VEC_REG_BASE,
  POWERPC_V1,
  POWERPC_V2,
  POWERPC_V3,
  POWERPC_V4,
  POWERPC_V5,
  POWERPC_V6,
  POWERPC_V7,
  POWERPC_V8,
  POWERPC_V9,
  POWERPC_V10,
  POWERPC_V11,
  POWERPC_V12,
  POWERPC_V13,
  POWERPC_V14,
  POWERPC_V15,
  POWERPC_V16,
  POWERPC_V17,
  POWERPC_V18,
  POWERPC_V19,
  POWERPC_V20,
  POWERPC_V21,
  POWERPC_V22,
  POWERPC_V23,
  POWERPC_V24,
  POWERPC_V25,
  POWERPC_V26,
  POWERPC_V27,
  POWERPC_V28,
  POWERPC_V29,
  POWERPC_V30,
  POWERPC_V31
};

const char * powerpc_get_regname(int i);
int powerpc_regnum (int i);

void powerpc_emit(OrcCompiler *compiler, unsigned int insn);

void powerpc_emit_addi (OrcCompiler *compiler, int regd, int rega, int imm);
void powerpc_emit_lwz (OrcCompiler *compiler, int regd, int rega, int imm);
void powerpc_emit_stwu (OrcCompiler *compiler, int regs, int rega, int offset);

void powerpc_emit_ret (OrcCompiler *compiler);
void powerpc_emit_b (OrcCompiler *compiler, int label);
void powerpc_emit_beq (OrcCompiler *compiler, int label);
void powerpc_emit_bne (OrcCompiler *compiler, int label);
void powerpc_emit_label (OrcCompiler *compiler, int label);
void powerpc_add_fixup (OrcCompiler *compiler, int type, unsigned char *ptr, int label);
void powerpc_do_fixups (OrcCompiler *compiler);
void powerpc_flush (OrcCompiler *compiler);

void powerpc_emit_srawi (OrcCompiler *compiler, int regd, int rega, int shift,
    int record);
void powerpc_emit_655510 (OrcCompiler *compiler, int major, int d, int a,
    int b, int minor);
void powerpc_emit_X (OrcCompiler *compiler, unsigned int insn, int d, int a,
    int b);
void powerpc_emit_VA (OrcCompiler *compiler, int major, int d, int a, int b,
    int c, int minor);
void powerpc_emit_VX (OrcCompiler *compiler, unsigned int insn, int d, int a,
    int b);
void powerpc_emit_VX_2 (OrcCompiler *p, const char *name, unsigned int insn,
    int d, int a, int b);
int powerpc_get_constant (OrcCompiler *p, int type, int value);

#endif

