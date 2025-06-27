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

static OrcRiscvRegister
orc_riscv_rule_normalize_src_arg (OrcCompiler *c, OrcInstruction *insn,
    OrcRiscvRegister src, OrcRiscvRegister dest)
{
  if (c->target_flags & ORC_TARGET_FAST_DENORMAL) {
    return src;
  } else {
    const OrcRiscvRuleInfo *info = insn->rule->emit_user;
    orc_riscv_insn_emit_flush_subnormals (c, info->element_width, src, dest);
    return dest;
  }
}

#define NORMALIZE_SRC_ARG(compiler, insn, arg) \
  orc_riscv_rule_normalize_src_arg(compiler, insn, ORC_SRC_ARG (compiler, insn, arg), ORC_RISCV_V4 + arg)

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
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  /* FIXME: use vrsub instead */
  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, -1);
  orc_riscv_insn_emit_vmul_vx (c, tmp, c->gp_tmpreg, src);
  orc_riscv_insn_emit_vmax_vv (c, dest, tmp, src);
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

  /* FIXME: should be implemented by multiplication and shift */
  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 255);
  orc_riscv_insn_emit_vdivu_vx (c, dest, src, c->gp_tmpreg);
}

static void
orc_riscv_rule_signX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  /* FIXME: optimize this */

  orc_riscv_insn_emit_vmv_vv (c, tmp, src);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);

  orc_riscv_insn_emit_vmsgt_vx (c, ORC_RISCV_V0, tmp, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, 1);

  orc_riscv_insn_emit_vmslt_vx (c, ORC_RISCV_V0, tmp, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vadd_vim (c, dest, dest, -1);
}

static void
orc_riscv_rule_divluw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 255);
  orc_riscv_insn_emit_vand_vx (c, tmp, c->gp_tmpreg, src2);
  orc_riscv_insn_emit_vdivu_vv (c, dest, src1, tmp);
  orc_riscv_insn_emit_vminu_vx (c, dest, c->gp_tmpreg, dest);
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
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_vext_s2 (c, tmp1, src1);
  orc_riscv_insn_emit_vext_s2 (c, tmp2, src2);
  orc_riscv_insn_emit_vmul_vv (c, dest, tmp1, tmp2);
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
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_vext_z2 (c, tmp1, src1);
  orc_riscv_insn_emit_vext_z2 (c, tmp2, src2);
  orc_riscv_insn_emit_vmul_vv (c, dest, tmp1, tmp2);
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
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_vext_z4 (c, tmp1, src1);
  orc_riscv_insn_emit_vext_z4 (c, tmp2, src2);
  orc_riscv_insn_emit_vadd_vv (c, dest, dest, tmp1);
  orc_riscv_insn_emit_vadd_vv (c, dest, dest, tmp2);
  orc_riscv_insn_emit_vmin_vv (c, tmp1, tmp2, tmp1);
  orc_riscv_insn_emit_vsub_vv (c, dest, dest, tmp1);
  orc_riscv_insn_emit_vsub_vv (c, dest, dest, tmp1);
}

static void
orc_riscv_rule_mergebw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_vext_z2 (c, tmp1, src1);
  orc_riscv_insn_emit_vext_z2 (c, tmp2, src2);
  orc_riscv_insn_emit_vsll_vi (c, tmp2, tmp2, 8);
  orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
}

static void
orc_riscv_rule_mergewl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_vext_z2 (c, tmp1, src1);
  orc_riscv_insn_emit_vext_z2 (c, tmp2, src2);
  orc_riscv_insn_emit_vsll_vi (c, tmp2, tmp2, 16);
  orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
}

static void
orc_riscv_rule_mergelq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 32);

  orc_riscv_insn_emit_vext_z2 (c, tmp1, src1);
  orc_riscv_insn_emit_vext_z2 (c, tmp2, src2);
  orc_riscv_insn_emit_vsll_vx (c, tmp2, tmp2, c->gp_tmpreg);
  orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
}

static void
orc_riscv_rule_cmpgtsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vmslt_vv (c, ORC_RISCV_V0, src2, src1);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vneg (c, tmp, dest);
  orc_riscv_insn_emit_vadd_vvm (c, dest, dest, tmp);
}

static void
orc_riscv_rule_cmpeqX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vmseq_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vneg (c, tmp, dest);
  orc_riscv_insn_emit_vadd_vvm (c, dest, dest, tmp);
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

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 32);
  orc_riscv_insn_emit_vnsrl_vx (c, dest, src, c->gp_tmpreg);
}

static void
orc_riscv_rule_splatw3q (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 48);
  orc_riscv_insn_emit_vsrl_vx (c, tmp2, src, c->gp_tmpreg);
  orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp2, 16);
  orc_riscv_insn_emit_vor_vv (c, tmp1, tmp2, tmp1);
  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 32);

  orc_riscv_insn_emit_vsll_vx (c, tmp2, tmp1, c->gp_tmpreg);
  orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
}

static void
orc_riscv_rule_splatbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vext_z2 (c, tmp, src);
  orc_riscv_insn_emit_vsll_vi (c, dest, tmp, 8);
  orc_riscv_insn_emit_vor_vv (c, dest, dest, tmp);
}

static void
orc_riscv_rule_splatbl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vext_z4 (c, tmp, src);
  orc_riscv_insn_emit_vsll_vi (c, dest, tmp, 8);
  orc_riscv_insn_emit_vor_vv (c, dest, dest, tmp);
  orc_riscv_insn_emit_vsll_vi (c, tmp, dest, 16);
  orc_riscv_insn_emit_vor_vv (c, dest, dest, tmp);
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

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 32);
  orc_riscv_insn_emit_vnsrl_vx (c, dest1, src, c->gp_tmpreg);
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
    const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
    const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

    orc_riscv_insn_emit_vsrl_vi (c, tmp1, src, 8);
    orc_riscv_insn_emit_vsll_vi (c, tmp2, src, 8);
    orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
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
    const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
    const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

    /* FIXME: optimize this */

    orc_riscv_insn_emit_vsll_vi (c, tmp1, src, 24);
    orc_riscv_insn_emit_vsrl_vi (c, tmp2, src, 24);
    orc_riscv_insn_emit_vor_vv (c, tmp2, tmp2, tmp1);

    orc_riscv_insn_emit_vsrl_vi (c, tmp1, src, 8);
    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 24);
    orc_riscv_insn_emit_vsrl_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vor_vv (c, tmp2, tmp2, tmp1);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, src, 8);
    orc_riscv_insn_emit_vsrl_vi (c, tmp1, tmp1, 24);
    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vor_vv (c, dest, tmp1, tmp2);
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
    const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
    const OrcRiscvRegister tmp2 = ORC_RISCV_V2;
    const OrcRiscvRegister tmp3 = ORC_RISCV_V3;

    /* FIXME: optimize this */

    orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, -1);
    orc_riscv_insn_emit_vmv_vx (c, tmp1, c->gp_tmpreg);

    orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 56);
    orc_riscv_insn_emit_vsrl_vx (c, tmp1, tmp1, c->gp_tmpreg);

    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_vsll_vx (c, tmp3, tmp2, c->gp_tmpreg);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 40);
    orc_riscv_insn_emit_vsll_vx (c, tmp2, tmp2, c->gp_tmpreg);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_vsll_vi (c, tmp2, tmp2, 24);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_vsll_vi (c, tmp2, tmp2, 8);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_vsrl_vi (c, tmp2, tmp2, 8);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_vsrl_vi (c, tmp2, tmp2, 24);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 40);
    orc_riscv_insn_emit_vsrl_vx (c, tmp2, tmp2, c->gp_tmpreg);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 56);
    orc_riscv_insn_emit_vsrl_vx (c, tmp2, tmp2, c->gp_tmpreg);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vsll_vi (c, tmp1, tmp1, 8);
    orc_riscv_insn_emit_vand_vv (c, tmp2, src, tmp1);
    orc_riscv_insn_emit_vor_vv (c, tmp3, tmp3, tmp2);

    orc_riscv_insn_emit_vor_vv (c, dest, tmp3, tmp2);
  }
}

static void
orc_riscv_rule_swapwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_vsrl_vi (c, tmp1, src, 16);
  orc_riscv_insn_emit_vsll_vi (c, tmp2, src, 16);
  orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
}

static void
orc_riscv_rule_swalq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp1 = ORC_RISCV_V1;
  const OrcRiscvRegister tmp2 = ORC_RISCV_V2;

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 32);
  orc_riscv_insn_emit_vsrl_vx (c, tmp1, src, c->gp_tmpreg);
  orc_riscv_insn_emit_vsll_vx (c, tmp2, src, c->gp_tmpreg);
  orc_riscv_insn_emit_vor_vv (c, dest, tmp2, tmp1);
}

static void
orc_riscv_rule_andnX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, -1);
  orc_riscv_insn_emit_vxor_vx (c, tmp, c->gp_tmpreg, src1);
  orc_riscv_insn_emit_vand_vv (c, dest, tmp, src2);
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
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants");
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
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants");
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
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants");
      break;
  }
}

static void
orc_riscv_rule_convn (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnsra_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_convw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  /* Operand overlap restriction, see RVV Spec chapter 5.2 */
  orc_riscv_insn_emit_vmv_vv (c, tmp, src);
  orc_riscv_insn_emit_vext_s2 (c, dest, tmp);
}

static void
orc_riscv_rule_convuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  /* Operand overlap restriction, see RVV Spec chapter 5.2 */
  orc_riscv_insn_emit_vmv_vv (c, tmp, src);
  orc_riscv_insn_emit_vext_z2 (c, dest, tmp);
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
orc_riscv_rule_convsssn (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnclip_vi (c, dest, src, 0);
}

static void
orc_riscv_rule_convsusX (OrcCompiler *c, void *user, OrcInstruction *insn)
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
orc_riscv_rule_convuusn (OrcCompiler *c, void *user, OrcInstruction *insn)
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

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 0x7f);
  orc_riscv_insn_emit_vminu_vx (c, src, c->gp_tmpreg, src);
}

static void
orc_riscv_rule_convusslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 0x7fff);
  orc_riscv_insn_emit_vminu_vx (c, dest, c->gp_tmpreg, dest);
}

static void
orc_riscv_rule_convussql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vnclipu_vi (c, dest, src, 0);
  orc_riscv_insn_emit_load_immediate (c, c->gp_tmpreg, 0x7fffffff);
  orc_riscv_insn_emit_vminu_vx (c, dest, c->gp_tmpreg, dest);
}

static void
orc_riscv_rule_cmpeqF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vmfeq_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vneg (c, tmp, dest);
  orc_riscv_insn_emit_vadd_vvm (c, dest, dest, tmp);
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
  orc_riscv_rule_normalize_src_arg (c, insn, dest, dest);
}

static void
orc_riscv_rule_convld (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  /* Operand overlap restriction, see RVV Spec chapter 5.2 */
  orc_riscv_insn_emit_vmv_vv (c, tmp, src);
  orc_riscv_insn_emit_vxfncvt_vv (c, dest, tmp);
}

static void
orc_riscv_rule_convfd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  /* Operand overlap restriction, see RVV Spec chapter 5.2 */
  orc_riscv_insn_emit_vmv_vv (c, tmp, src);
  orc_riscv_insn_emit_vffwcvt_vv (c, dest, tmp);
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
  orc_riscv_rule_normalize_src_arg (c, insn, dest, dest);
}

static void
orc_riscv_rule_divF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = NORMALIZE_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = NORMALIZE_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);

  orc_riscv_insn_emit_vfdiv_vv (c, dest, src1, src2);
  orc_riscv_rule_normalize_src_arg (c, insn, dest, dest);
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
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vmflt_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vneg (c, tmp, dest);
  orc_riscv_insn_emit_vadd_vvm (c, dest, dest, tmp);
}

static void
orc_riscv_rule_cmpleF (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcRiscvRegister src1 = ORC_SRC_ARG (c, insn, 0);
  const OrcRiscvRegister src2 = ORC_SRC_ARG (c, insn, 1);
  const OrcRiscvRegister dest = ORC_DEST_ARG (c, insn, 0);
  const OrcRiscvRegister tmp = ORC_RISCV_V1;

  orc_riscv_insn_emit_vmfle_vv (c, ORC_RISCV_V0, src1, src2);
  orc_riscv_insn_emit_vmv_vx (c, dest, ORC_RISCV_ZERO);
  orc_riscv_insn_emit_vneg (c, tmp, dest);
  orc_riscv_insn_emit_vadd_vvm (c, dest, dest, tmp);
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
    const OrcRiscvRegister tmp = ORC_RISCV_V1;

    orc_riscv_insn_emit_vxor_vv (c, tmp, src1, src2);
    orc_riscv_insn_emit_vmaxf_vv (c, dest, src2, src1);
    orc_riscv_insn_emit_vxor_vv (c, dest, dest, tmp);
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
    const OrcRiscvRegister tmp = ORC_RISCV_V1;

    orc_riscv_insn_emit_vxor_vv (c, tmp, src1, src2);
    orc_riscv_insn_emit_vminf_vv (c, dest, src2, src1);
    orc_riscv_insn_emit_vxor_vv (c, dest, dest, tmp);
  }
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

  REG (loadoffb, loadoffX, NOT_APPLICABLE, FALSE);
  REG (loadoffw, loadoffX, NOT_APPLICABLE, FALSE);
  REG (loadoffl, loadoffX, NOT_APPLICABLE, FALSE);

  REG (storeb, storeb, NOT_APPLICABLE, FALSE);
  REG (storew, storew, NOT_APPLICABLE, FALSE);
  REG (storel, storel, NOT_APPLICABLE, FALSE);
  REG (storeq, storeq, NOT_APPLICABLE, FALSE);

  REG (copyb, copyX, 8, FALSE);
  REG (copyw, copyX, 16, FALSE);
  REG (copyl, copyX, 32, FALSE);
  REG (copyq, copyX, 64, FALSE);

  REG (addb, addX, 8, FALSE);
  REG (addw, addX, 16, FALSE);
  REG (addl, addX, 32, FALSE);
  REG (addq, addX, 64, FALSE);

  REG (addssb, addssX, 8, FALSE);
  REG (addssw, addssX, 16, FALSE);
  REG (addssl, addssX, 32, FALSE);

  REG (addusb, addusX, 8, FALSE);
  REG (addusw, addusX, 16, FALSE);
  REG (addusl, addusX, 32, FALSE);

  REG (subb, subX, 8, FALSE);
  REG (subw, subX, 16, FALSE);
  REG (subl, subX, 32, FALSE);
  REG (subq, subX, 64, FALSE);

  REG (subssb, subssX, 8, FALSE);
  REG (subssw, subssX, 16, FALSE);
  REG (subssl, subssX, 32, FALSE);

  REG (subusb, subusX, 8, FALSE);
  REG (subusw, subusX, 16, FALSE);
  REG (subusl, subusX, 32, FALSE);

  REG (andb, andX, 8, FALSE);
  REG (andw, andX, 16, FALSE);
  REG (andl, andX, 32, FALSE);
  REG (andq, andX, 64, FALSE);
  REG (andf, andX, 32, FALSE);

  REG (andnb, andnX, 8, FALSE);
  REG (andnw, andnX, 16, FALSE);
  REG (andnl, andnX, 32, FALSE);
  REG (andnq, andnX, 64, FALSE);

  REG (orb, orX, 8, FALSE);
  REG (orw, orX, 16, FALSE);
  REG (orl, orX, 32, FALSE);
  REG (orq, orX, 64, FALSE);
  REG (orf, orX, 32, FALSE);

  REG (xorb, xorX, 8, FALSE);
  REG (xorw, xorX, 16, FALSE);
  REG (xorl, xorX, 32, FALSE);
  REG (xorq, xorX, 64, FALSE);

  REG (shlb, shlX, 8, FALSE);
  REG (shlw, shlX, 16, FALSE);
  REG (shll, shlX, 32, FALSE);
  REG (shlq, shlX, 64, FALSE);

  REG (shrsb, shrsX, 8, FALSE);
  REG (shrsw, shrsX, 16, FALSE);
  REG (shrsl, shrsX, 32, FALSE);
  REG (shrsq, shrsX, 64, FALSE);

  REG (shrub, shruX, 8, FALSE);
  REG (shruw, shruX, 16, FALSE);
  REG (shrul, shruX, 32, FALSE);
  REG (shruq, shruX, 64, FALSE);

  REG (minsb, minX, 8, FALSE);
  REG (minsw, minX, 16, FALSE);
  REG (minsl, minX, 32, FALSE);

  REG (minub, minuX, 8, FALSE);
  REG (minuw, minuX, 16, FALSE);
  REG (minul, minuX, 32, FALSE);

  REG (maxsb, maxX, 8, FALSE);
  REG (maxsw, maxX, 16, FALSE);
  REG (maxsl, maxX, 32, FALSE);

  REG (maxub, maxuX, 8, FALSE);
  REG (maxuw, maxuX, 16, FALSE);
  REG (maxul, maxuX, 32, FALSE);

  REG (absb, absX, 8, FALSE);
  REG (absw, absX, 16, FALSE);
  REG (absl, absX, 32, FALSE);

  REG (avgsb, avgX, 8, FALSE);
  REG (avgsw, avgX, 16, FALSE);
  REG (avgsl, avgX, 32, FALSE);
  REG (avgub, avguX, 8, FALSE);
  REG (avguw, avguX, 16, FALSE);
  REG (avgul, avguX, 32, FALSE);

  REG (cmpgtsb, cmpgtsX, 8, TRUE);
  REG (cmpgtsw, cmpgtsX, 16, TRUE);
  REG (cmpgtsl, cmpgtsX, 32, TRUE);
  REG (cmpgtsq, cmpgtsX, 64, TRUE);
  REG (cmpeqb, cmpeqX, 8, TRUE);
  REG (cmpeqw, cmpeqX, 16, TRUE);
  REG (cmpeql, cmpeqX, 32, TRUE);
  REG (cmpeqq, cmpeqX, 64, TRUE);

  REG (signb, signX, 8, TRUE);
  REG (signw, signX, 16, TRUE);
  REG (signl, signX, 32, TRUE);

  REG (swapw, swapw, 16, FALSE);
  REG (swapl, swapl, 32, FALSE);
  REG (swapq, swapq, 64, FALSE);
  REG (swapwl, swapwl, 32, FALSE);
  REG (swaplq, swalq, 64, FALSE);

  REG (mullb, mullX, 8, FALSE);
  REG (mullw, mullX, 16, FALSE);
  REG (mulll, mullX, 32, FALSE);

  REG (mulhsb, mulhsX, 8, FALSE);
  REG (mulhsw, mulhsX, 16, FALSE);
  REG (mulhsl, mulhsX, 32, FALSE);

  REG (mulhub, mulhuX, 8, FALSE);
  REG (mulhuw, mulhuX, 16, FALSE);
  REG (mulhul, mulhuX, 32, FALSE);

  REG (mulsbw, mulsX, 16, FALSE);
  REG (mulswl, mulsX, 32, FALSE);
  REG (mulslq, mulsX, 64, FALSE);

  REG (mulubw, muluX, 16, FALSE);
  REG (muluwl, muluX, 32, FALSE);
  REG (mululq, muluX, 64, FALSE);

  REG (accw, accX, 16, FALSE);
  REG (accl, accX, 32, FALSE);
  REG (accsadubl, accsadubl, 32, TRUE);

  REG (splatw3q, splatw3q, 64, FALSE);
  REG (splatbw, splatbw, 16, FALSE);
  REG (splatbl, splatbl, 32, FALSE);
  REG (mergebw, mergebw, 16, FALSE);
  REG (mergewl, mergewl, 32, FALSE);
  REG (mergelq, mergelq, 64, FALSE);
  REG (select0wb, select0X, 8, FALSE);
  REG (select0lw, select0X, 16, FALSE);
  REG (select0ql, select0X, 32, FALSE);

  REG (select1wb, select1wb, 8, FALSE);
  REG (select1lw, select1lw, 16, FALSE);
  REG (select1ql, select1ql, 32, FALSE);

  REG (splitwb, splitwb, 8, FALSE);
  REG (splitlw, splitlw, 16, FALSE);
  REG (splitql, splitql, 32, FALSE);

  REG (convwb, convn, 8, FALSE);
  REG (convlw, convn, 16, FALSE);
  REG (convql, convn, 32, FALSE);

  REG (convsbw, convw, 16, FALSE);
  REG (convswl, convw, 32, FALSE);
  REG (convslq, convw, 64, FALSE);

  REG (convssswb, convsssn, 8, FALSE);
  REG (convssslw, convsssn, 16, FALSE);
  REG (convsssql, convsssn, 32, FALSE);

  REG (convubw, convuX, 16, FALSE);
  REG (convuwl, convuX, 32, FALSE);
  REG (convulq, convuX, 64, FALSE);

  REG (convuuswb, convuusn, 8, FALSE);
  REG (convuuslw, convuusn, 16, FALSE);
  REG (convuusql, convuusn, 32, FALSE);

  REG (convsuswb, convsusX, 16, FALSE);
  REG (convsusql, convsusX, 64, FALSE);
  REG (convsuslw, convsusX, 32, FALSE);

  REG (convusswb, convusswb, 8, FALSE);
  REG (convusslw, convusslw, 16, FALSE);
  REG (convussql, convussql, 32, FALSE);

  REG (convhlw, convhlw, 16, FALSE);
  REG (convhwb, convhwb, 8, FALSE);

  REG (div255w, div255w, 16, FALSE);
  REG (divluw, divluw, 16, FALSE);

  REG (cmpeqf, cmpeqF, 32, TRUE);
  REG (cmpeqd, cmpeqF, 64, TRUE);
  REG (cmpltf, cmpltF, 32, TRUE);
  REG (cmpltd, cmpltF, 64, TRUE);
  REG (cmplef, cmpleF, 32, TRUE);
  REG (cmpled, cmpleF, 64, TRUE);

  REG (convdl, convdl, 32, FALSE);
  REG (convdf, convdf, 32, FALSE);
  REG (convld, convld, 32, FALSE);
  REG (convfd, convfd, 32, FALSE);

  REG (addf, addF, 32, FALSE);
  REG (addd, addF, 64, FALSE);

  REG (subf, subF, 32, FALSE);
  REG (subd, subF, 64, FALSE);

  REG (mulf, mulF, 32, FALSE);
  REG (muld, mulF, 64, FALSE);

  REG (divf, divF, 32, FALSE);
  REG (divd, divF, 64, FALSE);

  REG (sqrtf, sqrtF, 32, FALSE);
  REG (sqrtd, sqrtF, 64, FALSE);

  REG (minf, minF, 32, FALSE);
  REG (mind, minF, 64, FALSE);
  REG (maxf, maxF, 32, FALSE);
  REG (maxd, maxF, 64, FALSE);
}
