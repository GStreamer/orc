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
#include <orc/loongarch/orclsx-internal.h>
#include <orc/loongarch/orclsxinsn.h>
#include <orc/loongarch/orcloongarch-internal.h>
#include <orc/loongarch/orcloongarchinsn.h>
#include <orc/loongarch/orcloongarch.h>

void
orc_lsx_compiler_init (OrcCompiler *c)
{
  int i;

  if (c->target_flags & ORC_TARGET_LOONGARCH_64BIT) {
    c->is_64bit = TRUE;
  } else {
    ORC_COMPILER_ERROR (c, "LoongArch32 is currently unsupported");
    return;
  }

  for (i = 0; i < 32; i++) {
    c->valid_regs[ORC_LOONG_VR0 + i] = 1;
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

  c->tmpreg = ORC_LOONG_VR0;
  c->gp_tmpreg = ORC_LOONG_T0;
  c->valid_regs[c->tmpreg] = 0;
  c->valid_regs[c->gp_tmpreg] = 0;

  c->exec_reg = ORC_LOONG_A0;
  c->valid_regs[c->exec_reg] = 0;

  /* r23 to r31 are callee-saved */
  for (i = 23; i < 32; i++) {
    c->save_regs[ORC_GP_REG_BASE + i] = 1;
  }

  /* SIMD registers vr24-vr31 are callee-saved */
  for (i = 24; i < 32; i++) {
    c->save_regs[ORC_VEC_REG_BASE + i] = 1;
  }

  /*Small functions may run faster when unrolled.*/
  if (c->n_insns <= 10) {
    c->unroll_shift = 1;
  }

  c->load_params = TRUE;
}

void
orc_lsx_compiler_emit_prologue (OrcCompiler *c)
{
  int stack_size = 0;

  orc_compiler_append_code (c, ".section .text\n");
  orc_compiler_append_code (c, ".global %s\n", c->program->name);
  orc_compiler_append_code (c, "%s:\n", c->program->name);

  for (int i = 0; i < 32; i++) {
    if (c->used_regs[ORC_GP_REG_BASE + i] && c->save_regs[ORC_GP_REG_BASE + i]) {
      stack_size += 8;
      orc_loongarch_insn_emit_st_d (c, ORC_LOONG_SP, ORC_GP_REG_BASE + i, -stack_size);
    }
  }

  for (int i = 0; i < 32; i++) {
    if (c->used_regs[ORC_VEC_REG_BASE + i] && c->save_regs[ORC_VEC_REG_BASE + i]) {
      stack_size += 8;
      orc_lsx_insn_emit_vstelmd (c, ORC_VEC_REG_BASE + i, ORC_LOONG_SP, -stack_size, 0);
    }
  }

  if (stack_size > 0) {
    orc_loongarch_insn_emit_addi_d (c, ORC_LOONG_SP, ORC_LOONG_SP, -stack_size);
  }
}

void
orc_lsx_compiler_emit_epilogue (OrcCompiler *c)
{
  int stack_size = 0;

  for (int i = 31; i >= 0; i--) {
    if (c->used_regs[ORC_GP_REG_BASE + i] && c->save_regs[ORC_GP_REG_BASE + i]) {
      orc_loongarch_insn_emit_ld_d (c, ORC_GP_REG_BASE + i, ORC_LOONG_SP, stack_size);
      stack_size += 8;
    }
  }

  for (int i = 31; i >= 0; i--) {
    if (c->used_regs[ORC_VEC_REG_BASE + i] && c->save_regs[ORC_VEC_REG_BASE + i]) {
      orc_lsx_insn_emit_vldrepld (c, ORC_GP_REG_BASE + i, ORC_LOONG_SP, stack_size);
      stack_size += 8;
    }
  }

  if (stack_size > 0) {
    orc_loongarch_insn_emit_addi_d (c, ORC_LOONG_SP, ORC_LOONG_SP, stack_size);
  }

  orc_loongarch_insn_emit_ret (c);
}

void
orc_lsx_compiler_compute_loop_shift (OrcCompiler *c)
{
  switch (c->max_var_size) {
    case 1:
      c->loop_shift = 4;
      break;
    case 2:
      c->loop_shift = 3;
      break;
    case 4:
      c->loop_shift = 2;
      break;
    case 8:
      c->loop_shift = 1;
      break;
    default:
      ORC_ERROR ("unhandled max var size %d", c->max_var_size);
      break;
  }
}

void
orc_lsx_compiler_load_constants (OrcCompiler *c)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    OrcVariable *var = c->vars + i;
    if (var->name == NULL)
      continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_loongarch_insn_emit_ld_d (c, var->ptr_register, c->exec_reg,
            ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]));
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_lsx_insn_emit_vxorv (c, var->alloc, var->alloc, var->alloc);
        break;
      default:
        break;
    }
  }
  orc_compiler_emit_invariants (c);
}

void
orc_lsx_compiler_save_accumulators (OrcCompiler *c)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (c->vars[i].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
      const OrcLoongRegister reg = c->vars[i].alloc;
      const int offset = ORC_STRUCT_OFFSET (OrcExecutor, accumulators[i - ORC_VAR_A1]);
      orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, c->exec_reg, offset);
      if (c->vars[i].size == 2) {
        orc_lsx_insn_emit_vhaddwwh (c, reg, reg, reg);
        orc_lsx_insn_emit_vhaddwdw (c, reg, reg, reg);
        orc_lsx_insn_emit_vhaddwqd (c, reg, reg, reg);
        orc_lsx_insn_emit_vstelmh (c, reg, c->gp_tmpreg, 0, 0);
      }
      if (c->vars[i].size == 4) {
        orc_lsx_insn_emit_vhaddwduwu (c, reg, reg, reg);
        orc_lsx_insn_emit_vhaddwqudu (c, reg, reg, reg);
        orc_lsx_insn_emit_vstelmw (c, reg, c->gp_tmpreg, 0, 0);
      }
    }
  }
}

void
orc_lsx_compiler_assemble (OrcCompiler *c)
{
  orc_lsx_compiler_compute_loop_shift (c);
  orc_lsx_compiler_emit_prologue (c);
  orc_lsx_compiler_load_constants (c);
  orc_loongarch_compiler_emit_full_loop (c);
  orc_loongarch_compiler_do_fixups (c);
  orc_lsx_compiler_save_accumulators (c);
  orc_lsx_compiler_emit_epilogue (c);
}
