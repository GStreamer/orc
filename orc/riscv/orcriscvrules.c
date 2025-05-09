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
#include <orc/orcprogram.h>
#include <orc/orcrule.h>
#include <orc/orcutils-private.h>
#include <orc/orcutils.h>

#include <orc/orcinternal.h>

#include <orc/riscv/orcriscv-internal.h>
#include <orc/riscv/orcriscv.h>
#include <orc/riscv/orcriscvinsn.h>

static void
orc_riscv_rule_loadpX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRuleInfo *info = user;
  const OrcVariable *src = c->vars + insn->src_args[0];
  const OrcVariable *dest = c->vars + insn->dest_args[0];
  const OrcRiscvVtype vtype = {.vsew = info->element_width,.vlmul =
        ORC_RISCV_LMUL_1,.vma = TRUE,.vta = TRUE
  };

  orc_riscv_insn_emit_vsetvli (c, c->gp_tmpreg, ORC_RISCV_ZERO, vtype);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, src->value.i);
  } else {
    const int offset =
        ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]]);
    switch (vtype.vsew) {
      case ORC_RISCV_SEW_8:
        orc_riscv_insn_emit_lb (c, c->gp_tmpreg, c->exec_reg, offset);
        break;
      case ORC_RISCV_SEW_16:
        orc_riscv_insn_emit_lh (c, c->gp_tmpreg, c->exec_reg, offset);
        break;
      case ORC_RISCV_SEW_32:
      case ORC_RISCV_SEW_64:
        orc_riscv_insn_emit_lw (c, c->gp_tmpreg, c->exec_reg, offset);
        break;
      default:
        ORC_PROGRAM_ERROR (c, "unreachable");
        abort ();
    }
  }

  orc_riscv_insn_emit_vmv_vx (c, dest->alloc, c->gp_tmpreg);

  if (vtype.vsew == ORC_RISCV_SEW_64) {
    const int offset = ORC_STRUCT_OFFSET (OrcExecutor,
        params[insn->src_args[0] + ORC_N_PARAMS]);
    orc_riscv_insn_emit_lw (c, c->gp_tmpreg, c->exec_reg, offset);
    orc_riscv_insn_emit_slli (c, c->gp_tmpreg, c->gp_tmpreg, 32);
    orc_riscv_insn_emit_vor_vx (c, dest->alloc, c->gp_tmpreg, dest->alloc);
  }
}


static void
orc_riscv_rule_loadX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcVariable src = c->vars[insn->src_args[0]];
  const OrcVariable dest = c->vars[insn->dest_args[0]];

  /* FIXME this should be fixed at a higher level */
  if (src.vartype != ORC_VAR_TYPE_SRC && src.vartype != ORC_VAR_TYPE_DEST) {
    ORC_COMPILER_ERROR (c, "loadX used with non src/dest");
    return;
  }

  switch (dest.size) {
    case 1:
      orc_riscv_insn_emit_vle8 (c, dest.alloc, src.ptr_register);
      break;
    case 2:
      orc_riscv_insn_emit_vle16 (c, dest.alloc, src.ptr_register);
      break;
    case 4:
      orc_riscv_insn_emit_vle32 (c, dest.alloc, src.ptr_register);
      break;
    case 8:
      orc_riscv_insn_emit_vle64 (c, dest.alloc, src.ptr_register);
      break;
    default:
      ORC_COMPILER_ERROR (c, "loadX used with wrong size %d", dest.size);
      break;
  }
}

static void
orc_riscv_rule_storeb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = c->vars[insn->src_args[0]].alloc;
  const OrcRiscvRegister dest = c->vars[insn->dest_args[0]].ptr_register;

  orc_riscv_insn_emit_vse8 (c, src, dest);
}

static void
orc_riscv_rule_storew (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = c->vars[insn->src_args[0]].alloc;
  const OrcRiscvRegister dest = c->vars[insn->dest_args[0]].ptr_register;

  orc_riscv_insn_emit_vse16 (c, src, dest);
}

static void
orc_riscv_rule_storel (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = c->vars[insn->src_args[0]].alloc;
  const OrcRiscvRegister dest = c->vars[insn->dest_args[0]].ptr_register;

  orc_riscv_insn_emit_vse32 (c, src, dest);
}

static void
orc_riscv_rule_storeq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = c->vars[insn->src_args[0]].alloc;
  const OrcRiscvRegister dest = c->vars[insn->dest_args[0]].ptr_register;

  orc_riscv_insn_emit_vse64 (c, src, dest);
}

#define REG(opcode, rule, sew, mask) \
  static OrcRiscvRuleInfo opcode##_info; \
  orc_rule_register (rule_set, #opcode, orc_riscv_rule_##rule, (void*)&opcode##_info); \
  opcode##_info.element_width = ORC_RISCV_SEW_##sew; \
  opcode##_info.needs_mask_reg = mask;


void
orc_riscv_rules_init (OrcTarget *target)
{
  OrcRuleSet *rule_set =
      orc_rule_set_new (orc_opcode_set_get ("sys"), target, 0);

  REG (loadpb, loadpX, 8, FALSE);
  REG (loadpw, loadpX, 16, FALSE);
  REG (loadpl, loadpX, 32, FALSE);
  REG (loadpq, loadpX, 64, FALSE);

  REG (loadb, loadX, NOT_APPLICABLE, FALSE);
  REG (loadw, loadX, NOT_APPLICABLE, FALSE);
  REG (loadl, loadX, NOT_APPLICABLE, FALSE);
  REG (loadq, loadX, NOT_APPLICABLE, FALSE);

  REG (storeb, storeb, NOT_APPLICABLE, FALSE);
  REG (storew, storew, NOT_APPLICABLE, FALSE);
  REG (storel, storel, NOT_APPLICABLE, FALSE);
  REG (storeq, storeq, NOT_APPLICABLE, FALSE);
}
