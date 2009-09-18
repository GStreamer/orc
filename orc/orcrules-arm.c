
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcarm.h>

#define BINARY_DP(opcode,insn_name) \
static void \
arm_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  int src1 = ORC_SRC_ARG (p, insn, 0);                                 \
  int src2 = ORC_SRC_ARG (p, insn, 1);                                 \
  int dest = ORC_DEST_ARG (p, insn, 0);                                \
                                                                       \
  orc_arm_emit_ ##insn_name## _r (p, ORC_ARM_COND_AL, 0,               \
          dest, src1, src2);                                           \
}

#define BINARY_MM(opcode,insn_name) \
static void \
arm_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  int src1 = ORC_SRC_ARG (p, insn, 0);                                 \
  int src2 = ORC_SRC_ARG (p, insn, 1);                                 \
  int dest = ORC_DEST_ARG (p, insn, 0);                                \
                                                                       \
  orc_arm_emit_##insn_name (p, ORC_ARM_COND_AL, dest, src1, src2);     \
}

#define arm_emit_sxtb(cond,Rd,rot,Rm) (0x06af0070|((cond)<<28)|((Rd)<<12)|((rot)<<10)|(Rm))
#define arm_emit_sxth(cond,Rd,rot,Rm) (0x06bf0070|((cond)<<28)|((Rd)<<12)|((rot)<<10)|(Rm))
#define arm_emit_uxtb(cond,Rd,rot,Rm) (0x06ef0070|((cond)<<28)|((Rd)<<12)|((rot)<<10)|(Rm))
#define arm_emit_uxth(cond,Rd,rot,Rm) (0x06ff0070|((cond)<<28)|((Rd)<<12)|((rot)<<10)|(Rm))

void
orc_arm_loadw (OrcCompiler *compiler, int dest, int src1, int offset)
{
  uint32_t code;

  code = 0xe1d000b0;
  code |= (src1&0xf) << 16;
  code |= (dest&0xf) << 12;
  code |= (offset&0xf0) << 4;
  code |= offset&0x0f;

  ORC_ASM_CODE(compiler,"  ldrh %s, [%s, #%d]\n",
      orc_arm_reg_name (dest),
      orc_arm_reg_name (src1), offset);
  orc_arm_emit (compiler, code);
}

void
orc_arm_storew (OrcCompiler *compiler, int dest, int offset, int src1)
{
  uint32_t code;

  code = 0xe1c000b0;
  code |= (dest&0xf) << 16;
  code |= (src1&0xf) << 12;
  code |= (offset&0xf0) << 4;
  code |= offset&0x0f;

  ORC_ASM_CODE(compiler,"  strh %s, [%s, #%d]\n",
      orc_arm_reg_name (src1),
      orc_arm_reg_name (dest), offset);
  orc_arm_emit (compiler, code);
}

void
orc_arm_emit_mov_iw (OrcCompiler *p, int cond, int dest, int val, int loop)
{
  int shift = 0;

  while (val && ((val & 3) == 0)) {
    val >>= 2;
    shift++;
  }
  /* dest = val */
  orc_arm_emit_mov_i (p, cond, 0, dest, shift, val);
  if (loop > 1)
    /* 2 words:  dest |= dest << 16 */
    orc_arm_emit_orr_rsi (p, cond, 0, dest, dest, dest, ORC_ARM_LSL, 16);
}

void
orc_arm_emit_mov_ib (OrcCompiler *p, int cond, int dest, int val, int loop)
{
  int shift = 0;

  while (val && ((val & 3) == 0)) {
    val >>= 2;
    shift++;
  }
  /* 1 byte */
  orc_arm_emit_mov_i (p, cond, 0, dest, shift, val);
  if (loop > 1)
    /* 2 bytes:  dest |= dest << 8 */
    orc_arm_emit_orr_rsi (p, cond, 0, dest, dest, dest, ORC_ARM_LSL, 8);
  if (loop > 2)
    /* 4 bytes:  dest |= dest << 16 */
    orc_arm_emit_orr_rsi (p, cond, 0, dest, dest, dest, ORC_ARM_LSL, 16);
}

/* byte instructions */
static void
arm_rule_absX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int tmp = p->tmpreg;
  int type = ORC_PTR_TO_INT(user);

  orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, dest, 0, 0);

  if (type == 0) {
    /* negate tmp = 0 - src1 */
    orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, tmp, dest, src1);
    /* check sign dest = src1 - 0 */
    orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, dest, src1, dest);
  } else {
    /* negate tmp = 0 - src1 */
    orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, tmp, dest, src1);
    /* check sign dest = src1 - 0 */
    orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, dest, src1, dest);
  }

  /* take positive or negative values */
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src1, tmp);
}
BINARY_MM (addb, sadd8);
BINARY_MM (addssb, qadd8);
BINARY_MM (addusb, uqadd8);
BINARY_DP (andX, and);
BINARY_DP (andnX, bic);
static void
arm_rule_avgX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int mask = p->tmpreg;
  int tmp = ORC_ARM_IP;
  int dest = ORC_DEST_ARG (p, insn, 0);
  int type = ORC_PTR_TO_INT(user);

  /* signed variant, make a mask, FIXME, instruction constants */
  if (type >= 2) {
    /* mask for word 0x80008000 */
    orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, mask, 4, 8);
    orc_arm_emit_orr_rsi (p, ORC_ARM_COND_AL, 0, mask, mask, mask, ORC_ARM_LSR, 16);

    if (type >= 3) {
      /* mask for byte 0x80808080 */
      orc_arm_emit_orr_rsi (p, ORC_ARM_COND_AL, 0, mask, mask, mask, ORC_ARM_LSR, 8);
    }

    /* signed variant, bias the inputs */
    orc_arm_emit_eor_r (p, ORC_ARM_COND_AL, 0, src1, src1, mask);
    orc_arm_emit_eor_r (p, ORC_ARM_COND_AL, 0, src2, src2, mask);
  }

  /* dest = (s1 | s2) - (((s1 ^ s2) & ~(0x1 >>> (shift*2))) >> 1) */
  /* (s1 | s2) */
  orc_arm_emit_orr_r (p, ORC_ARM_COND_AL, 0, tmp, src1, src2);
  /* (s1 ^ s2) */
  orc_arm_emit_eor_r (p, ORC_ARM_COND_AL, 0, dest, src1, src2);

  /* clear the bits we will shift into view in the next instruction, FIXME, we
   * need instruction wide constants */
  if (type <= 1) {
    /* clear 0x00010000 */
    orc_arm_emit_bic_i (p, ORC_ARM_COND_AL, 0, dest, dest, 8, 1);
    if (type == 0) {
      /* clear 0x00000100 */
      orc_arm_emit_bic_i (p, ORC_ARM_COND_AL, 0, dest, dest, 12, 1);
      /* clear 0x01000000 */
      orc_arm_emit_bic_i (p, ORC_ARM_COND_AL, 0, dest, dest, 4, 1);
    }
  } else if (type >= 2) {
    /* already have a mask, use it here */
    orc_arm_emit_bic_rsi (p, ORC_ARM_COND_AL, 0, dest, dest, mask, ORC_ARM_LSR, 7);
  }

  /* do final right shift and subtraction */
  orc_arm_emit_sub_rsi (p, ORC_ARM_COND_AL, 0, dest, tmp, dest, ORC_ARM_LSR, 1);

  if (type >= 2) {
    /* signed variant, unbias input again */
    orc_arm_emit_eor_r (p, ORC_ARM_COND_AL, 0, src1, src1, mask);
    orc_arm_emit_eor_r (p, ORC_ARM_COND_AL, 0, src2, src2, mask);
  }
}
static void
arm_rule_cmpeqX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int tmp = p->tmpreg;
  int size = ORC_PTR_TO_INT(user);

  /* bytes that are equal will have all bits 0 */
  orc_arm_emit_eor_r (p, ORC_ARM_COND_AL, 0, tmp, src1, src2);

  /* clear dest register */
  orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, dest, 0, 0);

  /* tmp = 0 - tmp, set GE flags for 0 bytes */
  if (size == 1) {
    orc_arm_emit_usub8 (p, ORC_ARM_COND_AL, tmp, dest, tmp);
  } else {
    orc_arm_emit_usub16 (p, ORC_ARM_COND_AL, tmp, dest, tmp);
  }

  /* make 0xffffffff in tmp */
  orc_arm_emit_mvn_i (p, ORC_ARM_COND_AL, 0, tmp, 0, 0);

  /* set 0xff for 0 bytes, 0x00 otherwise */
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, tmp, dest);
}
static void
arm_rule_cmpgtsX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int tmp = p->tmpreg;
  int size = ORC_PTR_TO_INT(user);

  /* dest = src2 - src1, set GE flags for src2 >= src1 */
  if (size == 1) {
    orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, dest, src2, src1);
  } else {
    orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, dest, src2, src1);
  }

  /* clear dest register */
  orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, dest, 0, 0);
  /* make 0xffffffff in tmp */
  orc_arm_emit_mvn_i (p, ORC_ARM_COND_AL, 0, tmp, 0, 0);

  /* set 0x00 for src2 >= src1 bytes, 0xff if src2 < src1 */
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, dest, tmp);
}
static void
arm_rule_copyX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_mov_r (p, ORC_ARM_COND_AL, 0, dest, src1);
}

static void
arm_rule_maxsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src1, src2);
}
static void
arm_rule_maxub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_usub8 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src1, src2);
}
static void
arm_rule_minsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src2, src1);
}
static void
arm_rule_minub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_usub8 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src2, src1);
}

#if 0
BINARY_SB(mullb, "(%s * %s) & 0xff")
BINARY_SB(mulhsb, "(%s * %s) >> 8")
BINARY_UB(mulhub, "((uint32_t)(uint8_t)%s * (uint32_t)(uint8_t)%s) >> 8")
#endif
BINARY_DP (orX, orr);
static void
arm_rule_shlX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* degrades nicely to trivial shift when not doing parallel shifts */
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int mask = p->tmpreg;
  int src2type = ORC_SRC_TYPE (p, insn, 1);
  int size = ORC_PTR_TO_INT(user);
  int loop = 4 / size; /* number of items in one register */

  if (src2type == ORC_VAR_TYPE_CONST) {
    int val = ORC_SRC_VAL (p, insn, 1);

    if (loop > 1 && size != 4 && val < 5) {
      for (;val; val--) {
        /* small values, do a series of additions, we need at least 5
         * instructions for the generic case below. */
        if (size == 1)
          orc_arm_emit_uadd8 (p, ORC_ARM_COND_AL, dest, src1, src1);
        else
          orc_arm_emit_uadd16 (p, ORC_ARM_COND_AL, dest, src1, src1);
      }
    }
    else {
      /* bigger values, shift and mask out excess bits */
      if (val >= size) {
        /* too big, clear all */
        orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, dest, 0, 0);
      } else if (val > 0) {
        if (loop > 1 && size < 4) {
          /* shift, note that we skip the next instructions when 0 */
          orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 1, dest, src1, ORC_ARM_LSL, val);
          if (size == 1)
            /* make loop * 0x80 */
            orc_arm_emit_mov_ib (p, ORC_ARM_COND_NE, mask, 0x80, loop);
          else
            /* make loop * 0x8000 */
            orc_arm_emit_mov_iw (p, ORC_ARM_COND_NE, mask, 0x8000, loop);
          /* make mask, this mask has enough bits but is shifted one position to the right */
          orc_arm_emit_sub_rsi (p, ORC_ARM_COND_NE, 0, mask, mask, mask, ORC_ARM_LSR, val);
          /* clear upper bits */
          orc_arm_emit_bic_rsi (p, ORC_ARM_COND_NE, 0, dest, dest, mask, ORC_ARM_LSL, 1);
        } else {
          orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 0, dest, src1, ORC_ARM_LSL, val);
        }
      }
    }
  } else if (src2type == ORC_VAR_TYPE_PARAM) {
    int src2 = ORC_SRC_ARG (p, insn, 1);

    if (loop > 1 && size < 4) {
      /* shift with register value, note that we skip the next instructions when 0 */
      orc_arm_emit_mov_rsr (p, ORC_ARM_COND_AL, 1, dest, src1, ORC_ARM_LSL, src2);
      if (size == 1)
        /* make loop * 0x80 */
        orc_arm_emit_mov_ib (p, ORC_ARM_COND_NE, mask, 0x80, loop);
      else
        /* make loop * 0x8000 */
        orc_arm_emit_mov_iw (p, ORC_ARM_COND_NE, mask, 0x8000, loop);
      /* make mask */
      orc_arm_emit_sub_rsr (p, ORC_ARM_COND_NE, 0, mask, mask, mask, ORC_ARM_LSR, src2);
      /* clear bits */
      orc_arm_emit_bic_rsi (p, ORC_ARM_COND_NE, 0, dest, dest, mask, ORC_ARM_LSL, 1);
    } else {
      orc_arm_emit_mov_rsr (p, ORC_ARM_COND_AL, 0, dest, src1, ORC_ARM_LSL, src2);
    }
  } else {
    ORC_COMPILER_ERROR(p,"rule only works with constants or parameters");
  }
}

static void
arm_rule_shrsX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* degrades nicely to trivial shift when not doing parallel shifts */
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int mask = p->tmpreg;
  int tmp = ORC_ARM_IP;
  int src2type = ORC_SRC_TYPE (p, insn, 1);
  int size = ORC_PTR_TO_INT(user);
  int loop = 4 / size; /* number of items in one register */

  if (src2type == ORC_VAR_TYPE_CONST) {
    int val = ORC_SRC_VAL (p, insn, 1);

    if (val > 0) {
      /* clamp max shift so we can sign extend */
      if (val >= size)
        val = size - 1;

      /* shift */
      if (size < 4) {
        if (size == 1)
          /* make loop * 80, position of sign bit after shift */
          orc_arm_emit_mov_ib (p, ORC_ARM_COND_AL, mask, 0x80, loop);
        else
          /* make loop * 8000 */
          orc_arm_emit_mov_iw (p, ORC_ARM_COND_AL, mask, 0x8000, loop);
        /* make mask, save in tmp, we need the original mask */
        orc_arm_emit_sub_rsi (p, ORC_ARM_COND_AL, 0, tmp, mask, mask, ORC_ARM_LSR, val);

        /* do the shift */
        orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 1, dest, src1, ORC_ARM_LSR, val);
        /* clear upper bits */
        orc_arm_emit_bic_rsi (p, ORC_ARM_COND_NE, 0, dest, dest, tmp, ORC_ARM_LSL, 1);

        /* flip sign bit */
        orc_arm_emit_eor_r (p, ORC_ARM_COND_NE, 0, dest, dest, mask);
        /* extend sign bits */
        if (size == 1)
          orc_arm_emit_usub8 (p, ORC_ARM_COND_NE, dest, dest, mask);
        else
          orc_arm_emit_usub16 (p, ORC_ARM_COND_NE, dest, dest, mask);
      } else {
        /* full word shift */
        orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 0, dest, src1, ORC_ARM_ASR, val);
      }
    }
  } else if (src2type == ORC_VAR_TYPE_PARAM) {
    int src2 = ORC_SRC_ARG (p, insn, 1);

    if (size < 4) {
      if (size == 1)
        /* make loop * 0x80 */
        orc_arm_emit_mov_ib (p, ORC_ARM_COND_AL, mask, 0x80, loop);
      else
        /* make loop * 0x8000 */
        orc_arm_emit_mov_iw (p, ORC_ARM_COND_AL, mask, 0x8000, loop);
      /* make mask */
      orc_arm_emit_sub_rsr (p, ORC_ARM_COND_AL, 0, tmp, mask, mask, ORC_ARM_LSR, src2);

      /* do the shift */
      orc_arm_emit_mov_rsr (p, ORC_ARM_COND_AL, 1, dest, src1, ORC_ARM_LSR, src2);
      /* clear upper bits */
      orc_arm_emit_bic_rsi (p, ORC_ARM_COND_NE, 0, dest, dest, tmp, ORC_ARM_LSL, 1);

      /* flip sign bit */
      orc_arm_emit_eor_r (p, ORC_ARM_COND_NE, 0, dest, dest, mask);
      /* extend sign bits */
      if (size == 1)
        orc_arm_emit_usub8 (p, ORC_ARM_COND_NE, dest, dest, mask);
      else
        orc_arm_emit_usub16 (p, ORC_ARM_COND_NE, dest, dest, mask);
    } else {
      /* full word shift with register value */
      orc_arm_emit_mov_rsr (p, ORC_ARM_COND_AL, 0, dest, dest, ORC_ARM_ASR, src2);
    }
  } else {
    ORC_COMPILER_ERROR(p,"rule only works with constants or parameters");
  }
}

static void
arm_rule_shruX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* degrades nicely to trivial shift when not doing parallel shifts */
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int mask = p->tmpreg;
  int src2type = ORC_SRC_TYPE (p, insn, 1);
  int size = ORC_PTR_TO_INT(user);
  int loop = 4 / size; /* number of items in one register */

  if (src2type == ORC_VAR_TYPE_CONST) {
    int val = ORC_SRC_VAL (p, insn, 1);

    /* shift and mask out excess bits */
    if (val >= size) {
      /* too big, clear all */
      orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, dest, 0, 0);
    } else if (val > 0) {
      if (size < 4) {
        /* do the shift */
        orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 1, dest, src1, ORC_ARM_LSR, val);

        if (size == 1)
          /* make loop * 0x80 */
          orc_arm_emit_mov_ib (p, ORC_ARM_COND_NE, mask, 0x80, loop);
        else
          /* make loop * 0x8000 */
          orc_arm_emit_mov_iw (p, ORC_ARM_COND_NE, mask, 0x8000, loop);
        /* make mask */
        orc_arm_emit_sub_rsi (p, ORC_ARM_COND_NE, 0, mask, mask, mask, ORC_ARM_LSR, val);

        /* clear upper bits */
        orc_arm_emit_bic_rsi (p, ORC_ARM_COND_NE, 0, dest, dest, mask, ORC_ARM_LSL, 1);
      } else {
        orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 0, dest, src1, ORC_ARM_LSR, val);
      }
    }
  } else if (src2type == ORC_VAR_TYPE_PARAM) {
    int src2 = ORC_SRC_ARG (p, insn, 1);

    if (size < 4) {
      /* shift with register value */
      orc_arm_emit_mov_rsr (p, ORC_ARM_COND_AL, 1, dest, src1, ORC_ARM_LSR, src2);

      if (size == 1)
        /* make loop * 0x80 */
        orc_arm_emit_mov_ib (p, ORC_ARM_COND_NE, mask, 0x80, loop);
      else
        /* make loop * 0x8000 */
        orc_arm_emit_mov_iw (p, ORC_ARM_COND_NE, mask, 0x8000, loop);
      /* mask mask */
      orc_arm_emit_sub_rsr (p, ORC_ARM_COND_NE, 0, mask, mask, mask, ORC_ARM_LSR, src2);

      /* clear bits */
      orc_arm_emit_bic_rsi (p, ORC_ARM_COND_NE, 0, dest, dest, mask, ORC_ARM_LSL, 1);
    } else {
      /* shift with register value */
      orc_arm_emit_mov_rsr (p, ORC_ARM_COND_AL, 0, dest, src1, ORC_ARM_LSR, src2);
    }
  } else {
    ORC_COMPILER_ERROR(p,"rule only works with constants or parameters");
  }
}

static void
arm_rule_signX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int zero = p->tmpreg;
  int ones = ORC_ARM_IP;
  int tmp = ORC_ARM_V8;
  int type = ORC_PTR_TO_INT(user);

  /* make 0 */
  orc_arm_emit_mov_i (p, ORC_ARM_COND_AL, 0, zero, 0, 0);
  /* make 0xffffffff */
  orc_arm_emit_mvn_i (p, ORC_ARM_COND_AL, 0, ones, 0, 0);

  /* dest = src1 - 0 (src1 >= 0 ? 0 : -1) */
  if (type == 0)
    orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, dest, src1, zero);
  else
    orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, dest, src1, zero);

  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, zero, ones);

  /* tmp = 0 - src1 (src1 <= 0 ? 0 : -1) */
  if (type == 0)
    orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, tmp, zero, src1);
  else
    orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, tmp, zero, src1);

  orc_arm_emit_sel (p, ORC_ARM_COND_AL, tmp, zero, ones);

  /* (src1 >= 0 ? 0 : -1) - (src1 <= 0 ? 0 : -1) */
  if (type == 0)
    orc_arm_emit_ssub8 (p, ORC_ARM_COND_AL, dest, dest, tmp);
  else
    orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, dest, dest, tmp);
}

BINARY_MM (subb, ssub8);
BINARY_MM (subssb, qsub8);
BINARY_MM (subusb, uqsub8);
BINARY_DP (xorX, eor);

BINARY_MM (addw, sadd16);
BINARY_MM (addssw, qadd16);
BINARY_MM (addusw, uqadd16);

static void
arm_rule_maxsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src1, src2);
}
static void
arm_rule_maxuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_usub16 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src1, src2);
}
static void
arm_rule_minsw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_ssub16 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src2, src1);
}
static void
arm_rule_minuw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  orc_arm_emit_usub16 (p, ORC_ARM_COND_AL, dest, src1, src2);
  orc_arm_emit_sel (p, ORC_ARM_COND_AL, dest, src2, src1);
}

static void
arm_rule_mullw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* BINARY_SW(mullw, "(%s * %s) & 0xffff") */
  uint32_t code;
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  code = 0xe0000090;
  code |= (dest & 0xf) << 16;
  code |= (src1 & 0xf) << 0;
  code |= (src2 & 0xf) << 8;

  ORC_ASM_CODE(p,"  mul %s, %s, %s\n",
      orc_arm_reg_name (dest),
      orc_arm_reg_name (src1),
      orc_arm_reg_name (src2));
  orc_arm_emit (p, code);
}
#if 0
BINARY_SW(mulhsw, "(%s * %s) >> 16")
BINARY_UW(mulhuw, "((uint32_t)((uint16_t)%s) * (uint32_t)((uint16_t)%s)) >> 16")
#endif
BINARY_MM (subw, ssub16);
BINARY_MM (subssw, qsub16);
BINARY_MM (subusw, uqsub16);

static void
arm_rule_absl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* reverse sign 0 - src1, assume the value is negative */
  orc_arm_emit_rsb_i (p, ORC_ARM_COND_AL, 1, dest, src1, 0, 0);

  /* if we got negative, copy the original value again */
  orc_arm_emit_mov_r (p, ORC_ARM_COND_MI, 0, dest, src1);
}

BINARY_DP (addl, add);
BINARY_MM (addssl, qadd);
static void
arm_rule_addusl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* add numbers */
  orc_arm_emit_add_r (p, ORC_ARM_COND_AL, 1, dest, src1, src2);

  /* on overflow, move ffffffff */
  orc_arm_emit_mvn_i (p, ORC_ARM_COND_CS, 0, dest, 0, 0);
}
static void
arm_rule_avgXl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* set the carry flag */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src1);

  /* src1 + src2 + 1 */
  orc_arm_emit_adc_r (p, ORC_ARM_COND_AL, 0, dest, src1, src2);

  /* rotate right, top bit is the carry */
  orc_arm_emit_mov_rrx (p, ORC_ARM_COND_AL, 0, dest, dest);
}
static void
arm_rule_cmpeql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* compare */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src2);

  /* set to all 0 when not equal */
  orc_arm_emit_mov_i (p, ORC_ARM_COND_NE, 0, dest, 0, 0);

  /* set to all ff when equal */
  orc_arm_emit_mvn_i (p, ORC_ARM_COND_EQ, 0, dest, 0, 0);
}
static void
arm_rule_cmpgtsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* compare */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src2);

  /* set to all 0 when less or equal */
  orc_arm_emit_mov_i (p, ORC_ARM_COND_LE, 0, dest, 0, 0);

  /* set to all ff when greater */
  orc_arm_emit_mvn_i (p, ORC_ARM_COND_GT, 0, dest, 0, 0);
}

static void
arm_rule_maxsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* compare */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src2);

  /* conditionally move result */
  orc_arm_emit_mov_r (p, ORC_ARM_COND_GE, 0, dest, src1);
  orc_arm_emit_mov_r (p, ORC_ARM_COND_LT, 0, dest, src2);
}
static void
arm_rule_maxul (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* compare */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src2);

  /* conditionally move result */
  orc_arm_emit_mov_r (p, ORC_ARM_COND_CS, 0, dest, src1);
  orc_arm_emit_mov_r (p, ORC_ARM_COND_CC, 0, dest, src2);
}
static void
arm_rule_minsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* compare */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src2);

  /* conditionally move result */
  orc_arm_emit_mov_r (p, ORC_ARM_COND_GE, 0, dest, src2);
  orc_arm_emit_mov_r (p, ORC_ARM_COND_LT, 0, dest, src1);
}
static void
arm_rule_minul (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int src2 = ORC_SRC_ARG (p, insn, 1);
  int dest = ORC_DEST_ARG (p, insn, 0);

  /* compare */
  orc_arm_emit_cmp_r (p, ORC_ARM_COND_AL, src1, src2);

  /* conditionally move result */
  orc_arm_emit_mov_r (p, ORC_ARM_COND_CS, 0, dest, src2);
  orc_arm_emit_mov_r (p, ORC_ARM_COND_CC, 0, dest, src1);
}

#if 0
BINARY_SL(mulll, "(%s * %s) & 0xffffffff")
BINARY_SL(mulhsl, "((int64_t)%s * (int64_t)%s) >> 32")
BINARY_UL(mulhul, "((uint64_t)%s * (uint64_t)%s) >> 32")
#endif
static void
arm_rule_signl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = ORC_SRC_ARG (p, insn, 0);
  int dest = ORC_DEST_ARG (p, insn, 0);
  int tmp = p->tmpreg;

  /* dest = 0 - src1 */
  orc_arm_emit_rsb_i (p, ORC_ARM_COND_AL, 0, dest, src1, 0, 0);

  /* move src1 sign into tmp */
  orc_arm_emit_mov_rsi (p, ORC_ARM_COND_AL, 0, tmp, src1, ORC_ARM_ASR, 31);

  /* dest = tmp - (dest >> 31) */
  orc_arm_emit_sub_rsi (p, ORC_ARM_COND_AL, 0, dest, tmp, dest, ORC_ARM_ASR, 31);
}
BINARY_DP (subl, sub);
BINARY_MM (subssl, qsub);
#if 0
BINARY_UL(subusl, "ORC_CLAMP_UL((int64_t)(uint32_t)%s - (int64_t)(uint32_t)%s)")

UNARY_BW(convsbw, "%s")
UNARY_BW(convubw, "(uint8_t)%s")
UNARY_WL(convswl, "%s")
UNARY_WL(convuwl, "(uint16_t)%s")
UNARY_WB(convwb, "%s")
UNARY_WB(convssswb, "ORC_CLAMP_SB(%s)")
UNARY_WB(convsuswb, "ORC_CLAMP_UB(%s)")
UNARY_WB(convusswb, "ORC_CLAMP_SB((uint16_t)%s)")
UNARY_WB(convuuswb, "ORC_CLAMP_UB((uint16_t)%s)")
UNARY_LW(convlw, "%s")
UNARY_LW(convssslw, "ORC_CLAMP_SW(%s)")
UNARY_LW(convsuslw, "ORC_CLAMP_UW(%s)")
UNARY_LW(convusslw, "ORC_CLAMP_SW((uint32_t)%s)")
UNARY_LW(convuuslw, "ORC_CLAMP_UW((uint32_t)%s)")

BINARY_BW(mulsbw, "%s * %s")
BINARY_BW(mulubw, "(uint8_t)%s * (uint8_t)%s")
BINARY_WL(mulswl, "%s * %s")
BINARY_WL(muluwl, "(uint16_t)%s * (uint16_t)%s")

BINARY_WL(mergewl, "((uint16_t)%s) | ((uint16_t)%s << 16)")
BINARY_BW(mergebw, "((uint8_t)%s) | ((uint8_t)%s << 8)")
UNARY_WB(select0wb, "(uint16_t)%s & 0xff")
UNARY_WB(select1wb, "((uint16_t)%s >> 8)&0xff")
UNARY_LW(select0lw, "(uint32_t)%s & 0xffff")
UNARY_LW(select1lw, "((uint32_t)%s >> 16)&0xffff")
UNARY_UW(swapw, "ORC_SWAP_W(%s)")
UNARY_UL(swapl, "ORC_SWAP_L(%s)")
#endif

void
orc_compiler_orc_arm_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target, 0);

  orc_rule_register (rule_set, "absb", arm_rule_absX, (void *)0);
  orc_rule_register (rule_set, "addb", arm_rule_addb, NULL);
  orc_rule_register (rule_set, "addssb", arm_rule_addssb, NULL);
  orc_rule_register (rule_set, "addusb", arm_rule_addusb, NULL);
  orc_rule_register (rule_set, "andb", arm_rule_andX, NULL);
  orc_rule_register (rule_set, "andnb", arm_rule_andnX, NULL);
  orc_rule_register (rule_set, "avgsb", arm_rule_avgX, (void *)3);
  orc_rule_register (rule_set, "avgub", arm_rule_avgX, (void *)0);
  orc_rule_register (rule_set, "cmpeqb", arm_rule_cmpeqX, (void *)1);
  orc_rule_register (rule_set, "cmpgtsb", arm_rule_cmpgtsX, (void *)1);
  orc_rule_register (rule_set, "copyb", arm_rule_copyX, NULL);
  orc_rule_register (rule_set, "maxsb", arm_rule_maxsb, NULL);
  orc_rule_register (rule_set, "maxub", arm_rule_maxub, NULL);
  orc_rule_register (rule_set, "minsb", arm_rule_minsb, NULL);
  orc_rule_register (rule_set, "minub", arm_rule_minub, NULL);
  orc_rule_register (rule_set, "orb", arm_rule_orX, NULL);
  orc_rule_register (rule_set, "shlb", arm_rule_shlX, (void *)1);
  orc_rule_register (rule_set, "shrsb", arm_rule_shrsX, (void *)1);
  orc_rule_register (rule_set, "shrub", arm_rule_shruX, (void *)1);
  orc_rule_register (rule_set, "signb", arm_rule_signX, (void *)0);
  orc_rule_register (rule_set, "subb", arm_rule_subb, NULL);
  orc_rule_register (rule_set, "subssb", arm_rule_subssb, NULL);
  orc_rule_register (rule_set, "subusb", arm_rule_subusb, NULL);
  orc_rule_register (rule_set, "xorb", arm_rule_xorX, NULL);

  orc_rule_register (rule_set, "absw", arm_rule_absX, (void *)1);
  orc_rule_register (rule_set, "addw", arm_rule_addw, NULL);
  orc_rule_register (rule_set, "addssw", arm_rule_addssw, NULL);
  orc_rule_register (rule_set, "addusw", arm_rule_addusw, NULL);
  orc_rule_register (rule_set, "andw", arm_rule_andX, NULL);
  orc_rule_register (rule_set, "andnw", arm_rule_andnX, NULL);
  orc_rule_register (rule_set, "avgsw", arm_rule_avgX, (void *)2);
  orc_rule_register (rule_set, "avguw", arm_rule_avgX, (void *)1);
  orc_rule_register (rule_set, "cmpeqw", arm_rule_cmpeqX, (void *)2);
  orc_rule_register (rule_set, "cmpgtsw", arm_rule_cmpgtsX, (void *)2);
  orc_rule_register (rule_set, "copyw", arm_rule_copyX, NULL);
  orc_rule_register (rule_set, "maxsw", arm_rule_maxsw, NULL);
  orc_rule_register (rule_set, "maxuw", arm_rule_maxuw, NULL);
  orc_rule_register (rule_set, "minsw", arm_rule_minsw, NULL);
  orc_rule_register (rule_set, "minuw", arm_rule_minuw, NULL);
  orc_rule_register (rule_set, "orw", arm_rule_orX, NULL);
  orc_rule_register (rule_set, "shlw", arm_rule_shlX, (void *)2);
  orc_rule_register (rule_set, "shrsw", arm_rule_shrsX, (void *)2);
  orc_rule_register (rule_set, "shruw", arm_rule_shruX, (void *)2);
  orc_rule_register (rule_set, "signw", arm_rule_signX, (void *)1);
  orc_rule_register (rule_set, "subw", arm_rule_subw, NULL);
  orc_rule_register (rule_set, "subssw", arm_rule_subssw, NULL);
  orc_rule_register (rule_set, "subusw", arm_rule_subusw, NULL);
  orc_rule_register (rule_set, "xorw", arm_rule_xorX, NULL);
  orc_rule_register (rule_set, "mullw", arm_rule_mullw, NULL);

  orc_rule_register (rule_set, "absl", arm_rule_absl, NULL);
  orc_rule_register (rule_set, "addl", arm_rule_addl, NULL);
  orc_rule_register (rule_set, "addssl", arm_rule_addssl, NULL);
  orc_rule_register (rule_set, "addusl", arm_rule_addusl, NULL);
  orc_rule_register (rule_set, "andl", arm_rule_andX, NULL);
  orc_rule_register (rule_set, "andnl", arm_rule_andnX, NULL);
  orc_rule_register (rule_set, "avgul", arm_rule_avgXl, NULL);
  orc_rule_register (rule_set, "avgsl", arm_rule_avgXl, NULL);
  orc_rule_register (rule_set, "cmpeql", arm_rule_cmpeql, NULL);
  orc_rule_register (rule_set, "cmpgtsl", arm_rule_cmpgtsl, NULL);
  orc_rule_register (rule_set, "copyl", arm_rule_copyX, NULL);
  orc_rule_register (rule_set, "maxsl", arm_rule_maxsl, NULL);
  orc_rule_register (rule_set, "maxul", arm_rule_maxul, NULL);
  orc_rule_register (rule_set, "minsl", arm_rule_minsl, NULL);
  orc_rule_register (rule_set, "minul", arm_rule_minul, NULL);
  orc_rule_register (rule_set, "orl", arm_rule_orX, NULL);
  orc_rule_register (rule_set, "shll", arm_rule_shlX, (void *)4);
  orc_rule_register (rule_set, "shrsl", arm_rule_shrsX, (void *)4);
  orc_rule_register (rule_set, "shrul", arm_rule_shruX, (void *)4);
  orc_rule_register (rule_set, "signl", arm_rule_signl, NULL);
  orc_rule_register (rule_set, "subl", arm_rule_subl, NULL);
  orc_rule_register (rule_set, "subssl", arm_rule_subssl, NULL);
  orc_rule_register (rule_set, "xorl", arm_rule_xorX, NULL);

}

