
#ifndef _ORC_ARM_H_
#define _ORC_ARM_H_

#include <orc/orcprogram.h>

#define ARM_R0 (ORC_GP_REG_BASE+0)

#define ARM_A1 (ORC_GP_REG_BASE+0)
#define ARM_A2 (ORC_GP_REG_BASE+1)
#define ARM_A3 (ORC_GP_REG_BASE+2)
#define ARM_A4 (ORC_GP_REG_BASE+3)
#define ARM_V1 (ORC_GP_REG_BASE+4)
#define ARM_V2 (ORC_GP_REG_BASE+5)
#define ARM_V3 (ORC_GP_REG_BASE+6)
#define ARM_V4 (ORC_GP_REG_BASE+7)
#define ARM_V5 (ORC_GP_REG_BASE+8)
#define ARM_V6 (ORC_GP_REG_BASE+9)
#define ARM_V7 (ORC_GP_REG_BASE+10)
#define ARM_V8 (ORC_GP_REG_BASE+11)
#define ARM_IP (ORC_GP_REG_BASE+12)
#define ARM_SP (ORC_GP_REG_BASE+13)
#define ARM_LR (ORC_GP_REG_BASE+14)
#define ARM_PC (ORC_GP_REG_BASE+15)

#define ARM_SB (ORC_GP_REG_BASE+9)

enum {
  ARM_DP_AND = 0,
  ARM_DP_EOR,
  ARM_DP_SUB,
  ARM_DP_RSB,
  ARM_DP_ADD,
  ARM_DP_ADC,
  ARM_DP_SBC,
  ARM_DP_RSC,
  ARM_DP_TST,
  ARM_DP_TEQ,
  ARM_DP_CMP,
  ARM_DP_CMN,
  ARM_DP_ORR,
  ARM_DP_MOV,
  ARM_DP_BIC,
  ARM_DP_MVN
};

enum {
  ARM_COND_EQ = 0,
  ARM_COND_NE,
  ARM_COND_CS,
  ARM_COND_CC,
  ARM_COND_MI,
  ARM_COND_PL,
  ARM_COND_VS,
  ARM_COND_VC,
  ARM_COND_HI,
  ARM_COND_LS,
  ARM_COND_GE,
  ARM_COND_LT,
  ARM_COND_GT,
  ARM_COND_LE,
  ARM_COND_AL,
};

void arm_emit (OrcCompiler *compiler, uint32_t insn);
void arm_emit_bx_lr (OrcCompiler *compiler);
const char * arm_reg_name (int reg);
void arm_emit_load_imm (OrcCompiler *compiler, int dest, int imm);

void arm_emit_add (OrcCompiler *compiler, int dest, int src1, int src2);
void arm_emit_sub (OrcCompiler *compiler, int dest, int src1, int src2);
void arm_emit_add_imm (OrcCompiler *compiler, int dest, int src1, int value);
void arm_emit_and_imm (OrcCompiler *compiler, int dest, int src1, int value);
void arm_emit_sub_imm (OrcCompiler *compiler, int dest, int src1, int value);
void arm_emit_asr_imm (OrcCompiler *compiler, int dest, int src1, int value);
void arm_emit_cmp_imm (OrcCompiler *compiler, int src1, int value);
void arm_emit_cmp (OrcCompiler *compiler, int src1, int src2);

void arm_emit_label (OrcCompiler *compiler, int label);
void arm_emit_push (OrcCompiler *compiler, int regs);
void arm_emit_pop (OrcCompiler *compiler, int regs);
void arm_emit_mov (OrcCompiler *compiler, int dest, int src);
void arm_emit_branch (OrcCompiler *compiler, int cond, int label);

void arm_emit_dp_reg (OrcCompiler *compiler, int cond, int opcode, int dest,
    int src1, int src2);

void arm_loadw (OrcCompiler *compiler, int dest, int src1, int offset);
void arm_storew (OrcCompiler *compiler, int dest, int offset, int src1);

void arm_emit_load_reg (OrcCompiler *compiler, int dest, int src1, int offset);
void arm_emit_store_reg (OrcCompiler *compiler, int src, int dest, int offset);

void arm_do_fixups (OrcCompiler *compiler);

const char *neon_reg_name (int reg);
const char *neon_reg_name_quad (int reg);
void neon_emit_mov (OrcCompiler *compiler, int src, int dest);

#endif

