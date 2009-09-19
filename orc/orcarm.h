
#ifndef _ORC_ARM_H_
#define _ORC_ARM_H_

#include <orc/orcprogram.h>


typedef enum {
  ORC_ARM_A1 = ORC_GP_REG_BASE+0,
  ORC_ARM_A2,
  ORC_ARM_A3,
  ORC_ARM_A4,
  ORC_ARM_V1,
  ORC_ARM_V2,
  ORC_ARM_V3,
  ORC_ARM_V4,
  ORC_ARM_V5,
  ORC_ARM_V6,
  ORC_ARM_V7,
  ORC_ARM_V8,
  ORC_ARM_IP,
  ORC_ARM_SP,
  ORC_ARM_LR,
  ORC_ARM_PC
} OrcArmRegister;

typedef enum {
  ORC_ARM_DP_AND = 0,
  ORC_ARM_DP_EOR,
  ORC_ARM_DP_SUB,
  ORC_ARM_DP_RSB,
  ORC_ARM_DP_ADD,
  ORC_ARM_DP_ADC,
  ORC_ARM_DP_SBC,
  ORC_ARM_DP_RSC,
  ORC_ARM_DP_TST,
  ORC_ARM_DP_TEQ,
  ORC_ARM_DP_CMP,
  ORC_ARM_DP_CMN,
  ORC_ARM_DP_ORR,
  ORC_ARM_DP_MOV,
  ORC_ARM_DP_BIC,
  ORC_ARM_DP_MVN
} OrcArmDP;

typedef enum {
  ORC_ARM_COND_EQ = 0,
  ORC_ARM_COND_NE,
  ORC_ARM_COND_CS,
  ORC_ARM_COND_CC,
  ORC_ARM_COND_MI,
  ORC_ARM_COND_PL,
  ORC_ARM_COND_VS,
  ORC_ARM_COND_VC,
  ORC_ARM_COND_HI,
  ORC_ARM_COND_LS,
  ORC_ARM_COND_GE,
  ORC_ARM_COND_LT,
  ORC_ARM_COND_GT,
  ORC_ARM_COND_LE,
  ORC_ARM_COND_AL,
} OrcArmCond;

typedef enum {
  ORC_ARM_LSL,
  ORC_ARM_LSR,
  ORC_ARM_ASR,
  ORC_ARM_ROR
} OrcArmShift;

void orc_arm_emit (OrcCompiler *compiler, uint32_t insn);
void orc_arm_emit_bx_lr (OrcCompiler *compiler);
const char * orc_arm_reg_name (int reg);
const char * orc_arm_cond_name (OrcArmCond cond);
void orc_arm_emit_loadimm (OrcCompiler *compiler, int dest, int imm);

void orc_arm_emit_label (OrcCompiler *compiler, int label);
void orc_arm_emit_push (OrcCompiler *compiler, int regs);
void orc_arm_emit_pop (OrcCompiler *compiler, int regs);
void orc_arm_emit_branch (OrcCompiler *compiler, int cond, int label);

void orc_arm_loadw (OrcCompiler *compiler, int dest, int src1, int offset);
void orc_arm_storew (OrcCompiler *compiler, int dest, int offset, int src1);

void orc_arm_emit_load_reg (OrcCompiler *compiler, int dest, int src1, int offset);

void orc_arm_do_fixups (OrcCompiler *compiler);

void orc_arm_emit_dp (OrcCompiler *p, int type, OrcArmCond cond, OrcArmDP opcode,
        int S, int Rd, int Rn, int Rm, int shift, int val);
void orc_arm_emit_mm (OrcCompiler *p, const char *name, OrcArmCond cond, int mode,
        int op, int dest, int src1, int src2);

/* ALL cpus */
/* data procesing instructions */
/* <op>{<cond>}{s} {<Rd>}, <Rn>, #imm */
#define orc_arm_emit_and_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_AND,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_eor_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_EOR,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_sub_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_SUB,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_rsb_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_RSB,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_add_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_ADD,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_adc_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_ADC,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_sbc_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_SBC,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_rsc_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_RSC,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_tst_i(p,cond,Rn,rot,imm)         orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_TST,1, 0,Rn,0,rot,imm)
#define orc_arm_emit_teq_i(p,cond,Rn,rot,imm)         orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_TEQ,1, 0,Rn,0,rot,imm)
#define orc_arm_emit_cmp_i(p,cond,Rn,rot,imm)         orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_CMP,1, 0,Rn,0,rot,imm)
#define orc_arm_emit_cmn_i(p,cond,Rn,rot,imm)         orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_CMN,1, 0,Rn,0,rot,imm)
#define orc_arm_emit_orr_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_ORR,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_mov_i(p,cond,S,Rd,rot,imm)       orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_MOV,S,Rd, 0,0,rot,imm)
#define orc_arm_emit_bic_i(p,cond,S,Rd,Rn,rot,imm)    orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_BIC,S,Rd,Rn,0,rot,imm)
#define orc_arm_emit_mvn_i(p,cond,S,Rd,rot,imm)       orc_arm_emit_dp(p,0,cond,ORC_ARM_DP_MVN,S,Rd, 0,0,rot,imm)

/* <op>{<cond>}{s} {<Rd>}, <Rn>, <Rm> */
#define orc_arm_emit_and_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_AND,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_eor_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_EOR,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_sub_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_SUB,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_rsb_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_RSB,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_add_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_ADD,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_adc_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_ADC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_sbc_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_SBC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_rsc_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_RSC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_tst_r(p,cond,Rn,Rm)              orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_TST,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_teq_r(p,cond,Rn,Rm)              orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_TEQ,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_cmp_r(p,cond,Rn,Rm)              orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_CMP,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_cmn_r(p,cond,Rn,Rm)              orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_CMN,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_orr_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_ORR,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_mov_r(p,cond,S,Rd,Rm)            orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_MOV,S,Rd, 0,Rm,0,0)
#define orc_arm_emit_bic_r(p,cond,S,Rd,Rn,Rm)         orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_BIC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_mvn_r(p,cond,S,Rd,Rm)            orc_arm_emit_dp(p,1,cond,ORC_ARM_DP_MVN,S,Rd, 0,Rm,0,0)

/* <op>{<cond>}{s} {<Rd>}, <Rn>, <Rm>, [LSL|LSR|ASR] #imm */
#define orc_arm_emit_and_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_AND,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_eor_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_EOR,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_sub_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_SUB,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_rsb_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_RSB,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_add_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_ADD,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_adc_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_ADC,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_sbc_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_SBC,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_rsc_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_RSC,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_tst_rsi(p,cond,Rn,Rm,sh,im)      orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_TST,1, 0,Rn,Rm,sh,im)
#define orc_arm_emit_teq_rsi(p,cond,Rn,Rm,sh,im)      orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_TEQ,1, 0,Rn,Rm,sh,im)
#define orc_arm_emit_cmp_rsi(p,cond,Rn,Rm,sh,im)      orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_CMP,1, 0,Rn,Rm,sh,im)
#define orc_arm_emit_cmn_rsi(p,cond,Rn,Rm,sh,im)      orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_CMN,1, 0,Rn,Rm,sh,im)
#define orc_arm_emit_orr_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_ORR,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_mov_rsi(p,cond,S,Rd,Rm,sh,im)    orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_MOV,S,Rd, 0,Rm,sh,im)
#define orc_arm_emit_bic_rsi(p,cond,S,Rd,Rn,Rm,sh,im) orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_BIC,S,Rd,Rn,Rm,sh,im)
#define orc_arm_emit_mvn_rsi(p,cond,S,Rd,Rm,sh,im)    orc_arm_emit_dp(p,2,cond,ORC_ARM_DP_MVN,S,Rd, 0,Rm,sh,im)

/* <op>{<cond>}{s} {<Rd>}, <Rn>, <Rm>, [LSL|LSR|ASR] <Rs> */
#define orc_arm_emit_and_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_AND,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_eor_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_EOR,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_sub_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_SUB,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_rsb_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_RSB,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_add_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_ADD,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_adc_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_ADC,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_sbc_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_SBC,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_rsc_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_RSC,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_tst_rsr(p,cond,Rn,Rm,sh,Rs)      orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_TST,1, 0,Rn,Rm,sh,Rs)
#define orc_arm_emit_teq_rsr(p,cond,Rn,Rm,sh,Rs)      orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_TEQ,1, 0,Rn,Rm,sh,Rs)
#define orc_arm_emit_cmp_rsr(p,cond,Rn,Rm,sh,Rs)      orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_CMP,1, 0,Rn,Rm,sh,Rs)
#define orc_arm_emit_cmn_rsr(p,cond,Rn,Rm,sh,Rs)      orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_CMN,1, 0,Rn,Rm,sh,Rs)
#define orc_arm_emit_orr_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_ORR,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_mov_rsr(p,cond,S,Rd,Rm,sh,Rs)    orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_MOV,S,Rd, 0,Rm,sh,Rs)
#define orc_arm_emit_bic_rsr(p,cond,S,Rd,Rn,Rm,sh,Rs) orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_BIC,S,Rd,Rn,Rm,sh,Rs)
#define orc_arm_emit_mvn_rsr(p,cond,S,Rd,Rm,sh,Rs)    orc_arm_emit_dp(p,3,cond,ORC_ARM_DP_MVN,S,Rd, 0,Rm,sh,Rs)

/* <op>{<cond>}{s} {<Rd>,} <Rn>, <Rm>, RRX */
#define orc_arm_emit_and_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_AND,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_eor_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_EOR,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_sub_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_SUB,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_rsb_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_RSB,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_add_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_ADD,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_adc_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_ADC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_sbc_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_SBC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_rsc_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_RSC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_tst_rrx(p,cond,Rn,Rm)            orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_TST,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_teq_rrx(p,cond,Rn,Rm)            orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_TEQ,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_cmp_rrx(p,cond,Rn,Rm)            orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_CMP,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_cmn_rrx(p,cond,Rn,Rm)            orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_CMN,1, 0,Rn,Rm,0,0)
#define orc_arm_emit_orr_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_ORR,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_mov_rrx(p,cond,S,Rd,Rm)          orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_MOV,S,Rd, 0,Rm,0,0)
#define orc_arm_emit_bic_rrx(p,cond,S,Rd,Rn,Rm)       orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_BIC,S,Rd,Rn,Rm,0,0)
#define orc_arm_emit_mvn_rrx(p,cond,S,Rd,Rm)          orc_arm_emit_dp(p,4,cond,ORC_ARM_DP_MVN,S,Rd, 0,Rm,0,0)

/* parallel instructions */
#define orc_arm_emit_sadd16(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"sadd16",   cond,0x61,0x1,Rd,Rn,Rm)
#define orc_arm_emit_qadd16(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"qadd16",   cond,0x62,0x1,Rd,Rn,Rm)
#define orc_arm_emit_shadd16(p,cond,Rd,Rn,Rm)   orc_arm_emit_mm(p,"shadd16",  cond,0x63,0x1,Rd,Rn,Rm)
#define orc_arm_emit_uadd16(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"uadd16",   cond,0x65,0x1,Rd,Rn,Rm)
#define orc_arm_emit_uqadd16(p,cond,Rd,Rn,Rm)   orc_arm_emit_mm(p,"uqadd16",  cond,0x66,0x1,Rd,Rn,Rm)
#define orc_arm_emit_uhadd16(p,cond,Rd,Rn,Rm)   orc_arm_emit_mm(p,"uhadd16",  cond,0x67,0x1,Rd,Rn,Rm)

#define orc_arm_emit_saddsubx(p,cond,Rd,Rn,Rm)  orc_arm_emit_mm(p,"saddsubx", cond,0x61,0x3,Rd,Rn,Rm)
#define orc_arm_emit_qaddsubx(p,cond,Rd,Rn,Rm)  orc_arm_emit_mm(p,"qaddsubx", cond,0x62,0x3,Rd,Rn,Rm)
#define orc_arm_emit_shaddsubx(p,cond,Rd,Rn,Rm) orc_arm_emit_mm(p,"shaddsubx",cond,0x63,0x3,Rd,Rn,Rm)
#define orc_arm_emit_uaddsubx(p,cond,Rd,Rn,Rm)  orc_arm_emit_mm(p,"uaddsubx", cond,0x65,0x3,Rd,Rn,Rm)
#define orc_arm_emit_uqaddsubx(p,cond,Rd,Rn,Rm) orc_arm_emit_mm(p,"uqaddsubx",cond,0x66,0x3,Rd,Rn,Rm)
#define orc_arm_emit_uhaddsubx(p,cond,Rd,Rn,Rm) orc_arm_emit_mm(p,"uhaddsubx",cond,0x67,0x3,Rd,Rn,Rm)

#define orc_arm_emit_ssubaddx(p,cond,Rd,Rn,Rm)  orc_arm_emit_mm(p,"ssubaddx", cond,0x61,0x5,Rd,Rn,Rm)
#define orc_arm_emit_qsubaddx(p,cond,Rd,Rn,Rm)  orc_arm_emit_mm(p,"qsubaddx", cond,0x62,0x5,Rd,Rn,Rm)
#define orc_arm_emit_shsubaddx(p,cond,Rd,Rn,Rm) orc_arm_emit_mm(p,"shsubaddx",cond,0x63,0x5,Rd,Rn,Rm)
#define orc_arm_emit_usubaddx(p,cond,Rd,Rn,Rm)  orc_arm_emit_mm(p,"usubaddx", cond,0x65,0x5,Rd,Rn,Rm)
#define orc_arm_emit_uqsubaddx(p,cond,Rd,Rn,Rm) orc_arm_emit_mm(p,"uqsubaddx",cond,0x66,0x5,Rd,Rn,Rm)
#define orc_arm_emit_uhsubaddx(p,cond,Rd,Rn,Rm) orc_arm_emit_mm(p,"uhsubaddx",cond,0x67,0x5,Rd,Rn,Rm)

#define orc_arm_emit_ssub16(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"ssub16",   cond,0x61,0x7,Rd,Rn,Rm)
#define orc_arm_emit_qsub16(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"qsub16",   cond,0x62,0x7,Rd,Rn,Rm)
#define orc_arm_emit_shsub16(p,cond,Rd,Rn,Rm)   orc_arm_emit_mm(p,"shsub16",  cond,0x63,0x7,Rd,Rn,Rm)
#define orc_arm_emit_usub16(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"usub16",   cond,0x65,0x7,Rd,Rn,Rm)
#define orc_arm_emit_uqsub16(p,cond,Rd,Rn,Rm)   orc_arm_emit_mm(p,"uqsub16",  cond,0x66,0x7,Rd,Rn,Rm)
#define orc_arm_emit_uhsub16(p,cond,Rd,Rn,Rm)   orc_arm_emit_mm(p,"uhsub16",  cond,0x67,0x7,Rd,Rn,Rm)

#define orc_arm_emit_sadd8(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"sadd8",    cond,0x61,0x9,Rd,Rn,Rm)
#define orc_arm_emit_qadd8(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"qadd8",    cond,0x62,0x9,Rd,Rn,Rm)
#define orc_arm_emit_shadd8(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"shadd8",   cond,0x63,0x9,Rd,Rn,Rm)
#define orc_arm_emit_uadd8(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"uadd8",    cond,0x65,0x9,Rd,Rn,Rm)
#define orc_arm_emit_uqadd8(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"uqadd8",   cond,0x66,0x9,Rd,Rn,Rm)
#define orc_arm_emit_uhadd8(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"uhadd8",   cond,0x67,0x9,Rd,Rn,Rm)

#define orc_arm_emit_ssub8(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"ssub8",    cond,0x61,0xf,Rd,Rn,Rm)
#define orc_arm_emit_qsub8(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"qsub8",    cond,0x62,0xf,Rd,Rn,Rm)
#define orc_arm_emit_shsub8(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"shsub8",   cond,0x63,0xf,Rd,Rn,Rm)
#define orc_arm_emit_usub8(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"usub8",    cond,0x65,0xf,Rd,Rn,Rm)
#define orc_arm_emit_uqsub8(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"uqsub8",   cond,0x66,0xf,Rd,Rn,Rm)
#define orc_arm_emit_uhsub8(p,cond,Rd,Rn,Rm)    orc_arm_emit_mm(p,"uhsub8",   cond,0x67,0xf,Rd,Rn,Rm)

#define orc_arm_emit_ssat16(p,cond,Rd,sat,Rm)   orc_arm_emit_mm(p,"ssat16",   cond,0x6a,0x3,Rd,sat,Rm)
#define orc_arm_emit_usat16(p,cond,Rd,sat,Rm)   orc_arm_emit_mm(p,"usat16",   cond,0x6e,0x3,Rd,sat,Rm)

/* misc instructions */
#define orc_arm_emit_sel(p,cond,Rd,Rn,Rm)       orc_arm_emit_mm(p,"sel", cond,0x68,0xb,Rd,Rn,Rm)

#define orc_arm_emit_qadd(p,cond,Rd,Rn,Rm)      orc_arm_emit_mm(p,"qadd",     cond,0x10,0x5,Rd,Rn,Rm)
#define orc_arm_emit_qsub(p,cond,Rd,Rn,Rm)      orc_arm_emit_mm(p,"qsub",     cond,0x12,0x5,Rd,Rn,Rm)
#define orc_arm_emit_qdadd(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"qdadd",    cond,0x14,0x5,Rd,Rn,Rm)
#define orc_arm_emit_qdsub(p,cond,Rd,Rn,Rm)     orc_arm_emit_mm(p,"qdsub",    cond,0x16,0x5,Rd,Rn,Rm)

#endif

