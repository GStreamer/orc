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

#include <orc/orccompiler.h>
#include <orc/orcinternal.h>
#include <orc/orcdebug.h>
#include <orc/orcutils.h>
#include <orc/loongarch/orcloongarch-internal.h>
#include <orc/loongarch/orcloongarch.h>
#include <orc/loongarch/orclasx-internal.h>

void
orc_lasx_compiler_init (OrcCompiler *c)
{
  int i;

  if (c->target_flags & ORC_TARGET_LOONGARCH_64BIT) {
    c->is_64bit = TRUE;
  } else {
    ORC_COMPILER_ERROR (c, "LoongArch32 is currently unsupported");
    return;
  }

  for (i = 0; i < 32; i++) {
    c->valid_regs[ORC_LOONG_XR0 + i] = 1;
    c->valid_regs[ORC_LOONG_ZERO + i] = 1;
  }

  c->valid_regs[ORC_LOONG_ZERO] = 0;
  c->valid_regs[ORC_LOONG_RA] = 0;
  c->valid_regs[ORC_LOONG_TP] = 0;
  c->valid_regs[ORC_LOONG_SP] = 0;
  c->valid_regs[ORC_LOONG_FP] = 0;
  c->valid_regs[ORC_LOONG_R21] = 0;
  c->valid_regs[ORC_LOONG_T1] = 0;
  c->valid_regs[ORC_LOONG_T2] = 0;
  c->valid_regs[ORC_LOONG_T3] = 0;

  c->tmpreg = ORC_LOONG_XR0;
  c->gp_tmpreg = ORC_LOONG_T0;
  c->valid_regs[c->tmpreg] = 0;
  c->valid_regs[c->gp_tmpreg] = 0;

  c->exec_reg = ORC_LOONG_A0;
  c->valid_regs[c->exec_reg] = 0;

  /* r23 to r31 are callee-saved */
  for (i = 23; i < 32; i++) {
    c->save_regs[ORC_GP_REG_BASE + i] = 1;
  }

  /* SIMD registers xr24-xr31 are callee-saved */
  for (i = 24; i < 32; i++) {
    c->save_regs[ORC_VEC_REG_BASE + 32 + i] = 1;
  }

  c->load_params = TRUE;
}

void
orc_lasx_compiler_assemble (OrcCompiler *c)
{
  ORC_ASSERT (FALSE);           /* TODO */
}
