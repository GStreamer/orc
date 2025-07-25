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
#include <orc/orcprogram.h>
#include <orc/orcinternal.h>
#include <orc/orcdebug.h>
#include <orc/loongarch/orcloongarch-internal.h>
#include <orc/loongarch/orcloongarchinsn.h>

typedef enum {
  ORC_LOONG_OFFS16,
  ORC_LOONG_OFFS21,
  ORC_LOONG_OFFS26
} OrcLoongBranchType;

typedef enum {
  LABEL_LOOP_OUTER,
  LABEL_LOOP_INNER,
  LABEL_LOOP_TAIL,
  LABEL_END
} OrcLoongBranchLabel;

#define LABEL_SETUP(x) (4 + (x))

void
orc_loongarch_compiler_add_label (OrcCompiler *c, int label)
{
  ORC_ASSERT (label < ORC_N_LABELS);

  ORC_ASM_CODE (c, ".L%s_%d:\n", c->program->name, label);
  c->labels[label] = c->codeptr;
}

void
orc_loongarch_compiler_add_fixup (OrcCompiler *c, int label, int type)
{
  ORC_ASSERT (c->n_fixups < ORC_N_FIXUPS);

  c->fixups[c->n_fixups].ptr = c->codeptr;
  c->fixups[c->n_fixups].label = label;
  c->fixups[c->n_fixups].type = type;
  c->n_fixups++;
}

void
orc_loongarch_compiler_do_fixups (OrcCompiler *c)
{
  int i;
  for(i=0;i<c->n_fixups;i++){
    unsigned char *label = c->labels[c->fixups[i].label];
    unsigned char *ptr = c->fixups[i].ptr;
    orc_uint32 code;
    int offset;

    offset = (label - ptr) >> 2;
    code = ORC_READ_UINT32_LE (ptr);

    switch (c->fixups[i].type) {
      case ORC_LOONG_OFFS16:
        code |= (offset & 0xffff) << 10;
        break;
      case ORC_LOONG_OFFS21:
        code |= (offset & 0xffff) << 10 | ((offset >> 16) & 0x1f);
        break;
      case ORC_LOONG_OFFS26:
        code |= (offset & 0xffff) << 10 | ((offset >> 16) & 0x3ff);
        break;
       default:
        ORC_ASSERT ("Invalid type"); /* unreachable */
        break;
    }
    ORC_WRITE_UINT32_LE (ptr, code);
  }
}

orc_uint32
orc_loongarch_compiler_get_shift (orc_uint32 bytes)
{
  switch (bytes) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    case 8:
      return 3;
    default:
      ORC_ASSERT (FALSE);
      return -1;
  }
}

void
orc_loongarch_compiler_add_strides (OrcCompiler *c)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (c->vars[i].name == NULL)
      continue;
    switch (c->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_loongarch_insn_emit_ld_w (c, c->gp_tmpreg, c->exec_reg,
            ORC_STRUCT_OFFSET (OrcExecutor, n));
        if (c->vars[i].update_type == 1)
          orc_loongarch_insn_emit_srli_d (c, c->gp_tmpreg, c->gp_tmpreg, 1);
        if (c->vars[i].size != 1)
          orc_loongarch_insn_emit_slli_d (c, c->gp_tmpreg, c->gp_tmpreg,
              orc_loongarch_compiler_get_shift(c->vars[i].size));
        orc_loongarch_insn_emit_sub_d (c, c->vars[i].ptr_register,
            c->vars[i].ptr_register, c->gp_tmpreg);
        orc_loongarch_insn_emit_ld_w (c, c->gp_tmpreg, c->exec_reg,
            ORC_STRUCT_OFFSET (OrcExecutor, params[i]));
        orc_loongarch_insn_emit_add_d (c, c->vars[i].ptr_register,
            c->vars[i].ptr_register, c->gp_tmpreg);
        break;
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        ORC_COMPILER_ERROR (c, "bad vartype");
        break;
    }
  }
}

void
orc_loongarch_compiler_emit_loop (OrcCompiler *c, int update)
{
  for (int i = 0; i < c->n_insns; i++) {
    OrcInstruction *insn = c->insns + i;

    if (insn->flags & ORC_INSN_FLAG_INVARIANT)
      continue;

    orc_compiler_append_code (c, "/* %d: %s */\n", i, insn->opcode->name);

    if (insn->rule && insn->rule->emit) {
      c->insn_shift = c->loop_shift;
      if (insn->flags & ORC_INSTRUCTION_FLAG_X2) {
        c->insn_shift += 1;
      }
      if (insn->flags & ORC_INSTRUCTION_FLAG_X4) {
        c->insn_shift += 2;
      }
      insn->rule->emit (c, insn->rule->emit_user, insn);
    } else {
      ORC_COMPILER_ERROR (c, "No rule for %s\n", insn->opcode->name);
    }
  }

  orc_compiler_append_code (c, "/* loop tail */\n");

  if (update) {
    for (int j = 0; j < ORC_N_COMPILER_VARIABLES; j++) {
      const OrcVariable *var = c->vars + j;

      if (var->name == NULL) continue;
      if (var->vartype == ORC_VAR_TYPE_SRC
          || var->vartype == ORC_VAR_TYPE_DEST) {
        int offset = 0;
        if (var->update_type == 1) {
          offset = (var->size * update) >> 1;
        } else {
          offset = var->size * update;
        }
        if (offset != 0 && var->ptr_register) {
          orc_loongarch_insn_emit_addi_d ( c,
              var->ptr_register,
              var->ptr_register,
              offset
          );
        }
      }
    }
  }
}

void
orc_loongarch_compiler_emit_full_loop (OrcCompiler *c)
{
  int ui, ui_max, l;

  if (c->program->is_2d) {
    orc_loongarch_insn_emit_ld_w (c, ORC_LOONG_T1, c->exec_reg,
        ORC_STRUCT_OFFSET (OrcExecutorAlt, m));
    orc_loongarch_insn_emit_beqz (c, ORC_LOONG_T1, LABEL_END, ORC_LOONG_OFFS21);
    orc_loongarch_compiler_add_label (c, LABEL_LOOP_OUTER);
  }

  orc_loongarch_insn_emit_ld_w (c, ORC_LOONG_T2, c->exec_reg,
      ORC_STRUCT_OFFSET (OrcExecutor, n));

  orc_loongarch_insn_emit_beqz (c, ORC_LOONG_T2, LABEL_END, ORC_LOONG_OFFS21);
  orc_loongarch_insn_emit_andi (c, ORC_LOONG_T3, ORC_LOONG_T2,
      (1 << (c->loop_shift + c->unroll_shift)) - 1);
  orc_loongarch_insn_emit_srli_d (c, ORC_LOONG_T2, ORC_LOONG_T2,
      c->loop_shift + c->unroll_shift);
  orc_loongarch_insn_emit_beqz (c, ORC_LOONG_T2, LABEL_LOOP_TAIL, ORC_LOONG_OFFS21);

  //unroll loop ORC_LOONG_T2 != 0
  orc_loongarch_compiler_add_label (c, LABEL_LOOP_INNER);
  orc_compiler_append_code (c, "/* LOOP SHIFT %d */\n", c->loop_shift);
  ui_max = 1 << c->unroll_shift;
  for (ui = 0; ui < ui_max; ui++) {
    c->offset = ui << c->loop_shift;
    orc_loongarch_compiler_emit_loop (c,
        (ui == ui_max - 1) << (c->loop_shift + c->unroll_shift));
  }
  c->offset = 0;
  orc_loongarch_insn_emit_addi_d (c, ORC_LOONG_T2, ORC_LOONG_T2, -1);
  orc_loongarch_insn_emit_bnez (c, ORC_LOONG_T2, LABEL_LOOP_INNER, ORC_LOONG_OFFS21);

  //tail loop ORC_LOONG_T3 != 0
  orc_loongarch_compiler_add_label (c, LABEL_LOOP_TAIL);
  for (l = c->loop_shift; l >= 0; l--) {
    c->loop_shift = l;
    orc_compiler_append_code (c, "/* LOOP SHIFT %d */\n", c->loop_shift);
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1 << c->loop_shift);
    orc_loongarch_insn_emit_blt (c, ORC_LOONG_T3, c->gp_tmpreg,
        LABEL_SETUP (c->loop_shift), ORC_LOONG_OFFS16);
    orc_loongarch_insn_emit_addi_d (c, ORC_LOONG_T3, ORC_LOONG_T3, -(1 << c->loop_shift));
    orc_loongarch_compiler_emit_loop (c, 1 << c->loop_shift);
    orc_loongarch_compiler_add_label (c, LABEL_SETUP (c->loop_shift));
  }

  if (c->program->is_2d) {
    orc_loongarch_compiler_add_strides (c);
    orc_loongarch_insn_emit_addi_d (c, ORC_LOONG_T1, ORC_LOONG_T1, -1);
    orc_loongarch_insn_emit_bnez (c, ORC_LOONG_T1, LABEL_LOOP_OUTER, ORC_LOONG_OFFS21);
  }

  orc_loongarch_compiler_add_label (c, LABEL_END);
}
