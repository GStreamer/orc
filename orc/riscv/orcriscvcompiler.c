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

#include <orc/orccompiler.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>
#include <orc/orclimits.h>
#include <orc/orcutils.h>

#include <orc/riscv/orcriscv-internal.h>
#include <orc/riscv/orcriscv.h>

void
orc_riscv_compiler_init (OrcCompiler *c)
{
  if (c->target_flags & ORC_TARGET_RISCV_64BIT) {
    c->is_64bit = TRUE;
  } else {
    ORC_COMPILER_ERROR (c, "RV32 is currently unsupported");    /* FIXME */
  }

  for (int i = ORC_GP_REG_BASE; i < ORC_GP_REG_BASE + 32; i++) {
    c->valid_regs[i] = 1;
  }

  c->gp_tmpreg = ORC_RISCV_T0;
  c->valid_regs[c->gp_tmpreg] = 0;

  c->loop_counter = ORC_RISCV_INNER_COUNTER;
  c->valid_regs[ORC_RISCV_INNER_COUNTER] = 0;
  c->valid_regs[ORC_RISCV_OUTER_COUNTER] = 0;
  c->valid_regs[ORC_RISCV_VECTOR_LENGTH] = 0;

  c->exec_reg = ORC_RISCV_A0;
  c->valid_regs[c->exec_reg] = 0;

  c->valid_regs[ORC_RISCV_ZERO] = 0;
  c->valid_regs[ORC_RISCV_SP] = 0;

  c->save_regs[ORC_RISCV_RA] = 1;
  c->save_regs[ORC_RISCV_GP] = 1;
  c->save_regs[ORC_RISCV_TP] = 1;
  c->save_regs[ORC_RISCV_S0] = 1;
  c->save_regs[ORC_RISCV_S1] = 1;

  for (int i = ORC_RISCV_S2; i <= ORC_RISCV_S11; i++) {
    c->save_regs[i] = 1;
  }

  if (c->target_flags & ORC_TARGET_RISCV_V) {
    for (int i = ORC_VEC_REG_BASE + 8; i < ORC_VEC_REG_BASE + 32; i++) {
      c->valid_regs[i] = 1;
    }
  }

  c->min_temp_reg = ORC_VEC_REG_BASE;

  c->load_params = TRUE;
}

void
orc_riscv_compiler_add_fixup (OrcCompiler *c, int label)
{
  ORC_ASSERT (FALSE);           /* TODO */
}

void
orc_riscv_compiler_assemble (OrcCompiler *c)
{
  ORC_ASSERT (FALSE);           /* TODO */
}
