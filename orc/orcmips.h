#ifndef _ORC_MIPS_H_
#define _ORC_MIPS_H_

#include <orc/orcutils.h>
#include <orc/orcprogram.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  ORC_MIPS_ZERO = ORC_GP_REG_BASE+0,
  ORC_MIPS_AT,
  ORC_MIPS_V0,
  ORC_MIPS_V1,
  ORC_MIPS_A0,
  ORC_MIPS_A1,
  ORC_MIPS_A2,
  ORC_MIPS_A3,
  ORC_MIPS_T0,
  ORC_MIPS_T1,
  ORC_MIPS_T2,
  ORC_MIPS_T3,
  ORC_MIPS_T4,
  ORC_MIPS_T5,
  ORC_MIPS_T6,
  ORC_MIPS_T7,
  ORC_MIPS_S0,
  ORC_MIPS_S1,
  ORC_MIPS_S2,
  ORC_MIPS_S3,
  ORC_MIPS_S4,
  ORC_MIPS_S5,
  ORC_MIPS_S6,
  ORC_MIPS_S7,
  ORC_MIPS_T8,
  ORC_MIPS_T9,
  ORC_MIPS_K0,
  ORC_MIPS_K1,
  ORC_MIPS_GP,
  ORC_MIPS_SP,
  ORC_MIPS_FP,
  ORC_MIPS_RA
} OrcMipsRegister;

void orc_mips_emit_label (OrcCompiler *compiler, unsigned int label);

void orc_mips_emit_nop (OrcCompiler *compiler);

void orc_mips_emit_sw (OrcCompiler *compiler, OrcMipsRegister reg,
                       OrcMipsRegister base, unsigned int offset);

void orc_mips_emit_lw (OrcCompiler *compiler, OrcMipsRegister dest,
                       OrcMipsRegister base, unsigned int offset);

void orc_mips_emit_jr (OrcCompiler *compiler, OrcMipsRegister address_reg);
void orc_mips_emit_blez (OrcCompiler *compiler, OrcMipsRegister reg, unsigned int label);
void orc_mips_emit_bnez (OrcCompiler *compiler, OrcMipsRegister reg, unsigned int label);

void orc_mips_emit_addiu (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source, int value);
void orc_mips_emit_add (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source1, OrcMipsRegister source2);
void orc_mips_emit_move (OrcCompiler *compiler, OrcMipsRegister dest, OrcMipsRegister source);


#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_MIPS_H_ */
