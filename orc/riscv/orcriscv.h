/*
  Copyright © 2024-2025 Samsung Electronics

  Author: Maksymilian Knust, m.knust@samsung.com
  Author: Filip Wasil, f.wasil@samsung.com

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

#ifndef _ORC_RISCV_H_
#define _ORC_RISCV_H_

#include <orc/orclimits.h>
#include <orc/orcutils.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  /* Scalar registers */
  ORC_RISCV_ZERO = ORC_GP_REG_BASE,
  ORC_RISCV_RA,
  ORC_RISCV_SP,
  ORC_RISCV_GP,
  ORC_RISCV_TP,
  ORC_RISCV_T0,
  ORC_RISCV_T1,
  ORC_RISCV_T2,
  ORC_RISCV_S0,
  ORC_RISCV_S1,
  ORC_RISCV_A0,
  ORC_RISCV_A1,
  ORC_RISCV_A2,
  ORC_RISCV_A3,
  ORC_RISCV_A4,
  ORC_RISCV_A5,
  ORC_RISCV_A6,
  ORC_RISCV_A7,
  ORC_RISCV_S2,
  ORC_RISCV_S3,
  ORC_RISCV_S4,
  ORC_RISCV_S5,
  ORC_RISCV_S6,
  ORC_RISCV_S7,
  ORC_RISCV_S8,
  ORC_RISCV_S9,
  ORC_RISCV_S10,
  ORC_RISCV_S11,
  ORC_RISCV_T3,
  ORC_RISCV_T4,
  ORC_RISCV_T5,
  ORC_RISCV_T6,

  /* Vector registers */
  ORC_RISCV_V0 = ORC_VEC_REG_BASE,
  ORC_RISCV_V1,
  ORC_RISCV_V2,
  ORC_RISCV_V3,
  ORC_RISCV_V4,
  ORC_RISCV_V5,
  ORC_RISCV_V6,
  ORC_RISCV_V7,
  ORC_RISCV_V8,
  ORC_RISCV_V9,
  ORC_RISCV_V10,
  ORC_RISCV_V11,
  ORC_RISCV_V12,
  ORC_RISCV_V13,
  ORC_RISCV_V14,
  ORC_RISCV_V15,
  ORC_RISCV_V16,
  ORC_RISCV_V17,
  ORC_RISCV_V18,
  ORC_RISCV_V19,
  ORC_RISCV_V20,
  ORC_RISCV_V21,
  ORC_RISCV_V22,
  ORC_RISCV_V23,
  ORC_RISCV_V24,
  ORC_RISCV_V25,
  ORC_RISCV_V26,
  ORC_RISCV_V27,
  ORC_RISCV_V28,
  ORC_RISCV_V29,
  ORC_RISCV_V30,
  ORC_RISCV_V31,

  /* Alias names */
  ORC_RISCV_VECTOR_LENGTH = ORC_RISCV_T1,
  ORC_RISCV_INNER_COUNTER = ORC_RISCV_T2,
  ORC_RISCV_OUTER_COUNTER = ORC_RISCV_T3,
} OrcRiscvRegister;

typedef enum {
  ORC_RISCV_SEW_8,
  ORC_RISCV_SEW_16,
  ORC_RISCV_SEW_32,
  ORC_RISCV_SEW_64,
  ORC_RISCV_SEW_NOT_APPLICABLE,
} OrcRiscvSEW;

typedef enum {
  ORC_RISCV_LMUL_1,
  ORC_RISCV_LMUL_2,
  ORC_RISCV_LMUL_4,
  ORC_RISCV_LMUL_8,
  ORC_RISCV_LMUL_RESERVED,
  ORC_RISCV_LMUL_F8,
  ORC_RISCV_LMUL_F4,
  ORC_RISCV_LMUL_F2,
} OrcRiscvLMUL;

typedef struct {
  OrcRiscvLMUL vlmul : 3;
  OrcRiscvSEW vsew : 3;
  orc_bool vta : 1;
  orc_bool vma : 1;
} OrcRiscvVtype;

typedef struct {
  OrcRiscvSEW element_width;
  orc_bool needs_mask_reg;
  int temp_regs_needed;
  orc_bool normalized_inputs;
  const orc_uint64 *constants;
} OrcRiscvRuleInfo;

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_RISCV_H_ */
