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

#include <orc/orcvariable.h>
#include <orc/orccompiler.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>
#include <orc/orclimits.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>

#include <orc/riscv/orcriscv-internal.h>
#include <orc/riscv/orcriscv.h>
#include <orc/riscv/orcriscvinsn.h>

typedef enum
{
  LABEL_END,
  LABEL_INNER_LOOP,
  LABEL_OUTER_LOOP,
} OrcRiscvLabel;

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

OrcRiscvSEW
orc_riscv_compiler_bytes_to_sew (int bytes)
{
  switch (bytes) {
    case 1:
      return ORC_RISCV_SEW_8;
    case 2:
      return ORC_RISCV_SEW_16;
    case 4:
      return ORC_RISCV_SEW_32;
    case 8:
      return ORC_RISCV_SEW_64;
    default:
      ORC_ASSERT (FALSE);
      return ORC_RISCV_SEW_NOT_APPLICABLE;
  }
}

static void
orc_riscv_compiler_load_constants (OrcCompiler *c)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    OrcVariable *var = c->vars + i;
    if (var->name == NULL)
      continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_riscv_insn_emit_ld (c, var->ptr_register, c->exec_reg,
            ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]));
        break;
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_riscv_insn_emit_vsetvli (c, c->gp_tmpreg, ORC_RISCV_ZERO,
            orc_riscv_compiler_compute_vtype (c,
                orc_riscv_compiler_bytes_to_sew (c->vars[i].size), 0));
        orc_riscv_insn_emit_vmv_vx (c, var->alloc, ORC_RISCV_ZERO);
        break;
      default:
        break;
    }

    if (var->ptr_offset)
      orc_riscv_insn_emit_addi (c, var->ptr_offset, ORC_RISCV_ZERO, 0);
  }

  orc_compiler_emit_invariants (c);
}

static void
orc_riscv_compiler_emit_loop (OrcCompiler *c, OrcRiscvVtype vtype)
{
  for (int i = 0; i < c->n_insns; i++) {
    OrcInstruction *insn = c->insns + i;

    if (insn->flags & ORC_INSN_FLAG_INVARIANT)
      continue;

    orc_compiler_append_code (c, "/* %d: %s */\n", i, insn->opcode->name);

    if (insn->rule && insn->rule->emit) {
      OrcRiscvRuleInfo *info = insn->rule->emit_user;

      OrcRiscvVtype old_vtype = vtype;

      c->insn_shift = 0;
      if (insn->flags & ORC_INSTRUCTION_FLAG_X2)
        c->insn_shift = 1;
      if (insn->flags & ORC_INSTRUCTION_FLAG_X4)
        c->insn_shift = 2;

      if (info->element_width != ORC_RISCV_SEW_NOT_APPLICABLE) {
        vtype =
            orc_riscv_compiler_compute_vtype (c, info->element_width,
            c->insn_shift);
      } else {
        vtype = orc_riscv_compiler_compute_vtype (c, old_vtype.vsew, 0);
      }

      if (*(orc_uint8 *) & old_vtype != *(orc_uint8 *) & vtype) {
        if (c->insn_shift)
          orc_riscv_insn_emit_vsetvli (c, c->gp_tmpreg, ORC_RISCV_ZERO, vtype);
        else
          orc_riscv_insn_emit_vsetvli (c, ORC_RISCV_ZERO,
              ORC_RISCV_VECTOR_LENGTH, vtype);
      }

      insn->rule->emit (c, insn->rule->emit_user, insn);
    } else {
      ORC_COMPILER_ERROR (c, "No rule for %s\n", insn->opcode->name);
    }
  }

  orc_compiler_append_code (c, "/* loop tail */\n");

  for (int i = 0, length_shift = 0; i < 4; i++) {
    for (int j = 0; j < ORC_N_COMPILER_VARIABLES; j++) {
      const OrcVariable *var = c->vars + j;

      if (var->size != 1 << i)
        continue;
      if (!var->name)
        continue;
      if (!var->ptr_register)
        continue;
      if (var->vartype != ORC_VAR_TYPE_SRC && var->vartype != ORC_VAR_TYPE_DEST)
        continue;

      if (length_shift != i) {
        orc_riscv_insn_emit_slli (c, ORC_RISCV_VECTOR_LENGTH,
            ORC_RISCV_VECTOR_LENGTH, i - length_shift);
        length_shift = i;
      }

      orc_riscv_insn_emit_add (c, var->ptr_register, var->ptr_register,
          ORC_RISCV_VECTOR_LENGTH);
    }
  }
}

static OrcRiscvVtype
orc_riscv_compiler_compute_initial_vtype (OrcCompiler *c)
{
  for (int i = 0; i < c->n_insns; i++) {
    const OrcRiscvRuleInfo *info = c->insns[i].rule->emit_user;

    if (info->element_width != ORC_RISCV_SEW_NOT_APPLICABLE)
      return orc_riscv_compiler_compute_vtype (c, info->element_width, 0);
  }

  return orc_riscv_compiler_compute_vtype (c, ORC_RISCV_SEW_8, 0);
}

static void
orc_riscv_compiler_add_strides (OrcCompiler *c)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (c->vars[i].name == NULL)
      continue;
    switch (c->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_riscv_insn_emit_ld (c, c->gp_tmpreg, c->exec_reg,
            ORC_STRUCT_OFFSET (OrcExecutor, n));

        if (c->vars[i].size != 1)
          orc_riscv_insn_emit_slli (c, c->gp_tmpreg, c->gp_tmpreg,
              orc_riscv_compiler_bytes_to_sew (c->vars[i].size));

        orc_riscv_insn_emit_sub (c, c->vars[i].ptr_register,
            c->vars[i].ptr_register, c->gp_tmpreg);
        orc_riscv_insn_emit_lw (c, c->gp_tmpreg, c->exec_reg,
            ORC_STRUCT_OFFSET (OrcExecutor, params[i]));
        orc_riscv_insn_emit_add (c, c->vars[i].ptr_register,
            c->vars[i].ptr_register, c->gp_tmpreg);
        break;
      default:
        ORC_COMPILER_ERROR (c, "bad vartype");
        break;
    }
  }
}

static void
orc_riscv_compiler_emit_full_loop (OrcCompiler *c)
{
  OrcRiscvVtype initial = orc_riscv_compiler_compute_initial_vtype (c);

  if (c->program->is_2d) {
    orc_riscv_insn_emit_lw (c, ORC_RISCV_OUTER_COUNTER, c->exec_reg,
        ORC_STRUCT_OFFSET (OrcExecutorAlt, m));
    orc_riscv_insn_emit_beq (c, ORC_RISCV_OUTER_COUNTER, ORC_RISCV_ZERO,
        LABEL_END);
    orc_riscv_compiler_add_label (c, LABEL_OUTER_LOOP);
  }

  orc_riscv_insn_emit_lw (c, c->loop_counter, c->exec_reg,
      ORC_STRUCT_OFFSET (OrcExecutor, n));

  orc_riscv_insn_emit_beq (c, c->loop_counter, ORC_RISCV_ZERO, LABEL_END);
  orc_riscv_compiler_add_label (c, LABEL_INNER_LOOP);

  orc_riscv_insn_emit_vsetvli (c, ORC_RISCV_VECTOR_LENGTH, c->loop_counter,
      initial);
  orc_riscv_insn_emit_sub (c, c->loop_counter, c->loop_counter,
      ORC_RISCV_VECTOR_LENGTH);

  orc_riscv_compiler_emit_loop (c, initial);

  orc_riscv_insn_emit_bne (c, c->loop_counter, ORC_RISCV_ZERO,
      LABEL_INNER_LOOP);

  if (c->program->is_2d) {
    orc_riscv_compiler_add_strides (c);
    orc_riscv_insn_emit_addi (c, ORC_RISCV_OUTER_COUNTER,
        ORC_RISCV_OUTER_COUNTER, -1);
    orc_riscv_insn_emit_bne (c, ORC_RISCV_OUTER_COUNTER, ORC_RISCV_ZERO,
        LABEL_OUTER_LOOP);
  }

  orc_riscv_compiler_add_label (c, LABEL_END);
}

static void
orc_riscv_compiler_save_accumulators (OrcCompiler *c)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (c->vars[i].vartype == ORC_VAR_TYPE_ACCUMULATOR) {
      const OrcRiscvSEW sew = orc_riscv_compiler_bytes_to_sew (c->vars[i].size);
      const OrcRiscvVtype vtype = orc_riscv_compiler_compute_vtype (c, sew, 0);
      const OrcRiscvRegister reg = c->vars[i].alloc;
      const int offset =
          ORC_STRUCT_OFFSET (OrcExecutor, accumulators[i - ORC_VAR_A1]);

      orc_riscv_insn_emit_vsetvli (c, c->gp_tmpreg, ORC_RISCV_ZERO, vtype);
      orc_riscv_insn_emit_vand_vi (c, ORC_RISCV_V0, ORC_RISCV_V0, 0);
      orc_riscv_insn_emit_vredsum_vv (c, reg, ORC_RISCV_V0, reg);
      orc_riscv_insn_emit_vmv_xs (c, c->gp_tmpreg, reg);

      if (c->vars[i].size == 2) {
        orc_riscv_insn_emit_sh (c, c->exec_reg, c->gp_tmpreg, offset);
      } else {
        orc_riscv_insn_emit_sw (c, c->exec_reg, c->gp_tmpreg, offset);
      }
    }
  }
}

static int
orc_riscv_compiler_var_priority_comp (const void *var_x, const void *var_y)
{
  const OrcVariable *x = *(OrcVariable **) var_x, *y = *(OrcVariable **) var_y;
  if (x->size == y->size)
    return x->first_use - y->first_use;
  else
    return x->size - y->size;
}

static void
orc_riscv_compiler_reallocate_registers (OrcCompiler *c)
{
  OrcVariable *vars[ORC_N_COMPILER_VARIABLES] = { NULL };

  int n_vars = 0;
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++)
    if (c->vars[i].alloc >= ORC_RISCV_V0 && c->vars[i].alloc <= ORC_RISCV_V31)
      vars[n_vars++] = &c->vars[i];

  qsort (vars, n_vars, sizeof (&vars[0]), orc_riscv_compiler_var_priority_comp);

  for (int i = 0; i < n_vars; i++) {
    orc_uint32 cost[ORC_N_REGS] = { 0 };

    for (int j = 0; j < 32; j++)
      for (int k = 1; k <= 3; k++)
        if (j & ((1 << k) - 1))
          cost[ORC_VEC_REG_BASE + j]++;

    for (int j = 0; j < c->n_insns; j++)
      if (j >= vars[i]->first_use
          && (j <= vars[i]->last_use || vars[i]->first_use == -1))
        if (((OrcRiscvRuleInfo *) c->insns[j].rule->emit_user)->needs_mask_reg)
          cost[ORC_RISCV_V0] = 1000;

    /* FIXME: remove this */
    for (int i = ORC_RISCV_V0; i <= ORC_RISCV_V7; i++)
      cost[i] = 1000;

    for (int j = 0; j < i; j++) {
      if ((vars[i]->first_use > vars[j]->last_use ||
              vars[j]->first_use > vars[i]->last_use) &&
          vars[i]->first_use != -1 && vars[j]->first_use != -1)
        continue;

      cost[vars[j]->alloc] = 1000;

      for (int k = 0; 1 << k <= vars[j]->size * 8 / c->max_var_size; k++)
        for (int w = 1; w < 1 << k; w++)
          cost[vars[j]->alloc + w] += 10;

      for (int k = 0; 1 << k <= vars[i]->size * 8 / c->max_var_size; k++)
        for (int w = 1; w < 1 << k; w++)
          cost[vars[j]->alloc - w] += 10;
    }

    for (int j = ORC_RISCV_V0; j <= ORC_RISCV_V31; j++)
      if (cost[j] < cost[vars[i]->alloc])
        vars[i]->alloc = j;

    if (cost[vars[i]->alloc] >= 1000)
      ORC_COMPILER_ERROR (c, "cannot allocate register for var %s",
          vars[i]->name);
  }
}

void
orc_riscv_compiler_assemble (OrcCompiler *c)
{
  orc_riscv_compiler_reallocate_registers (c);
  c->loop_shift = 3 - orc_riscv_bytes_to_sew (c->max_var_size);
  orc_riscv_compiler_emit_prologue (c);
  orc_riscv_compiler_load_constants (c);
  orc_riscv_compiler_emit_full_loop (c);
  orc_riscv_compiler_do_fixups (c);
  orc_riscv_compiler_save_accumulators (c);
  orc_riscv_compiler_emit_epilogue (c);
}
