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

#include <stdint.h>
#include <sys/types.h>
#include <orc/orccompiler.h>
#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcrule.h>
#include <orc/orcutils.h>
#include <orc/loongarch/orcloongarchinsn.h>
#include <orc/loongarch/orclsxinsn.h>
#include <orc/loongarch/orclasxinsn.h>

#define ORC_LASX_TO_LSX_REG(i) (ORC_LOONG_VR0 + (i - ORC_LOONG_XR0))
#define ORC_LSX_TO_LASX_REG(i) (ORC_LOONG_XR0 + (i - ORC_LOONG_VR0))

#define NORMALIZE_SRC_ARG(c, insn, arg, element_width) \
  orc_lasx_insn_emit_normalize(c, ORC_SRC_ARG (c, insn, arg), ORC_LOONG_XR10 + arg, element_width)

#define NORMALIZE_SRC_ARG1(c, insn, arg, element_width) \
  orc_lsx_insn_emit_normalize(c, ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, arg)), ORC_LOONG_VR10 + arg, element_width)

static void
orc_lasx_rule_loadpX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcVariable *const src = c->vars + insn->src_args[0];
  const OrcVariable *const dest = c->vars + insn->dest_args[0];
  /* user contains the size of the underlying type */
  const int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_load_imm (c, c->gp_tmpreg, src->value.i);

    if (size == 1) {
      orc_lasx_insn_emit_xvreplgr2vrb (c, dest->alloc, c->gp_tmpreg);
    } else if (size == 2) {
      orc_lasx_insn_emit_xvreplgr2vrh (c, dest->alloc, c->gp_tmpreg);
    } else if (size == 4) {
      orc_lasx_insn_emit_xvreplgr2vrw (c, dest->alloc, c->gp_tmpreg);
    } else if (size == 8) {
      orc_lasx_insn_emit_xvreplgr2vrd (c, dest->alloc, c->gp_tmpreg);
    } else {
      ORC_PROGRAM_ERROR(c, "constants not imclemented");
    }
  } else {
    int offset = ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]]);
    if (size == 1) {
      orc_lasx_insn_emit_xvldreplb (c, dest->alloc, c->exec_reg, offset);
    } else if (size == 2) {
      orc_lasx_insn_emit_xvldreplh (c, dest->alloc, c->exec_reg, offset);
    } else if (size == 4) {
      orc_lasx_insn_emit_xvldreplw (c, dest->alloc, c->exec_reg, offset);
    } else if (size == 8) {
      const OrcLoongRegister t0 = ORC_LOONG_XR4;
      int offset1 = ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]+(ORC_N_PARAMS)]);
      orc_lasx_insn_emit_xvldreplw (c, t0, c->exec_reg, offset);
      orc_lasx_insn_emit_xvldreplw (c, dest->alloc, c->exec_reg, offset1);
      orc_lasx_insn_emit_xvilvlw (c, dest->alloc, dest->alloc, t0);
    } else {
      ORC_PROGRAM_ERROR(c, "unimplemented");
    }
  }
}

static void
orc_lasx_rule_loadX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcVariable *src = c->vars + insn->src_args[0];
  const OrcVariable *dest = c->vars + insn->dest_args[0];
  int ptr_reg = 0;
  const int offset = c->offset * src->size;

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
      orc_lsx_insn_emit_vldreplb (c, ORC_LASX_TO_LSX_REG(dest->alloc), ptr_reg, offset);
      break;
    case 2:
      orc_lsx_insn_emit_vldreplh (c, ORC_LASX_TO_LSX_REG(dest->alloc), ptr_reg, offset);
      break;
    case 4:
      orc_lsx_insn_emit_vldreplw (c, ORC_LASX_TO_LSX_REG(dest->alloc), ptr_reg, offset);
      break;
    case 8:
      orc_lsx_insn_emit_vldrepld (c, ORC_LASX_TO_LSX_REG(dest->alloc), ptr_reg, offset);
      break;
    case 16:
      orc_lsx_insn_emit_vld (c, ORC_LASX_TO_LSX_REG(dest->alloc), ptr_reg, offset);
      break;
    case 32:
      orc_lasx_insn_emit_xvld (c, dest->alloc, ptr_reg, offset);
      break;
    default:
      orc_compiler_error (c, "loadX bad load size %d", src->size << c->loop_shift);
      break;
  }
}

static void
orc_lasx_rule_loadupdb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  OrcVariable *src = c->vars + insn->src_args[0];
  OrcVariable *dest = c->vars + insn->dest_args[0];
  int ptr_reg;
  int offset = 0;
  int size = src->size << c->loop_shift;

  offset = (c->offset * src->size)>>1;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_loongarch_insn_emit_ld_d (c, c->gp_tmpreg, c->exec_reg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]));
    ptr_reg = c->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }

  switch (size) {
    case 1:
    case 2:
      orc_lasx_insn_emit_xvldreplb (c, dest->alloc, ptr_reg, offset);
      break;
    case 4:
      orc_lasx_insn_emit_xvldreplh (c, dest->alloc, ptr_reg, offset);
      break;
    case 8:
      orc_lasx_insn_emit_xvldreplw (c, dest->alloc, ptr_reg, offset);
      break;
    case 16:
      orc_lasx_insn_emit_xvldrepld (c, dest->alloc, ptr_reg, offset);
      break;
    case 32:
      orc_lsx_insn_emit_vld (c, ORC_LASX_TO_LSX_REG(dest->alloc), ptr_reg, offset);
      break;
    default:
      orc_compiler_error(c,"bad load size %d", src->size << c->loop_shift);
      break;
  }

  switch (src->size) {
    case 1:
      if (size >= 32) {
        orc_lsx_insn_emit_vilvlb (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
        orc_lsx_insn_emit_vilvhb (c, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
        orc_lasx_insn_emit_xvpermiq (c, dest->alloc, ORC_LOONG_XR0, 0x20);
      } else {
         orc_lsx_insn_emit_vilvlb (c, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
      }
      break;
    case 2:
      if (size >= 32) {
        orc_lsx_insn_emit_vilvlh (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
        orc_lsx_insn_emit_vilvhh (c, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
        orc_lasx_insn_emit_xvpermiq (c, dest->alloc, ORC_LOONG_XR0, 0x20);
      } else {
         orc_lsx_insn_emit_vilvlh (c, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
      }
      break;
    case 4:
      if (size >= 32) {
        orc_lsx_insn_emit_vilvlw (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
        orc_lsx_insn_emit_vilvhw (c, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
        orc_lasx_insn_emit_xvpermiq (c, dest->alloc, ORC_LOONG_XR0, 0x20);
      } else {
         orc_lsx_insn_emit_vilvlw (c, ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc), ORC_LASX_TO_LSX_REG(dest->alloc));
      }
      break;
  }
  src->update_type = 1;
}

static void
orc_lasx_rule_storeX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const OrcVariable *const src = c->vars + insn->src_args[0];
  const OrcVariable *const dest = c->vars + insn->dest_args[0];
  int ptr_reg;
  int offset;

  offset = c->offset * dest->size;
  if (dest->ptr_register == 0) {
    orc_loongarch_insn_emit_ld_d (c, c->gp_tmpreg, c->exec_reg,
        dest->ptr_offset);
    ptr_reg = c->gp_tmpreg;
  } else {
    ptr_reg = dest->ptr_register;
  }
  switch (dest->size << c->loop_shift) {
    case 1:
      orc_lsx_insn_emit_vstelmb (c, ORC_LASX_TO_LSX_REG(src->alloc), ptr_reg, offset, 0);
      break;
    case 2:
      orc_lsx_insn_emit_vstelmh (c, ORC_LASX_TO_LSX_REG(src->alloc), ptr_reg, offset, 0);
      break;
    case 4:
      orc_lsx_insn_emit_vstelmw (c, ORC_LASX_TO_LSX_REG(src->alloc), ptr_reg, offset, 0);
      break;
    case 8:
      orc_lsx_insn_emit_vstelmd (c, ORC_LASX_TO_LSX_REG(src->alloc), ptr_reg, offset, 0);
      break;
    case 16:
      orc_lsx_insn_emit_vst (c, ORC_LASX_TO_LSX_REG(src->alloc), ptr_reg, offset);
      break;
    case 32:
      orc_lasx_insn_emit_xvst (c, src->alloc, ptr_reg, offset);
      break;
    default:
      orc_compiler_error (c, "bad size");
      break;
  }
}

static void
orc_lasx_rule_copyX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvorv (c, dest, src, src);
  } else {
    orc_lsx_insn_emit_vorv (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
  }
}

static void
orc_lasx_rule_addb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (ORC_SRC_TYPE (c, insn, 1) == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, ORC_SRC_VAL (c, insn, 1));
    orc_lasx_insn_emit_xvreplgr2vrb (c, src2, c->gp_tmpreg);
  }
  if (size >= 32) {
    orc_lasx_insn_emit_xvaddb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vaddb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (ORC_SRC_TYPE (c, insn, 1) == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_addi_d(c, c->gp_tmpreg, ORC_LOONG_ZERO, ORC_SRC_VAL (c, insn, 1));
    orc_lasx_insn_emit_xvreplgr2vrh (c, src2, c->gp_tmpreg);
  }
  if (size >= 32) {
    orc_lasx_insn_emit_xvaddh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vaddh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (ORC_SRC_TYPE (c, insn, 1) == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_addi_d(c, c->gp_tmpreg, ORC_LOONG_ZERO, ORC_SRC_VAL (c, insn, 1));
    orc_lasx_insn_emit_xvreplgr2vrw (c, src2, c->gp_tmpreg);
  }
  if (size >= 32) {
    orc_lasx_insn_emit_xvaddw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vaddw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (ORC_SRC_TYPE (c, insn, 1) == ORC_VAR_TYPE_CONST) {
    orc_loongarch_insn_emit_addi_d(c, c->gp_tmpreg, ORC_LOONG_ZERO, ORC_SRC_VAL (c, insn, 1));
    orc_lasx_insn_emit_xvreplgr2vrd (c, src2, c->gp_tmpreg);
  }
  if (size >= 32) {
    orc_lasx_insn_emit_xvaddd (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vaddd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addssb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsaddb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsaddb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addssw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsaddh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsaddh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addssl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsaddw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsaddw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addusb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsaddbu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsaddbu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addusw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsaddhu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsaddhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_addusl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsaddwu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsaddwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsubb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsubb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsubh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsubh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsubw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsubw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsubd (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vsubd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subssb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvssubb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vssubb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subssw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvssubh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vssubh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subssl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvssubw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vssubw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subusb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvssubbu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vssubbu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subusw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvssubhu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vssubhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_subusl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvssubwu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vssubwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_andX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvandv (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vandv (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvminb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vminb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvminh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vminh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvminw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vminw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minub (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvminbu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vminbu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minuw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvminhu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vminhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minul (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvminwu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vminwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_maxb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmaxb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmaxb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_maxw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmaxh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmaxh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_maxl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmaxw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmaxw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_maxub (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmaxbu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmaxbu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_maxuw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmaxhu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmaxhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_maxul (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmaxwu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmaxwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_absb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvreplgr2vrb (c, ORC_LOONG_XR0, ORC_LOONG_ZERO);
    orc_lasx_insn_emit_xvabsdb (c, dest, ORC_LOONG_XR0, src);
  } else {
    orc_lsx_insn_emit_vreplgr2vrb (c, ORC_LOONG_VR0, ORC_LOONG_ZERO);
    orc_lsx_insn_emit_vabsdb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src));
  }
}

static void
orc_lasx_rule_absw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvreplgr2vrh (c, ORC_LOONG_XR0, ORC_LOONG_ZERO);
    orc_lasx_insn_emit_xvabsdh (c, dest, ORC_LOONG_XR0, src);
  } else {
    orc_lsx_insn_emit_vreplgr2vrh (c, ORC_LOONG_VR0, ORC_LOONG_ZERO);
    orc_lsx_insn_emit_vabsdh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src));
  }
}

static void
orc_lasx_rule_absl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvreplgr2vrw (c, ORC_LOONG_XR0, ORC_LOONG_ZERO);
    orc_lasx_insn_emit_xvabsdw (c, dest, ORC_LOONG_XR0, src);
  } else {
    orc_lsx_insn_emit_vreplgr2vrw (c, ORC_LOONG_VR0, ORC_LOONG_ZERO);
    orc_lsx_insn_emit_vabsdw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src));
  }
}

static void
orc_lasx_rule_avgsb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvavgrb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vavgrb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_avgsw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvavgrh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vavgrh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_avgsl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvavgrw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vavgrw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_avgub (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvavgrbu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vavgrbu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_avguw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvavgrhu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vavgrhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_avgul (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvavgrwu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vavgrwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_div255w (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 255);
    orc_lasx_insn_emit_xvreplgr2vrh (c, ORC_LOONG_XR0, c->gp_tmpreg);
    orc_lasx_insn_emit_xvdivhu (c, dest, src, ORC_LOONG_XR0);
  } else {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 255);
    orc_lsx_insn_emit_vreplgr2vrh (c, ORC_LOONG_VR0, c->gp_tmpreg);
    orc_lsx_insn_emit_vdivhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LOONG_VR0);
  }
}

static void
orc_lasx_rule_signb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
    orc_lasx_insn_emit_xvreplgr2vrb (c, ORC_LOONG_XR0, c->gp_tmpreg);
    orc_lasx_insn_emit_xvsigncovb (c, dest, src, ORC_LOONG_XR0);
  } else {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
    orc_lsx_insn_emit_vreplgr2vrb (c, ORC_LOONG_VR0, c->gp_tmpreg);
    orc_lsx_insn_emit_vsigncovb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LOONG_VR0);
  }
}

static void
orc_lasx_rule_signw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
    orc_lasx_insn_emit_xvreplgr2vrh (c, ORC_LOONG_XR0, c->gp_tmpreg);
    orc_lasx_insn_emit_xvsigncovh (c, dest, src, ORC_LOONG_XR0);
  } else {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
    orc_lsx_insn_emit_vreplgr2vrh (c, ORC_LOONG_VR0, c->gp_tmpreg);
    orc_lsx_insn_emit_vsigncovh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LOONG_VR0);
  }
}

static void
orc_lasx_rule_signl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
    orc_lasx_insn_emit_xvreplgr2vrw (c, ORC_LOONG_XR0, c->gp_tmpreg);
    orc_lasx_insn_emit_xvsigncovw (c, dest, src, ORC_LOONG_XR0);
  } else {
    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 1);
    orc_lsx_insn_emit_vreplgr2vrw (c, ORC_LOONG_VR0, c->gp_tmpreg);
    orc_lsx_insn_emit_vsigncovw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LOONG_VR0);
  }
}

static void
orc_lasx_rule_divluw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const OrcLoongRegister tmp1 = ORC_LOONG_XR0;
    const OrcLoongRegister zero = ORC_LOONG_XR4;
    const OrcLoongRegister mask = ORC_LOONG_XR5;

    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 255);
    orc_lasx_insn_emit_xvreplgr2vrh (c, tmp1, c->gp_tmpreg);
    orc_lasx_insn_emit_xvxorv (c, zero, zero, zero);
    orc_lasx_insn_emit_xvandv (c, src2, src2, tmp1);
    orc_lasx_insn_emit_xvseqh (c, mask, zero, src2);
    orc_lasx_insn_emit_xvaddh (c, src2, src2, mask);
    orc_lasx_insn_emit_xvdivhu (c, dest, src1, src2);
    orc_lasx_insn_emit_xvandnv (c, dest, mask, dest);
    orc_lasx_insn_emit_xvaddh (c, dest, dest, mask);
    orc_lasx_insn_emit_xvminhu (c, dest, tmp1, dest);
  } else {
    const OrcLoongRegister tmp1 = ORC_LOONG_VR0;
    const OrcLoongRegister zero = ORC_LOONG_VR4;
    const OrcLoongRegister mask = ORC_LOONG_VR5;

    orc_loongarch_insn_emit_addi_d (c, c->gp_tmpreg, ORC_LOONG_ZERO, 255);
    orc_lsx_insn_emit_vreplgr2vrh (c, tmp1, c->gp_tmpreg);
    orc_lsx_insn_emit_vxorv (c, zero, zero, zero);
    orc_lsx_insn_emit_vandv (c, ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src2), tmp1);
    orc_lsx_insn_emit_vseqh (c, mask, zero, ORC_LASX_TO_LSX_REG(src2));
    orc_lsx_insn_emit_vaddh (c,  ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src2), mask);
    orc_lsx_insn_emit_vdivhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
    orc_lsx_insn_emit_vandnv (c, ORC_LASX_TO_LSX_REG(dest), mask, ORC_LASX_TO_LSX_REG(dest));
    orc_lsx_insn_emit_vaddh (c,  ORC_LASX_TO_LSX_REG(dest), mask, ORC_LASX_TO_LSX_REG(dest));
    orc_lsx_insn_emit_vminhu (c, ORC_LASX_TO_LSX_REG(dest), tmp1, ORC_LASX_TO_LSX_REG(dest));
  }
}

static void
orc_lasx_rule_mullb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmulb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmulb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mullw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmulh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmulh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulll (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmulw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmulw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulsbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_vext2xvhb (c, ORC_LOONG_XR0, src1);
    orc_lasx_insn_emit_vext2xvhb (c, ORC_LOONG_XR4, src2);
    orc_lasx_insn_emit_xvmulh (c, dest, ORC_LOONG_XR0, ORC_LOONG_XR4);
  } else {
    orc_lsx_insn_emit_vsllwilhb (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
    orc_lsx_insn_emit_vsllwilhb (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
    orc_lsx_insn_emit_vmulh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR4);
  }
}

static void
orc_lasx_rule_mulswl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_vext2xvwh (c, ORC_LOONG_XR0, src1);
    orc_lasx_insn_emit_vext2xvwh (c, ORC_LOONG_XR4, src2);
    orc_lasx_insn_emit_xvmulw (c, dest, ORC_LOONG_XR0, ORC_LOONG_XR4);
  } else {
    orc_lsx_insn_emit_vsllwilwh (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
    orc_lsx_insn_emit_vsllwilwh (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
    orc_lsx_insn_emit_vmulw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR4);
  }
}

static void
orc_lasx_rule_mulslq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_vext2xvdw (c, ORC_LOONG_XR0, src1);
    orc_lasx_insn_emit_vext2xvdw (c, ORC_LOONG_XR4, src2);
    orc_lasx_insn_emit_xvmuld (c, dest, ORC_LOONG_XR0, ORC_LOONG_XR4);
  } else {
    orc_lsx_insn_emit_vsllwildw (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
    orc_lsx_insn_emit_vsllwildw (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
    orc_lsx_insn_emit_vmuld (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR4);
  }
}

static void
orc_lasx_rule_mulhsb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmuhb (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmuhb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulhsw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmuhh (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmuhh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulhsl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmuhw (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmuhw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulhub (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmuhbu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmuhbu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulhuw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmuhhu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmuhhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulhul (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvmuhwu (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vmuhwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_mulubw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_vext2xvhubu (c, ORC_LOONG_XR0, src1);
    orc_lasx_insn_emit_vext2xvhubu (c, ORC_LOONG_XR4, src2);
    orc_lasx_insn_emit_xvmulh (c, dest, ORC_LOONG_XR0, ORC_LOONG_XR4);
  } else {
    orc_lsx_insn_emit_vsllwilhubu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
    orc_lsx_insn_emit_vsllwilhubu (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
    orc_lsx_insn_emit_vmulh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR4);
  }
}

static void
orc_lasx_rule_muluwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_vext2xvwuhu (c, ORC_LOONG_XR0, src1);
    orc_lasx_insn_emit_vext2xvwuhu (c, ORC_LOONG_XR4, src2);
    orc_lasx_insn_emit_xvmulw (c, dest, ORC_LOONG_XR0, ORC_LOONG_XR4);
  } else {
    orc_lsx_insn_emit_vsllwilwuhu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
    orc_lsx_insn_emit_vsllwilwuhu (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
    orc_lsx_insn_emit_vmulw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR4);
  }
}

static void
orc_lasx_rule_mululq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_vext2xvduwu (c, ORC_LOONG_XR0, src1);
    orc_lasx_insn_emit_vext2xvduwu (c, ORC_LOONG_XR4, src2);
    orc_lasx_insn_emit_xvmuld (c, dest, ORC_LOONG_XR0, ORC_LOONG_XR4);
  } else {
    orc_lsx_insn_emit_vsllwilduwu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
    orc_lsx_insn_emit_vsllwilduwu (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
    orc_lsx_insn_emit_vmuld (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR4);
  }
}

static void
orc_lasx_rule_accw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  if (c->loop_shift != 4) {
      orc_lsx_insn_emit_vxorv (c, ORC_LOONG_VR0, ORC_LOONG_VR0, ORC_LOONG_VR0);
  }

  switch (c->loop_shift) {
    case 4:
      orc_lasx_insn_emit_xvaddh (c, dest, src1, dest);
      break;
    case 3:
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddh (c, dest, src1, dest);
      break;
    case 2:
      orc_lsx_insn_emit_vbsllv (c, ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1), 8);
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddh (c, dest, src1, dest);
      break;
    case 1:
      orc_lsx_insn_emit_vbsllv (c, ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1), 12);
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddh (c, dest, src1, dest);
      break;
    case 0:
      orc_lsx_insn_emit_vbsllv (c, ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1), 14);
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddh (c, dest, src1, dest);
      break;
    default:
      orc_compiler_error (c, "accw bad loop_shift %d", c->loop_shift);
      break;
  }
}

static void
orc_lasx_rule_accl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  if (c->loop_shift != 3) {
      orc_lsx_insn_emit_vxorv (c, ORC_LOONG_VR0, ORC_LOONG_VR0, ORC_LOONG_VR0);
  }

  switch (c->loop_shift) {
    case 3:
      orc_lasx_insn_emit_xvaddw (c, dest, src1, dest);
      break;
    case 2:
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddw (c, dest, src1, dest);
      break;
    case 1:
      orc_lsx_insn_emit_vbsllv (c, ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1), 8);
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddw (c, dest, src1, dest);
      break;
    case 0:
      orc_lsx_insn_emit_vbsllv (c, ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1), 12);
      orc_lasx_insn_emit_xvpermiq (c, src1, ORC_LOONG_XR0, 0x2);
      orc_lasx_insn_emit_xvaddw (c, dest, src1, dest);
      break;
    default:
      orc_compiler_error (c, "accl bad loop_shift %d", c->loop_shift);
      break;
  }
}

static void
orc_lasx_rule_accsadubl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  switch (c->loop_shift) {
    case 3:
      orc_lasx_insn_emit_vext2xvwubu (c, ORC_LOONG_XR0, src1);
      orc_lasx_insn_emit_vext2xvwubu (c, ORC_LOONG_XR4, src2);
      orc_lasx_insn_emit_xvabsdwu (c, ORC_LOONG_XR0, ORC_LOONG_XR0, ORC_LOONG_XR4);
      orc_lasx_insn_emit_xvsaddwu (c, dest, dest, ORC_LOONG_XR0);
      break;
    case 0:
    case 1:
    case 2:
      orc_lsx_insn_emit_vsllwilhubu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1), 0);
      orc_lsx_insn_emit_vsllwilhubu (c, ORC_LOONG_VR4, ORC_LASX_TO_LSX_REG(src2), 0);
      orc_lsx_insn_emit_vsllwilwuhu (c, ORC_LOONG_VR0, ORC_LOONG_VR0, 0);
      orc_lsx_insn_emit_vsllwilwuhu (c, ORC_LOONG_VR4, ORC_LOONG_VR4, 0);
      orc_lsx_insn_emit_vabsdwu (c, ORC_LOONG_VR0, ORC_LOONG_VR0, ORC_LOONG_VR4);

      if (c->loop_shift == 0) {
        orc_lsx_insn_emit_vbsllv (c, ORC_LOONG_VR0, ORC_LOONG_VR0, 12);
      } else if (c->loop_shift == 1) {
        orc_lsx_insn_emit_vbsllv (c, ORC_LOONG_VR0, ORC_LOONG_VR0, 8);
      }
      orc_lsx_insn_emit_vxorv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, ORC_LOONG_VR4);
      orc_lasx_insn_emit_xvpermiq (c, ORC_LOONG_XR0, ORC_LOONG_XR4, 0x2);
      orc_lasx_insn_emit_xvsaddwu (c, dest, dest, ORC_LOONG_XR0);
      break;

    default:
      orc_compiler_error (c, "accsadubl bad loop_shift %d", c->loop_shift);
      break;
  }
}

static void
orc_lasx_rule_mergebw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vilvlb (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
    orc_lsx_insn_emit_vilvhb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vilvlb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_mergewl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vilvlh (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
    orc_lsx_insn_emit_vilvhh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vilvlh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_mergelq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vilvlw (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
    orc_lsx_insn_emit_vilvhw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vilvlw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpgtsb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsltb (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vsltb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpgtsw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvslth (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vslth (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpgtsl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsltw (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vsltw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpgtsq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsltd (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vsltd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpeqb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvseqb (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vseqb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpeqw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvseqh (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vseqh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpeql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvseqw (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vseqw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_cmpeqq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvseqd (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vseqd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_select0wb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlnibh (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlnibh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_select0lw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlnihw (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlnihw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_select0ql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlniwd (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlniwd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_select1wb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlnibh (c, dest, src, 8);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlnibh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 8);
  }
}

static void
orc_lasx_rule_select1lw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlnihw (c, dest, src, 16);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlnihw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 16);
  }
}

static void
orc_lasx_rule_select1ql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlniwd (c, dest, src, 32);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlniwd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 32);
  }
}

static void
orc_lasx_rule_splatw3q (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvrepl128veih (c, ORC_LOONG_XR0, src, 3);
    orc_lasx_insn_emit_xvrepl128veih (c, dest, src, 7);
    orc_lasx_insn_emit_xvilvld(c, dest, dest, ORC_LOONG_XR0);
  } else {
    orc_lsx_insn_emit_vreplveih (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 3);
    orc_lsx_insn_emit_vreplveih (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 7);
    orc_lsx_insn_emit_vilvld (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0);
  }
}

static void
orc_lasx_rule_splatbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vilvlb (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
    orc_lsx_insn_emit_vilvhb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
     orc_lsx_insn_emit_vilvlb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
  }
}

static void
orc_lasx_rule_splatbl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 8) {
    orc_lsx_insn_emit_vilvlb (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
    orc_lsx_insn_emit_vilvlh (c, ORC_LOONG_VR4, ORC_LOONG_VR0, ORC_LOONG_VR0);
    orc_lsx_insn_emit_vilvhh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LOONG_VR0, ORC_LOONG_VR0);
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR4, 0x20);
  } else {
    orc_lsx_insn_emit_vilvlb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
    orc_lsx_insn_emit_vilvlh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), ORC_LASX_TO_LSX_REG(src));
  }
}

static void
orc_lasx_rule_splitwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest1 = ORC_DEST_ARG (c, insn, 0);
  const int dest2 = ORC_DEST_ARG (c, insn, 1);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlnibh (c, dest1, src, 8);
    orc_lasx_insn_emit_xvsrlnibh (c, dest2, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest1, dest1, 0x8);
    orc_lasx_insn_emit_xvpermid (c, dest2, dest2, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlnibh (c, ORC_LASX_TO_LSX_REG(dest1), ORC_LASX_TO_LSX_REG(src), 8);
    orc_lsx_insn_emit_vsrlnibh (c, ORC_LASX_TO_LSX_REG(dest2), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_splitlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest1 = ORC_DEST_ARG (c, insn, 0);
  const int dest2 = ORC_DEST_ARG (c, insn, 1);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlnihw (c, dest1, src, 16);
    orc_lasx_insn_emit_xvsrlnihw (c, dest2, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest1, dest1, 0x8);
    orc_lasx_insn_emit_xvpermid (c, dest2, dest2, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlnihw (c, ORC_LASX_TO_LSX_REG(dest1), ORC_LASX_TO_LSX_REG(src), 16);
    orc_lsx_insn_emit_vsrlnihw (c, ORC_LASX_TO_LSX_REG(dest2), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_splitql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest1 = ORC_DEST_ARG (c, insn, 0);
  const int dest2 = ORC_DEST_ARG (c, insn, 1);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvsrlniwd (c, dest1, src, 32);
    orc_lasx_insn_emit_xvsrlniwd (c, dest2, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest1, dest1, 0x8);
    orc_lasx_insn_emit_xvpermid (c, dest2, dest2, 0x8);
  } else {
    orc_lsx_insn_emit_vsrlniwd (c, ORC_LASX_TO_LSX_REG(dest1), ORC_LASX_TO_LSX_REG(src), 32);
    orc_lsx_insn_emit_vsrlniwd (c, ORC_LASX_TO_LSX_REG(dest2), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_swapw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvshuf4ib (c, dest, src, 0xB1);
  } else {
    orc_lsx_insn_emit_vshuf4ib (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0xB1);
  }
}

static void
orc_lasx_rule_swapl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvshuf4ib (c, dest, src, 0x1B);
  } else {
    orc_lsx_insn_emit_vshuf4ib (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0x1B);
  }
}

static void
orc_lasx_rule_swapq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvshuf4ih (c, dest, src, 0x4E);
    orc_lasx_insn_emit_xvshuf4ib (c, dest, dest, 0x1B);
  } else {
    orc_lsx_insn_emit_vshuf4ih (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0x4E);
    orc_lsx_insn_emit_vshuf4ib (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(dest), 0x1B);
  }
}

static void
orc_lasx_rule_swapwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvshuf4ih (c, dest, src, 0xB1);
  } else {
    orc_lsx_insn_emit_vshuf4ih (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0xB1);
  }
}

static void
orc_lasx_rule_swaplq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvshuf4iw (c, dest, src, 0xB1);
  } else {
    orc_lsx_insn_emit_vshuf4iw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0xB1);
  }
}

static void
orc_lasx_rule_andnX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvandnv (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vandnv (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_orX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvorv (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vorv (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_xorX (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvxorv (c, dest, src2, src1);
  } else {
    orc_lsx_insn_emit_vxorv (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src2), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_shlb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsllib (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsllib (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsllb (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsllb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsllih (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsllih (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsllh (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsllh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shll (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvslliw (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vslliw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsllw (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsllw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shlq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsllid (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsllid (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvslld (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vslld (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shrsb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsraib (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsraib (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrab (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrab (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shrsw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsraih (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsraih (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrah (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrah (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shrsl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsraiw (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsraiw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsraw (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsraw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shrsq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsraid (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsraid (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrad (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrad (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shrub (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrlib (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrlib (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrlb (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrlb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shruw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrlih (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrlih (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrlh (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrlh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shrul (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrliw (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrliw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrlw (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrlw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_shruq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  switch (ORC_SRC_TYPE (c, insn, 1)) {
    case ORC_VAR_TYPE_CONST:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrlid (c, dest, src1, ORC_SRC_VAL (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrlid (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_SRC_VAL (c, insn, 1));
      }
      break;
    case ORC_VAR_TYPE_TEMP:
    case ORC_VAR_TYPE_PARAM:
      if (size >= 32) {
        orc_lasx_insn_emit_xvsrld (c, dest, src1, ORC_SRC_ARG (c, insn, 1));
      } else {
        orc_lsx_insn_emit_vsrld (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(ORC_SRC_ARG (c, insn, 1)));
      }
      break;
    default:
      ORC_PROGRAM_ERROR (c, "shift rule only works with constants, temps and params");
      break;
  }
}

static void
orc_lasx_rule_convnwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvsrarnibh (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrarnibh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convnlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvsrarnihw (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrarnihw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convnql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvsrarniwd (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsrarniwd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convwbw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vsllwilhb (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 0);
    orc_lsx_insn_emit_vexthhb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vsllwilhb (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convwwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vsllwilwh (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 0);
    orc_lsx_insn_emit_vexthwh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vsllwilwh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convwlq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vsllwildw (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 0);
    orc_lsx_insn_emit_vexthdw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vsllwildw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convubw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vsllwilhubu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 0);
    orc_lsx_insn_emit_vexthhubu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vsllwilhubu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convuwl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vsllwilwuhu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 0);
    orc_lsx_insn_emit_vexthwuhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vsllwilwuhu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convulq (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lsx_insn_emit_vsllwilduwu (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src), 0);
    orc_lsx_insn_emit_vexthduwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vsllwilduwu (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convhwb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvsranibh (c, dest, src, 8);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsranibh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 8);
  }
}

static void
orc_lasx_rule_convhlw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvsranihw (c, dest, src, 16);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vsranihw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 16);
  }
}

static void
orc_lasx_rule_convssswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssranibh (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssranibh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convssslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssranihw (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssranihw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convsssql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssraniwd (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssraniwd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convsuswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssranibuh (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssranibuh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convsuslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssranihuw (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssranihuw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convsusql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssraniwud (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssraniwud (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convuuswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssrlnibuh (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssrlnibuh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convuuslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssrlnihuw (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssrlnihuw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convuusql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssrlniwud (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssrlniwud (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convusswb (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssrlnibh (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssrlnibh (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convusslw (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssrlnihw (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssrlnihw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convussql (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 16) {
    orc_lasx_insn_emit_xvssrlniwd (c, dest, src, 0);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vssrlniwd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src), 0);
  }
}

static void
orc_lasx_rule_convlf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvffintsw (c, dest, src1);
  } else {
    orc_lsx_insn_emit_vffintsw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_convld (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;
  if (size >= 16) {
    orc_lsx_insn_emit_vffintldw (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1));
    orc_lsx_insn_emit_vffinthdw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vffintldw (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_convfl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;
  if (size >= 32) {
    orc_lasx_insn_emit_xvftintrzws (c, dest, src1);
  } else {
    orc_lsx_insn_emit_vftintrzws (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_convfd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;
  if (size >= 16) {
    orc_lsx_insn_emit_vfcvtlds (c, ORC_LOONG_VR0, ORC_LASX_TO_LSX_REG(src1));
    orc_lsx_insn_emit_vfcvthds (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1));
    orc_lasx_insn_emit_xvpermiq (c, dest, ORC_LOONG_XR0, 0x20);
  } else {
    orc_lsx_insn_emit_vfcvtlds (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_convdl (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int dest = ORC_DEST_ARG (c, insn, 0);

  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;
  if (size >= 32) {
    orc_lasx_insn_emit_xvftintrnewd (c, dest, src1, src1);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
  } else {
    orc_lsx_insn_emit_vftintrnewd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1));
  }
}

static void
orc_lasx_rule_convdf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcvtsd (c, dest, src1, src1);
    orc_lasx_insn_emit_xvpermid (c, dest, dest, 0x8);
    orc_lasx_insn_emit_normalize (c, dest, dest, 4);
  } else {
    orc_lsx_insn_emit_vfcvtsd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src1));
    orc_lsx_insn_emit_normalize (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(dest), 4);
  }
}

static void
orc_lasx_rule_addf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfadds (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfadds (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 4);
  }
}

static void
orc_lasx_rule_addd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfaddd (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfaddd (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 8);
  }
}

static void
orc_lasx_rule_subf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfsubs (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfsubs (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 4);
  }
}

static void
orc_lasx_rule_subd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfsubd (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfsubd (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 8);
  }
}

static void
orc_lasx_rule_mulf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfmuls (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfmuls (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 4);
  }
}

static void
orc_lasx_rule_muld (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfmuld (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfmuld (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 8);
  }
}

static void
orc_lasx_rule_divf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfdivs (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfdivs (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 4);
  }
}

static void
orc_lasx_rule_divd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfdivd (c, dest, src1, src2);
    orc_lasx_insn_emit_normalize (c, dest, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfdivd (c, dest, src1, src2);
    orc_lsx_insn_emit_normalize (c, dest, dest, 8);
  }
}

static void
orc_lasx_rule_sqrtf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfsqrts (c, dest, src1);
    orc_lasx_insn_emit_normalize (c, dest, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfsqrts (c, dest, src1);
    orc_lsx_insn_emit_normalize (c, dest, dest, 4);
  }
}

static void
orc_lasx_rule_sqrtd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfsqrtd (c, dest, src1);
    orc_lasx_insn_emit_normalize (c, dest, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfsqrtd (c, dest, src1);
    orc_lsx_insn_emit_normalize (c, dest, dest, 8);
  }
}

static void
orc_lasx_rule_cmpeqf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcmpceqs (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vfcmpceqs (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_cmpeqd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcmpceqd (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vfcmpceqd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_cmpltf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcmpclts (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vfcmpclts (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_cmpltd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcmpcltd (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vfcmpcltd (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_cmplef (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcmpcles (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vfcmpcles (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_cmpled (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int src1 = ORC_SRC_ARG (c, insn, 0);
  const int src2 = ORC_SRC_ARG (c, insn, 1);
  const int dest = ORC_DEST_ARG (c, insn, 0);
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    orc_lasx_insn_emit_xvfcmpcled (c, dest, src1, src2);
  } else {
    orc_lsx_insn_emit_vfcmpcled (c, ORC_LASX_TO_LSX_REG(dest), ORC_LASX_TO_LSX_REG(src1), ORC_LASX_TO_LSX_REG(src2));
  }
}

static void
orc_lasx_rule_minf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfmins (c, ORC_LOONG_XR4, src1, src2);
    orc_lasx_insn_emit_xvfcmpcuns (c, ORC_LOONG_XR5, src1, src1);
    orc_lasx_insn_emit_xvfcmpcuns (c, ORC_LOONG_XR6, src2, src2);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src1, ORC_LOONG_XR5);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src2, ORC_LOONG_XR6);
    orc_lasx_insn_emit_normalize (c, ORC_LOONG_XR4, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfmins (c, ORC_LOONG_VR4, src1, src2);
    orc_lsx_insn_emit_vfcmpcuns (c, ORC_LOONG_VR5, src1, src1);
    orc_lsx_insn_emit_vfcmpcuns (c, ORC_LOONG_VR6, src2, src2);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src1, ORC_LOONG_VR5);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src2, ORC_LOONG_VR6);
    orc_lsx_insn_emit_normalize (c, ORC_LOONG_VR4, dest, 4);
  }
}

static void
orc_lasx_rule_mind (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfmind (c, ORC_LOONG_XR4, src1, src2);
    orc_lasx_insn_emit_xvfcmpcund (c, ORC_LOONG_XR5, src1, src1);
    orc_lasx_insn_emit_xvfcmpcund (c, ORC_LOONG_XR6, src2, src2);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src1, ORC_LOONG_XR5);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src2, ORC_LOONG_XR6);
    orc_lasx_insn_emit_normalize (c, ORC_LOONG_XR4, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfmind (c, ORC_LOONG_VR4, src1, src2);
    orc_lsx_insn_emit_vfcmpcund (c, ORC_LOONG_VR5, src1, src1);
    orc_lsx_insn_emit_vfcmpcund (c, ORC_LOONG_VR6, src2, src2);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src1, ORC_LOONG_VR5);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src2, ORC_LOONG_VR6);
    orc_lsx_insn_emit_normalize (c, ORC_LOONG_VR4, dest, 8);
  }
}

static void
orc_lasx_rule_maxf (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 4);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfmaxs (c, ORC_LOONG_XR4, src1, src2);
    orc_lasx_insn_emit_xvfcmpcuns (c, ORC_LOONG_XR5, src1, src1);
    orc_lasx_insn_emit_xvfcmpcuns (c, ORC_LOONG_XR6, src2, src2);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src1, ORC_LOONG_XR5);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src2, ORC_LOONG_XR6);
    orc_lasx_insn_emit_normalize (c, ORC_LOONG_XR4, dest, 4);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 4);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 4);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfmaxs (c, ORC_LOONG_VR4, src1, src2);
    orc_lsx_insn_emit_vfcmpcuns (c, ORC_LOONG_VR5, src1, src1);
    orc_lsx_insn_emit_vfcmpcuns (c, ORC_LOONG_VR6, src2, src2);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src1, ORC_LOONG_VR5);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src2, ORC_LOONG_VR6);
    orc_lsx_insn_emit_normalize (c, ORC_LOONG_VR4, dest, 4);
  }
}

static void
orc_lasx_rule_maxd (OrcCompiler *c, void *user, OrcInstruction *insn)
{
  const int size = c->vars[insn->src_args[0]].size << c->loop_shift;

  if (size >= 32) {
    const int src1 = NORMALIZE_SRC_ARG (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG (c, insn, 1, 8);
    const int dest = ORC_DEST_ARG (c, insn, 0);

    orc_lasx_insn_emit_xvfmaxd (c, ORC_LOONG_XR4, src1, src2);
    orc_lasx_insn_emit_xvfcmpcund (c, ORC_LOONG_XR5, src1, src1);
    orc_lasx_insn_emit_xvfcmpcund (c, ORC_LOONG_XR6, src2, src2);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src1, ORC_LOONG_XR5);
    orc_lasx_insn_emit_xvbitselv (c, ORC_LOONG_XR4, ORC_LOONG_XR4, src2, ORC_LOONG_XR6);
    orc_lasx_insn_emit_normalize (c, ORC_LOONG_XR4, dest, 8);
  } else {
    const int src1 = NORMALIZE_SRC_ARG1 (c, insn, 0, 8);
    const int src2 = NORMALIZE_SRC_ARG1 (c, insn, 1, 8);
    const int dest = ORC_LASX_TO_LSX_REG (ORC_DEST_ARG (c, insn, 0));

    orc_lsx_insn_emit_vfmaxd (c, ORC_LOONG_VR4, src1, src2);
    orc_lsx_insn_emit_vfcmpcund (c, ORC_LOONG_VR5, src1, src1);
    orc_lsx_insn_emit_vfcmpcund (c, ORC_LOONG_VR6, src2, src2);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src1, ORC_LOONG_VR5);
    orc_lsx_insn_emit_vbitselv (c, ORC_LOONG_VR4, ORC_LOONG_VR4, src2, ORC_LOONG_VR6);
    orc_lsx_insn_emit_normalize (c, ORC_LOONG_VR4, dest, 8);
  }
}

#define REG(opcode, rule, size) \
  orc_rule_register (rule_set, #opcode, orc_lasx_rule_##rule, (void*)size)

void
orc_lasx_rules_init (OrcTarget *target)
{
  OrcRuleSet *rule_set =
      orc_rule_set_new (orc_opcode_set_get ("sys"), target, ORC_TARGET_LOONGARCH_LASX);

  REG (loadpb, loadpX, 1);
  REG (loadpw, loadpX, 2);
  REG (loadpl, loadpX, 4);
  REG (loadpq, loadpX, 8);

  REG (loadb, loadX, NULL);
  REG (loadw, loadX, NULL);
  REG (loadl, loadX, NULL);
  REG (loadq, loadX, NULL);
  REG (loadupdb, loadupdb, NULL);

  REG (storeb, storeX, NULL);
  REG (storew, storeX, NULL);
  REG (storel, storeX, NULL);
  REG (storeq, storeX, NULL);

  REG (copyb, copyX, NULL);
  REG (copyw, copyX, NULL);
  REG (copyl, copyX, NULL);
  REG (copyq, copyX, NULL);

  REG (addb, addb, NULL);
  REG (addw, addw, NULL);
  REG (addl, addl, NULL);
  REG (addq, addq, NULL);

  REG (addssb, addssb, NULL);
  REG (addssw, addssw, NULL);
  REG (addssl, addssl, NULL);

  REG (addusb, addusb, NULL);
  REG (addusw, addusw, NULL);
  REG (addusl, addusl, NULL);

  REG (subb, subb, NULL);
  REG (subw, subw, NULL);
  REG (subl, subl, NULL);
  REG (subq, subq, NULL);

  REG (subssb, subssb, NULL);
  REG (subssw, subssw, NULL);
  REG (subssl, subssl, NULL);

  REG (subusb, subusb, NULL);
  REG (subusw, subusw, NULL);
  REG (subusl, subusl, NULL);

  REG (andb, andX, NULL);
  REG (andw, andX, NULL);
  REG (andl, andX, NULL);
  REG (andq, andX, NULL);
  REG (andf, andX, NULL);

  REG (minsb, minb, NULL);
  REG (minsw, minw, NULL);
  REG (minsl, minl, NULL);

  REG (minub, minub, NULL);
  REG (minuw, minuw, NULL);
  REG (minul, minul, NULL);

  REG (maxsb, maxb, NULL);
  REG (maxsw, maxw, NULL);
  REG (maxsl, maxl, NULL);

  REG (maxub, maxub, NULL);
  REG (maxuw, maxuw, NULL);
  REG (maxul, maxul, NULL);

  REG (absb, absb, NULL);
  REG (absw, absw, NULL);
  REG (absl, absl, NULL);

  REG (avgsb, avgsb, NULL);
  REG (avgsw, avgsw, NULL);
  REG (avgsl, avgsl, NULL);

  REG (avgub, avgub, NULL);
  REG (avguw, avguw, NULL);
  REG (avgul, avgul, NULL);

  REG (signb, signb, NULL);
  REG (signw, signw, NULL);
  REG (signl, signl, NULL);

  REG (mullb, mullb, NULL);
  REG (mullw, mullw, NULL);
  REG (mulll, mulll, NULL);

  REG (mulsbw, mulsbw, NULL);
  REG (mulswl, mulswl, NULL);
  REG (mulslq, mulslq, NULL);

  REG (mulhsb, mulhsb, NULL);
  REG (mulhsw, mulhsw, NULL);
  REG (mulhsl, mulhsl, NULL);

  REG (mulhub, mulhub, NULL);
  REG (mulhuw, mulhuw, NULL);
  REG (mulhul, mulhul, NULL);

  REG (mulubw, mulubw, NULL);
  REG (muluwl, muluwl, NULL);
  REG (mululq, mululq, NULL);

  REG (accw, accw, NULL);
  REG (accl, accl, NULL);
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
  REG (convuuslw, convuuslw, NULL);
  REG (convuusql, convuusql, NULL);

  REG (convusswb, convusswb, NULL);
  REG (convusslw, convusslw, NULL);
  REG (convussql, convussql, NULL);

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
