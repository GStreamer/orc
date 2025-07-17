
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orcavx-internal.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <orc/orcavx.h>
#include <orc/orcsse.h>
#include <orc/orcavxinsn.h>
#include <orc/orcdebug.h>
#include <orc/orccompiler.h>
#include <orc/orcinternal.h>

/* avx/avx2 rules */
// rules for calculating vector width:
// - if the calculation requires interleaving high half + permute to work,
//  check if src->size << compiler->loop_shift >= 16
// - if the calculation requires widening the data, halve the width above
// - if the calculation requires the high quad, and the original does not, halve
// again the width above
// - otherwise, check if src->size << compiler->loop_shift >= 32

static void
/* load a constant */
avx_rule_loadpX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  const OrcVariable *const src = compiler->vars + insn->src_args[0];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  /* user contains the size of the underlying type */
  const int size = ORC_PTR_TO_INT (user);

  if (src->vartype == ORC_VAR_TYPE_PARAM) {
    const int REGISTER_RULE = dest->alloc;

    if (size == 8 && src->size == 8) {
      // load the lower word (???0)
      // we need it into a XMM register
      orc_x86_emit_mov_memoffset_avx (compiler, 4,
          ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]]),
          compiler->exec_reg, REGISTER_RULE, FALSE);
      // load the upper word
      orc_avx_sse_emit_pinsrd_memoffset (compiler, 1,
          ORC_STRUCT_OFFSET (OrcExecutor,
              params[insn->src_args[0] + (ORC_N_PARAMS)]),
          ORC_AVX_SSE_REG (REGISTER_RULE), compiler->exec_reg,
          ORC_AVX_SSE_REG (REGISTER_RULE));
      // Now treat the REGISTER_RULE as YMM, and unpack
      orc_avx_emit_broadcast (compiler, REGISTER_RULE, REGISTER_RULE, 8);
    } else {
      // load the lower word (???0)
      // we need it into a XMM register
      orc_x86_emit_mov_memoffset_avx (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[0]]),
          compiler->exec_reg, REGISTER_RULE, FALSE);
      orc_avx_emit_broadcast (compiler, REGISTER_RULE, REGISTER_RULE, size);
    }
  } else if (src->vartype == ORC_VAR_TYPE_CONST) {
    orc_compiler_load_constant_from_size_and_value (compiler, dest->alloc,
        size, src->value.i);
  } else {
    ORC_ERROR ("Unknown variable type %d", src->vartype);
    ORC_ASSERT (0);
  }
}

static void
/* load a vector */
avx_rule_loadX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *const src = compiler->vars + insn->src_args[0];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  int ptr_reg = 0;
  const int offset = compiler->offset * src->size;

  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]), compiler->exec_reg,
        compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }

  orc_x86_emit_mov_memoffset_avx (compiler, src->size << compiler->loop_shift,
      offset, ptr_reg, dest->alloc, src->is_aligned);

  src->update_type = 2;
}

static void
/* load a vector with offset */
avx_rule_loadoffX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *const src = compiler->vars + insn->src_args[0];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  int ptr_reg = 0;

  if (compiler->vars[insn->src_args[1]].vartype != ORC_VAR_TYPE_CONST) {
    ORC_COMPILER_ERROR (compiler,
        "code generation rule for %s only works with constant offset",
        insn->opcode->name);
    return;
  }

  const int offset
      = (compiler->offset + compiler->vars[insn->src_args[1]].value.i)
        * src->size;
  if (src->ptr_register == 0) {
    const int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]), compiler->exec_reg,
        compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }

  orc_x86_emit_mov_memoffset_avx (compiler, src->size << compiler->loop_shift,
      offset, ptr_reg, dest->alloc, FALSE);

  src->update_type = 2;
}

static void
// load upsampled interpolate
avx_rule_loadupib_avx2 (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *const src = compiler->vars + insn->src_args[0];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  int ptr_reg = 0;
  const int tmp = orc_compiler_get_temp_reg (compiler);

  const int offset = (compiler->offset * src->size) >> 1;
  if (src->ptr_register == 0) {
    int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]), compiler->exec_reg,
        compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }

  const int size = src->size << compiler->loop_shift;
  switch (size) {
    case 1:
    case 2:
      orc_x86_emit_mov_memoffset_avx (compiler, 2, offset, ptr_reg, dest->alloc,
          FALSE);
      orc_avx_emit_psrlw_imm (compiler, 8, dest->alloc, tmp);
      break;
    default:
      orc_x86_emit_mov_memoffset_avx (compiler, size >> 1, offset, ptr_reg,
          dest->alloc, FALSE);
      orc_x86_emit_mov_memoffset_avx (compiler, size >> 1, offset + 1, ptr_reg,
          tmp, FALSE);
      break;
  }

  // Average
  // This average in AVX is OK, it operates across lanes
  orc_avx_emit_pavgb (compiler, dest->alloc, tmp, tmp);
  // Interleave average with original (interpolate)
  if (size >= 32) {
    const int tmp2 = orc_compiler_get_temp_reg (compiler);
    orc_avx_emit_punpckhbw (compiler, dest->alloc, tmp, tmp2);
    orc_avx_emit_punpcklbw (compiler, dest->alloc, tmp, dest->alloc);
    orc_avx_emit_permute2i128 (compiler, ORC_AVX_PERMUTE(2, 0), dest->alloc, tmp2, dest->alloc);
  } else {
    orc_avx_sse_emit_punpcklbw (compiler, ORC_AVX_SSE_REG (dest->alloc),
        ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest->alloc));
  }

  src->update_type = 1;
}

static void
/* load upsampled duplicate */
avx_rule_loadupdb_avx2 (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  OrcVariable *const src = compiler->vars + insn->src_args[0];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  const int tmp = orc_compiler_get_temp_reg (compiler);
  int ptr_reg = 0;

  const int offset = (compiler->offset * src->size) >> 1;
  if (src->ptr_register == 0) {
    const int i = insn->src_args[0];
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]), compiler->exec_reg,
        compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = src->ptr_register;
  }

  const int size = src->size << compiler->loop_shift;

  switch (size) {
    case 1:
    case 2:
      orc_x86_emit_mov_memoffset_avx (compiler, 1, offset, ptr_reg, dest->alloc,
          src->is_aligned);
      break;
    default:
      orc_x86_emit_mov_memoffset_avx (compiler, size >> 1, offset, ptr_reg,
          dest->alloc, src->is_aligned);
      break;
  }
  switch (src->size) {
    case 1:
      if (size >= 32) {
        orc_avx_emit_punpckhbw (compiler, dest->alloc, dest->alloc, tmp);
        orc_avx_emit_punpcklbw (compiler, dest->alloc, dest->alloc, dest->alloc);
        orc_avx_emit_permute2i128 (compiler, ORC_AVX_PERMUTE(2, 0), dest->alloc, tmp, dest->alloc);
      } else {
        orc_avx_sse_emit_punpcklbw (compiler, ORC_AVX_SSE_REG (dest->alloc),
            ORC_AVX_SSE_REG (dest->alloc), ORC_AVX_SSE_REG (dest->alloc));
      }
      break;
    case 2:
      if (size >= 32) {
        orc_avx_emit_punpckhwd (compiler, dest->alloc, dest->alloc, tmp);
        orc_avx_emit_punpcklwd (compiler, dest->alloc, dest->alloc, dest->alloc);
        orc_avx_emit_permute2i128 (compiler, ORC_AVX_PERMUTE(2, 0), dest->alloc, tmp, dest->alloc);
      } else {
        orc_avx_sse_emit_punpcklwd (compiler, ORC_AVX_SSE_REG (dest->alloc),
            ORC_AVX_SSE_REG (dest->alloc), ORC_AVX_SSE_REG (dest->alloc));
      }
      break;
    case 4:
      if (size >= 32) {
        orc_avx_emit_punpckhdq (compiler, dest->alloc, dest->alloc, tmp);
        orc_avx_emit_punpckldq (compiler, dest->alloc, dest->alloc, dest->alloc);
        orc_avx_emit_permute2i128 (compiler, ORC_AVX_PERMUTE(2, 0), dest->alloc, tmp, dest->alloc);
      } else {
        orc_avx_sse_emit_punpckldq (compiler, ORC_AVX_SSE_REG (dest->alloc),
            ORC_AVX_SSE_REG (dest->alloc), ORC_AVX_SSE_REG (dest->alloc));
      }
      break;
  }

  src->update_type = 1;
}

static void
/* store scalar */
avx_rule_storeX (OrcCompiler *compiler, void *user, OrcInstruction *insn)
{
  const OrcVariable *const src = compiler->vars + insn->src_args[0];
  OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  int ptr_reg = 0;

  const int offset = compiler->offset * dest->size;
  if (dest->ptr_register == 0) {
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        dest->ptr_offset, compiler->exec_reg, compiler->gp_tmpreg);
    ptr_reg = compiler->gp_tmpreg;
  } else {
    ptr_reg = dest->ptr_register;
  }

  orc_x86_emit_mov_avx_memoffset (compiler, dest->size << compiler->loop_shift,
      src->alloc, offset, ptr_reg, dest->is_aligned, dest->is_uncached);

  dest->update_type = 2;
}

ORC_GNUC_UNUSED static void
// load, nearest neighbor resampled
avx_rule_ldresnearl_avx2 (OrcCompiler *compiler, void *user,
    OrcInstruction *insn)
{
  OrcVariable *const src = compiler->vars + insn->src_args[0];
  const int increment_var = insn->src_args[2];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  const int tmp = orc_compiler_get_temp_reg (compiler);

  for (int i = 0; i < (1 << compiler->loop_shift); i++) {
    if (i == 0) {
      orc_x86_emit_mov_memoffset_avx (compiler, 4, 0, src->ptr_register,
          dest->alloc, FALSE);
    } else {
      orc_x86_emit_mov_memindex_avx (compiler, 4, 0, src->ptr_register,
          compiler->gp_tmpreg, 2, tmp, FALSE);
      /* orc_mmx_emit_punpckldq (compiler, tmp, dest->alloc); */
      int shift_in_dwords = i;
      if (shift_in_dwords >= 4) {
        shift_in_dwords -= 4;
        // Permute: zero out low lane, copy x0 to high lane
        orc_avx_emit_permute2i128 (compiler,
            ORC_AVX_PERMUTE (0, ORC_AVX_ZERO_LANE), tmp, tmp, tmp);

        if (shift_in_dwords) {
          orc_avx_emit_pslldq_imm (compiler, 4 * shift_in_dwords, tmp, tmp);
        }

        orc_avx_emit_por (compiler, tmp, dest->alloc, dest->alloc);
      } else {
        orc_avx_sse_emit_pslldq_imm (compiler, 4 * shift_in_dwords,
            ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
        orc_avx_sse_emit_por (compiler, ORC_AVX_SSE_REG (tmp),
            ORC_AVX_SSE_REG (dest->alloc), ORC_AVX_SSE_REG (dest->alloc));
      }
    }

    if (compiler->vars[increment_var].vartype == ORC_VAR_TYPE_PARAM) {
      orc_x86_emit_add_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[increment_var]),
          compiler->exec_reg, src->ptr_offset);
    } else {
      orc_x86_emit_add_imm_reg (compiler, 4,
          compiler->vars[increment_var].value.i, src->ptr_offset, FALSE);
    }

    orc_x86_emit_mov_reg_reg (compiler, 4, src->ptr_offset,
        compiler->gp_tmpreg);
    orc_x86_emit_sar_imm_reg (compiler, 4, 16, compiler->gp_tmpreg);
  }

  orc_x86_emit_add_reg_reg_shift (compiler, compiler->is_64bit ? 8 : 4,
      compiler->gp_tmpreg, src->ptr_register, 2);
  orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, src->ptr_offset);

  src->update_type = 0;
}

ORC_GNUC_UNUSED static void
// load, bilinear resampled
avx_rule_ldreslinl_avx2 (OrcCompiler *compiler, void *user,
    OrcInstruction *insn)
{
  OrcVariable *const src = compiler->vars + insn->src_args[0];
  const int increment_var = insn->src_args[2];
  const OrcVariable *const dest = compiler->vars + insn->dest_args[0];
  const int tmp = orc_compiler_get_temp_reg (compiler);
  const int tmp2 = orc_compiler_get_temp_reg (compiler);
  const int regsize = compiler->is_64bit ? 8 : 4;

  if (compiler->loop_shift == 0) {
    // a, b <- ptr4[tmp>>16, (tmp>>16)+1];
    // (tmp == 0 since we have no other pixels)
    orc_x86_emit_mov_memoffset_avx (compiler, 8, 0, src->ptr_register, tmp,
        FALSE);

    // Unsigned extend a and b to 16 bits (see below)
    orc_avx_sse_emit_pxor (compiler, ORC_AVX_SSE_REG (tmp2),
        ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpcklbw (compiler, ORC_AVX_SSE_REG (tmp),
        ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));
    // Key insight here: a * (256 - i) + b * i
    // = a * 256 - a * i + b * i
    // = (a * 256) + (b - a) * i
    // === tmp2 <- (b - a)
    // with b in tmp2, a in tmp
    orc_avx_sse_emit_pshufd (compiler, ORC_AVX_SSE_SHUF (3, 2, 3, 2),
        ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
    // only the low quad is active as per src->ptr_register above
    orc_avx_sse_emit_psubw (compiler, ORC_AVX_SSE_REG (tmp2),
        ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));

    // Load offset -- time to calculate tmp
    orc_avx_sse_emit_movd_load_register (compiler, src->ptr_offset,
        ORC_AVX_SSE_REG (tmp));
    // tmp is i32 in the emulator
    // but when using it we only care about the low 24 bits
    // due to & 0xFF windowing we only care about 0.24 -> 8..15
    // hence why only the low word is used
    orc_avx_sse_emit_pshuflw (compiler, ORC_AVX_SSE_SHUF (0, 0, 0, 0),
        ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    // i <-tmp >> 8
    orc_avx_sse_emit_psrlw_imm (compiler, 8, ORC_AVX_SSE_REG (tmp),
        ORC_AVX_SSE_REG (tmp));
    // (...) <- (-a + b) * i
    orc_avx_sse_emit_pmullw (compiler, ORC_AVX_SSE_REG (tmp),
        ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));
    // (...) <- (...) >> 8
    orc_avx_sse_emit_psraw_imm (compiler, 8, ORC_AVX_SSE_REG (tmp),
        ORC_AVX_SSE_REG (tmp));
    // var32.x4 <- (orc_uint8) (...)
    orc_avx_sse_emit_pxor (compiler, ORC_AVX_SSE_REG (tmp2),
        ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_packsswb (compiler, ORC_AVX_SSE_REG (tmp),
        ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));

    // Insert into the destination
    orc_x86_emit_mov_memoffset_avx (compiler, 4, 0, src->ptr_register,
        dest->alloc, FALSE);
    orc_avx_sse_emit_paddb (compiler, ORC_AVX_SSE_REG (tmp),
      ORC_AVX_SSE_REG (dest->alloc), ORC_AVX_SSE_REG (dest->alloc));

    // offset <- offset + i
    if (compiler->vars[increment_var].vartype == ORC_VAR_TYPE_PARAM) {
      orc_x86_emit_add_memoffset_reg (compiler, 4,
          ORC_STRUCT_OFFSET (OrcExecutor, params[increment_var]),
          compiler->exec_reg, src->ptr_offset);
    } else {
      orc_x86_emit_add_imm_reg (compiler, regsize,
          compiler->vars[increment_var].value.i, src->ptr_offset, FALSE);
    }

    // offset (tmp) >> 16
    orc_x86_emit_mov_reg_reg (compiler, 4, src->ptr_offset,
        compiler->gp_tmpreg);
    orc_x86_emit_sar_imm_reg (compiler, 4, 16, compiler->gp_tmpreg);

    // (unsure why they keep the exponent in unshifted form -- prolly because
    // of the offset + i step?)
    // ptr += offset << 2
    // 000000fd  lea     qword ptr [%r8+%rdx*4], %r8
    // keeping only the exponent for the remaining ops
    orc_x86_emit_add_reg_reg_shift (compiler, regsize, compiler->gp_tmpreg,
        src->ptr_register, 2);
    orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, src->ptr_offset);
  } else {
    // Interpolate 2 pixels or higher
    const int tmp3 = orc_compiler_get_temp_reg (compiler);
    const int offset = orc_compiler_get_temp_reg (compiler);

    const int boundary = 1 << compiler->loop_shift;

    for (int i = 0; i < boundary; i += 2) {
      // a, b <- ptr4[tmp>>16, (tmp>>16)+1];
      orc_x86_emit_mov_memoffset_avx (compiler, 8, 0, src->ptr_register, tmp,
          FALSE);
      // (unlike the zeroth case, this loop is pre increment)
      // Load offset -- time to calculate tmp
      orc_avx_sse_emit_movd_load_register (compiler, src->ptr_offset, offset);

      // offset <- offset + i
      if (compiler->vars[increment_var].vartype == ORC_VAR_TYPE_PARAM) {
        orc_x86_emit_add_memoffset_reg (compiler, 4,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, params[increment_var]),
            compiler->exec_reg, src->ptr_offset);
      } else {
        orc_x86_emit_add_imm_reg (compiler, 4,
            compiler->vars[increment_var].value.i, src->ptr_offset, FALSE);
      }

      // offset (tmp) >> 16
      orc_x86_emit_mov_reg_reg (compiler, 4, src->ptr_offset,
          compiler->gp_tmpreg);
      orc_x86_emit_sar_imm_reg (compiler, 4, 16, compiler->gp_tmpreg);
      // (unsure why they keep the exponent in unshifted form -- prolly because
      // of the offset + i step?)
      // c, d <- ptr4[updated offset >>16, (updated offset >>16)+1];
      orc_x86_emit_mov_memindex_avx (compiler, 8, 0, src->ptr_register,
          compiler->gp_tmpreg, 2, tmp2, FALSE);

      // tmp2 contains at this moment the correct odd interpolated pixels
      // interleave ab with cd
      orc_avx_sse_emit_punpckldq (compiler, ORC_AVX_SSE_REG (tmp),
          ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));
      // copy tmp to tmp2
      orc_avx_sse_emit_movdqa (compiler, ORC_AVX_SSE_REG (tmp),
          ORC_AVX_SSE_REG (tmp2));
      // Insert the pixels in place
      switch (i) {
        case 0:
          // dest <- tmp[0..64]
          orc_avx_sse_emit_movdqa (compiler, ORC_AVX_SSE_REG (tmp),
              ORC_AVX_SSE_REG (dest->alloc));
          break;
        case 2:
          // dest <- tmp[0..64] dest[0..64]
          orc_avx_sse_emit_punpcklqdq (compiler, ORC_AVX_SSE_REG (dest->alloc),
              ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest->alloc));
          break;
        case 4:
          // dest <- dest[255...192] tmp[0..64] dest[0..127]
          orc_avx_emit_broadcast (compiler, tmp, tmp3, 8);
          orc_avx_emit_permute4x64_imm (compiler, ORC_AVX_SSE_SHUF (2, 2, 1, 0), dest->alloc, dest->alloc);
          // Copy the 3rd quad from tmp3
          orc_avx_emit_pblendd (compiler, ORC_AVX_SSE_SHUF(0, 3, 0, 0), dest->alloc, tmp3, dest->alloc);
          break;
        case 6:
          // dest <- tmp[0..64] dest[128..191] dest[0..127]
          orc_avx_emit_broadcast (compiler, tmp, tmp3, 8);
          // Copy the 4th quad from tmp3
          orc_avx_emit_pblendd (compiler, ORC_AVX_SSE_SHUF(3, 0, 0, 0), dest->alloc, tmp3, dest->alloc);
          break;
        default:
          ORC_COMPILER_ERROR (compiler, "Invalid shift value for masking");
          ORC_ASSERT (0);
          break;
      }

      orc_avx_sse_emit_pxor (compiler, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3));
      // Unsigned extend ac to 16 bits
      orc_avx_sse_emit_punpcklbw (compiler, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp));
      // Unsigned extend bd to 16 bits
      orc_avx_sse_emit_punpckhbw (compiler, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp2));

      // tmp2 <- (b - a, d - c)
      orc_avx_sse_emit_psubw (compiler, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));

      // tmp4 now has the (offset + i, offset + i + 1)
      // (from the previous iteration, from the earlier addition)
      orc_avx_sse_emit_pinsrw_register (compiler, 1, ORC_AVX_SSE_REG (offset), src->ptr_offset,
          ORC_AVX_SSE_REG (offset));

      // (offset+i, offset+i, offset+i+1, offset+i+1, 0, 0, 0, 0)
      orc_avx_sse_emit_pshuflw (compiler, ORC_AVX_SSE_SHUF (1, 1, 0, 0), ORC_AVX_SSE_REG (offset),
          offset);
      // (o+i, o+i, o+i, o+i, o+i+1, o+i+1, o+i+1, o+i+1)
      orc_avx_sse_emit_pshufd (compiler, ORC_AVX_SSE_SHUF (1, 1, 0, 0), ORC_AVX_SSE_REG (offset),
          offset);

      // i <-tmp >> 8
      orc_avx_sse_emit_psrlw_imm (compiler, 8, ORC_AVX_SSE_REG (offset), ORC_AVX_SSE_REG (offset));
      // (...) <- (b - a, d - c) * i
      orc_avx_sse_emit_pmullw (compiler, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (offset), ORC_AVX_SSE_REG (tmp2));
      // (...) <- (...) >> 8
      orc_avx_sse_emit_psraw_imm (compiler, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
      // var32.x4 <- (orc_uint8) (...)
      orc_avx_emit_pxor (compiler, tmp, tmp, tmp);
      orc_avx_sse_emit_packsswb (compiler, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
      orc_avx_sse_emit_punpcklqdq (compiler, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));

      // store the next two pixels
      // interpolant goes in the middle
      switch (i) {
        case 0:
          break;
        case 2:
          orc_avx_sse_emit_pslldq_imm (compiler, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
          break;
        case 4:
          orc_avx_emit_permute4x64_imm (compiler, ORC_AVX_SSE_SHUF(2, 1, 3, 3), tmp2, tmp2);
          break;
        case 6:
          orc_avx_emit_permute4x64_imm (compiler, ORC_AVX_SSE_SHUF(1, 2, 3, 3), tmp2, tmp2);
          break;
        default:
          ORC_COMPILER_ERROR (compiler, "Invalid shift value for masking");
          ORC_ASSERT (0);
          break;
      }

      if (i >= 4) {
        // ptr0[i] = var32;
        // (note: this means the space making happens elsewhere -- somewhere with
        // lqdq ;) )
        orc_avx_emit_por (compiler, tmp2, dest->alloc, dest->alloc);
      } else {
        // ptr0[i] = var32;
        // (note: this means the space making happens elsewhere -- somewhere with
        // lqdq ;) )
        orc_avx_sse_emit_por (compiler, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest->alloc), ORC_AVX_SSE_REG (dest->alloc));
      }

      // offset <- offset + i
      if (compiler->vars[increment_var].vartype == ORC_VAR_TYPE_PARAM) {
        orc_x86_emit_add_memoffset_reg (compiler, 4,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, params[increment_var]),
            compiler->exec_reg, src->ptr_offset);
      } else {
        orc_x86_emit_add_imm_reg (compiler, 4,
            compiler->vars[increment_var].value.i, src->ptr_offset, FALSE);
      }

      // offset (tmp) >> 16
      orc_x86_emit_mov_reg_reg (compiler, 4, src->ptr_offset,
          compiler->gp_tmpreg);
      orc_x86_emit_sar_imm_reg (compiler, 4, 16, compiler->gp_tmpreg);

      // (unsure why they keep the exponent in unshifted form -- prolly because
      // of the offset + i step?)
      // ptr += offset << 2
      // 000000fd  lea     qword ptr [%r8+%rdx*4], %r8
      // keeping only the exponent for the remaining ops
      orc_x86_emit_add_reg_reg_shift (compiler, 8, compiler->gp_tmpreg,
          src->ptr_register, 2);
      orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, src->ptr_offset);
    }
  }

  src->update_type = 0;
}

static void
avx_rule_copyx (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  if (p->vars[insn->src_args[0]].alloc == p->vars[insn->dest_args[0]].alloc) {
    return;
  }

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_movdqa (p, p->vars[insn->src_args[0]].alloc,
        p->vars[insn->dest_args[0]].alloc);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc),
        ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc));
  }
}

// Unary opcode, narrowing to SSE at width bytes
#define UNARY_W(opcode, insn_name, width) \
  static void avx_rule_##opcode (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    const int size = p->vars[insn->src_args[0]].size << p->loop_shift; \
    if (size >= width) { \
      orc_avx_emit_##insn_name (p, p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->dest_args[0]].alloc); \
    } else { \
      orc_avx_sse_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } \
  }

#define UNARY_W_SSE(opcode, insn_name, width) \
  static void avx_rule_##opcode (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    const int size = p->vars[insn->src_args[0]].size << p->loop_shift; \
    if (size >= width) { \
      orc_avx_emit_##insn_name (p, p->vars[insn->src_args[0]].alloc, \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } else { \
      orc_avx_sse_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } \
  }

#define UNARY(opcode, insn_name) UNARY_W(opcode, insn_name, 32)

// Binary opcode
// careful about the implicit src0 == dest here!
#define BINARY(opcode, insn_name) \
  static void avx_rule_##opcode (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    const int size = p->vars[insn->src_args[0]].size << p->loop_shift; \
    if (size >= 32) { \
      orc_avx_emit_##insn_name (p, p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->src_args[1]].alloc, \
          p->vars[insn->dest_args[0]].alloc); \
    } else { \
      orc_avx_sse_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[1]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } \
  }

// Unary opcode, AVX2 only, narrowing to SSE at width bytes, SSE src
#define UNARY_AVX2_ONLY_W_SSE(opcode, insn_name, width) \
  static void avx_rule_##opcode##_avx2 (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    const int size = p->vars[insn->src_args[0]].size << p->loop_shift; \
    if (size >= width) { \
      orc_avx_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          p->vars[insn->dest_args[0]].alloc); \
    } else { \
      orc_avx_sse_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } \
  }

// Unary opcode, AVX2 only, narrowing to SSE at width bytes
#define UNARY_AVX2_ONLY_W(opcode, insn_name, width) \
  static void avx_rule_##opcode##_avx2 (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    const int size = p->vars[insn->src_args[0]].size << p->loop_shift; \
    if (size >= width) { \
      orc_avx_emit_##insn_name (p, p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->dest_args[0]].alloc); \
    } else { \
      orc_avx_sse_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } \
  }

#define UNARY_AVX2_ONLY(opcode, insn_name) UNARY_AVX2_ONLY_W(opcode, insn_name, 32)

// Binary opcode, AVX2 only
#define BINARY_AVX2_ONLY(opcode, insn_name) \
  static void avx_rule_##opcode##_avx2 (OrcCompiler *p, void *user, \
      OrcInstruction *insn) \
  { \
    const int size = p->vars[insn->src_args[0]].size << p->loop_shift; \
    if (size >= 32) { \
      orc_avx_emit_##insn_name (p, p->vars[insn->src_args[0]].alloc, \
          p->vars[insn->src_args[1]].alloc, \
          p->vars[insn->dest_args[0]].alloc); \
    } else { \
      orc_avx_sse_emit_##insn_name (p, \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[0]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->src_args[1]].alloc), \
          ORC_AVX_SSE_REG (p->vars[insn->dest_args[0]].alloc)); \
    } \
  }

UNARY_AVX2_ONLY (absb, pabsb)
BINARY_AVX2_ONLY (addb, paddb)
BINARY_AVX2_ONLY (addssb, paddsb)
BINARY_AVX2_ONLY (addusb, paddusb)
BINARY_AVX2_ONLY (andb, pand)
BINARY_AVX2_ONLY (andnb, pandn)
BINARY_AVX2_ONLY (avgub, pavgb)
BINARY_AVX2_ONLY (cmpeqb, pcmpeqb)
BINARY_AVX2_ONLY (cmpgtsb, pcmpgtb)
BINARY_AVX2_ONLY (maxsb, pmaxsb)
BINARY_AVX2_ONLY (maxub, pmaxub)
BINARY_AVX2_ONLY (minsb, pminsb)
BINARY_AVX2_ONLY (minub, pminub)
BINARY_AVX2_ONLY (orb, por)
BINARY_AVX2_ONLY (subb, psubb)
BINARY_AVX2_ONLY (subssb, psubsb)
BINARY_AVX2_ONLY (subusb, psubusb)
BINARY (xorb, pxor)

UNARY_AVX2_ONLY (absw, pabsw)
BINARY_AVX2_ONLY (addw, paddw)
BINARY_AVX2_ONLY (addssw, paddsw)
BINARY_AVX2_ONLY (addusw, paddusw)
BINARY_AVX2_ONLY (andw, pand)
BINARY_AVX2_ONLY (andnw, pandn)
BINARY_AVX2_ONLY (avguw, pavgw)
BINARY_AVX2_ONLY (cmpeqw, pcmpeqw)
BINARY_AVX2_ONLY (cmpgtsw, pcmpgtw)
BINARY_AVX2_ONLY (maxsw, pmaxsw)
BINARY_AVX2_ONLY (maxuw, pmaxuw)
BINARY_AVX2_ONLY (minsw, pminsw)
BINARY_AVX2_ONLY (minuw, pminuw)
BINARY_AVX2_ONLY (mullw, pmullw)
BINARY_AVX2_ONLY (mulhsw, pmulhw)
BINARY_AVX2_ONLY (mulhuw, pmulhuw)
BINARY_AVX2_ONLY (orw, por)
BINARY_AVX2_ONLY (subw, psubw)
BINARY_AVX2_ONLY (subssw, psubsw)
BINARY_AVX2_ONLY (subusw, psubusw)
BINARY (xorw, pxor)

UNARY_AVX2_ONLY (absl, pabsd)
BINARY_AVX2_ONLY (addl, paddd)
/* The following opcodes don't exist on SSE/AVX */
/* BINARY(addssl, paddsd) */
/* BINARY(addusl, paddusd) */
BINARY_AVX2_ONLY (andl, pand)
BINARY_AVX2_ONLY (andnl, pandn)
/* BINARY(avgul, pavgd) */
BINARY_AVX2_ONLY (cmpeql, pcmpeqd)
BINARY_AVX2_ONLY (cmpgtsl, pcmpgtd)
BINARY_AVX2_ONLY (maxsl, pmaxsd)
BINARY_AVX2_ONLY (maxul, pmaxud)
BINARY_AVX2_ONLY (minsl, pminsd)
BINARY_AVX2_ONLY (minul, pminud)
BINARY_AVX2_ONLY (mulll, pmulld)
/* BINARY(mulhsl, pmulhd) */
/* BINARY(mulhul, pmulhud) */
BINARY_AVX2_ONLY (orl, por)
/* UNARY(signl, psignd) */
BINARY_AVX2_ONLY (subl, psubd)
/* BINARY(subssl, psubsd) */
/* BINARY(subusl, psubusd) */
BINARY (xorl, pxor)

BINARY_AVX2_ONLY (andq, pand)
BINARY_AVX2_ONLY (andnq, pandn)
BINARY_AVX2_ONLY (orq, por)
BINARY (xorq, pxor)
BINARY_AVX2_ONLY (cmpeqq, pcmpeqq)
BINARY_AVX2_ONLY (cmpgtsq, pcmpgtq)

BINARY_AVX2_ONLY (addq, paddq)
BINARY_AVX2_ONLY (subq, psubq)

static void
avx_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  // More than one element and it's unsafe
  if (size >= 2) {
    orc_avx_emit_paddw (p, dest, src, dest);
  } else {
    orc_avx_sse_emit_paddw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  if (p->loop_shift == 0) {
    orc_avx_sse_emit_pslldq_imm (p, 12, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src));
  }

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  // More than one element and it's unsafe
  if (size >= 4) {
    orc_avx_emit_paddd (p, dest, src, dest);
  } else {
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
// accumulate absolute difference, acc += abs(a - b)
avx_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src1 = p->vars[insn->src_args[0]].alloc;
  const int src2 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

  // src1 contains 1 << p->loop_shift elements
  // src2 may or may not contain 1 << p->loop_shift elements
  // if the loop_shift isn't the maximum, we need to ensure no exceeding
  // elements are added in the accumulator variable
  // this is done with four things:
  //  - the iteration with shift <= 4  must be SSE only (zonking high lane)
  //  - if shift <= 3, we need to shift the whole halves

  if (p->loop_shift <= 2) { // <= 32b
    orc_avx_sse_emit_pslldq_imm (p, 16 - (1 << p->loop_shift), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pslldq_imm (p, 16 - (1 << p->loop_shift), ORC_AVX_SSE_REG (src2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psadbw (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  } else if (p->loop_shift == 3) {
    orc_avx_sse_emit_psadbw (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src2), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pslldq_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp)); // zonk out the upper garbage
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  } else if (p->loop_shift == 4) {                // 128b
    orc_avx_sse_emit_psadbw (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src2), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  } else {
    orc_avx_emit_psadbw (p, src1, src2, tmp);
    orc_avx_emit_paddd (p, dest, tmp, dest);
  }
}

static void
avx_rule_signX_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int opcodes[] = { ORC_AVX_psignb, ORC_AVX_psignw, ORC_AVX_psignd };
  const int opcodes_sse[] = { ORC_AVX_SSE_psignb, ORC_AVX_SSE_psignw, ORC_AVX_SSE_psignd };
  const int type = ORC_PTR_TO_INT (user);

  const int tmpc = orc_compiler_get_temp_constant (p, 1 << type, 1);

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_vex_emit_cpuinsn_avx (p, opcodes[type], tmpc, src, 0, dest);
  } else {
    orc_vex_emit_cpuinsn_avx (p, opcodes_sse[type], ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (src), 0, ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_shift (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int type = ORC_PTR_TO_INT (user);
  const int opcodes[] = { ORC_AVX_psllw, ORC_AVX_psrlw, ORC_AVX_psraw,
    ORC_AVX_pslld, ORC_AVX_psrld, ORC_AVX_psrad, ORC_AVX_psllq, ORC_AVX_psrlq };
  const int opcodes_sse[] = { ORC_AVX_SSE_psllw, ORC_AVX_SSE_psrlw, ORC_AVX_SSE_psraw,
    ORC_AVX_SSE_pslld, ORC_AVX_SSE_psrld, ORC_AVX_SSE_psrad, ORC_AVX_SSE_psllq, ORC_AVX_SSE_psrlq };
  const int opcodes_imm[] = { ORC_AVX_psllw_imm, ORC_AVX_psrlw_imm,
    ORC_AVX_psraw_imm, ORC_AVX_pslld_imm, ORC_AVX_psrld_imm, ORC_AVX_psrad_imm,
    ORC_AVX_psllq_imm, ORC_AVX_psrlq_imm };
  const int opcodes_sse_imm[] = { ORC_AVX_SSE_psllw_imm, ORC_AVX_SSE_psrlw_imm,
    ORC_AVX_SSE_psraw_imm, ORC_AVX_SSE_pslld_imm, ORC_AVX_SSE_psrld_imm, ORC_AVX_SSE_psrad_imm,
    ORC_AVX_SSE_psllq_imm, ORC_AVX_SSE_psrlq_imm };
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    // The original inserts garbage in the source (to validate?)
    // Here we need a explicit source
    if (size >= 32) {
      orc_vex_emit_cpuinsn_imm (p, opcodes_imm[type],
        0, p->vars[insn->src_args[1]].value.i, src, 0, dest);
    } else {
      orc_vex_emit_cpuinsn_imm (p, opcodes_sse_imm[type],
        0, p->vars[insn->src_args[1]].value.i, ORC_AVX_SSE_REG (src), 0, ORC_AVX_SSE_REG (dest));
    }
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    int tmp = orc_compiler_get_temp_reg (p);

    /* FIXME this is a gross hack to reload the register with a
     * 64-bit version of the parameter. */
    orc_x86_emit_mov_memoffset_avx (p, 4,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[1]]),
        p->exec_reg, tmp, FALSE);

    if (size >= 32) {
      orc_vex_emit_cpuinsn_avx (p, opcodes[type], src, ORC_AVX_SSE_REG (tmp), 0, dest);
    } else {
      orc_vex_emit_cpuinsn_avx (p, opcodes_sse[type], ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), 0, ORC_AVX_SSE_REG (dest));
    }
  } else {
    ORC_COMPILER_ERROR (p,
        "code generation rule for %s only works with "
        "constant or parameter shifts",
        insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
avx_rule_shlb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 1,
        0xff & (0xff << p->vars[insn->src_args[1]].value.i));
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    if (size >= 32) {
      orc_avx_emit_psllw_imm (p, p->vars[insn->src_args[1]].value.i, src, dest);
      orc_avx_emit_pand (p, dest, tmp, dest);
    } else {
      orc_avx_sse_emit_psllw_imm (p, p->vars[insn->src_args[1]].value.i, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
      orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  } else {
    ORC_COMPILER_ERROR (p,
        "code generation rule for %s only works with "
        "constant shifts",
        insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
avx_rule_shrsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    if (size >= 32) {
      orc_avx_emit_movdqa (p, src, tmp);
      orc_avx_emit_psllw_imm (p, 8, src, tmp);
      orc_avx_emit_psraw_imm (p, p->vars[insn->src_args[1]].value.i, tmp, tmp);
      orc_avx_emit_psrlw_imm (p, 8, tmp, tmp);

      orc_avx_emit_psraw_imm (p, 8 + p->vars[insn->src_args[1]].value.i, src,
          dest);
      orc_avx_emit_psllw_imm (p, 8, dest, dest);

      orc_avx_emit_por (p, dest, tmp, dest);
    } else {
      orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
      orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
      orc_avx_sse_emit_psraw_imm (p, p->vars[insn->src_args[1]].value.i, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
      orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

      orc_avx_sse_emit_psraw_imm (p, 8 + p->vars[insn->src_args[1]].value.i, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (dest));
      orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

      orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  } else {
    ORC_COMPILER_ERROR (p,
        "code generation rule for %s only works with "
        "constant shifts",
        insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
avx_rule_shrub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 1,
      (0xff >> p->vars[insn->src_args[1]].value.i));
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    if (size >= 32) {
      orc_avx_emit_psrlw_imm (p, p->vars[insn->src_args[1]].value.i, src, dest);
      orc_avx_emit_pand (p, dest, tmp, dest);
    } else {
      orc_avx_sse_emit_psrlw_imm (p, p->vars[insn->src_args[1]].value.i, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (dest));
      orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  } else {
    ORC_COMPILER_ERROR (p,
        "code generation rule for %s only works with "
        "constant shifts",
        insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
avx_rule_shrsq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    if (size >= 32) {
      orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 3, 1, 1), src, tmp);
      orc_avx_emit_psrad_imm (p, 31, tmp, tmp);
      orc_avx_emit_psllq_imm (p, 64 - p->vars[insn->src_args[1]].value.i, tmp,
          tmp);

      orc_avx_emit_psrlq_imm (p, p->vars[insn->src_args[1]].value.i, src, dest);
      orc_avx_emit_por (p, dest, tmp, dest);
    } else {
      orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 3, 1, 1), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
      orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
      orc_avx_sse_emit_psllq_imm (p, 64 - p->vars[insn->src_args[1]].value.i,
          ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

      orc_avx_sse_emit_psrlq_imm (p, p->vars[insn->src_args[1]].value.i, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (dest));
      orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  } else {
    ORC_COMPILER_ERROR (p,
        "code generation rule for %s only works with "
        "constant shifts",
        insn->opcode->name);
    p->result = ORC_COMPILE_RESULT_UNKNOWN_COMPILE;
  }
}

static void
avx_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  // data narrowing!
  if (size >= 16) {
    orc_avx_emit_packsswb (p, src, src, dest);
    // packsswb does quad interleave ><
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_packsswb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_packuswb (p, src, src, dest);
    // packsswb does quad interleave ><
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_packuswb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convusswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_compiler_load_constant_from_size_and_value (p, tmp, 2, INT8_MAX); // set all of dest to 127
    orc_avx_emit_pminuw (p, src, tmp, dest);
    orc_avx_emit_pxor (p, tmp, tmp, tmp);
    orc_avx_emit_packuswb (p, dest, tmp, dest);
    // packsswb does quad interleave ><
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_compiler_load_constant_from_size_and_value (p, tmp, 2, INT8_MAX); // set all of dest to 127
    orc_avx_sse_emit_pminuw (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_packuswb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convuuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_movdqa (p, src, tmp);
    orc_avx_emit_psrlw_imm (p, 15, src, tmp);
    orc_avx_emit_psllw_imm (p, 14, tmp, tmp);
    orc_avx_emit_por (p, src, tmp, dest);
    orc_avx_emit_psllw_imm (p, 1, tmp, tmp);
    orc_avx_emit_pxor (p, dest, tmp, dest);
    orc_avx_emit_packuswb (p, dest, dest, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 15, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psllw_imm (p, 14, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psllw_imm (p, 1, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packuswb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_psllw_imm (p, 8, src, dest);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);
    orc_avx_emit_packuswb (p, dest, dest, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packuswb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convhwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_psrlw_imm (p, 8, src, dest);
    orc_avx_emit_packuswb (p, dest, dest, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packuswb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convussql_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;
  const int tmpc = orc_compiler_get_temp_reg(p);
  orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, (orc_uint64)INT32_MIN);

  if (size >= 32) {
    orc_avx_emit_pcmpgtq (p, tmpc, src, tmp);
    orc_avx_emit_blendvpd (p, tmpc, src, tmp, dest);
    orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, (orc_uint64)INT32_MAX);
    orc_avx_emit_pcmpgtq (p, dest, tmpc, tmp);
    orc_avx_emit_blendvpd (p, tmpc, dest, tmp, dest);
    // full interleave required again
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, (orc_uint64)INT32_MAX);
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convusslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  orc_compiler_load_constant_from_size_and_value (p, tmp, 4, INT16_MAX);
  if (size >= 16) {
    orc_avx_emit_pminud (p, src, tmp, dest);
    orc_avx_emit_pxor (p, tmp, tmp, tmp);
    orc_avx_emit_packssdw (p, dest, tmp, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pminud (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convuuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  orc_compiler_load_constant_from_size_and_value (p, tmp, 4, UINT16_MAX);
  if (size >= 16) {
    orc_avx_emit_pminud (p, src, tmp, dest);
    orc_avx_emit_pxor (p, tmp, tmp, tmp);
    orc_avx_emit_packusdw (p, dest, tmp, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pminud (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_packusdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_pslld_imm (p, 16, src, dest);
    orc_avx_emit_psrad_imm (p, 16, dest, dest);
    orc_avx_emit_packssdw (p, dest, dest, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pslld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrad_imm (p, 16, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convhlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_psrad_imm (p, 16, src, dest);
    orc_avx_emit_packssdw (p, dest, dest, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_psrad_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convsusql_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmpc = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, UINT32_MAX);
    orc_avx_emit_pcmpgtq (p, src, tmpc, tmp);
    orc_avx_emit_blendvpd(p, src, tmpc, tmp, dest);
    orc_avx_emit_pxor (p, tmpc, tmpc, tmpc);
    orc_avx_emit_pcmpgtq (p, dest, tmpc, tmp);
    orc_avx_emit_blendvpd(p, tmpc, dest, tmp, dest);
    // same as above
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, UINT32_MAX);
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd(p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmpc));
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd(p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG(tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convuusql_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmpc = orc_compiler_get_temp_reg (p);
  const int tmpsub = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_compiler_load_constant_from_size_and_value (p, tmpsub, 8, INT64_MIN);
    orc_avx_emit_psubq (p, src, tmpsub, tmp);
    orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, UINT32_MAX);
    // tmpc = uint32_max - int64_min
    orc_avx_emit_psubq (p, tmpc, tmpsub, tmpsub);
    orc_avx_emit_pcmpgtq (p, tmp, tmpsub, tmp);
    orc_avx_emit_blendvpd(p, src, tmpc, tmp, dest);
    // same as above
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_compiler_load_constant_from_size_and_value (p, tmpsub, 8, INT64_MIN);
    orc_avx_sse_emit_psubq (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpsub), ORC_AVX_SSE_REG (tmp));
    orc_compiler_load_constant_from_size_and_value (p, tmpc, 8, UINT32_MAX);
    // tmpc = uint32_max - int64_min
    orc_avx_sse_emit_psubq (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmpsub), ORC_AVX_SSE_REG (tmpsub));
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmpsub), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd(p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int zero = orc_compiler_get_temp_constant (p, 4, 0);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 2, 0), src, dest);
    orc_avx_emit_punpcklqdq (p, dest, zero, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 2, 0), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpcklqdq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (zero), ORC_AVX_SSE_REG (dest));
  }
}

static void
// duplicates high 16-bits to lower 48 bits
avx_rule_splatw3q (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pshuflw (p, ORC_AVX_SSE_SHUF (3, 3, 3, 3), src, dest);
    orc_avx_emit_pshufhw (p, ORC_AVX_SSE_SHUF (3, 3, 3, 3), dest, dest);
  } else {
    orc_avx_sse_emit_pshuflw (p, ORC_AVX_SSE_SHUF (3, 3, 3, 3), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufhw (p, ORC_AVX_SSE_SHUF (3, 3, 3, 3), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
// duplicates 8 bits to both halfs of 16 bits
avx_rule_splatbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_sse_emit_punpckhbw (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp, dest);
  } else {
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
// duplicates 8 bits to all parts of 32 bits
avx_rule_splatbl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 8) {
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpckhwd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklwd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp, dest);
  } else {
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpcklwd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_div255w (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmpc = orc_compiler_get_constant (p, 2, 0x8081);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pmulhuw(p, src, tmpc, dest);
    orc_avx_emit_psrlw_imm (p, 7, dest, dest);
  } else {
    orc_avx_sse_emit_pmulhuw(p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 7, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
// saturated unsigned divide 16-bit by 8-bit -- clamp(a/(b & 255),0,255)
avx_rule_divluw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* About 5.2 cycles per array member on ginger */
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int a = orc_compiler_get_temp_reg (p);
  const int j = orc_compiler_get_temp_reg (p);
  const int j2 = orc_compiler_get_temp_reg (p);
  const int l = orc_compiler_get_temp_reg (p);
  const int divisor = orc_compiler_get_temp_reg (p);
  const int tmp = orc_compiler_get_constant (p, 2, 0x8000);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* FIXME use an orc_compiler_get_constant to be able to cache it */
  orc_compiler_load_constant_from_size_and_value (p, a, 2, 0x00ff);
  if (size >= 32) {
    orc_avx_emit_movdqa (p, src1, divisor);
    orc_avx_emit_psllw_imm (p, 8, src1, divisor);
    orc_avx_emit_psrlw_imm (p, 1, divisor, divisor);

    orc_avx_emit_psrlw_imm (p, 8, tmp, j);

    orc_avx_emit_pxor (p, src0, tmp, dest);

    for (int i = 0; i < 7; i++) {
      orc_avx_emit_pxor (p, divisor, tmp, l);
      orc_avx_emit_pcmpgtw (p, l, dest, l);
      orc_avx_emit_movdqa (p, l, j2);
      orc_avx_emit_pandn (p, l, divisor, l);
      orc_avx_emit_psubw (p, dest, l, dest);
      orc_avx_emit_psrlw_imm (p, 1, divisor, divisor);

      orc_avx_emit_pand (p, j2, j, j2);
      orc_avx_emit_pxor (p, a, j2, a);
      orc_avx_emit_psrlw_imm (p, 1, j, j);
    }

    orc_avx_emit_movdqa (p, divisor, l);
    orc_avx_emit_pxor (p, l, tmp, l);
    orc_avx_emit_pcmpgtw (p, l, dest, l);
    orc_avx_emit_pand (p, l, j, l);
    orc_avx_emit_pxor (p, a, l, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (divisor));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (divisor));
    orc_avx_sse_emit_psrlw_imm (p, 1, ORC_AVX_SSE_REG (divisor), ORC_AVX_SSE_REG (divisor));

    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (j));

    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));

    for (int i = 0; i < 7; i++) {
      orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (divisor), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (l));
      orc_avx_sse_emit_pcmpgtw (p, ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (l));
      orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (j2));
      orc_avx_sse_emit_pandn (p, ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (divisor), ORC_AVX_SSE_REG (l));
      orc_avx_sse_emit_psubw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (dest));
      orc_avx_sse_emit_psrlw_imm (p, 1, ORC_AVX_SSE_REG (divisor), ORC_AVX_SSE_REG (divisor));

      orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (j2), ORC_AVX_SSE_REG (j), ORC_AVX_SSE_REG (j2));
      orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (a), ORC_AVX_SSE_REG (j2), ORC_AVX_SSE_REG (a));
      orc_avx_sse_emit_psrlw_imm (p, 1, ORC_AVX_SSE_REG (j), ORC_AVX_SSE_REG (j));
    }

    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (divisor), ORC_AVX_SSE_REG (l));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (l));
    orc_avx_sse_emit_pcmpgtw (p, ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (l));
    orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (j), ORC_AVX_SSE_REG (l));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (a), ORC_AVX_SSE_REG (l), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int tmp3 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    // FIXME: in the original, tmp is not zeroed before usage
    // src0 == dest, so don't assume we can read dest post-facto
    orc_avx_emit_movdqa (p, src0, tmp2);

    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_punpckhbw (p, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_punpckhbw (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));

    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp2, dest);
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), tmp, tmp3, tmp);
    orc_avx_emit_pmullw (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int tmp3 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    // FIXME: in the original, tmp is not zeroed before usage
    // src0 == dest, so don't assume we can read dest post-facto
    orc_avx_emit_movdqa (p, src0, tmp2);

    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_punpckhbw (p, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_punpckhbw (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));

    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp2, dest);
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), tmp, tmp3, tmp);
    orc_avx_emit_pmullw (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mullb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    // src0 == dest, so don't assume we can read dest post-facto
    orc_avx_emit_movdqa (p, src0, tmp);

    orc_avx_emit_pmullw (p, src0, src1, dest);
    orc_avx_emit_psllw_imm (p, 8, dest, dest);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);

    orc_avx_emit_movdqa (p, src1, tmp2);
    orc_avx_emit_psraw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_psraw_imm (p, 8, tmp, tmp);
    orc_avx_emit_pmullw (p, tmp, tmp2, tmp);
    orc_avx_emit_psllw_imm (p, 8, tmp, tmp);

    orc_avx_emit_por (p, dest, tmp, dest);
  } else {
    // src0 == dest, so don't assume we can read dest post-facto
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulhsb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    // src0 == dest, so don't assume we can read dest post-facto
    orc_avx_emit_movdqa (p, src0, tmp2);

    orc_avx_emit_psllw_imm (p, 8, src1, tmp);
    orc_avx_emit_psraw_imm (p, 8, tmp, tmp);

    orc_avx_emit_psllw_imm (p, 8, src0, dest);
    orc_avx_emit_psraw_imm (p, 8, dest, dest);

    orc_avx_emit_pmullw (p, dest, tmp, dest);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);

    orc_avx_emit_psraw_imm (p, 8, src1, tmp);
    orc_avx_emit_psraw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_pmullw (p, tmp2, tmp, tmp2);
    orc_avx_emit_psrlw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_psllw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_por (p, dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp2));

    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulhub (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    // src0 == dest, so don't assume we can read dest post-facto
    orc_avx_emit_movdqa (p, src0, tmp2);

    orc_avx_emit_psllw_imm (p, 8, src1, tmp);
    orc_avx_emit_psrlw_imm (p, 8, tmp, tmp);

    orc_avx_emit_psllw_imm (p, 8, src0, dest);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);

    orc_avx_emit_pmullw (p, dest, tmp, dest);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);

    orc_avx_emit_psrlw_imm (p, 8, src1, tmp);
    orc_avx_emit_psrlw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_pmullw (p, tmp2, tmp, tmp2);
    orc_avx_emit_psrlw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_psllw_imm (p, 8, tmp2, tmp2);
    orc_avx_emit_por (p, dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp2));

    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_pmulhw (p, src0, src1, tmp);
    orc_avx_emit_pmullw (p, src0, src1, dest);

    // this needs to be a total interleave
    orc_avx_emit_punpckhwd (p, dest, tmp, tmp2);
    orc_avx_emit_punpcklwd (p, dest, tmp, dest);
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_pmulhw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpcklwd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_muluwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_pmulhuw (p, src0, src1, tmp);
    orc_avx_emit_pmullw (p, src0, src1, dest);

    // this needs to be a total interleave
    orc_avx_emit_punpckhwd (p, dest, tmp, tmp2);
    orc_avx_emit_punpcklwd (p, dest, tmp, dest);
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_pmulhuw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pmullw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpcklwd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulhsl_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), src0, tmp);
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), src1, tmp2);
    orc_avx_emit_pmuldq (p, src0, src1, dest);
    orc_avx_emit_pmuldq (p, tmp2, tmp, tmp2);
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), dest, dest);
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), tmp2, tmp2);
    orc_avx_emit_punpckldq (p, dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pmuldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pmuldq (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulhul (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), src0, tmp);
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), src1, tmp2);
    orc_avx_emit_pmuludq (p, src0, src1, dest);
    orc_avx_emit_pmuludq (p, tmp2, tmp, tmp2);
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), dest, dest);
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), tmp2, tmp2);
    orc_avx_emit_punpckldq (p, dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pmuludq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pmuludq (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mulslq_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_sse_emit_punpckhdq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp2, dest);

    orc_avx_sse_emit_punpckhdq (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), tmp, tmp2, tmp);

    orc_avx_emit_pmuldq (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pmuldq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mululq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_sse_emit_punpckhdq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), dest, tmp2, dest);

    orc_avx_sse_emit_punpckhdq (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(2, 0), tmp, tmp2, tmp);

    orc_avx_emit_pmuludq (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pmuludq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

/* vextractf128 for integrals */

static void
// select low word
avx_rule_select0lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* FIXME slow */
  /* same as convlw */

  if (size >= 32) {
    orc_avx_emit_pslld_imm (p, 16, src, dest);
    orc_avx_emit_psrad_imm (p, 16, dest, dest);
    orc_avx_emit_packssdw (p, dest, dest, dest);
    // here we need to consolidate the low lanes of each now
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pslld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psrad_imm (p, 16, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
// select high word
avx_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* FIXME slow */
  /* same as convhlw */

  if (size >= 32) {
    orc_avx_emit_psrad_imm (p, 16, src, dest);
    orc_avx_emit_packssdw (p, dest, dest, dest);
    // same as above
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_psrad_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
// select low doubleword
avx_rule_select0ql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int zero = orc_compiler_get_temp_constant (p, 4, 0);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* values of dest are shifted away so don't matter */

  /* same as convql */

  if (size >= 32) {
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 2, 0), src, dest);
    orc_avx_emit_punpcklqdq (p, dest, zero, dest);
    // full interleave required again
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF(3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 2, 0), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpcklqdq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (zero), ORC_AVX_SSE_REG (dest));
  }
}

static void
// select high doubleword
avx_rule_select1ql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int zero = orc_compiler_get_temp_constant (p, 4, 0);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;
  /* values of dest are shifted away so don't matter */

  if (size >= 32) {
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 3, 1), src, dest);
    orc_avx_emit_punpcklqdq (p, dest, zero, dest);
    // full interleave required again
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF(3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 3, 1), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_punpcklqdq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (zero), ORC_AVX_SSE_REG (dest));
  }
}

static void
// select low half
avx_rule_select0wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* FIXME slow */
  /* same as convwb */

  if (size >= 32) {
    orc_avx_emit_psllw_imm (p, 8, src, dest);
    orc_avx_emit_psraw_imm (p, 8, dest, dest);
    orc_avx_emit_packsswb (p, dest, dest, dest);
    // full interleave required again
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packsswb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
// select high half
avx_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* FIXME slow */

  if (size >= 32) {
    orc_avx_emit_psraw_imm (p, 8, src, dest);
    orc_avx_emit_packsswb (p, dest, dest, dest);
    // full interleave required again
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_packsswb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

static void
// split first/second quads
avx_rule_splitql (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int zero = orc_compiler_get_temp_constant (p, 4, 0);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    // Group into 64-bit operands to be copied around
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 3, 1), src, tmp);
    // Sort lanes out
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF(3, 1, 2, 0), tmp, tmp);
    // Output
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE (ORC_AVX_ZERO_LANE, 0), tmp,
        tmp, dest1);
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE(ORC_AVX_ZERO_LANE, 1), tmp, tmp, dest2);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 3, 1), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest1));
    orc_avx_sse_emit_punpcklqdq (p, ORC_AVX_SSE_REG (dest1), ORC_AVX_SSE_REG (zero), ORC_AVX_SSE_REG (dest1));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 0, 2, 0), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest2));
    orc_avx_sse_emit_punpcklqdq (p, ORC_AVX_SSE_REG (dest2), ORC_AVX_SSE_REG (zero), ORC_AVX_SSE_REG (dest2));
  }
}

static void
// split first/second words
avx_rule_splitlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* values of dest are shifted away so don't matter */

  /* FIXME slow */

  if (size >= 32) {
    // Note: unlike higher sized operands, there's no "pshufw"
    orc_avx_emit_psrad_imm (p, 16, src, dest1);
    orc_avx_emit_packssdw (p, dest1, dest1, dest1);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest1,
        dest1);

    orc_avx_emit_pslld_imm (p, 16, src, dest2);
    orc_avx_emit_psrad_imm (p, 16, dest2, dest2);
    orc_avx_emit_packssdw (p, dest2, dest2, dest2);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest2,
        dest2);
  } else {
    orc_avx_sse_emit_psrad_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest1));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest1), ORC_AVX_SSE_REG (dest1), ORC_AVX_SSE_REG (dest1));
    orc_avx_sse_emit_pslld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest2));
    orc_avx_sse_emit_psrad_imm (p, 16, ORC_AVX_SSE_REG (dest2), ORC_AVX_SSE_REG (dest2));
    orc_avx_sse_emit_packssdw (p, ORC_AVX_SSE_REG (dest2), ORC_AVX_SSE_REG (dest2), ORC_AVX_SSE_REG (dest2));
  }
}

static void
// split first/second bytes
avx_rule_splitwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int tmp = orc_compiler_get_constant (p, 2, 0xff);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* values of dest are shifted away so don't matter */

  ORC_DEBUG ("got tmp %d", tmp);
  /* FIXME slow */
  if (size >= 32) {
    orc_avx_emit_psraw_imm (p, 8, src, dest1);
    orc_avx_emit_packsswb (p, dest1, dest1, dest1);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest1,
        dest1);

    orc_avx_emit_pand (p, src, tmp, dest2);
    orc_avx_emit_packuswb (p, dest2, dest2, dest2);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest2,
        dest2);
  } else {
    orc_avx_sse_emit_psraw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest1));
    orc_avx_sse_emit_packsswb (p, ORC_AVX_SSE_REG (dest1), ORC_AVX_SSE_REG (dest1), ORC_AVX_SSE_REG (dest1));
    orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest2));
    orc_avx_sse_emit_packuswb (p, ORC_AVX_SSE_REG (dest2), ORC_AVX_SSE_REG (dest2), ORC_AVX_SSE_REG (dest2));
  }
}

static void
avx_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_sse_emit_punpckhbw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE (2, 0), dest, tmp, dest);
  } else {
    orc_avx_sse_emit_punpcklbw (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_sse_emit_punpckhwd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpcklwd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE (2, 0), dest, tmp, dest);
  } else {
    orc_avx_sse_emit_punpcklwd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_mergelq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_sse_emit_punpckhdq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_emit_permute2i128 (p, ORC_AVX_PERMUTE (2, 0), dest, tmp, dest);
  } else {
    orc_avx_sse_emit_punpckldq (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_swapw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_movdqa (p, src, tmp);
    orc_avx_emit_psllw_imm (p, 8, src, tmp);
    orc_avx_emit_psrlw_imm (p, 8, src, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_swapl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pslld_imm (p, 16, src, tmp);
    orc_avx_emit_psrld_imm (p, 16, src, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
    orc_avx_emit_psllw_imm (p, 8, dest, tmp);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pslld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_swapwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pslld_imm (p, 16, src, tmp);
    orc_avx_emit_psrld_imm (p, 16, src, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pslld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrld_imm (p, 16, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_swapq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_psllq_imm (p, 32, src, tmp);
    orc_avx_emit_psrlq_imm (p, 32, src, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
    orc_avx_emit_pslld_imm (p, 16, dest, tmp);
    orc_avx_emit_psrld_imm (p, 16, dest, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
    orc_avx_emit_psllw_imm (p, 8, dest, tmp);
    orc_avx_emit_psrlw_imm (p, 8, dest, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_psllq_imm (p, 32, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlq_imm (p, 32, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pslld_imm (p, 16, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrld_imm (p, 16, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psllw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrlw_imm (p, 8, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_swaplq (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), src, dest);
  } else {
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (2, 3, 0, 1), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_swapw_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_swapw (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x02030001,
        0x06070405, 0x0a0b0809, 0x0e0f0c0d);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_swapw (p, user, insn);
    }
  }
}

static void
avx_rule_swapl_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_swapl (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x00010203,
      0x04050607, 0x08090a0b, 0x0c0d0e0f);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp),
          ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_swapl (p, user, insn);
    }
  }
}

static void
avx_rule_swapwl_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_swapwl (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x01000302,
        0x05040706, 0x09080b0a, 0x0d0c0f0e);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp),
          ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_swapwl (p, user, insn);
    }
  }
}

static void
avx_rule_swapq_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_swapq (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x04050607,
        0x00010203, 0x0c0d0e0f, 0x08090a0b);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp),
          ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_swapq (p, user, insn);
    }
  }
}

static void
avx_rule_splitlw_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_splitlw (p, user, insn);
  } else {
    const int tmp1 = orc_compiler_try_get_constant_long (p, 0x07060302,
        0x0f0e0b0a, 0x07060302, 0x0f0e0b0a);
    const int tmp2 = orc_compiler_try_get_constant_long (p, 0x05040100,
        0x0d0c0908, 0x05040100, 0x0d0c0908);
    if (tmp1 != ORC_REG_INVALID && tmp2 != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp1),
          ORC_AVX_SSE_REG (dest1));
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp2),
          ORC_AVX_SSE_REG (dest2));
    } else {
      avx_rule_splitlw (p, user, insn);
    }
  }
}

static void
avx_rule_splitwb_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest1 = p->vars[insn->dest_args[0]].alloc;
  const int dest2 = p->vars[insn->dest_args[1]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_splitwb (p, user, insn);
  } else {
    const int tmp1 = orc_compiler_try_get_constant_long (p, 0x07050301,
        0x0f0d0b09, 0x07050301, 0x0f0d0b09);
    const int tmp2 = orc_compiler_try_get_constant_long (p, 0x06040200,
        0x0e0c0a08, 0x06040200, 0x0e0c0a08);
    if (tmp1 != ORC_REG_INVALID && tmp2 != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp1), ORC_AVX_SSE_REG (dest1));
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest2));
    } else {
      avx_rule_splitwb (p, user, insn);
    }
  }
}

static void
avx_rule_select0lw_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_select0lw (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x05040100,
        0x0d0c0908, 0x05040100, 0x0d0c0908);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_select0lw (p, user, insn);
    }
  }
}

static void
avx_rule_select1lw_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_select1lw (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x07060302,
        0x0f0e0b0a, 0x07060302, 0x0f0e0b0a);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_select1lw (p, user, insn);
    }
  }
}

static void
avx_rule_select0wb_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_select0wb (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x06040200,
        0x0e0c0a08, 0x06040200, 0x0e0c0a08);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_select0wb (p, user, insn);
    }
  }
}

static void
avx_rule_select1wb_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    avx_rule_select1wb (p, user, insn);
  } else {
    const int tmp = orc_compiler_try_get_constant_long (p, 0x07050301,
        0x0f0d0b09, 0x07050301, 0x0f0d0b09);
    if (tmp != ORC_REG_INVALID) {
      orc_avx_sse_emit_pshufb (p, ORC_AVX_SSE_REG (src),
          ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    } else {
      avx_rule_select1wb (p, user, insn);
    }
  }
}

/* slow rules */
/* note that we aim for AVX2, hence some were not ported */

static void
avx_rule_avgsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 1, 0x80);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pxor (p, src1, tmp, src1);
    orc_avx_emit_pxor (p, src0, tmp, dest);
    orc_avx_emit_pavgb (p, dest, src1, dest);
    orc_avx_emit_pxor (p, src1, tmp, src1);
    orc_avx_emit_pxor (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pavgb (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
// (a + b + 1) >> 1 (signed)
avx_rule_avgsw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_constant (p, 2, 0x8000);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pxor (p, src1, tmp, src1);
    orc_avx_emit_pxor (p, src0, tmp, dest);
    orc_avx_emit_pavgw (p, dest, src1, dest);
    orc_avx_emit_pxor (p, src1, tmp, src1);
    orc_avx_emit_pxor (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pavgw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
// (a + b + 1) >> 1 (signed)
avx_rule_avgsl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* (a+b+1) >> 1 = (a|b) - ((a^b)>>1) */

  if (size >= 32) {
    orc_avx_emit_pxor (p, src0, src1, tmp);
    orc_avx_emit_psrad_imm (p, 1, tmp, tmp);

    orc_avx_emit_por (p, src0, src1, dest);
    orc_avx_emit_psubd (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrad_imm (p, 1, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psubd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
// (a + b + 1) >> 1 (unsigned)
avx_rule_avgul (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  /* (a+b+1) >> 1 = (a|b) - ((a^b)>>1) */

  if (size >= 32) {
    orc_avx_emit_movdqa (p, src0, tmp);
    orc_avx_emit_pxor (p, tmp, src1, tmp);
    orc_avx_emit_psrld_imm (p, 1, tmp, tmp);

    orc_avx_emit_por (p, src0, src1, dest);
    orc_avx_emit_psubd (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrld_imm (p, 1, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_psubd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
// clamp(a + b) (signed saturate)
avx_rule_addssl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

#if 0
  int tmp2 = orc_compiler_get_temp_reg (p);
  int tmp3 = orc_compiler_get_temp_reg (p);

  orc_sse_emit_movdqa (p, src1, tmp);
  orc_sse_emit_pand (p, dest, tmp);

  orc_sse_emit_movdqa (p, src1, tmp2);
  orc_sse_emit_pxor (p, dest, tmp2);
  orc_sse_emit_psrad_imm (p, 1, tmp2);
  orc_sse_emit_paddd (p, tmp2, tmp);

  orc_sse_emit_psrad (p, 30, tmp);
  orc_sse_emit_pslld (p, 30, tmp);
  orc_sse_emit_movdqa (p, tmp, tmp2);
  orc_sse_emit_pslld_imm (p, 1, tmp2);
  orc_sse_emit_movdqa (p, tmp, tmp3);
  orc_sse_emit_pxor (p, tmp2, tmp3);
  orc_sse_emit_psrad_imm (p, 31, tmp3);

  orc_sse_emit_psrad_imm (p, 31, tmp2);
  tmp = orc_compiler_get_constant (p, 4, 0x80000000);
  orc_sse_emit_pxor (p, tmp, tmp2); /*  clamped value */
  orc_sse_emit_pand (p, tmp3, tmp2);

  orc_sse_emit_paddd (p, src1, dest);
  orc_sse_emit_pandn (p, dest, tmp3); /*  tmp is mask: ~0 is for clamping */
  orc_sse_emit_movdqa (p, tmp3, dest);

  orc_sse_emit_por (p, tmp2, dest);
#endif

  const int s = orc_compiler_get_temp_reg (p);
  const int t = orc_compiler_get_temp_reg (p);

  /*
     From Tim Terriberry: (slightly faster than above)

     m=0xFFFFFFFF;
     s=_a;
     t=_a;
     s^=_b;
     _a+=_b;
     t^=_a;
     t^=m;
     m>>=1;
     s|=t;
     t=_b;
     s>>=31;
     t>>=31;
     _a&=s;
     t^=m;
     s=~s&t;
     _a|=s;
  */

  // Again, src0 == dest means we may not be able to access it again
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_movdqa (p, src0, s);
    orc_avx_emit_movdqa (p, src0, t);
    orc_avx_emit_pxor (p, src0, src1, s);
    orc_avx_emit_paddd (p, src0, src1, dest);
    orc_avx_emit_pxor (p, t, dest, t);
    int tmp = orc_compiler_get_constant (p, 4, 0xffffffff);
    orc_avx_emit_pxor (p, t, tmp, t);
    orc_avx_emit_por (p, s, t, s);
    orc_avx_emit_psrad_imm (p, 31, s, s);
    orc_avx_emit_psrad_imm (p, 31, src1, t);
    orc_avx_emit_pand (p, dest, s, dest);
    tmp = orc_compiler_get_constant (p, 4, 0x7fffffff);
    orc_avx_emit_pxor (p, t, tmp, t);
    orc_avx_emit_pandn (p, s, t, s);
    orc_avx_emit_por (p, dest, s, dest);
  } else {
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (s));
    orc_avx_sse_emit_movdqa (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (t));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (s));
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (t), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (t));
    int tmp = orc_compiler_get_constant (p, 4, 0xffffffff);
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (t), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (t));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (s), ORC_AVX_SSE_REG (t), ORC_AVX_SSE_REG (s));
    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (s), ORC_AVX_SSE_REG (s));
    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (t));
    orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (s), ORC_AVX_SSE_REG (dest));
    tmp = orc_compiler_get_constant (p, 4, 0x7fffffff);
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (t), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (t));
    orc_avx_sse_emit_pandn (p, ORC_AVX_SSE_REG (s), ORC_AVX_SSE_REG (t), ORC_AVX_SSE_REG (s));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (s), ORC_AVX_SSE_REG (dest));
  }
}

static void
// clamp(a - b) (signed saturate)
avx_rule_subssl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = orc_compiler_get_temp_constant (p, 4, 0xffffffff);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int tmp3 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pxor (p, tmp, src1, tmp);
    orc_avx_emit_pxor (p, tmp, src0, tmp2);
    // Exchanging the ops here breaks the need for movdqa
    orc_avx_emit_por (p, tmp, src0, tmp);
    orc_avx_emit_psrad_imm (p, 1, tmp2, tmp2);
    orc_avx_emit_psubd (p, tmp, tmp2, tmp);

    orc_avx_emit_psrad_imm (p, 30, tmp, tmp);
    orc_avx_emit_pslld_imm (p, 30, tmp, tmp);
    orc_avx_emit_pslld_imm (p, 1, tmp, tmp2);
    orc_avx_emit_pxor (p, tmp, tmp2, tmp3);
    orc_avx_emit_psrad_imm (p, 31, tmp3,
        tmp3); /*  tmp3 is mask: ~0 is for clamping */

    orc_avx_emit_psrad_imm (p, 31, tmp2, tmp2);
    tmp = orc_compiler_get_constant (p, 4, 0x80000000);
    orc_avx_emit_pxor (p, tmp2, tmp, tmp2); /*  clamped value */
    orc_avx_emit_pand (p, tmp2, tmp3, tmp2);

    orc_avx_emit_psubd (p, src0, src1, dest);
    orc_avx_emit_pandn (p, tmp3, dest, dest);

    orc_avx_emit_por (p, dest, tmp2, dest);
  } else {
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp2));
    // Exchanging the ops here breaks the need for movdqa
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psrad_imm (p, 1, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psubd (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_psrad_imm (p, 30, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pslld_imm (p, 30, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_pslld_imm (p, 1, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp3));
    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (tmp3),
        ORC_AVX_SSE_REG (tmp3)); /*  tmp3 is mask: ~0 is for clamping */

    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    tmp = orc_compiler_get_constant (p, 4, 0x80000000);
    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2)); /*  clamped value */
    orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (tmp2));

    orc_avx_sse_emit_psubd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pandn (p, ORC_AVX_SSE_REG (tmp3), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));

    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest));
  }
}

static void
/* clamp (a + b) (unsigned saturate)*/
avx_rule_addusl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);

#if 0
  /* an alternate version.  slower. */
  /* Compute the bit that gets carried from bit 0 to bit 1 */
  orc_sse_emit_movdqa (p, src1, tmp);
  orc_sse_emit_pand (p, dest, tmp);
  orc_sse_emit_pslld_imm (p, 31, tmp);
  orc_sse_emit_psrld_imm (p, 31, tmp);

  /* Add in (src>>1) */
  orc_sse_emit_movdqa (p, src1, tmp2);
  orc_sse_emit_psrld_imm (p, 1, tmp2);
  orc_sse_emit_paddd (p, tmp2, tmp);

  /* Add in (dest>>1) */
  orc_sse_emit_movdqa (p, dest, tmp2);
  orc_sse_emit_psrld_imm (p, 1, tmp2);
  orc_sse_emit_paddd (p, tmp2, tmp);

  /* turn overflow bit into mask */
  orc_sse_emit_psrad_imm (p, 31, tmp);

  /* compute the sum, then or over the mask */
  orc_sse_emit_paddd (p, src1, dest);
  orc_sse_emit_por (p, tmp, dest);
#endif
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_pand (p, src0, src1, tmp);

    orc_avx_emit_pxor (p, src1, src0, tmp2);
    orc_avx_emit_psrld_imm (p, 1, tmp2, tmp2);
    orc_avx_emit_paddd (p, tmp, tmp2, tmp);

    orc_avx_emit_psrad_imm (p, 31, tmp, tmp);
    orc_avx_emit_paddd (p, dest, src1, dest);
    orc_avx_emit_por (p, dest, tmp, dest);
  } else {
    orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_pxor (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_psrld_imm (p, 1, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp));

    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
/* clamp(a - b) */
avx_rule_subusl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmp2 = orc_compiler_get_temp_reg (p);
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_psrld_imm (p, 1, src1, tmp2);

    orc_avx_emit_psrld_imm (p, 1, src0, tmp);
    orc_avx_emit_psubd (p, tmp2, tmp, tmp2);

    /* turn overflow bit into mask */
    orc_avx_emit_psrad_imm (p, 31, tmp2, tmp2);

    /* compute the difference, then and over the mask */
    orc_avx_emit_psubd (p, src0, src1, dest);
    orc_avx_emit_pand (p, tmp2, dest, dest);
  } else {
    orc_avx_sse_emit_psrld_imm (p, 1, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp2));

    orc_avx_sse_emit_psrld_imm (p, 1, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_psubd (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp2));

    /* turn overflow bit into mask */
    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (tmp2));

    /* compute the difference, then and over the mask */
    orc_avx_sse_emit_psubd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pand (p, ORC_AVX_SSE_REG (tmp2), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

/* float ops */

BINARY (addf, addps)
BINARY (subf, subps)
BINARY (mulf, mulps)
BINARY (divf, divps)
UNARY (sqrtf, sqrtps)
BINARY (orf, orps)
BINARY (andf, andps)

BINARY (addd, addpd)
BINARY (subd, subpd)
BINARY (muld, mulpd)
BINARY (divd, divpd)
UNARY (sqrtd, sqrtpd)

static void
avx_rule_minf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->target_flags & ORC_TARGET_FAST_NAN) {
    if (size >= 32) {
      orc_avx_emit_minps (p, src1, src0, dest);
    } else {
      orc_avx_sse_emit_minps (p, src1, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
    }
  } else {
    const int tmp = orc_compiler_get_temp_reg (p);
    if (size >= 32) {
      // src1 = min(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_emit_minps (p, src1, src0, tmp);
      // dest = min(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_emit_minps (p, src0, src1, dest);
      // OR the results to combine NaNs
      // 4-23 Vol. 2B Intel Intrinsics Manual
      orc_avx_emit_por (p, dest, tmp, dest);
    } else {
      // src1 = min(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_sse_emit_minps (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
      // dest = min(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_sse_emit_minps (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
      // OR the results to combine NaNs
      // 4-23 Vol. 2B Intel Intrinsics Manual
      orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  }
}

static void
/* return the maximum of two values or NaN if present */
avx_rule_mind (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->target_flags & ORC_TARGET_FAST_NAN) {
    if (size >= 32) {
      orc_avx_emit_minpd (p, src0, src1, dest);
    } else {
      /* FIXME, should be sse version right? */
      orc_avx_emit_minpd (p, src0, src1, dest);
    }
  } else {
    const int tmp = orc_compiler_get_temp_reg (p);
    if (size >= 32) {
      // src1 = min(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_emit_minpd (p, src1, src0, tmp);
      // dest = min(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_emit_minpd (p, src0, src1, dest);
      // OR the results to combine NaNs
      // 4-23 Vol. 2B Intel Intrinsics Manual
      orc_avx_emit_por (p, tmp, dest, dest);
    } else {
      // src1 = min(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_sse_emit_minpd (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (tmp));
      // dest = min(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_sse_emit_minpd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
      // OR the results to combine NaNs
      // 4-23 Vol. 2B Intel Intrinsics Manual
      orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
    }
  }
}

static void
/* return the maximum of two values or NaN if present */
avx_rule_maxf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->target_flags & ORC_TARGET_FAST_NAN) {
    if (size >= 32) {
      orc_avx_emit_maxps (p, src0, src1, dest);
    } else {
      orc_avx_sse_emit_maxps (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    }
  } else {
    const int tmp = orc_compiler_get_temp_reg (p);
    if (size >= 32) {
      // src1 = max(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_emit_maxps (p, src0, src1, tmp);
      // dest = max(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_emit_maxps (p, src1, src0, dest);
      // OR the results to combine NaNs
      // 4-12 Vol. 2B Intel Intrinsics Manual
      orc_avx_emit_por (p, dest, tmp, dest);
    } else {
      // src1 = max(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_sse_emit_maxps (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
      // dest = max(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_sse_emit_maxps (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
      // OR the results to combine NaNs
      // 4-12 Vol. 2B Intel Intrinsics Manual
      orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  }
}

static void
/* return the maximum of two values or NaN if present */
avx_rule_maxd (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src0 = p->vars[insn->src_args[0]].alloc;
  const int src1 = p->vars[insn->src_args[1]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (p->target_flags & ORC_TARGET_FAST_NAN) {
    if (size >= 32) {
      orc_avx_emit_maxpd (p, src0, src1, dest);
    } else {
      orc_avx_sse_emit_maxpd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (dest));
    }
  } else {
    const int tmp = orc_compiler_get_temp_reg (p);
    if (size >= 32) {
      // src1 = max(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_emit_maxpd (p, src0, src1, tmp);
      // dest = max(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_emit_maxpd (p, src1, src0, dest);
      // OR the results to combine NaNs
      // 4-12 Vol. 2B Intel Intrinsics Manual
      orc_avx_emit_por (p, dest, tmp, dest);
    } else {
      // src1 = max(src0, src1)
      // if src1 contains a SNaN, it is returned
      // if src0 contains a NaN, the second operand is returned
      orc_avx_sse_emit_maxpd (p, ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (tmp));
      // dest = max(src1, src0)
      // if src0 contains a SNaN, it is returned
      // if src1 contains a NaN, the second operand is returned
      orc_avx_sse_emit_maxpd (p, ORC_AVX_SSE_REG (src1), ORC_AVX_SSE_REG (src0), ORC_AVX_SSE_REG (dest));
      // OR the results to combine NaNs
      // 4-12 Vol. 2B Intel Intrinsics Manual
      orc_avx_sse_emit_por (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    }
  }
}

// equality
BINARY (cmpeqf, cmpeqps);
BINARY (cmpeqd, cmpeqpd);
// less than
BINARY (cmpltf, cmpltps);
BINARY (cmpltd, cmpltpd);
// less than or equal
BINARY (cmplef, cmpleps);
BINARY (cmpled, cmplepd);

static void
// convert floats to int32_t
avx_rule_convfl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmpc = orc_compiler_get_temp_constant (p, 4, 0x80000000);

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    // extract the sign bit before we do anything (see later)
    orc_avx_emit_psrad_imm (p, 31, src, tmp);
    // convert from float (YMM) to integer (YMM)
    orc_avx_emit_cvttps2dq (p, src, dest);
    // DENORMAL CHECK -- cvt* is not IEEE 754 compliant
    // if the resulting integer is 0 (positive)
    orc_avx_emit_pcmpeqd (p, tmpc, dest, tmpc);
    // and the sign bit of the source is not true...
    orc_avx_emit_pandn (p, tmp, tmpc, tmp);
    // set all the matching integers to -1
    orc_avx_emit_paddd (p, dest, tmp, dest);
  } else {
    // extract the sign bit before we do anything (see later)
    orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    // convert from float (YMM) to integer (YMM)
    orc_avx_sse_emit_cvttps2dq (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    // DENORMAL CHECK -- cvt* is not IEEE 754 compliant
    // if the resulting integer is 0 (positive)
    orc_avx_sse_emit_pcmpeqd (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmpc));
    // and the sign bit of the source is not true...
    orc_avx_sse_emit_pandn (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp));
    // set all the matching integers to -1
    orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
  }
}

static void
// convert doubles to int32_t
avx_rule_convdl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int tmp = orc_compiler_get_temp_reg (p);
  const int tmpc = orc_compiler_get_temp_constant (p, 4, 0x80000000);

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    // extract high halves from all operands
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 3, 1), src, tmp);
    // now we need that high lane, low half from tmp into its low lane, high half
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF(3, 1, 2, 0), tmp, tmp);
    // convert from double (YMM) to integer (XMM)
    orc_avx_emit_cvttpd2dq (p, src, dest);
  } else {
    // extract high halves from all operands
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 3, 1), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    // convert from double (YMM) to integer (XMM)
    orc_avx_sse_emit_cvttpd2dq (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
  // DENORMAL CHECK -- cvt* is not IEEE 754 compliant
  // if the conversion resulted in an invalid integer...
  orc_avx_sse_emit_psrad_imm (p, 31, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmp));
  orc_avx_sse_emit_pcmpeqd (p, ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmpc));
  // and the resulting integer is 0 (positive)
  orc_avx_sse_emit_pandn (p, ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (tmpc), ORC_AVX_SSE_REG (tmp));
  // set all the matching integers to -1
  orc_avx_sse_emit_paddd (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
}

// convert int32_t to float
UNARY (convlf, cvtdq2ps);

static void
// convert int16 to floats
avx_rule_convwf (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 16) {
    orc_avx_emit_pmovsxwd (p, ORC_AVX_SSE_REG (src), dest);
    orc_avx_emit_cvtdq2ps (p, dest, dest);
  } else {
    orc_avx_sse_emit_pmovsxwd (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_cvtdq2ps (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

// convert int32_t to double, upper lane of src is ignored
UNARY_W (convld, cvtdq2pd, 16);
// convert float to double, upper lane of src is ignored
UNARY_W (convfd, cvtps2pd, 16);
// convert two doubles to floats
UNARY_W_SSE (convdf, cvtpd2ps, 32);

// convert to signed
UNARY_AVX2_ONLY_W_SSE (convsbw, pmovsxbw, 16);
UNARY_AVX2_ONLY_W_SSE (convswl, pmovsxwd, 16);
UNARY_AVX2_ONLY_W_SSE (convslq, pmovsxdq, 16);
// convert to unsigned
UNARY_AVX2_ONLY_W_SSE (convubw, pmovzxbw, 16);
UNARY_AVX2_ONLY_W_SSE (convuwl, pmovzxwd, 16);
UNARY_AVX2_ONLY_W_SSE (convulq, pmovzxdq, 16);

// clamp(a) from word to byte
static void
avx_rule_convssslw_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_packssdw (p, dest, src, dest);
    // full interleave required again
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF(3, 1, 2, 0), dest, dest);
  } else {
    /* FIXME, should be sse version right? */
    orc_avx_emit_packssdw (p, dest, src, dest);
  }
}

static void
avx_rule_convsuslw_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;

  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;

  if (size >= 32) {
    orc_avx_emit_packusdw (p, dest, src, dest);
    // full interleave required again
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF(3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_packusdw (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (dest));
  }
}

static void
avx_rule_convsssql_avx2 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  const int src = p->vars[insn->src_args[0]].alloc;
  const int dest = p->vars[insn->dest_args[0]].alloc;
  const int size = p->vars[insn->src_args[0]].size << p->loop_shift;
  const int tmpc_max = orc_compiler_get_temp_constant (p, 8, INT32_MAX);
  const int tmpc_min = orc_compiler_get_temp_constant (p, 8, INT32_MIN);
  const int tmp = orc_compiler_get_temp_reg (p);

  if (size >= 32) {
    orc_avx_emit_pcmpgtq (p, src, tmpc_max, tmp);
    orc_avx_emit_blendvpd (p, src, tmpc_max, tmp, dest);
    orc_avx_emit_pcmpgtq (p, dest, tmpc_min, tmp);
    orc_avx_emit_blendvpd (p, tmpc_min, dest, tmp, dest);
    // full interleave required again
    orc_avx_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
    orc_avx_emit_permute4x64_imm (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), dest, dest);
  } else {
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpc_max), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd (p, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmpc_max), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pcmpgtq (p, ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmpc_min), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_blendvpd (p, ORC_AVX_SSE_REG (tmpc_min), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (dest));
    orc_avx_sse_emit_pshufd (p, ORC_AVX_SSE_SHUF (3, 1, 2, 0), ORC_AVX_SSE_REG (dest), ORC_AVX_SSE_REG (dest));
  }
}

void
orc_compiler_avx_register_rules (OrcTarget *target)
{
#define REGISTER_RULE(x) orc_rule_register (rule_set, #x, avx_rule_##x, NULL)
#define REGISTER_RULE_WITH_GENERIC(x, y) \
  orc_rule_register (rule_set, #x, avx_rule_##y, NULL)
#define REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD(x, y, z) \
  orc_rule_register (rule_set, #x, avx_rule_##y, (void *)z)

  /* AVX */
  OrcRuleSet *rule_set = orc_rule_set_new (orc_opcode_set_get ("sys"), target,
      ORC_TARGET_AVX_AVX);

  REGISTER_RULE_WITH_GENERIC (loadb, loadX);
  REGISTER_RULE_WITH_GENERIC (loadw, loadX);
  REGISTER_RULE_WITH_GENERIC (loadl, loadX);
  REGISTER_RULE_WITH_GENERIC (loadq, loadX);
  REGISTER_RULE_WITH_GENERIC (loadoffb, loadoffX);
  REGISTER_RULE_WITH_GENERIC (loadoffw, loadoffX);
  REGISTER_RULE_WITH_GENERIC (loadoffl, loadoffX);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (loadpb, loadpX, 1);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (loadpw, loadpX, 2);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (loadpl, loadpX, 4);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (loadpq, loadpX, 8);

  REGISTER_RULE_WITH_GENERIC (storeb, storeX);
  REGISTER_RULE_WITH_GENERIC (storew, storeX);
  REGISTER_RULE_WITH_GENERIC (storel, storeX);
  REGISTER_RULE_WITH_GENERIC (storeq, storeX);

  REGISTER_RULE (xorb);

  REGISTER_RULE (xorw);

  REGISTER_RULE (xorl);

  REGISTER_RULE (xorq);

  REGISTER_RULE (select0ql);
  REGISTER_RULE (select1ql);
  REGISTER_RULE (select0lw);
  REGISTER_RULE (select1lw);
  REGISTER_RULE (select0wb);
  REGISTER_RULE (select1wb);
  REGISTER_RULE (mergebw);
  REGISTER_RULE (mergewl);
  REGISTER_RULE (mergelq);

  REGISTER_RULE_WITH_GENERIC (copyb, copyx);
  REGISTER_RULE_WITH_GENERIC (copyw, copyx);
  REGISTER_RULE_WITH_GENERIC (copyl, copyx);
  REGISTER_RULE_WITH_GENERIC (copyq, copyx);

  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shlw, shift, 0);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shruw, shift, 1);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shrsw, shift, 2);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shll, shift, 3);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shrul, shift, 4);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shrsl, shift, 5);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shlq, shift, 6);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (shruq, shift, 7);
  REGISTER_RULE (shrsq);

  REGISTER_RULE (convssswb);
  REGISTER_RULE (convsuswb);
  REGISTER_RULE (convuuswb);
  REGISTER_RULE (convusswb);
  REGISTER_RULE (convwb);

  REGISTER_RULE (convql);

  REGISTER_RULE (mulsbw);
  REGISTER_RULE (mulubw);
  REGISTER_RULE (mulswl);
  REGISTER_RULE (muluwl);

  REGISTER_RULE (accw);
  REGISTER_RULE (accl);
  REGISTER_RULE (accsadubl);

  REGISTER_RULE (mululq);

  REGISTER_RULE (addf);
  REGISTER_RULE (subf);
  REGISTER_RULE (mulf);
  REGISTER_RULE (divf);
  REGISTER_RULE (minf);
  REGISTER_RULE (maxf);
  REGISTER_RULE (sqrtf);
  REGISTER_RULE (cmpeqf);
  REGISTER_RULE (cmpltf);
  REGISTER_RULE (cmplef);
  REGISTER_RULE (convfl);
  REGISTER_RULE (convwf);
  REGISTER_RULE (convlf);
  REGISTER_RULE (orf);
  REGISTER_RULE (andf);

  REGISTER_RULE (addd);
  REGISTER_RULE (subd);
  REGISTER_RULE (muld);
  REGISTER_RULE (divd);
  REGISTER_RULE (mind);
  REGISTER_RULE (maxd);
  REGISTER_RULE (sqrtd);
  REGISTER_RULE (cmpeqd);
  REGISTER_RULE (cmpltd);
  REGISTER_RULE (cmpled);
  REGISTER_RULE (convdl);
  REGISTER_RULE (convld);

  REGISTER_RULE (convfd);
  REGISTER_RULE (convdf);

  /* slow rules */
  REGISTER_RULE_WITH_GENERIC (avgsb, avgsb_slow);
  REGISTER_RULE_WITH_GENERIC (avgsw, avgsw_slow);
  REGISTER_RULE (convusslw);
  REGISTER_RULE (convuuslw);
  REGISTER_RULE (convlw);
  REGISTER_RULE (swapw);
  REGISTER_RULE (swapl);
  REGISTER_RULE (swapwl);
  REGISTER_RULE (swapq);
  REGISTER_RULE (swaplq);
  REGISTER_RULE (splitql);
  REGISTER_RULE (splitlw);
  REGISTER_RULE (splitwb);
  REGISTER_RULE (avgsl);
  REGISTER_RULE (avgul);
  REGISTER_RULE (shlb);
  REGISTER_RULE (shrsb);
  REGISTER_RULE (shrub);
  REGISTER_RULE (mulhul);
  REGISTER_RULE (mullb);
  REGISTER_RULE (mulhsb);
  REGISTER_RULE (mulhub);
  REGISTER_RULE_WITH_GENERIC (addssl, addssl_slow);
  REGISTER_RULE_WITH_GENERIC (subssl, subssl_slow);
  REGISTER_RULE_WITH_GENERIC (addusl, addusl_slow);
  REGISTER_RULE_WITH_GENERIC (subusl, subusl_slow);
  REGISTER_RULE (convhwb);
  REGISTER_RULE (convhlw);
  REGISTER_RULE (splatw3q);
  REGISTER_RULE (splatbw);
  REGISTER_RULE (splatbl);
  REGISTER_RULE (div255w);
  REGISTER_RULE (divluw);

  /* AVX2 comprises most post-SSE2 instructions */
  rule_set = orc_rule_set_new (orc_opcode_set_get ("sys"), target,
      ORC_TARGET_AVX_AVX | ORC_TARGET_AVX_AVX2);

  REGISTER_RULE_WITH_GENERIC (addb, addb_avx2);
  REGISTER_RULE_WITH_GENERIC (addssb, addssb_avx2);
  REGISTER_RULE_WITH_GENERIC (addusb, addusb_avx2);
  REGISTER_RULE_WITH_GENERIC (andb, andb_avx2);
  REGISTER_RULE_WITH_GENERIC (andnb, andnb_avx2);
  REGISTER_RULE_WITH_GENERIC (avgub, avgub_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpeqb, cmpeqb_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpgtsb, cmpgtsb_avx2);
  REGISTER_RULE_WITH_GENERIC (maxub, maxub_avx2);
  REGISTER_RULE_WITH_GENERIC (minub, minub_avx2);
  REGISTER_RULE_WITH_GENERIC (orb, orb_avx2);
  REGISTER_RULE_WITH_GENERIC (subb, subb_avx2);
  REGISTER_RULE_WITH_GENERIC (subssb, subssb_avx2);
  REGISTER_RULE_WITH_GENERIC (subusb, subusb_avx2);

  REGISTER_RULE_WITH_GENERIC (addw, addw_avx2);
  REGISTER_RULE_WITH_GENERIC (addssw, addssw_avx2);
  REGISTER_RULE_WITH_GENERIC (addusw, addusw_avx2);
  REGISTER_RULE_WITH_GENERIC (andw, andw_avx2);
  REGISTER_RULE_WITH_GENERIC (andnw, andnw_avx2);
  REGISTER_RULE_WITH_GENERIC (avguw, avguw_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpeqw, cmpeqw_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpgtsw, cmpgtsw_avx2);
  REGISTER_RULE_WITH_GENERIC (maxsw, maxsw_avx2);
  REGISTER_RULE_WITH_GENERIC (minsw, minsw_avx2);
  REGISTER_RULE_WITH_GENERIC (mullw, mullw_avx2);
  REGISTER_RULE_WITH_GENERIC (mulhsw, mulhsw_avx2);
  REGISTER_RULE_WITH_GENERIC (mulhuw, mulhuw_avx2);
  REGISTER_RULE_WITH_GENERIC (orw, orw_avx2);
  REGISTER_RULE_WITH_GENERIC (subw, subw_avx2);
  REGISTER_RULE_WITH_GENERIC (subssw, subssw_avx2);
  REGISTER_RULE_WITH_GENERIC (subusw, subusw_avx2);

  REGISTER_RULE_WITH_GENERIC (addl, addl_avx2);
  REGISTER_RULE_WITH_GENERIC (andl, andl_avx2);
  REGISTER_RULE_WITH_GENERIC (andnl, andnl_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpeql, cmpeql_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpgtsl, cmpgtsl_avx2);
  REGISTER_RULE_WITH_GENERIC (orl, orl_avx2);
  REGISTER_RULE_WITH_GENERIC (subl, subl_avx2);

  REGISTER_RULE_WITH_GENERIC (addq, addq_avx2);
  REGISTER_RULE_WITH_GENERIC (andq, andq_avx2);
  REGISTER_RULE_WITH_GENERIC (andnq, andnq_avx2);
  REGISTER_RULE_WITH_GENERIC (orq, orq_avx2);
  REGISTER_RULE_WITH_GENERIC (subq, subq_avx2);

  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (signb, signX_avx2, 0);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (signw, signX_avx2, 1);
  REGISTER_RULE_WITH_GENERIC_AND_PAYLOAD (signl, signX_avx2, 2);
  REGISTER_RULE_WITH_GENERIC (absb, absb_avx2);
  REGISTER_RULE_WITH_GENERIC (absw, absw_avx2);
  REGISTER_RULE_WITH_GENERIC (absl, absl_avx2);
  REGISTER_RULE_WITH_GENERIC (swapw, swapw_avx2);
  REGISTER_RULE_WITH_GENERIC (swapl, swapl_avx2);
  REGISTER_RULE_WITH_GENERIC (swapwl, swapwl_avx2);
  REGISTER_RULE_WITH_GENERIC (swapq, swapq_avx2);
  REGISTER_RULE_WITH_GENERIC (splitlw, splitlw_avx2);
  REGISTER_RULE_WITH_GENERIC (splitwb, splitwb_avx2);
  REGISTER_RULE_WITH_GENERIC (select0lw, select0lw_avx2);
  REGISTER_RULE_WITH_GENERIC (select1lw, select1lw_avx2);
  REGISTER_RULE_WITH_GENERIC (select0wb, select0wb_avx2);
  REGISTER_RULE_WITH_GENERIC (select1wb, select1wb_avx2);

  REGISTER_RULE_WITH_GENERIC (maxsb, maxsb_avx2);
  REGISTER_RULE_WITH_GENERIC (minsb, minsb_avx2);
  REGISTER_RULE_WITH_GENERIC (maxuw, maxuw_avx2);
  REGISTER_RULE_WITH_GENERIC (minuw, minuw_avx2);
  REGISTER_RULE_WITH_GENERIC (maxsl, maxsl_avx2);
  REGISTER_RULE_WITH_GENERIC (maxul, maxul_avx2);
  REGISTER_RULE_WITH_GENERIC (minsl, minsl_avx2);
  REGISTER_RULE_WITH_GENERIC (minul, minul_avx2);
  REGISTER_RULE_WITH_GENERIC (mulll, mulll_avx2);

  REGISTER_RULE_WITH_GENERIC (convsbw, convsbw_avx2);
  REGISTER_RULE_WITH_GENERIC (convubw, convubw_avx2);
  REGISTER_RULE_WITH_GENERIC (convslq, convslq_avx2);
  REGISTER_RULE_WITH_GENERIC (convulq, convulq_avx2);
  REGISTER_RULE_WITH_GENERIC (convswl, convswl_avx2);
  REGISTER_RULE_WITH_GENERIC (convuwl, convuwl_avx2);
  REGISTER_RULE_WITH_GENERIC (convslq, convslq_avx2);
  REGISTER_RULE_WITH_GENERIC (convulq, convulq_avx2);
  REGISTER_RULE_WITH_GENERIC (convssslw, convssslw_avx2);
  REGISTER_RULE_WITH_GENERIC (convsuslw, convsuslw_avx2);
  REGISTER_RULE_WITH_GENERIC (convuusql, convuusql_avx2);
  REGISTER_RULE_WITH_GENERIC (convsusql, convsusql_avx2);
  REGISTER_RULE_WITH_GENERIC (convussql, convussql_avx2);
  REGISTER_RULE_WITH_GENERIC (convsssql, convsssql_avx2);
  REGISTER_RULE_WITH_GENERIC (mulslq, mulslq_avx2);
  REGISTER_RULE_WITH_GENERIC (mulhsl, mulhsl_avx2);
  REGISTER_RULE_WITH_GENERIC (cmpeqq, cmpeqq_avx2);

  REGISTER_RULE_WITH_GENERIC (cmpgtsq, cmpgtsq_avx2);

  // These rules require dropping into SSE to be implemented in straight AVX
  REGISTER_RULE_WITH_GENERIC (loadupdb, loadupdb_avx2);
  REGISTER_RULE_WITH_GENERIC (loadupib, loadupib_avx2);
  // The following rules need optimization -- they're more than twice slower
  // than their SSE counterparts, and even more wrt the scalar implementation
  // REGISTER_RULE_WITH_GENERIC (ldresnearl, ldresnearl_avx2);
  // REGISTER_RULE_WITH_GENERIC (ldreslinl, ldreslinl_avx2);
}
