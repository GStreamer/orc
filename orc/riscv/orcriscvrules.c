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

#define GET_TEMP_REGS(temp_regs, c, insn) \
  OrcRiscvRegister temp_regs[32] = {0}; \
  ORC_ASSERT(orc_riscv_compiler_get_temp_regs(c, insn, temp_regs));

static OrcRiscvRegister
orc_riscv_rule_normalize_src_arg (OrcCompiler *c, OrcInstruction *insn, int arg)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, arg);
  if (c->target_flags & ORC_TARGET_FAST_DENORMAL) {
    return src;
  } else {
    GET_TEMP_REGS (temp, c, insn);

    const OrcRiscvRuleInfo *info = insn->rule->emit_user;
    ORC_ASSERT (info->needs_mask_reg);

    int i = 0;
    while (temp[i])
      i++;
    i -= arg + 1;

    orc_riscv_insn_emit_flush_subnormals (c, info->element_width, src, temp[i],
        0);
    return temp[i];
  }
}

#define NORMALIZE_SRC_ARG(compiler, insn, arg) \
  orc_riscv_rule_normalize_src_arg(compiler, insn, arg)

static void
orc_riscv_rule_normalize_result (OrcCompiler *c, OrcInstruction *insn)
{
  if (!(c->target_flags & ORC_TARGET_FAST_DENORMAL)) {
    GET_TEMP_REGS (temp, c, insn);
    const OrcRiscvRuleInfo *info = insn->rule->emit_user;
    const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
    orc_riscv_insn_emit_flush_subnormals (c, info->element_width, dest, dest,
        *temp);
  }
}

static void
orc_riscv_rule_loadpX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRuleInfo *info = user;
  const OrcVariable *src = c->vars + insn->src_args[0];
  const OrcVariable *dest = c->vars + insn->dest_args[0];
  const OrcRiscvVtype vtype = {.vsew = info->element_width,.vlmul =
        c->loop_shift + orc_riscv_compiler_bytes_to_sew (c->max_var_size) -
        3,.vma = TRUE,.vta = TRUE
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
orc_riscv_rule_loadoffX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcVariable src1 = c->vars[insn->src_args[0]];
  const OrcVariable src2 = c->vars[insn->src_args[1]];
  const OrcVariable dest = c->vars[insn->dest_args[0]];

  /* FIXME this should be fixed at a higher level */
  if (src1.vartype != ORC_VAR_TYPE_SRC && src1.vartype != ORC_VAR_TYPE_DEST) {
    ORC_COMPILER_ERROR (c, "loadoffX used with non src/dest");
    return;
  }
  if (src2.vartype != ORC_VAR_TYPE_CONST) {
    ORC_COMPILER_ERROR (c, "loadoffX offset must be const");
    return;
  }

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg,
      src2.value.i * src1.size);
  orc_riscv_insn_emit_add (c, c->gp_tmpreg, c->gp_tmpreg, src1.ptr_register);

  switch (dest.size) {
    case 1:
      orc_riscv_insn_emit_vle8 (c, dest.alloc, c->gp_tmpreg);
      break;
    case 2:
      orc_riscv_insn_emit_vle16 (c, dest.alloc, c->gp_tmpreg);
      break;
    case 4:
      orc_riscv_insn_emit_vle32 (c, dest.alloc, c->gp_tmpreg);
      break;
    case 8:
      orc_riscv_insn_emit_vle64 (c, dest.alloc, c->gp_tmpreg);
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

static void
orc_riscv_rule_copyX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vadd_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_addX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, ORC_SRC_VAL (c, insn,
              1));
      orc_riscv_insn_emit_vadd_vx (c, dest, src1, c->gp_tmpreg);
      break;
    default:
      orc_riscv_insn_emit_vadd_vv (c, dest, src1, src2);
      break;
  }
}

static void
orc_riscv_rule_addssX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vsadd_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_addusX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vsaddu_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_subX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vsub_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_subssX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vssub_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_subusX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vssubu_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_andX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vand_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_minX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmin_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_minuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vminu_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_maxX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmax_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_maxuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmaxu_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_absX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vrsub_vx (c, *temp, src, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vmax_vv (c, dest, *temp, src);
}

static void
orc_riscv_rule_avgX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vaadd_vv (c, dest, src2, src1);
}

static void
orc_riscv_rule_avguX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vaaddu_vv (c, dest, src2, src1);
}

static void
orc_riscv_rule_div255w (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  const OrcRiscvRegister const0x8081 =
      orc_riscv_compiler_get_constant (c, 0x8081);
  orc_riscv_insn_emit_vmulhu_vx (c, dest, src, const0x8081);
  orc_riscv_insn_emit_vsrl_vi (c, dest, dest, 7);
}

static void
orc_riscv_rule_signX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  const OrcRiscvRegister const1s = orc_riscv_compiler_get_constant (c, -1ll);
  orc_riscv_insn_emit_vmax_vx (c, dest, const1s, src);

  const OrcRiscvRegister const1 = orc_riscv_compiler_get_constant (c, 1);
  orc_riscv_insn_emit_vmin_vx (c, dest, const1, dest);
}

static void
orc_riscv_rule_divluw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const255 = orc_riscv_compiler_get_constant (c, 255);

  orc_riscv_insn_emit_vand_vx (c, *temp, const255, src2);
  orc_riscv_insn_emit_vdivu_vv (c, dest, src1, *temp);
  orc_riscv_insn_emit_vminu_vx (c, dest, const255, dest);
}

static void
orc_riscv_rule_mullX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmul_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_mulsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_s2 (c, temp[0], src1);
  orc_riscv_insn_emit_vext_s2 (c, temp[1], src2);
  orc_riscv_insn_emit_vmul_vv (c, dest, temp[0], temp[1]);
}

static void
orc_riscv_rule_mulhsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmulh_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_mulhuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmulhu_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_muluX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_z2 (c, temp[0], src1);
  orc_riscv_insn_emit_vext_z2 (c, temp[1], src2);
  orc_riscv_insn_emit_vmul_vv (c, dest, temp[0], temp[1]);
}

static void
orc_riscv_rule_accX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vadd_vv (c, dest, src, dest);
}

static void
orc_riscv_rule_accsadubl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_z4 (c, temp[0], src1);
  orc_riscv_insn_emit_vext_z4 (c, temp[1], src2);
  orc_riscv_insn_emit_vadd_vv (c, dest, dest, temp[0]);
  orc_riscv_insn_emit_vadd_vv (c, dest, dest, temp[1]);
  orc_riscv_insn_emit_vmin_vv (c, temp[0], temp[1], temp[0]);
  orc_riscv_insn_emit_vsub_vv (c, dest, dest, temp[0]);
  orc_riscv_insn_emit_vsub_vv (c, dest, dest, temp[0]);
}

static void
orc_riscv_rule_mergebw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_z2 (c, temp[0], src1);
  orc_riscv_insn_emit_vext_z2 (c, temp[1], src2);
  orc_riscv_insn_emit_vsll_vi (c, temp[1], temp[1], 8);
  orc_riscv_insn_emit_vor_vv (c, dest, temp[1], temp[0]);
}

static void
orc_riscv_rule_mergewl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_z2 (c, temp[0], src1);
  orc_riscv_insn_emit_vext_z2 (c, temp[1], src2);
  orc_riscv_insn_emit_vsll_vi (c, temp[1], temp[1], 16);
  orc_riscv_insn_emit_vor_vv (c, dest, temp[1], temp[0]);
}

static void
orc_riscv_rule_mergelq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const32 = orc_riscv_compiler_get_constant (c, 32);

  orc_riscv_insn_emit_vext_z2 (c, temp[0], src1);
  orc_riscv_insn_emit_vext_z2 (c, temp[1], src2);
  orc_riscv_insn_emit_vsll_vx (c, temp[1], temp[1], const32);
  orc_riscv_insn_emit_vor_vv (c, dest, temp[1], temp[0]);
}

static void
orc_riscv_rule_cmpgtsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmslt_vv (c, ORC_RISCV_V0, src2, src1);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, -1);
}

static void
orc_riscv_rule_cmpeqX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmseq_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, -1);
}

static void
orc_riscv_rule_select0X (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsrl_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_select1wb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsrl_vi (c, dest, src, 8);
}

static void
orc_riscv_rule_select1lw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsrl_vi (c, dest, src, 16);
}

static void
orc_riscv_rule_select1ql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const32 = orc_riscv_compiler_get_constant (c, 32);

  orc_riscv_insn_emit_vnsrl_vx (c, dest, src, const32);
}

static void
orc_riscv_rule_splatw3q (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  const OrcRiscvRegister const48 = orc_riscv_compiler_get_constant (c, 48);
  orc_riscv_insn_emit_vsrl_vx (c, dest, src, const48);

  const OrcRiscvRegister templ =
      orc_riscv_compiler_get_constant (c, 0x0001000100010001ll);
  orc_riscv_insn_emit_vmul_vx (c, dest, templ, dest);
}

static void
orc_riscv_rule_splatbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  const OrcRiscvRegister templ =
      orc_riscv_compiler_get_constant (c, 0x0101010101010101ll);
  orc_riscv_insn_emit_vext_z2 (c, dest, src);
  orc_riscv_insn_emit_vmul_vx (c, dest, templ, dest);
}

static void
orc_riscv_rule_splatbl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  const OrcRiscvRegister templ =
      orc_riscv_compiler_get_constant (c, 0x0101010101010101ll);
  orc_riscv_insn_emit_vext_z4 (c, dest, src);
  orc_riscv_insn_emit_vmul_vx (c, dest, templ, dest);
}

static void
orc_riscv_rule_splitwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest1 = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister dest2 = ORC_DEST_ARG (c, insn, 1);

  orc_riscv_insn_emit_vnsrl_vi (c, dest1, src, 8);
  orc_riscv_insn_emit_vnsrl_vi (c, dest2, src, 0);
}

static void
orc_riscv_rule_splitlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest1 = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister dest2 = ORC_DEST_ARG (c, insn, 1);

  orc_riscv_insn_emit_vnsrl_vi (c, dest1, src, 16);
  orc_riscv_insn_emit_vnsrl_vi (c, dest2, src, 0);
}

static void
orc_riscv_rule_splitql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest1 = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister dest2 = ORC_DEST_ARG (c, insn, 1);
  const OrcRiscvRegister const32 = orc_riscv_compiler_get_constant (c, 32);

  orc_riscv_insn_emit_vnsrl_vx (c, dest1, src, const32);
  orc_riscv_insn_emit_vnsrl_vi (c, dest2, src, 0);
}

static void
orc_riscv_rule_swapw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  if (c->target_flags & ORC_TARGET_RISCV_ZVKB) {
    orc_riscv_insn_emit_vrev8 (c, dest, src);
  } else {
    GET_TEMP_REGS (temp, c, insn);

    orc_riscv_insn_emit_vsrl_vi (c, *temp, src, 8);
    orc_riscv_insn_emit_vsll_vi (c, dest, src, 8);
    orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);
  }
}

static void
orc_riscv_rule_swapl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  if (c->target_flags & ORC_TARGET_RISCV_ZVKB) {
    orc_riscv_insn_emit_vrev8 (c, dest, src);
  } else {
    GET_TEMP_REGS (temp, c, insn);

    const OrcRiscvRegister templ =
        orc_riscv_compiler_get_constant (c, 0x00FF00FF00FF00FFll);
    orc_riscv_insn_emit_vand_vx (c, *temp, templ, src);
    orc_riscv_insn_emit_vsll_vi (c, *temp, *temp, 8);
    orc_riscv_insn_emit_vsrl_vi (c, dest, src, 8);
    orc_riscv_insn_emit_vand_vx (c, dest, templ, dest);
    orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);

    orc_riscv_insn_emit_vsll_vi (c, *temp, dest, 16);
    orc_riscv_insn_emit_vsrl_vi (c, dest, dest, 16);
    orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);
  }
}

static void
orc_riscv_rule_swapq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  if (c->target_flags & ORC_TARGET_RISCV_ZVKB) {
    orc_riscv_insn_emit_vrev8 (c, dest, src);
  } else {
    GET_TEMP_REGS (temp, c, insn);

    const OrcRiscvRegister templ1 =
        orc_riscv_compiler_get_constant (c, 0x00FF00FF00FF00FFll);
    orc_riscv_insn_emit_vand_vx (c, *temp, templ1, src);
    orc_riscv_insn_emit_vsll_vi (c, *temp, *temp, 8);
    orc_riscv_insn_emit_vsrl_vi (c, dest, src, 8);
    orc_riscv_insn_emit_vand_vx (c, dest, templ1, dest);
    orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);

    const OrcRiscvRegister templ2 =
        orc_riscv_compiler_get_constant (c, 0x0000FFFF0000FFFFll);
    orc_riscv_insn_emit_vand_vx (c, *temp, templ2, dest);
    orc_riscv_insn_emit_vsll_vi (c, *temp, *temp, 16);
    orc_riscv_insn_emit_vsrl_vi (c, dest, dest, 16);
    orc_riscv_insn_emit_vand_vx (c, dest, templ2, dest);
    orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);

    const OrcRiscvRegister const32 = orc_riscv_compiler_get_constant (c, 32);
    orc_riscv_insn_emit_vsll_vx (c, *temp, dest, const32);
    orc_riscv_insn_emit_vsrl_vx (c, dest, dest, const32);
    orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);
  }
}

static void
orc_riscv_rule_swapwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vsrl_vi (c, *temp, src, 16);
  orc_riscv_insn_emit_vsll_vi (c, dest, src, 16);
  orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);
}

static void
orc_riscv_rule_swaplq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const32 = orc_riscv_compiler_get_constant (c, 32);

  orc_riscv_insn_emit_vsrl_vx (c, *temp, src, const32);
  orc_riscv_insn_emit_vsll_vx (c, dest, src, const32);
  orc_riscv_insn_emit_vor_vv (c, dest, dest, *temp);
}

static void
orc_riscv_rule_andnX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const1s = orc_riscv_compiler_get_constant (c, -1ll);

  orc_riscv_insn_emit_vxor_vx (c, *temp, const1s, src1);
  orc_riscv_insn_emit_vand_vv (c, dest, *temp, src2);
}

static void
orc_riscv_rule_orX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vor_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_xorX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vxor_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_shlX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, ORC_SRC_VAL (c, insn,
              1));
      orc_riscv_insn_emit_vsll_vx (c, dest, src1, c->gp_tmpreg);
      break;
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
      orc_riscv_insn_emit_vsll_vv (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants and params");
      break;
  }
}

static void
orc_riscv_rule_shrsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, ORC_SRC_VAL (c, insn,
              1));
      orc_riscv_insn_emit_vsra_vx (c, dest, src1, c->gp_tmpreg);
      break;
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
      orc_riscv_insn_emit_vsra_vv (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants and params");
      break;
  }
}

static void
orc_riscv_rule_shruX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, ORC_SRC_VAL (c, insn,
              1));
      orc_riscv_insn_emit_vsrl_vx (c, dest, src1, c->gp_tmpreg);
      break;
    case ORC_VAR_TYPE_PARAM:
    case ORC_VAR_TYPE_TEMP:
      orc_riscv_insn_emit_vsrl_vv (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants and params");
      break;
  }
}

static void
orc_riscv_rule_convN (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsra_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_convW (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_s2 (c, dest, src);
}

static void
orc_riscv_rule_convuW (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vext_z2 (c, dest, src);
}

static void
orc_riscv_rule_convhlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsra_vi (c, dest, src, 16);
}

static void
orc_riscv_rule_convhwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsra_vi (c, dest, src, 8);
}

static void
orc_riscv_rule_convsssN (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnclip_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_convsusN (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcRiscvRuleInfo *info = user;

  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmax_vx (c, dest, ORC_RISCV_ZERO, src);
  orc_riscv_insn_emit_vsetvli (c, ORC_RISCV_ZERO, ORC_RISCV_ZERO,
      orc_riscv_compiler_compute_vtype (c, info->element_width - 1, 0));

  orc_riscv_insn_emit_vnclipu_vi (c, dest, dest, 0);
  orc_riscv_insn_emit_vsetvli (c, ORC_RISCV_ZERO, ORC_RISCV_ZERO,
      orc_riscv_compiler_compute_vtype (c, info->element_width, 0));
}

static void
orc_riscv_rule_convuusN (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_convusswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const7f = orc_riscv_compiler_get_constant (c, 0x7f);

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
  orc_riscv_insn_emit_vminu_vx (c, dest, const7f, dest);
}

static void
orc_riscv_rule_convusslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const7fff =
      orc_riscv_compiler_get_constant (c, 0x7fff);

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
  orc_riscv_insn_emit_vminu_vx (c, dest, const7fff, dest);
}

static void
orc_riscv_rule_convussql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister const7fffffff =
      orc_riscv_compiler_get_constant (c, 0x7fffffff);

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
  orc_riscv_insn_emit_vminu_vx (c, dest, const7fffffff, dest);
}

static void
orc_riscv_rule_cmpeqF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmfeq_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, -1);
}

static void
orc_riscv_rule_convdl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfxncvt_vv (c, dest, src);
}

static void
orc_riscv_rule_convdf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vffncvt_vv (c, dest, src);
  orc_riscv_rule_normalize_result (c, insn);
}

static void
orc_riscv_rule_convlf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfxcvt_vv (c, dest, src);
}

static void
orc_riscv_rule_convld (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vxfncvt_vv (c, dest, src);
}

static void
orc_riscv_rule_convfd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vffwcvt_vv (c, dest, src);
}

static void
orc_riscv_rule_convfl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfcvt_rtz_vv (c, dest, src);
}

static void
orc_riscv_rule_addF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfadd_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_subF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfsub_vv (c, dest, src1, src2);
}

static void
orc_riscv_rule_mulF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfmul_vv (c, dest, src1, src2);
  orc_riscv_rule_normalize_result (c, insn);
}

static void
orc_riscv_rule_divF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfdiv_vv (c, dest, src1, src2);
  orc_riscv_rule_normalize_result (c, insn);
}

static void
orc_riscv_rule_sqrtF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfsqrt_vv (c, dest, src);
}

static void
orc_riscv_rule_cmpltF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmflt_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, -1);
}

static void
orc_riscv_rule_cmpleF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  GET_TEMP_REGS (temp, c, insn);
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vmfle_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, -1);
}

static void
orc_riscv_rule_minF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  if (c->target_flags & ORC_TARGET_FAST_NAN) {
    orc_riscv_insn_emit_vminf_vv (c, dest, src1, src2);
  } else {
    GET_TEMP_REGS (temp, c, insn);
    orc_riscv_insn_emit_vxor_vv (c, *temp, src1, src2);
    orc_riscv_insn_emit_vmaxf_vv (c, dest, src2, src1);
    orc_riscv_insn_emit_vxor_vv (c, dest, dest, *temp);
  }
}

static void
orc_riscv_rule_maxF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  if (c->target_flags & ORC_TARGET_FAST_NAN) {
    orc_riscv_insn_emit_vmaxf_vv (c, dest, src1, src2);
  } else {
    GET_TEMP_REGS (temp, c, insn);
    orc_riscv_insn_emit_vxor_vv (c, *temp, src1, src2);
    orc_riscv_insn_emit_vminf_vv (c, dest, src2, src1);
    orc_riscv_insn_emit_vxor_vv (c, dest, dest, *temp);
  }
}

#define REG(opcode, rule, sew, mask, temps, normals) \
  static OrcRiscvRuleInfo opcode##_info; \
  orc_rule_register (rule_set, #opcode, orc_riscv_rule_##rule, (void*)&opcode##_info); \
  opcode##_info.element_width = ORC_RISCV_SEW_##sew; \
  opcode##_info.needs_mask_reg = mask; \
  opcode##_info.temp_regs_needed = temps; \
  opcode##_info.normalized_inputs = normals;

#define REG_CONSTS(opcode, ...) \
  static orc_uint64 opcode##_consts[] = {__VA_ARGS__ +0, 0}; \
  opcode##_info.constants = opcode##_consts;

void
orc_riscv_rules_init (OrcTarget *target)
{
  OrcRuleSet *rule_set =
      orc_rule_set_new (orc_opcode_set_get ("sys"), target, 0);

  REG (loadpb, loadpX, 8, FALSE, 0, 0);
  REG (loadpw, loadpX, 16, FALSE, 0, 0);
  REG (loadpl, loadpX, 32, FALSE, 0, 0);
  REG (loadpq, loadpX, 64, FALSE, 0, 0);

  REG (loadb, loadX, NOT_APPLICABLE, FALSE, 0, 0);
  REG (loadw, loadX, NOT_APPLICABLE, FALSE, 0, 0);
  REG (loadl, loadX, NOT_APPLICABLE, FALSE, 0, 0);
  REG (loadq, loadX, NOT_APPLICABLE, FALSE, 0, 0);

  REG (loadoffb, loadoffX, NOT_APPLICABLE, FALSE, 0, 0);
  REG (loadoffw, loadoffX, NOT_APPLICABLE, FALSE, 0, 0);
  REG (loadoffl, loadoffX, NOT_APPLICABLE, FALSE, 0, 0);

  REG (storeb, storeb, NOT_APPLICABLE, FALSE, 0, 0);
  REG (storew, storew, NOT_APPLICABLE, FALSE, 0, 0);
  REG (storel, storel, NOT_APPLICABLE, FALSE, 0, 0);
  REG (storeq, storeq, NOT_APPLICABLE, FALSE, 0, 0);

  REG (copyb, copyX, 8, FALSE, 0, 0);
  REG (copyw, copyX, 16, FALSE, 0, 0);
  REG (copyl, copyX, 32, FALSE, 0, 0);
  REG (copyq, copyX, 64, FALSE, 0, 0);

  REG (addb, addX, 8, FALSE, 0, 0);
  REG (addw, addX, 16, FALSE, 0, 0);
  REG (addl, addX, 32, FALSE, 0, 0);
  REG (addq, addX, 64, FALSE, 0, 0);

  REG (addssb, addssX, 8, FALSE, 0, 0);
  REG (addssw, addssX, 16, FALSE, 0, 0);
  REG (addssl, addssX, 32, FALSE, 0, 0);

  REG (addusb, addusX, 8, FALSE, 0, 0);
  REG (addusw, addusX, 16, FALSE, 0, 0);
  REG (addusl, addusX, 32, FALSE, 0, 0);

  REG (subb, subX, 8, FALSE, 0, 0);
  REG (subw, subX, 16, FALSE, 0, 0);
  REG (subl, subX, 32, FALSE, 0, 0);
  REG (subq, subX, 64, FALSE, 0, 0);

  REG (subssb, subssX, 8, FALSE, 0, 0);
  REG (subssw, subssX, 16, FALSE, 0, 0);
  REG (subssl, subssX, 32, FALSE, 0, 0);

  REG (subusb, subusX, 8, FALSE, 0, 0);
  REG (subusw, subusX, 16, FALSE, 0, 0);
  REG (subusl, subusX, 32, FALSE, 0, 0);

  REG (andb, andX, 8, FALSE, 0, 0);
  REG (andw, andX, 16, FALSE, 0, 0);
  REG (andl, andX, 32, FALSE, 0, 0);
  REG (andq, andX, 64, FALSE, 0, 0);
  REG (andf, andX, 32, FALSE, 0, 0);

  REG (andnb, andnX, 8, FALSE, 1, 0);
  REG_CONSTS (andnb, -1ll);
  REG (andnw, andnX, 16, FALSE, 1, 0);
  REG (andnl, andnX, 32, FALSE, 1, 0);
  REG (andnq, andnX, 64, FALSE, 1, 0);

  REG (orb, orX, 8, FALSE, 0, 0);
  REG (orw, orX, 16, FALSE, 0, 0);
  REG (orl, orX, 32, FALSE, 0, 0);
  REG (orq, orX, 64, FALSE, 0, 0);
  REG (orf, orX, 32, FALSE, 0, 0);

  REG (xorb, xorX, 8, FALSE, 0, 0);
  REG (xorw, xorX, 16, FALSE, 0, 0);
  REG (xorl, xorX, 32, FALSE, 0, 0);
  REG (xorq, xorX, 64, FALSE, 0, 0);

  REG (shlb, shlX, 8, FALSE, 0, 0);
  REG (shlw, shlX, 16, FALSE, 0, 0);
  REG (shll, shlX, 32, FALSE, 0, 0);
  REG (shlq, shlX, 64, FALSE, 0, 0);

  REG (shrsb, shrsX, 8, FALSE, 0, 0);
  REG (shrsw, shrsX, 16, FALSE, 0, 0);
  REG (shrsl, shrsX, 32, FALSE, 0, 0);
  REG (shrsq, shrsX, 64, FALSE, 0, 0);

  REG (shrub, shruX, 8, FALSE, 0, 0);
  REG (shruw, shruX, 16, FALSE, 0, 0);
  REG (shrul, shruX, 32, FALSE, 0, 0);
  REG (shruq, shruX, 64, FALSE, 0, 0);

  REG (minsb, minX, 8, FALSE, 0, 0);
  REG (minsw, minX, 16, FALSE, 0, 0);
  REG (minsl, minX, 32, FALSE, 0, 0);

  REG (minub, minuX, 8, FALSE, 0, 0);
  REG (minuw, minuX, 16, FALSE, 0, 0);
  REG (minul, minuX, 32, FALSE, 0, 0);

  REG (maxsb, maxX, 8, FALSE, 0, 0);
  REG (maxsw, maxX, 16, FALSE, 0, 0);
  REG (maxsl, maxX, 32, FALSE, 0, 0);

  REG (maxub, maxuX, 8, FALSE, 0, 0);
  REG (maxuw, maxuX, 16, FALSE, 0, 0);
  REG (maxul, maxuX, 32, FALSE, 0, 0);

  REG (absb, absX, 8, FALSE, 1, 0);
  REG (absw, absX, 16, FALSE, 1, 0);
  REG (absl, absX, 32, FALSE, 1, 0);

  REG (avgsb, avgX, 8, FALSE, 0, 0);
  REG (avgsw, avgX, 16, FALSE, 0, 0);
  REG (avgsl, avgX, 32, FALSE, 0, 0);
  REG (avgub, avguX, 8, FALSE, 0, 0);
  REG (avguw, avguX, 16, FALSE, 0, 0);
  REG (avgul, avguX, 32, FALSE, 0, 0);

  REG (cmpgtsb, cmpgtsX, 8, TRUE, 1, 0);
  REG (cmpgtsw, cmpgtsX, 16, TRUE, 1, 0);
  REG (cmpgtsl, cmpgtsX, 32, TRUE, 1, 0);
  REG (cmpgtsq, cmpgtsX, 64, TRUE, 1, 0);
  REG (cmpeqb, cmpeqX, 8, TRUE, 1, 0);
  REG (cmpeqw, cmpeqX, 16, TRUE, 1, 0);
  REG (cmpeql, cmpeqX, 32, TRUE, 1, 0);
  REG (cmpeqq, cmpeqX, 64, TRUE, 1, 0);

  REG (signb, signX, 8, TRUE, 0, 0);
  REG_CONSTS (signb, 1, -1ll);
  REG (signw, signX, 16, TRUE, 0, 0);
  REG_CONSTS (signw, 1, -1ll);
  REG (signl, signX, 32, TRUE, 0, 0);
  REG_CONSTS (signl, 1, -1ll);

  REG (swapw, swapw, 16, FALSE, 1, 0);
  REG (swapl, swapl, 32, FALSE, 1, 0);
  REG_CONSTS (swapl, 0x00FF00FF00FF00FFll);
  REG (swapq, swapq, 64, FALSE, 1, 0);
  REG_CONSTS (swapq, 0x00FF00FF00FF00FFll, 0x0000FFFF0000FFFFll, 32);
  REG (swapwl, swapwl, 32, FALSE, 1, 0);
  REG (swaplq, swaplq, 64, FALSE, 1, 0);
  REG_CONSTS (swaplq, 32);

  REG (mullb, mullX, 8, FALSE, 0, 0);
  REG (mullw, mullX, 16, FALSE, 0, 0);
  REG (mulll, mullX, 32, FALSE, 0, 0);

  REG (mulhsb, mulhsX, 8, FALSE, 0, 0);
  REG (mulhsw, mulhsX, 16, FALSE, 0, 0);
  REG (mulhsl, mulhsX, 32, FALSE, 0, 0);

  REG (mulhub, mulhuX, 8, FALSE, 0, 0);
  REG (mulhuw, mulhuX, 16, FALSE, 0, 0);
  REG (mulhul, mulhuX, 32, FALSE, 0, 0);

  REG (mulsbw, mulsX, 16, FALSE, 2, 0);
  REG (mulswl, mulsX, 32, FALSE, 2, 0);
  REG (mulslq, mulsX, 64, FALSE, 2, 0);

  REG (mulubw, muluX, 16, FALSE, 2, 0);
  REG (muluwl, muluX, 32, FALSE, 2, 0);
  REG (mululq, muluX, 64, FALSE, 2, 0);

  REG (accw, accX, 16, FALSE, 0, 0);
  REG (accl, accX, 32, FALSE, 0, 0);
  REG (accsadubl, accsadubl, 32, TRUE, 2, 0);

  REG (splatw3q, splatw3q, 64, FALSE, 0, 0);
  REG_CONSTS (splatw3q, 48, 0x0001000100010001ll);
  REG (splatbw, splatbw, 16, FALSE, 0, 0);
  REG_CONSTS (splatbw, 0x0101010101010101ll);
  REG (splatbl, splatbl, 32, FALSE, 0, 0);
  REG_CONSTS (splatbl, 0x0101010101010101ll);
  REG (mergebw, mergebw, 16, FALSE, 2, 0);
  REG (mergewl, mergewl, 32, FALSE, 2, 0);
  REG (mergelq, mergelq, 64, FALSE, 2, 0);
  REG_CONSTS (mergelq, 32);
  REG (select0wb, select0X, 8, FALSE, 0, 0);
  REG (select0lw, select0X, 16, FALSE, 0, 0);
  REG (select0ql, select0X, 32, FALSE, 0, 0);

  REG (select1wb, select1wb, 8, FALSE, 0, 0);
  REG (select1lw, select1lw, 16, FALSE, 0, 0);
  REG (select1ql, select1ql, 32, FALSE, 0, 0);
  REG_CONSTS (select1ql, 32);

  REG (splitwb, splitwb, 8, FALSE, 0, 0);
  REG (splitlw, splitlw, 16, FALSE, 0, 0);
  REG (splitql, splitql, 32, FALSE, 0, 0);
  REG_CONSTS (splitql, 32);

  REG (convwb, convN, 8, FALSE, 0, 0);
  REG (convlw, convN, 16, FALSE, 0, 0);
  REG (convql, convN, 32, FALSE, 0, 0);

  REG (convsbw, convW, 16, FALSE, 0, 0);
  REG (convswl, convW, 32, FALSE, 0, 0);
  REG (convslq, convW, 64, FALSE, 0, 0);

  REG (convssswb, convsssN, 8, FALSE, 0, 0);
  REG (convssslw, convsssN, 16, FALSE, 0, 0);
  REG (convsssql, convsssN, 32, FALSE, 0, 0);

  REG (convubw, convuW, 16, FALSE, 0, 0);
  REG (convuwl, convuW, 32, FALSE, 0, 0);
  REG (convulq, convuW, 64, FALSE, 0, 0);

  REG (convuuswb, convuusN, 8, FALSE, 0, 0);
  REG (convuuslw, convuusN, 16, FALSE, 0, 0);
  REG (convuusql, convuusN, 32, FALSE, 0, 0);

  REG (convsuswb, convsusN, 16, FALSE, 0, 0);
  REG (convsusql, convsusN, 64, FALSE, 0, 0);
  REG (convsuslw, convsusN, 32, FALSE, 0, 0);

  REG (convusswb, convusswb, 8, FALSE, 0, 0);
  REG_CONSTS (convusswb, 0x7f);
  REG (convusslw, convusslw, 16, FALSE, 0, 0);
  REG_CONSTS (convusslw, 0x7fff);
  REG (convussql, convussql, 32, FALSE, 0, 0);
  REG_CONSTS (convussql, 0x7fffffff);

  REG (convhlw, convhlw, 16, FALSE, 0, 0);
  REG (convhwb, convhwb, 8, FALSE, 0, 0);

  REG (div255w, div255w, 16, FALSE, 0, 0);
  REG_CONSTS (div255w, 0x8081);
  REG (divluw, divluw, 16, FALSE, 1, 0);
  REG_CONSTS (divluw, 255);

  REG (cmpeqf, cmpeqF, 32, TRUE, 1, 0);
  REG (cmpeqd, cmpeqF, 64, TRUE, 1, 0);
  REG (cmpltf, cmpltF, 32, TRUE, 1, 0);
  REG (cmpltd, cmpltF, 64, TRUE, 1, 0);
  REG (cmplef, cmpleF, 32, TRUE, 1, 0);
  REG (cmpled, cmpleF, 64, TRUE, 1, 0);

  REG (convdl, convdl, 32, FALSE, 0, 0);
  REG (convdf, convdf, 32, TRUE, 1, 0);
  REG (convlf, convlf, 32, FALSE, 0, 0);
  REG (convld, convld, 32, FALSE, 0, 0);
  REG (convfd, convfd, 32, TRUE, 1, 0);
  REG (convfl, convfl, 32, FALSE, 0, 0);

  REG (addf, addF, 32, TRUE, 0, 2);
  REG (addd, addF, 64, TRUE, 0, 2);

  REG (subf, subF, 32, TRUE, 0, 2);
  REG (subd, subF, 64, TRUE, 0, 2);

  REG (mulf, mulF, 32, TRUE, 0, 2);
  REG (muld, mulF, 64, TRUE, 0, 2);

  REG (divf, divF, 32, TRUE, 0, 2);
  REG (divd, divF, 64, TRUE, 0, 2);

  REG (sqrtf, sqrtF, 32, TRUE, 0, 1);
  REG (sqrtd, sqrtF, 64, TRUE, 0, 1);

  REG (minf, minF, 32, TRUE, 1, 2);
  REG (mind, minF, 64, TRUE, 1, 2);
  REG (maxf, maxF, 32, TRUE, 1, 2);
  REG (maxd, maxF, 64, TRUE, 1, 2);
}
