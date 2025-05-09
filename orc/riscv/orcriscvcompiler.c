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
#include <orc/orcprogram.h>
#include <orc/orcutils.h>

#include <orc/riscv/orcriscv-internal.h>
#include <orc/riscv/orcriscv.h>
#include <orc/riscv/orcriscvinsn.h>

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

static void
orc_riscv_compiler_add_label (OrcCompiler *c, int label)
{
  ORC_ASSERT (label < ORC_N_LABELS);
  ORC_ASM_CODE (c, ".L%s_%d:\n", c->program->name, label);
  c->labels[label] = c->codeptr;
}

void
orc_riscv_compiler_add_fixup (OrcCompiler *c, int label)
{
  ORC_ASSERT (c->n_fixups < ORC_N_FIXUPS);
  c->fixups[c->n_fixups].ptr = c->codeptr;
  c->fixups[c->n_fixups].label = label;
  c->fixups[c->n_fixups].type = 0;
  c->n_fixups++;
}

static void
orc_riscv_compiler_do_fixups (OrcCompiler *c)
{
  for (int i = 0; i < c->n_fixups; i++) {
    const void *label = c->labels[c->fixups[i].label];
    const void *ptr = c->fixups[i].ptr;
    const int diff = label - ptr;

    orc_uint32 code = ORC_READ_UINT32_LE (ptr);

    /* We only need to support branch-type fixups */
    ORC_ASSERT (diff < 4096);
    ORC_ASSERT (diff >= -4096);
    code |= (0b1000000000000 & diff) << (31 - 12);
    code |= (0b0100000000000 & diff) >> (11 - 7);
    code |= (0b0011111100000 & diff) << (25 - 5);
    code |= (0b0000000011110 & diff) << (8 - 1);

    ORC_WRITE_UINT32_LE (ptr, code);
  }
}

OrcRiscvVtype
orc_riscv_compiler_compute_vtype (OrcCompiler *c, OrcRiscvSEW element_width,
    int insn_shift)
{
  OrcRiscvVtype vtype = { };

  vtype.vsew = element_width;
  vtype.vlmul = (c->loop_shift + insn_shift + vtype.vsew - 3) & 0x7;
  vtype.vma = TRUE;
  vtype.vta = TRUE;

  return vtype;
}

static void
orc_riscv_compiler_emit_prologue (OrcCompiler *c)
{
  int stack_size = 0;

  orc_compiler_append_code (c, ".section .text\n");
  orc_compiler_append_code (c, ".global %s\n", c->program->name);
  orc_compiler_append_code (c, "%s:\n", c->program->name);

  for (int i = 0; i < 32; i++) {
    if (c->used_regs[ORC_GP_REG_BASE + i] && c->save_regs[ORC_GP_REG_BASE + i]) {
      stack_size += c->is_64bit ? 8 : 4;
      orc_riscv_insn_emit_sd (c, ORC_RISCV_SP, ORC_GP_REG_BASE + i,
          -stack_size);
    }
  }

  if (stack_size > 0) {
    orc_riscv_insn_emit_addi (c, ORC_RISCV_SP, ORC_RISCV_SP, -stack_size);
  }
}

static void
orc_riscv_compiler_emit_epilogue (OrcCompiler *c)
{
  int stack_size = 0;

  for (int i = 31, stack_size = 0; i >= 0; i--) {
    if (c->used_regs[ORC_GP_REG_BASE + i] && c->save_regs[ORC_GP_REG_BASE + i]) {
      orc_riscv_insn_emit_ld (c, ORC_GP_REG_BASE + i, ORC_RISCV_SP, stack_size);
      stack_size += c->is_64bit ? 8 : 4;
    }
  }

  if (stack_size > 0) {
    orc_riscv_insn_emit_addi (c, ORC_RISCV_SP, ORC_RISCV_SP, stack_size);
  }

  orc_riscv_insn_emit_ret (c);
}

void
orc_riscv_compiler_assemble (OrcCompiler *c)
{
  orc_riscv_compiler_emit_prologue (c);
  /* TODO: load constants */
  /* TODO: emit loop */
  orc_riscv_compiler_do_fixups (c);
  /* TODO: save accumulators */
  orc_riscv_compiler_emit_epilogue (c);
}
