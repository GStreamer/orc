
#ifndef _ORC_C64X_H_
#define _ORC_C64X_H_

#include <orc/orcprogram.h>

#define C64X_R0 (ORC_GP_REG_BASE+0)

#define C64X_A1 (ORC_GP_REG_BASE+0)
#define C64X_A2 (ORC_GP_REG_BASE+1)
#define C64X_A3 (ORC_GP_REG_BASE+2)
#define C64X_A4 (ORC_GP_REG_BASE+3)
#define C64X_V1 (ORC_GP_REG_BASE+4)
#define C64X_V2 (ORC_GP_REG_BASE+5)
#define C64X_V3 (ORC_GP_REG_BASE+6)
#define C64X_V4 (ORC_GP_REG_BASE+7)
#define C64X_V5 (ORC_GP_REG_BASE+8)
#define C64X_V6 (ORC_GP_REG_BASE+9)
#define C64X_V7 (ORC_GP_REG_BASE+10)
#define C64X_V8 (ORC_GP_REG_BASE+11)
#define C64X_IP (ORC_GP_REG_BASE+12)
#define C64X_SP (ORC_GP_REG_BASE+13)
#define C64X_LR (ORC_GP_REG_BASE+14)
#define C64X_PC (ORC_GP_REG_BASE+15)

#define C64X_SB (ORC_GP_REG_BASE+9)

enum {
  C64X_DP_AND = 0,
  C64X_DP_EOR,
  C64X_DP_SUB,
  C64X_DP_RSB,
  C64X_DP_ADD,
  C64X_DP_ADC,
  C64X_DP_SBC,
  C64X_DP_RSC,
  C64X_DP_TST,
  C64X_DP_TEQ,
  C64X_DP_CMP,
  C64X_DP_CMN,
  C64X_DP_ORR,
  C64X_DP_MOV,
  C64X_DP_BIC,
  C64X_DP_MVN
};

enum {
  C64X_COND_EQ = 0,
  C64X_COND_NE,
  C64X_COND_CS,
  C64X_COND_CC,
  C64X_COND_MI,
  C64X_COND_PL,
  C64X_COND_VS,
  C64X_COND_VC,
  C64X_COND_HI,
  C64X_COND_LS,
  C64X_COND_GE,
  C64X_COND_LT,
  C64X_COND_GT,
  C64X_COND_LE,
  C64X_COND_AL,
};

void orc_c64x_emit (OrcCompiler *compiler, uint32_t insn);
void orc_c64x_emit_bx_lr (OrcCompiler *compiler);
const char * orc_c64x_reg_name (int reg);
void orc_c64x_emit_load_imm (OrcCompiler *compiler, int dest, int imm);

void orc_c64x_emit_add (OrcCompiler *compiler, int dest, int src1, int src2);
void orc_c64x_emit_sub (OrcCompiler *compiler, int dest, int src1, int src2);
void orc_c64x_emit_add_imm (OrcCompiler *compiler, int dest, int src1, int value);
void orc_c64x_emit_and_imm (OrcCompiler *compiler, int dest, int src1, int value);
void orc_c64x_emit_sub_imm (OrcCompiler *compiler, int dest, int src1, int value);
void orc_c64x_emit_asr_imm (OrcCompiler *compiler, int dest, int src1, int value);
void orc_c64x_emit_cmp_imm (OrcCompiler *compiler, int src1, int value);
void orc_c64x_emit_cmp (OrcCompiler *compiler, int src1, int src2);

void orc_c64x_emit_label (OrcCompiler *compiler, int label);
void orc_c64x_emit_push (OrcCompiler *compiler, int regs);
void orc_c64x_emit_pop (OrcCompiler *compiler, int regs);
void orc_c64x_emit_mov (OrcCompiler *compiler, int dest, int src);
void orc_c64x_emit_branch (OrcCompiler *compiler, int cond, int label);

void orc_c64x_emit_dp_reg (OrcCompiler *compiler, int cond, int opcode, int dest,
    int src1, int src2);

void orc_c64x_emit_load_reg (OrcCompiler *compiler, int dest, int src1, int offset);
void orc_c64x_emit_store_reg (OrcCompiler *compiler, int src, int dest, int offset);

void orc_c64x_do_fixups (OrcCompiler *compiler);

void orc_c64x_loadb (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned);
void orc_c64x_loadw (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned);
void orc_c64x_loadl (OrcCompiler *compiler, int dest, int src1, int update, int is_aligned);
void orc_c64x_neg (OrcCompiler *compiler, int dest);
void orc_c64x_storeb (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned);
void orc_c64x_storew (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned);
void orc_c64x_storel (OrcCompiler *compiler, int dest, int update, int src1, int is_aligned);
void orc_c64x_emit_loadib (OrcCompiler *p, int reg, int value);
void orc_c64x_emit_loadiw (OrcCompiler *p, int reg, int value);
void orc_c64x_emit_loadil (OrcCompiler *p, int reg, int value);
void orc_c64x_emit_loadpb (OrcCompiler *p, int reg, int param);
void orc_c64x_emit_loadpw (OrcCompiler *p, int reg, int param);
void orc_c64x_emit_loadpl (OrcCompiler *p, int reg, int param);


#endif

