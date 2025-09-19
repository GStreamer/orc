/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: hecai yuan, yuanhecai@loongson.cn
  Author: jinbo, jinbo@loongson.cn

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

#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcrule.h>
#include <orc/orcutils.h>
#include <orc/loongarch/orcloongarchinsn.h>
#include <orc/loongarch/orclsxinsn.h>
#include <orc/orcinternal.h>

#define NORMALIZE_SRC_ARG(c, insn, arg, element_width) \
  orc_lsx_insn_emit_normalize(c, ORC_SRC_ARG (c, insn, arg), ORC_LOONG_VR10 + arg, element_width)

static void
orc_lsx_rule_loadpX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcVariable *src = c->vars + insn->src_args[0];
  OrcVariable *dest = c->vars + insn->dest_args[0];
  int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_load_imm (c, c->gp_tmpreg, src->value.i);

    if (size == 1) {
      orc_lsx_insn_emit_vreplgr2vrb (c, dest->alloc, c->gp_tmpreg);
    } else if (size == 2) {
      orc_lsx_insn_emit_vreplgr2vrh (c, dest->alloc, c->gp_tmpreg);
    } else if (size == 4) {
      orc_lsx_insn_emit_vreplgr2vrw (c, dest->alloc, c->gp_tmpreg);
    } else if (size == 8) {
      orc_lsx_insn_emit_vreplgr2vrd (c, dest->alloc, c->gp_tmpreg);
    } else {
      ORC_PROGRAM_ERROR(c, "unimplemented");
    }
  } else {
    int offset = ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]]);
    if (size == 1) {
      orc_lsx_insn_emit_vldreplb (c, dest->alloc, c->exec_reg, offset);
    } else if (size == 2) {
      orc_lsx_insn_emit_vldreplh (c, dest->alloc, c->exec_reg, offset);
    } else if (size == 4) {
      orc_lsx_insn_emit_vldreplw (c, dest->alloc, c->exec_reg, offset);
    } else if (size == 8) {
      const OrcLoongRegister t0 = ORC_LOONG_VR4;
      int offset1 = ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]+(ORC_N_PARAMS)]);
      orc_lsx_insn_emit_vldreplw (c, t0, c->exec_reg, offset);
      orc_lsx_insn_emit_vldreplw (c, dest->alloc, c->exec_reg, offset1);
      orc_lsx_insn_emit_vilvlw (c, dest->alloc, dest->alloc, t0);
    } else {
      ORC_PROGRAM_ERROR(c, "unimplemented");
    }
  }
}

static void
orc_lsx_rule_loadX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcVariable *src = c->vars + insn->src_args[0];
  OrcVariable *dest = c->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;

  offset = c->offset * src->size;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_loongarch_insn_emit_ld_d (c, c->gp_tmpreg, c->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
    ptr_reg = c->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }
  switch (src->size << c->loop_shift) {
    case 1:
      orc_lsx_insn_emit_vldreplb (c, dest->alloc, ptr_reg, offset);
      break;
    case 2:
      orc_lsx_insn_emit_vldreplh (c, dest->alloc, ptr_reg, offset);
      break;
    case 4:
      orc_lsx_insn_emit_vldreplw (c, dest->alloc, ptr_reg, offset);
      break;
    case 8:
      orc_lsx_insn_emit_vldrepld (c, dest->alloc, ptr_reg, offset);
      break;
    case 16:
      orc_lsx_insn_emit_vld (c, dest->alloc, ptr_reg, offset);
      break;
    default:
      orc_compiler_error (c, "bad load size %d",
          src->size << c->loop_shift);
      break;
  }
  src->update_type = 2;
}

static void
orc_lsx_rule_loadupdb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcVariable *src = c->vars + insn->src_args[0];
  OrcVariable *dest = c->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;

  offset = (c->offset * src->size)>>1;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_loongarch_insn_emit_ld_d (c, c->gp_tmpreg, c->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
    ptr_reg = c->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }

  switch (src->size << c->loop_shift) {
    case 1:
    case 2:
      orc_lsx_insn_emit_vldreplb (c, dest->alloc, ptr_reg, offset);
      break;
    case 4:
      orc_lsx_insn_emit_vldreplh (c, dest->alloc, ptr_reg, offset);
      break;
    case 8:
      orc_lsx_insn_emit_vldreplw (c, dest->alloc, ptr_reg, offset);
      break;
    case 16:
      orc_lsx_insn_emit_vldrepld (c, dest->alloc, ptr_reg, offset);
      break;
    default:
      orc_compiler_error(c,"bad load size %d", src->size << c->loop_shift);
      break;
  }

  switch (src->size) {
    case 1:
      orc_lsx_insn_emit_vilvlb (c, dest->alloc, dest->alloc, dest->alloc);
      break;
    case 2:
      orc_lsx_insn_emit_vilvlh (c, dest->alloc, dest->alloc, dest->alloc);
      break;
    case 4:
      orc_lsx_insn_emit_vilvlw (c, dest->alloc, dest->alloc, dest->alloc);
      break;
  }
  src->update_type = 1;
}

static void
orc_lsx_rule_loadoffX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcVariable *src = c->vars + insn->src_args[0];
  OrcVariable *dest = c->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;

  if (c->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_CONST) {
    orc_compiler_error (c, "code generation rule for %s only works with constant offset",
        insn->opcode->name);
    return;
  }

  offset = (c->offset + c->vars[insn->src_args[1]].value.i) * src->size;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_loongarch_insn_emit_ld_d (c, c->gp_tmpreg, c->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
    ptr_reg = c->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }
  switch (src->size << c->loop_shift) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
      orc_lsx_insn_emit_vld (c, dest->alloc, ptr_reg, offset);
      break;
    default:
      orc_compiler_error (c,"bad load size %d",
          src->size << c->loop_shift);
      break;
  }
  src->update_type = 2;
}

static void
orc_lsx_rule_storeX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcVariable *src = c->vars + insn->src_args[0];
  OrcVariable *dest = c->vars + insn->dest_args[0];
  int offset;
  int ptr_reg;

  offset = c->offset * dest->size;
  if (dest->ptr_register == 0) {
    orc_loongarch_insn_emit_ld_d (c, c->gp_tmpreg,
        c->exec_reg, dest->ptr_offset);
    ptr_reg = c->gp_tmpreg;
  } else {
    ptr_reg = dest->ptr_register;
  }
  switch (dest->size << c->loop_shift) {
    case 1:
      orc_lsx_insn_emit_vstelmb (c, src->alloc, ptr_reg, offset, 0);
      break;
    case 2:
      orc_lsx_insn_emit_vstelmh (c, src->alloc, ptr_reg, offset, 0);
      break;
    case 4:
      orc_lsx_insn_emit_vstelmw (c, src->alloc, ptr_reg, offset, 0);
      break;
    case 8:
      orc_lsx_insn_emit_vstelmd (c, src->alloc, ptr_reg, offset, 0);
      break;
    case 16:
      orc_lsx_insn_emit_vst (c, src->alloc, ptr_reg, offset);
      break;
    default:
      orc_compiler_error (c, "bad size");
      break;
  }
}

static void
orc_lsx_rule_copyX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vorv (c, dest, src, src);
}

static void
orc_lsx_rule_addX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  int size = ORC_PTR_TO_INT (user);

  if (ORC_SRC_TYPE (c, insn, 1) == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_addi_d(c, c->gp_tmpreg, ORC_LOONG_ZERO, ORC_SRC_VAL (c, insn, 1));
    switch (size) {
      case 1:
        orc_lsx_insn_emit_vreplgr2vrb (c, dest, c->gp_tmpreg);
        orc_lsx_insn_emit_vaddb (c, dest, src1, dest);
        break;
      case 2:
        orc_lsx_insn_emit_vreplgr2vrh (c, dest, c->gp_tmpreg);
        orc_lsx_insn_emit_vaddh (c, dest, src1, dest);
        break;
      case 4:
        orc_lsx_insn_emit_vreplgr2vrw (c, dest, c->gp_tmpreg);
        orc_lsx_insn_emit_vaddw (c, dest, src1, dest);
        break;
      case 8:
        orc_lsx_insn_emit_vreplgr2vrd (c, dest, c->gp_tmpreg);
        orc_lsx_insn_emit_vaddd (c, dest, src1, dest);
        break;
      }
  } else {
    switch (size) {
      case 1:
        orc_lsx_insn_emit_vaddb (c, dest, src1, src2);
        break;
      case 2:
        orc_lsx_insn_emit_vaddh (c, dest, src1, src2);
        break;
      case 4:
        orc_lsx_insn_emit_vaddw (c, dest, src1, src2);
        break;
      case 8:
        orc_lsx_insn_emit_vaddd (c, dest, src1, src2);
        break;
    }
  }
}

static void
orc_lsx_rule_addssX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vsaddb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vsaddh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vsaddw (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_addusX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vsaddbu (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vsaddhu (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vsaddwu (c, dest, src1, src2);
      break;
    case 8:
      orc_lsx_insn_emit_vsadddu (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_subX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vsubb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vsubh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vsubw (c, dest, src1, src2);
      break;
    case 8:
      orc_lsx_insn_emit_vsubd (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_subssX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vssubb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vssubh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vssubw (c, dest, src1, src2);
      break;
    case 8:
      orc_lsx_insn_emit_vssubd (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_subusX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vssubbu (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vssubhu (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vssubwu (c, dest, src1, src2);
      break;
    case 8:
      orc_lsx_insn_emit_vssubdu (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_andX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vandv (c, dest, src1, src2);
}

static void
orc_lsx_rule_minX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vminb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vminh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vminw (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_minuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vminbu (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vminhu (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vminwu (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_maxX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vmaxb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vmaxh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vmaxw (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_maxuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vmaxbu (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vmaxhu (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vmaxwu (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_absX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0;
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vreplgr2vrb(c, tmp1, ORC_LOONG_ZERO);
      orc_lsx_insn_emit_vabsdb (c, dest, tmp1, src);
      break;
    case 2:
      orc_lsx_insn_emit_vreplgr2vrh(c, tmp1, ORC_LOONG_ZERO);
      orc_lsx_insn_emit_vabsdh (c, dest, tmp1, src);
      break;
    case 4:
      orc_lsx_insn_emit_vreplgr2vrw(c, tmp1, ORC_LOONG_ZERO);
      orc_lsx_insn_emit_vabsdw (c, dest, tmp1, src);
      break;
  }
}

static void
orc_lsx_rule_avgX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vavgrb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vavgrh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vavgrw (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_avguX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vavgrbu (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vavgrhu (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vavgrwu (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_div255w (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0;

  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 255);
  orc_lsx_insn_emit_vreplgr2vrh (c, tmp1, c->gp_tmpreg);
  orc_lsx_insn_emit_vdivhu (c, dest, src, tmp1);
}

static void
orc_lsx_rule_signX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0;

  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
      orc_lsx_insn_emit_vreplgr2vrb (c, tmp1, c->gp_tmpreg);
      orc_lsx_insn_emit_vsigncovb (c, dest, src, tmp1);
      break;
    case 2:
      orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
      orc_lsx_insn_emit_vreplgr2vrh (c, tmp1, c->gp_tmpreg);
      orc_lsx_insn_emit_vsigncovh (c, dest, src, tmp1);
      break;
    case 4:
      orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
      orc_lsx_insn_emit_vreplgr2vrw (c, tmp1, c->gp_tmpreg);
      orc_lsx_insn_emit_vsigncovw (c, dest, src, tmp1);
      break;
  }
}

static void
orc_lsx_rule_divluw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0;
  const OrcLoongRegister zero = ORC_LOONG_VR4;
  const OrcLoongRegister mask = ORC_LOONG_VR5;

  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 255);
  orc_lsx_insn_emit_vreplgr2vrh (c, tmp1, c->gp_tmpreg);
  orc_lsx_insn_emit_vxorv (c, zero, zero, zero);
  orc_lsx_insn_emit_vandv (c, src2, src2, tmp1);
  orc_lsx_insn_emit_vseqh (c, mask, zero, src2);
  orc_lsx_insn_emit_vaddh (c, src2, src2, mask);
  orc_lsx_insn_emit_vdivhu (c, dest, src1, src2);
  orc_lsx_insn_emit_vandnv (c, dest, mask, dest);
  orc_lsx_insn_emit_vaddh (c, dest, dest, mask);
  orc_lsx_insn_emit_vminhu (c, dest, tmp1, dest);
}

static void
orc_lsx_rule_mullX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vmulb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vmulh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vmulw (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_mulsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0, tmp2 = ORC_LOONG_VR4;

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vsllwilhb (c, tmp1, src1, 0);
      orc_lsx_insn_emit_vsllwilhb (c, tmp2, src2, 0);
      orc_lsx_insn_emit_vmulh (c, dest, tmp1, tmp2);
      break;
    case 2:
      orc_lsx_insn_emit_vsllwilwh (c, tmp1, src1, 0);
      orc_lsx_insn_emit_vsllwilwh (c, tmp2, src2, 0);
      orc_lsx_insn_emit_vmulw (c, dest, tmp1, tmp2);
      break;
    case 4:
      orc_lsx_insn_emit_vsllwildw (c, tmp1, src1, 0);
      orc_lsx_insn_emit_vsllwildw (c, tmp2, src2, 0);
      orc_lsx_insn_emit_vmuld (c, dest, tmp1, tmp2);
      break;
  }
}

static void
orc_lsx_rule_mulhsX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vmuhb (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vmuhh (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vmuhw (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_mulhuX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vmuhbu (c, dest, src1, src2);
      break;
    case 2:
      orc_lsx_insn_emit_vmuhhu (c, dest, src1, src2);
      break;
    case 4:
      orc_lsx_insn_emit_vmuhwu (c, dest, src1, src2);
      break;
  }
}

static void
orc_lsx_rule_muluX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0, tmp2 = ORC_LOONG_VR4;

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 1:
      orc_lsx_insn_emit_vsllwilhubu (c, tmp1, src1, 0);
      orc_lsx_insn_emit_vsllwilhubu (c, tmp2, src2, 0);
      orc_lsx_insn_emit_vmulh (c, dest, tmp1, tmp2);
      break;
    case 2:
      orc_lsx_insn_emit_vsllwilwuhu (c, tmp1, src1, 0);
      orc_lsx_insn_emit_vsllwilwuhu (c, tmp2, src2, 0);
      orc_lsx_insn_emit_vmulw (c, dest, tmp1, tmp2);
      break;
    case 4:
      orc_lsx_insn_emit_vsllwilduwu (c, tmp1, src1, 0);
      orc_lsx_insn_emit_vsllwilduwu (c, tmp2, src2, 0);
      orc_lsx_insn_emit_vmuld (c, dest, tmp1, tmp2);
      break;
  }
}

static void
orc_lsx_rule_accX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  int src = ORC_SRC_ARG (c, insn, 0);
  int dest = ORC_DEST_ARG (c, insn, 0);

  int size = ORC_PTR_TO_INT (user);

  switch (size) {
    case 2:
      if (c->loop_shift == 0) {
        orc_lsx_insn_emit_vbsllv (c, src, src, 14);
      } else if (c->loop_shift == 1) {
        orc_lsx_insn_emit_vbsllv (c, src, src, 12);
      } else if (c->loop_shift == 2) {
        orc_lsx_insn_emit_vbsllv (c, src, src, 8);
      }
      orc_lsx_insn_emit_vaddh (c, dest, src, dest);
      break;
    case 4:
      if (c->loop_shift == 0) {
        orc_lsx_insn_emit_vbsllv (c, src, src, 12);
      } else if (c->loop_shift == 1) {
        orc_lsx_insn_emit_vbsllv (c, src, src, 8);
      }
      orc_lsx_insn_emit_vaddw (c, dest, src, dest);
      break;
  }
}

static void
orc_lsx_rule_accsadubl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0, tmp2 = ORC_LOONG_VR4;

  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwilhubu (c, tmp1, src1, 0);
  orc_lsx_insn_emit_vsllwilhubu (c, tmp2, src2, 0);
  orc_lsx_insn_emit_vsllwilwuhu (c, tmp1, tmp1, 0);
  orc_lsx_insn_emit_vsllwilwuhu (c, tmp2, tmp2, 0);
  orc_lsx_insn_emit_vabsdwu (c, tmp1, tmp1, tmp2);
  if (c->loop_shift == 0) {
    orc_lsx_insn_emit_vbsllv (c, tmp1, tmp1, 12);
  } else if (c->loop_shift == 1) {
    orc_lsx_insn_emit_vbsllv (c, tmp1, tmp1, 8);
  }
  orc_lsx_insn_emit_vsaddwu (c, dest, dest, tmp1);
}

static void
orc_lsx_rule_mergebw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vilvlb (c, dest, src2, src1);
}

static void
orc_lsx_rule_mergewl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vilvlh (c, dest, src2, src1);
}

static void
orc_lsx_rule_mergelq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vilvlw (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpgtsb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsltb (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpgtsw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vslth (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpgtsl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsltw (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpgtsq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsltd (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpeqb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vseqb (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpeqw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vseqh (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpeql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vseqw (c, dest, src2, src1);
}

static void
orc_lsx_rule_cmpeqq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vseqd (c, dest, src2, src1);
}

static void
orc_lsx_rule_select0wb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrlnibh (c, dest, src, 0);
}

static void
orc_lsx_rule_select0lw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrlnihw (c, dest, src, 0);
}

static void
orc_lsx_rule_select0ql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrlniwd (c, dest, src, 0);
}

static void
orc_lsx_rule_select1wb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrlnibh (c, dest, src, 8);
}

static void
orc_lsx_rule_select1lw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrlnihw (c, dest, src, 16);
}

static void
orc_lsx_rule_select1ql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrlniwd (c, dest, src, 32);
}

static void
orc_lsx_rule_splatw3q (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcLoongRegister tmp1 = ORC_LOONG_VR0;

  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vreplveih (c, tmp1, src, 3);
  orc_lsx_insn_emit_vreplveih (c, dest, src, 7);
  orc_lsx_insn_emit_vilvld(c, dest, dest, tmp1);
}

static void
orc_lsx_rule_splatbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vilvlb (c, dest, src, src);
}

static void
orc_lsx_rule_splatbl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vilvlb (c, dest, src, src);
  orc_lsx_insn_emit_vilvlh (c, dest, src, src);
}

static void
orc_lsx_rule_splitwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest1 = ORC_DEST_ARG (c, insn, 0);
  const int dest2 = ORC_DEST_ARG (c, insn, 1);

  orc_lsx_insn_emit_vsrlnibh (c, dest1, src, 8);
  orc_lsx_insn_emit_vsrlnibh (c, dest2, src, 0);
}

static void
orc_lsx_rule_splitlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest1 = ORC_DEST_ARG (c, insn, 0);
  const int dest2 = ORC_DEST_ARG (c, insn, 1);

  orc_lsx_insn_emit_vsrlnihw (c, dest1, src, 16);
  orc_lsx_insn_emit_vsrlnihw (c, dest2, src, 0);
}

static void
orc_lsx_rule_splitql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest1 = ORC_DEST_ARG (c, insn, 0);
  const int dest2 = ORC_DEST_ARG (c, insn, 1);

  const int zero = c->tmpreg;
  orc_lsx_insn_emit_vxorv (c,zero, zero, zero);
  orc_lsx_insn_emit_vpickodw (c, dest1, zero, src);
  orc_lsx_insn_emit_vpickevw (c, dest2, zero, src);
}

static void
orc_lsx_rule_swapw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vshuf4ib (c, dest, src, 0xB1);
}

static void
orc_lsx_rule_swapl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vshuf4ib (c, dest, src, 0x1B);
}

static void
orc_lsx_rule_swapq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vshuf4ih (c, dest, src, 0x4E);
  orc_lsx_insn_emit_vshuf4ib (c, dest, dest, 0x1B);
}

static void
orc_lsx_rule_swapwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vshuf4ih (c, dest, src, 0xB1);
}

static void
orc_lsx_rule_swaplq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vshuf4iw (c, dest, src, 0xB1);
}

static void
orc_lsx_rule_andnX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vandnv (c, dest, src1, src2);
}

static void
orc_lsx_rule_orX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vorv (c, dest, src1, src2);
}

static void
orc_lsx_rule_xorX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vxorv (c, dest, src1, src2);
}

static void
orc_lsx_rule_shlb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsllib (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsllb (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsllih (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsllh (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shll (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vslliw (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsllw (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shlq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsllid (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vslld (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shrsb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsraib (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrab (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shrsw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsraih (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrah (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shrsl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsraiw (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsraw (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shrsq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsraid (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrad (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shrub (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsrlib (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrlb (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shruw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsrlih (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrlh (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shrul (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsrliw (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrlw (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_shruq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      orc_lsx_insn_emit_vsrlid (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      orc_lsx_insn_emit_vsrld (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lsx_rule_convnwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrarnibh (c, dest, src, 0);
}

static void
orc_lsx_rule_convnlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrarnihw (c, dest, src, 0);
}

static void
orc_lsx_rule_convnql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsrarniwd (c, dest, src, 0);
}

static void
orc_lsx_rule_convwbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwilhb (c, dest, src, 0);
}

static void
orc_lsx_rule_convwwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwilwh (c, dest, src, 0);
}

static void
orc_lsx_rule_convwlq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwildw (c, dest, src, 0);
}

static void
orc_lsx_rule_convubw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwilhubu (c, dest, src, 0);
}

static void
orc_lsx_rule_convuwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwilwuhu (c, dest, src, 0);
}

static void
orc_lsx_rule_convulq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsllwilduwu (c, dest, src, 0);
}

static void
orc_lsx_rule_convhwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsranibh (c, dest, src, 8);
}

static void
orc_lsx_rule_convhlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vsranihw (c, dest, src, 16);
}

static void
orc_lsx_rule_convssswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssranibh (c, dest, src, 0);
}

static void
orc_lsx_rule_convssslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssranihw (c, dest, src, 0);
}

static void
orc_lsx_rule_convsssql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssraniwd (c, dest, src, 0);
}

static void
orc_lsx_rule_convsuswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssranibuh (c, dest, src, 0);
}

static void
orc_lsx_rule_convsuslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssranihuw (c, dest, src, 0);
}

static void
orc_lsx_rule_convsusql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssraniwud (c, dest, src, 0);
}

static void
orc_lsx_rule_convuuswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssrlnibuh (c, dest, src, 0);
}

static void
orc_lsx_rule_convuuslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssrlnihuw(c, dest, src, 0);
}

static void
orc_lsx_rule_convuusql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssrlniwud(c, dest, src, 0);
}

static void
orc_lsx_rule_convusswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssrlnibh(c, dest, src, 0);
}

static void
orc_lsx_rule_convusslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vssrlnihw(c, dest, src, 0);
}

static void
orc_lsx_rule_addf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfadds (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 4);
}

static void
orc_lsx_rule_addd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfaddd (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 8);
}

static void
orc_lsx_rule_subf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfsubs (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 4);
}

static void
orc_lsx_rule_subd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfsubd (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 8);
}

static void
orc_lsx_rule_mulf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfmuls (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 4);
}

static void
orc_lsx_rule_muld (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfmuld (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 8);
}

static void
orc_lsx_rule_divf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfdivs (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 4);
}

static void
orc_lsx_rule_divd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfdivd (c, dest, src1, src2);
  orc_lsx_insn_emit_normalize (c, dest, dest, 8);
}

static void
orc_lsx_rule_sqrtf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfsqrts (c, dest, src1);
  orc_lsx_insn_emit_normalize (c, dest, dest, 4);
}

static void
orc_lsx_rule_sqrtd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfsqrtd (c, dest, src1);
  orc_lsx_insn_emit_normalize (c, dest, dest, 8);
}

static void
orc_lsx_rule_convlf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vffintsw (c, dest, src1);
}

static void
orc_lsx_rule_convld (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vffintldw (c, dest, src1);
}

static void
orc_lsx_rule_convfl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vftintrzws (c, dest, src1);
}

static void
orc_lsx_rule_convfd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcvtlds (c, dest, src1);
}

static void
orc_lsx_rule_convdl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vftintrnewd (c, dest, src1, src1);
}

static void
orc_lsx_rule_convdf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcvtsd (c, dest, src1, src1);
  orc_lsx_insn_emit_normalize (c, dest, dest, 4);
}

static void
orc_lsx_rule_cmpeqf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcmpceqs (c, dest, src1, src2);
}

static void
orc_lsx_rule_cmpeqd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcmpceqd (c, dest, src1, src2);
}

static void
orc_lsx_rule_cmpltf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcmpclts (c, dest, src1, src2);
}

static void
orc_lsx_rule_cmpltd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcmpcltd (c, dest, src1, src2);
}

static void
orc_lsx_rule_cmplef (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcmpcles (c, dest, src1, src2);
}

static void
orc_lsx_rule_cmpled (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  orc_lsx_insn_emit_vfcmpcled (c, dest, src1, src2);
}

static void
orc_lsx_rule_minf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const OrcLoongRegister t0 = ORC_LOONG_VR4;
  const OrcLoongRegister t1 = ORC_LOONG_VR5;
  const OrcLoongRegister t2 = ORC_LOONG_VR6;

  orc_lsx_insn_emit_vfmins (c, t0, src1, src2);
  orc_lsx_insn_emit_vfcmpcuns (c, t1, src1, src1);
  orc_lsx_insn_emit_vfcmpcuns (c, t2, src2, src2);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src1, t1);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src2, t2);
  orc_lsx_insn_emit_normalize (c, t0, dest, 4);
}

static void
orc_lsx_rule_mind (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const OrcLoongRegister t0 = ORC_LOONG_VR4;
  const OrcLoongRegister t1 = ORC_LOONG_VR5;
  const OrcLoongRegister t2 = ORC_LOONG_VR6;

  orc_lsx_insn_emit_vfmind (c, t0, src1, src2);
  orc_lsx_insn_emit_vfcmpcund (c, t1, src1, src1);
  orc_lsx_insn_emit_vfcmpcund (c, t2, src2, src2);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src1, t1);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src2, t2);
  orc_lsx_insn_emit_normalize (c, t0, dest, 8);
}

static void
orc_lsx_rule_maxf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const OrcLoongRegister t0 = ORC_LOONG_VR4;
  const OrcLoongRegister t1 = ORC_LOONG_VR5;
  const OrcLoongRegister t2 = ORC_LOONG_VR6;

  orc_lsx_insn_emit_vfmaxs (c, t0, src1, src2);
  orc_lsx_insn_emit_vfcmpcuns (c, t1, src1, src1);
  orc_lsx_insn_emit_vfcmpcuns (c, t2, src2, src2);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src1, t1);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src2, t2);
  orc_lsx_insn_emit_normalize (c, t0, dest, 4);
}

static void
orc_lsx_rule_maxd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
  const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const OrcLoongRegister t0 = ORC_LOONG_VR4;
  const OrcLoongRegister t1 = ORC_LOONG_VR5;
  const OrcLoongRegister t2 = ORC_LOONG_VR6;

  orc_lsx_insn_emit_vfmaxd (c, t0, src1, src2);
  orc_lsx_insn_emit_vfcmpcund (c, t1, src1, src1);
  orc_lsx_insn_emit_vfcmpcund (c, t2, src2, src2);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src1, t1);
  orc_lsx_insn_emit_vbitselv (c, t0, t0, src2, t2);
  orc_lsx_insn_emit_normalize (c, t0, dest, 8);
}

#define REG(opcode, rule, size) \
  orc_rule_register (rule_set, #opcode, orc_lsx_rule_##rule, (void*)size)

void
orc_lsx_rules_init (OrcTarget *target)
{
  OrcRuleSet *rule_set =
      orc_rule_set_new (orc_opcode_set_get ("sys"), target, ORC_TARGET_LOONGARCH_LSX);

  REG (loadpb, loadpX, 1);
  REG (loadpw, loadpX, 2);
  REG (loadpl, loadpX, 4);
  REG (loadpq, loadpX, 8);

  REG (loadb, loadX, NULL);
  REG (loadw, loadX, NULL);
  REG (loadl, loadX, NULL);
  REG (loadq, loadX, NULL);

  REG (loadupdb, loadupdb, NULL);

  REG (loadoffb, loadoffX, NULL);
  REG (loadoffw, loadoffX, NULL);
  REG (loadoffl, loadoffX, NULL);

  REG (storeb, storeX, NULL);
  REG (storew, storeX, NULL);
  REG (storel, storeX, NULL);
  REG (storeq, storeX, NULL);

  REG (copyb, copyX, NULL);
  REG (copyw, copyX, NULL);
  REG (copyl, copyX, NULL);
  REG (copyq, copyX, NULL);

  REG (addb, addX, 1);
  REG (addw, addX, 2);
  REG (addl, addX, 4);
  REG (addq, addX, 8);

  REG (addssb, addssX, 1);
  REG (addssw, addssX, 2);
  REG (addssl, addssX, 4);

  REG (addusb, addusX, 1);
  REG (addusw, addusX, 2);
  REG (addusl, addusX, 4);

  REG (subb, subX, 1);
  REG (subw, subX, 2);
  REG (subl, subX, 4);
  REG (subq, subX, 8);

  REG (subssb, subssX, 1);
  REG (subssw, subssX, 2);
  REG (subssl, subssX, 4);

  REG (subusb, subusX, 1);
  REG (subusw, subusX, 2);
  REG (subusl, subusX, 4);

  REG (andb, andX, NULL);
  REG (andw, andX, NULL);
  REG (andl, andX, NULL);
  REG (andq, andX, NULL);

  REG (minsb, minX, 1);
  REG (minsw, minX, 2);
  REG (minsl, minX, 4);

  REG (minub, minuX, 1);
  REG (minuw, minuX, 2);
  REG (minul, minuX, 4);

  REG (maxsb, maxX, 1);
  REG (maxsw, maxX, 2);
  REG (maxsl, maxX, 4);

  REG (maxub, maxuX, 1);
  REG (maxuw, maxuX, 2);
  REG (maxul, maxuX, 4);

  REG (absb, absX, 1);
  REG (absw, absX, 2);
  REG (absl, absX, 4);

  REG (avgsb, avgX, 1);
  REG (avgsw, avgX, 2);
  REG (avgsl, avgX, 4);

  REG (avgub, avguX, 1);
  REG (avguw, avguX, 2);
  REG (avgul, avguX, 4);

  REG (mulsbw, mulsX, 1);
  REG (mulswl, mulsX, 2);
  REG (mulslq, mulsX, 4);

  REG (mulubw, muluX, 1);
  REG (muluwl, muluX, 2);
  REG (mululq, muluX, 4);

  REG (mullb, mullX, 1);
  REG (mullw, mullX, 2);
  REG (mulll, mullX, 4);

  REG (mulhsb, mulhsX, 1);
  REG (mulhsw, mulhsX, 2);
  REG (mulhsl, mulhsX, 4);

  REG (mulhub, mulhuX, 1);
  REG (mulhuw, mulhuX, 2);
  REG (mulhul, mulhuX, 4);

  REG (accw, accX, 2);
  REG (accl, accX, 4);
  REG (accsadubl, accsadubl, NULL);

  REG (mergebw, mergebw, NULL);
  REG (mergewl, mergewl, NULL);
  REG (mergelq, mergelq, NULL);

  REG (cmpgtsb, cmpgtsb, NULL);
  REG (cmpgtsw, cmpgtsw, NULL);
  REG (cmpgtsl, cmpgtsl, NULL);
  REG (cmpgtsq, cmpgtsq, NULL);

  REG (cmpeqb, cmpeqb, NULL);
  REG (cmpeqw, cmpeqw, NULL);
  REG (cmpeql, cmpeql, NULL);
  REG (cmpeqq, cmpeqq, NULL);

  REG (signb, signX, 1);
  REG (signw, signX, 2);
  REG (signl, signX, 4);

  REG (select0wb, select0wb, NULL);
  REG (select0lw, select0lw, NULL);
  REG (select0ql, select0ql, NULL);

  REG (select1wb, select1wb, NULL);
  REG (select1lw, select1lw, NULL);
  REG (select1ql, select1ql, NULL);

  REG (splatw3q, splatw3q, NULL);
  REG (splatbw, splatbw, NULL);
  REG (splatbl, splatbl, NULL);

  REG (splitwb, splitwb, NULL);
  REG (splitlw, splitlw, NULL);
  REG (splitql, splitql, NULL);

  REG (swapw, swapw, NULL);
  REG (swapl, swapl, NULL);
  REG (swapq, swapq, NULL);
  REG (swapwl, swapwl, NULL);
  REG (swaplq, swaplq, NULL);

  REG (andnb, andnX, NULL);
  REG (andnw, andnX, NULL);
  REG (andnl, andnX, NULL);
  REG (andnq, andnX, NULL);
  REG (andf, andX, NULL);

  REG (orb, orX, NULL);
  REG (orw, orX, NULL);
  REG (orl, orX, NULL);
  REG (orq, orX, NULL);
  REG (orf, orX, NULL);

  REG (xorb, xorX, NULL);
  REG (xorw, xorX, NULL);
  REG (xorl, xorX, NULL);
  REG (xorq, xorX, NULL);

  REG (shlb, shlb, NULL);
  REG (shlw, shlw, NULL);
  REG (shll, shll, NULL);
  REG (shlq, shlq, NULL);

  REG (shrsb, shrsb, NULL);
  REG (shrsw, shrsw, NULL);
  REG (shrsl, shrsl, NULL);
  REG (shrsq, shrsq, NULL);

  REG (shrub, shrub, NULL);
  REG (shruw, shruw, NULL);
  REG (shrul, shrul, NULL);
  REG (shruq, shruq, NULL);

  REG (convwb, convnwb, NULL);
  REG (convlw, convnlw, NULL);
  REG (convql, convnql, NULL);

  REG (convsbw, convwbw, NULL);
  REG (convswl, convwwl, NULL);
  REG (convslq, convwlq, NULL);

  REG (convubw, convubw, NULL);
  REG (convuwl, convuwl, NULL);
  REG (convulq, convulq, NULL);

  REG (convhlw, convhlw, NULL);
  REG (convhwb, convhwb, NULL);

  REG (convssswb, convssswb, NULL);
  REG (convssslw, convssslw, NULL);
  REG (convsssql, convsssql, NULL);

  REG (convsuswb, convsuswb, NULL);
  REG (convsuslw, convsuslw, NULL);
  REG (convsusql, convsusql, NULL);

  REG (convuuswb, convuuswb, NULL);
  REG (convuuslw, convuuslw,  NULL);
  REG (convuusql, convuusql,  NULL);

  REG (convusswb, convusswb, NULL);
  REG (convusslw, convusslw, NULL);

  REG (div255w, div255w, NULL);
  REG (divluw, divluw, NULL);

  REG (convlf, convlf, NULL);
  REG (convld, convld, NULL);
  REG (convfl, convfl, NULL);
  REG (convfd, convfd, NULL);
  REG (convdl, convdl, NULL);
  REG (convdf, convdf, NULL);

  REG (addf, addf, NULL);
  REG (addd, addd, NULL);
  REG (subf, subf, NULL);
  REG (subd, subd, NULL);
  REG (mulf, mulf, NULL);
  REG (muld, muld, NULL);
  REG (divf, divf, NULL);
  REG (divd, divd, NULL);
  REG (sqrtf, sqrtf, NULL);
  REG (sqrtd, sqrtd, NULL);

  REG (cmpeqf, cmpeqf, NULL);
  REG (cmpeqd, cmpeqd, NULL);
  REG (cmpltf, cmpltf, NULL);
  REG (cmpltd, cmpltd, NULL);
  REG (cmplef, cmplef, NULL);
  REG (cmpled, cmpled, NULL);

  REG (minf, minf, NULL);
  REG (mind, mind, NULL);
  REG (maxf, maxf, NULL);
  REG (maxd, maxd, NULL);
}
