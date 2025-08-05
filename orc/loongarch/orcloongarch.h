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
} OrcLoongRegister;

ORC_API orc_uint32 orc_loongarch_get_cpu_flags (void);
ORC_API void orc_loongarch_flush_cache (OrcCode *code);
ORC_API orc_uint32 orc_loongarch_target_get_default_flags (void);
ORC_API const char * orc_loongarch_reg_name (OrcLoongRegister reg);

#endif /* ORC_ENABLE_UNSTABLE_API */

ORC_END_DECLS

#endif /* _ORC_LOONGARCH_H_ */
