
#ifndef _ORC_ARM_H_
#define _ORC_ARM_H_

#include <orc/orcprogram.h>

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

void arm_emit (OrcProgram *program, uint32_t insn);
void arm_emit_bx_lr (OrcProgram *program);
const char * arm_reg_name (int reg);
void arm_emit_loadimm (OrcProgram *program, int dest, int imm);

void arm_emit_add (OrcProgram *program, int dest, int src1, int src2);
void arm_emit_sub (OrcProgram *program, int dest, int src1, int src2);
void arm_emit_sub_imm (OrcProgram *program, int dest, int src1, int value);
void arm_emit_cmp_imm (OrcProgram *program, int src1, int value);

void arm_emit_label (OrcProgram *program, int label);
void arm_emit_push (OrcProgram *program, int regs);
void arm_emit_pop (OrcProgram *program, int regs);
void arm_emit_mov (OrcProgram *program, int dest, int src);
void arm_emit_branch (OrcProgram *program, int cond, int label);

void arm_emit_dp_reg (OrcProgram *program, int cond, int opcode, int dest,
    int src1, int src2);

void arm_loadw (OrcProgram *program, int dest, int src1, int offset);
void arm_storew (OrcProgram *program, int dest, int offset, int src1);

void arm_emit_load_reg (OrcProgram *program, int dest, int src1, int offset);

void arm_do_fixups (OrcProgram *program);


#endif

