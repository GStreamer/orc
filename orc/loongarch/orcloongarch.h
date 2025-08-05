/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: jinbo, jinbo@loongson.cn
  Author: hecai yuan, yuanhecai@loongson.cn

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _ORC_LOONGARCH_H_
#define _ORC_LOONGARCH_H_

#include <orc/orcutils.h>
#include <orc/orclimits.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  /* Scalar registers */
  ORC_LOONG_ZERO = ORC_GP_REG_BASE,
  ORC_LOONG_RA,
  ORC_LOONG_TP,
  ORC_LOONG_SP,
  ORC_LOONG_A0,
  ORC_LOONG_A1,
  ORC_LOONG_A2,
  ORC_LOONG_A3,
  ORC_LOONG_A4,
  ORC_LOONG_A5,
  ORC_LOONG_A6,
  ORC_LOONG_A7,
  ORC_LOONG_T0,
  ORC_LOONG_T1,
  ORC_LOONG_T2,
  ORC_LOONG_T3,
  ORC_LOONG_T4,
  ORC_LOONG_T5,
  ORC_LOONG_T6,
  ORC_LOONG_T7,
  ORC_LOONG_T8,
  ORC_LOONG_R21,
  ORC_LOONG_FP, //S9
  ORC_LOONG_S0,
  ORC_LOONG_S1,
  ORC_LOONG_S2,
  ORC_LOONG_S3,
  ORC_LOONG_S4,
  ORC_LOONG_S5,
  ORC_LOONG_S6,
  ORC_LOONG_S7,
  ORC_LOONG_S8,

  /* Vector registers (LSX) */
  ORC_LOONG_VR0 = ORC_VEC_REG_BASE,
  ORC_LOONG_VR1,
  ORC_LOONG_VR2,
  ORC_LOONG_VR3,
  ORC_LOONG_VR4,
  ORC_LOONG_VR5,
  ORC_LOONG_VR6,
  ORC_LOONG_VR7,
  ORC_LOONG_VR8,
  ORC_LOONG_VR9,
  ORC_LOONG_VR10,
  ORC_LOONG_VR11,
  ORC_LOONG_VR12,
  ORC_LOONG_VR13,
  ORC_LOONG_VR14,
  ORC_LOONG_VR15,
  ORC_LOONG_VR16,
  ORC_LOONG_VR17,
  ORC_LOONG_VR18,
  ORC_LOONG_VR19,
  ORC_LOONG_VR20,
  ORC_LOONG_VR21,
  ORC_LOONG_VR22,
  ORC_LOONG_VR23,
  ORC_LOONG_VR24,
  ORC_LOONG_VR25,
  ORC_LOONG_VR26,
  ORC_LOONG_VR27,
  ORC_LOONG_VR28,
  ORC_LOONG_VR29,
  ORC_LOONG_VR30,
  ORC_LOONG_VR31,

  /* Vector registers (ALSX) */
  ORC_LOONG_XR0 = ORC_VEC_REG_BASE + 32,
  ORC_LOONG_XR1,
  ORC_LOONG_XR2,
  ORC_LOONG_XR3,
  ORC_LOONG_XR4,
  ORC_LOONG_XR5,
  ORC_LOONG_XR6,
  ORC_LOONG_XR7,
  ORC_LOONG_XR8,
  ORC_LOONG_XR9,
  ORC_LOONG_XR10,
  ORC_LOONG_XR11,
  ORC_LOONG_XR12,
  ORC_LOONG_XR13,
  ORC_LOONG_XR14,
  ORC_LOONG_XR15,
  ORC_LOONG_XR16,
  ORC_LOONG_XR17,
  ORC_LOONG_XR18,
  ORC_LOONG_XR19,
  ORC_LOONG_XR20,
  ORC_LOONG_XR21,
  ORC_LOONG_XR22,
  ORC_LOONG_XR23,
  ORC_LOONG_XR24,
  ORC_LOONG_XR25,
  ORC_LOONG_XR26,
  ORC_LOONG_XR27,
  ORC_LOONG_XR28,
  ORC_LOONG_XR29,
  ORC_LOONG_XR30,
  ORC_LOONG_XR31,
} OrcLoongRegister;

/* Instruction encoding formats on LoongArch
           3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
           1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
2R-TYPE   |--------------------opcode-----------------|----rj---|---rd----|
3R-TYPE   |-------------opcode--------------|---rk----|----rj---|---rd----|
4R-TYPE   |-------------opcode----|----ra---|---rk----|----rj---|---rd----|
2RI8-TYPE |-------------opcode--------|-------I8------|----rj---|---rd----|
2RI12-TYPE|-----opcode--------|---------I12-----------|----rj---|---rd----|
2RI14-TYPE|-----opcode----|-----------I14-------------|----rj---|---rd----|
2RI16-TYPE|---opcode--|---------I16-------------------|----rj---|---rd----|
1RI21-TYPE|---opcode--|------------I21[15:0]----------|----rj---|-I21[20:16]-|
I26-TYPE  |---opcode--|------------I26[15:0]----------|----I26[25:16]-----|
*/

#define LOONG_2R_INSTRUCTION(opcode,rd,rj) \
    (((opcode) & 0x3fffff) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_3R_INSTRUCTION(opcode,rd,rj,rk) \
    (((opcode) & 0x1ffff) << 15 | ((rk) & 0x1f) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_4R_INSTRUCTION(opcode,rd,rj,rk,ra) \
    (((opcode) & 0xfff) << 20 | ((ra) & 0x1f) << 15 | ((rk) & 0x1f) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_2RI5_INSTRUCTION(opcode,rd,rj,imm5) \
    (((opcode) & 0x1ffff) << 15 | ((imm5) & 0x1f) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_2RI8_INSTRUCTION(opcode,rd,rj,imm8) \
    (((opcode) & 0x3fff) << 18 | ((imm8) & 0xff) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_2RI12_INSTRUCTION(opcode,rd,rj,imm12) \
    (((opcode) & 0x3ff) << 22 | ((imm12) & 0xfff) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_2RI14_INSTRUCTION(opcode,rd,rj,imm14) \
    (((opcode) & 0xff) << 24 | ((imm14) & 0x3fff) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_2RI16_INSTRUCTION(opcode,rd,rj,imm16) \
    (((opcode) & 0x3f) << 26 | ((imm16) & 0xffff) << 10 | ((rj) & 0x1f) << 5 | (rd & 0x1f))

#define LOONG_1RI21_INSTRUCTION(opcode,rj,imm21) \
    (((opcode) & 0x3f) << 26 | ((imm21) & 0xffff) << 10 | ((rj) & 0x1f) << 5 | ((imm21 >> 16) & 0x1f))

#define LOONG_1RI25_INSTRUCTION(opcode,rd,imm20) \
    (((opcode) & 0x7f) << 25 | ((imm20) & 0xfffff) << 5 | (rd & 0x1f))

#define LOONG_I26_INSTRUCTION(opcode,imm26) \
    (((opcode) & 0x3f) << 26 | ((imm26) & 0xffff) << 10 | ((imm26 >> 16) & 0x3ff))

static inline orc_uint32
orc_loongarch_gp_reg (OrcLoongRegister reg)
{
  ORC_ASSERT (reg >= ORC_GP_REG_BASE && reg < ORC_VEC_REG_BASE);
  return reg - ORC_GP_REG_BASE;
}

static inline orc_uint32
orc_loongarch_lsx_reg (OrcLoongRegister reg)
{
  ORC_ASSERT (reg >= ORC_VEC_REG_BASE && reg < ORC_VEC_REG_BASE + 32);
  return reg - ORC_VEC_REG_BASE;
}

static inline orc_uint32
orc_loongarch_lasx_reg (OrcLoongRegister reg)
{
  ORC_ASSERT (reg >= ORC_VEC_REG_BASE + 32 && reg < ORC_VEC_REG_BASE + 64);
  return reg - ORC_VEC_REG_BASE;
}

#define GREG(reg) orc_loongarch_gp_reg(reg)
#define VREG(reg) orc_loongarch_lsx_reg(reg)
#define XREG(reg) orc_loongarch_lasx_reg(reg)
#define NAME(reg) orc_loongarch_reg_name(reg)

ORC_API orc_uint32 orc_loongarch_get_cpu_flags (void);
ORC_API void orc_loongarch_flush_cache (OrcCode *code);
ORC_API orc_uint32 orc_loongarch_target_get_default_flags (void);
ORC_API const char * orc_loongarch_reg_name (OrcLoongRegister reg);
ORC_API void orc_loongarch_insn_emit32 (OrcCompiler *const c, const orc_uint32 insn);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_LOONGARCH_H_ */
