
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <orc/orcx86.h>
#include <orc/orcx86-private.h>
#include <orc/orcavx.h>
#include <orc/orcavxinsn.h>
#include <orc/orcavx-internal.h>
#include <orc/orcinternal.h>
#include <orc/orcprogram.h>

/* X86 specific */
static unsigned int
avx_get_default_flags (void)
{
  unsigned int flags = 0;

#if defined(HAVE_AMD64)
  flags |= ORC_TARGET_SSE_64BIT;
#endif
  if (orc_compiler_is_debug ()) {
    flags |= ORC_TARGET_SSE_FRAME_POINTER;
  }

#if defined(HAVE_I386) || defined(HAVE_AMD64)
  flags |= orc_avx_get_cpu_flags ();
#else
  flags |= ORC_TARGET_AVX_AVX;
  flags |= ORC_TARGET_AVX_AVX2;
#endif

  return flags;
}

static const char *
avx_get_flag_name (const int shift)
{
  static const char *flags[] = {
    /* To keep backwards compatibility */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "frame_pointer",
    "short_jumps",
    "64bit",
    "avx",
    "avx2"
  };

  if (shift >= 0 && shift < sizeof (flags) / sizeof (flags[0])) {
    return flags[shift];
  }

  return NULL;
}

static int
avx_is_executable (void)
{
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  /* initializes cache information */
  const int flags = orc_avx_get_cpu_flags ();

  if ((flags & ORC_TARGET_AVX_AVX) && (flags & ORC_TARGET_AVX_AVX2)) {
    return TRUE;
  }
#endif
  return FALSE;
}

static void
avx_validate_registers (int *regs, int is_64bit)
{
  int i;

  if (is_64bit) {
    for (i = 0; i < ORC_AVX_REG_AMOUNT; i++) {
      regs[X86_YMM0 + i] = 1;
    }
  } else {
    for (i = 0; i < ORC_AVX_REG_AMOUNT - 8; i++) {
      regs[X86_YMM0 + i] = 1;
    }
  }
}

static void
avx_saveable_registers (int *regs, int is_64bit)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  if (is_64bit) {
    int i;
    for (i = 6; i < ORC_AVX_REG_AMOUNT; i++) {
      regs[X86_YMM0 + i] = 1;
    }
  }
#endif
}

static int
avx_is_64bit (int flags)
{
  if (flags & ORC_TARGET_SSE_64BIT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
avx_use_frame_pointer (int flags)
{
  if (flags & ORC_TARGET_SSE_FRAME_POINTER) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
avx_use_long_jumps (int flags)
{
  if (!(flags & ORC_TARGET_SSE_SHORT_JUMPS)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
avx_loop_shift (int max_var_size)
{
  switch (max_var_size) {
    case 1:
      return 5;
    case 2:
      return 4;
    case 4:
      return 3;
    case 8:
      return 2;
    default:
      ORC_ERROR ("unhandled max var size %d", max_var_size);
      break;
  }

  return -1;
}

static void
avx_init_accumulator (OrcCompiler *compiler, OrcVariable *var)
{
  orc_avx_emit_pxor (compiler, var->alloc, var->alloc, var->alloc);
}

static void
avx_reduce_accumulator (OrcCompiler *compiler, int i, OrcVariable *var)
{
  const int src = var->alloc;
  const int tmp = orc_compiler_get_temp_reg (compiler);

  // duplicate the high lane
  orc_avx_emit_extractf128_si256 (compiler, 1, src, ORC_AVX_SSE_REG (tmp));

  // Pairwise summation
  if (var->size == 2) {
    orc_avx_sse_emit_paddw (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  } else {
    orc_avx_sse_emit_paddd (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  }

  // Duplicate the high half now
  orc_avx_sse_emit_pshufd (compiler, ORC_AVX_SSE_SHUF (3, 2, 3, 2), ORC_AVX_SSE_REG (src),
      ORC_AVX_SSE_REG (tmp));

  // Pairwise summation
  if (var->size == 2) {
    orc_avx_sse_emit_paddw (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  } else {
    orc_avx_sse_emit_paddd (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  }

  // Combine the remaining two pairs in the low half
  orc_avx_sse_emit_pshufd (compiler, ORC_AVX_SSE_SHUF (1, 1, 1, 1), ORC_AVX_SSE_REG (src),
      ORC_AVX_SSE_REG (tmp));

  // Pairwise summation
  if (var->size == 2) {
    orc_avx_sse_emit_paddw (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  } else {
    orc_avx_sse_emit_paddd (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  }

  // Reduce the last pair if it's 16-bit
  if (var->size == 2) {
    orc_avx_sse_emit_pshuflw (compiler, ORC_AVX_SSE_SHUF (1, 1, 1, 1),
        ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp));
    orc_avx_sse_emit_paddw (compiler, ORC_AVX_SSE_REG (src), ORC_AVX_SSE_REG (tmp), ORC_AVX_SSE_REG (src));
  }

  if (var->size == 2) {
    orc_avx_sse_emit_pextrw_memoffset (compiler, 0,
        (int)ORC_STRUCT_OFFSET (OrcExecutor,
            accumulators[i - ORC_VAR_A1]),
        src, compiler->exec_reg);
  } else {
    orc_x86_emit_mov_avx_memoffset (compiler, 4, src,
        (int)ORC_STRUCT_OFFSET (OrcExecutor,
            accumulators[i - ORC_VAR_A1]),
        compiler->exec_reg, var->is_aligned, var->is_uncached);
  }
}

static void
orc_avx_load_constant_full (OrcCompiler *c, int reg, OrcConstant *cnst)
{
  orc_x86_emit_cpuinsn_comment (c, "# loading full constant %" PRIu64
      " %" PRIu64 " 0x%16" PRIx64 " 0x%16", cnst->v[0].i, cnst->v[0].i,
      cnst->v[1].i, cnst->v[1].i);

  /* The rules for AVX are currently loading 128-bit (SSE) constants only */
  if (!cnst->v[2].i && !cnst->v[3].i) {
    int i;
    for (i = 0; i < 4; i++) {
      orc_x86_emit_mov_imm_reg (c, 4, cnst->v[i/2].x2[i%2],
          c->gp_tmpreg);
      orc_avx_sse_emit_pinsrd_register (c, i, ORC_AVX_SSE_REG (reg),
          c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
    }
    orc_avx_emit_broadcast (c, reg, reg, 16);
  } else {
    ORC_ERROR ("Unsupported size of constant loading");
  }
}

static void
orc_avx_load_constant_u8 (OrcCompiler *c, int reg, orc_uint8 value)
{
  orc_x86_emit_cpuinsn_comment (c, "# loading u8 constant %d 0x%02x", value, value);
  if (value == 0) {
    orc_avx_emit_pxor (c, reg, reg, reg);
    return;
  } else if (value == UINT8_MAX) {
    orc_avx_emit_pcmpeqb (c, reg, reg, reg);
    return;
  } else  if (value == 1) {
    if (c->target_flags & ORC_TARGET_AVX_AVX2) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_pabsb (c, reg, reg);
      return;
    }
  }

  /* default case */
  orc_x86_emit_mov_imm_reg (c, 4, value, c->gp_tmpreg);
  orc_avx_sse_emit_movd_load_register (c, c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
  orc_avx_emit_broadcast (c, reg, reg, 1);
}

static void
orc_avx_load_constant_u16 (OrcCompiler *c, int reg, orc_uint16 value)
{
  int i;

  orc_x86_emit_cpuinsn_comment (c, "# loading u16 constant %d 0x%04x", value, value);
  if (value == 0) {
    orc_avx_emit_pxor (c, reg, reg, reg);
    return;
  } else if (value == UINT16_MAX) {
    orc_avx_emit_pcmpeqb (c, reg, reg, reg);
    return;
  } else if (value == 1) {
    if (c->target_flags & ORC_TARGET_AVX_AVX2) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_pabsw (c, reg, reg);
      return;
    }
  }

  for (i = 1; i < 16; i++) {
    orc_uint16 v;
    v = 0xffff << i;
    if (value == v) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_psllw_imm (c, i, reg, reg);
      return;
    }
    v = 0xffff >> i;
    if (value == v) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_psrlw_imm (c, i, reg, reg);
      return;
    }
  }

  /* default case */
  orc_x86_emit_mov_imm_reg (c, 4, value, c->gp_tmpreg);
  orc_avx_sse_emit_movd_load_register (c, c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
  orc_avx_emit_broadcast (c, reg, reg, 2);
}

static void
orc_avx_load_constant_u32 (OrcCompiler *c, int reg, orc_uint32 value)
{
  int i;

  orc_x86_emit_cpuinsn_comment (c, "# loading u32 constant %d 0x%08x", value, value);
  if (value == 0) {
    orc_avx_emit_pxor (c, reg, reg, reg);
    return;
  } else if (value == UINT32_MAX) {
    orc_avx_emit_pcmpeqb (c, reg, reg, reg);
    return;
  } else if (value == 1) {
    if (c->target_flags & ORC_TARGET_AVX_AVX2) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_pabsd (c, reg, reg);
      return;
    }
  }

  for (i = 1; i < 32; i++) {
    orc_uint32 v;
    v = 0xffffffff << i;
    if (value == v) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_pslld_imm (c, i, reg, reg);
      return;
    }
    v = 0xffffffff >> i;
    if (value == v) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_psrld_imm (c, i, reg, reg);
      return;
    }
  }

  /* default case */
  orc_x86_emit_mov_imm_reg (c, 4, value, c->gp_tmpreg);
  orc_avx_sse_emit_movd_load_register (c, c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
  orc_avx_emit_broadcast (c, reg, reg, 4);
}

static void
orc_avx_load_constant_u64 (OrcCompiler *c, int reg, orc_uint64 value)
{
  int i;

  orc_x86_emit_cpuinsn_comment (c, "# loading u64 constant %" PRIu64
      " 0x%16" PRIx64, value, value);
  if (value == 0) {
    orc_avx_emit_pxor (c, reg, reg, reg);
    return;
  } else if (value == UINT64_MAX) {
    orc_avx_emit_pcmpeqb (c, reg, reg, reg);
    return;
  }

  for (i = 1; i < 64; i++) {
    orc_uint64 v;
    v = 0xffffffffffffffffUL << i;
    if (value == v) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_psllq_imm (c, i, reg, reg);
      return;
    }
    v = 0xffffffffffffffffUL >> i;
    if (value == v) {
      orc_avx_emit_pcmpeqb (c, reg, reg, reg);
      orc_avx_emit_psrlq_imm (c, i, reg, reg);
      return;
    }
  }

  /* default case */
  if (c->is_64bit) {
    orc_x86_emit_mov_imm_reg64 (c, 8, value, c->gp_tmpreg);
    orc_avx_sse_emit_movq_load_register (c, c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
  } else {
    // Store the upper half
    if (value >> 32) {
      orc_x86_emit_mov_imm_reg (c, 4, value >> 32, c->gp_tmpreg);
      orc_avx_sse_emit_pinsrd_register (c, 1, ORC_AVX_SSE_REG (reg), c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
    } else {
      orc_avx_emit_pxor (c, reg, reg, reg);
    }

    // Store the lower half
    orc_x86_emit_mov_imm_reg (c, 4, value & UINT32_MAX, c->gp_tmpreg);
    orc_avx_sse_emit_pinsrd_register (c, 0,  ORC_AVX_SSE_REG (reg), c->gp_tmpreg, ORC_AVX_SSE_REG (reg));
  }

  // broadcast mm0 to the rest
  orc_avx_emit_broadcast (c, reg, reg, 8);
}

static void
orc_avx_load_constant_long (OrcCompiler *c, int reg, OrcConstant *cnst)
{
  switch (cnst->type) {
    case ORC_CONST_ZERO:
      orc_avx_emit_pxor (c, reg, reg, reg);
      break;

    case ORC_CONST_SPLAT_B:
      orc_avx_load_constant_u8 (c, reg, cnst->v[0].x8[0]);
      break;

    case ORC_CONST_SPLAT_W:
      orc_avx_load_constant_u16 (c, reg, cnst->v[0].x4[0]);
      break;

    case ORC_CONST_SPLAT_L:
      orc_avx_load_constant_u32 (c, reg, cnst->v[0].x2[0]);
      break;

    case ORC_CONST_SPLAT_Q:
      orc_avx_load_constant_u64 (c, reg, cnst->v[0].i);
      break;

    case ORC_CONST_SPLAT_DQ:
    case ORC_CONST_FULL:
      orc_avx_load_constant_full (c, reg, cnst);
      break;

    default:
      ORC_COMPILER_ERROR (c, "Unsupported constant type %d", cnst->type);
      break;
  }
}

static void
avx_move_register_to_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2, int aligned, int uncached)
{
  orc_x86_emit_mov_avx_memoffset (compiler, size, reg1, offset, reg2, aligned, uncached);
}

static void
avx_move_memoffset_to_register (OrcCompiler *compiler, int size, int offset, int reg1, int reg2, int is_aligned)
{
  orc_x86_emit_mov_memoffset_avx (compiler, size, offset, reg1, reg2, is_aligned);

}

static void
avx_zeroupper (OrcCompiler *compiler)
{
  orc_vex_emit_cpuinsn_none (compiler, ORC_AVX_vzeroupper);
}

static int
avx_get_shift (int size)
{
  switch (size) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    case 8:
      return 3;
    case 16: // AVX2 shifts
      return 4;
    case 32:
      return 5;
    default:
      ORC_ERROR ("bad size %d", size);
  }
  return -1;
}

static void
avx_set_mxcsr (OrcCompiler *c)
{
  orc_avx_set_mxcsr (c);
}

static void
avx_restore_mxcsr(OrcCompiler *c)
{
  orc_avx_restore_mxcsr (c);
}

void
orc_avx_init (void)
{
  // clang-format off
  static OrcX86Target target = {
    "avx",
    avx_get_default_flags,
    avx_get_flag_name,
    avx_is_executable,
    avx_validate_registers,
    avx_saveable_registers,
    avx_is_64bit,
    avx_use_frame_pointer,
    avx_use_long_jumps,
    avx_loop_shift,
    avx_init_accumulator,
    avx_reduce_accumulator,
    NULL,
    orc_avx_load_constant_long,
    avx_move_register_to_memoffset,
    avx_move_memoffset_to_register,
    avx_get_shift,
    avx_set_mxcsr,
    avx_restore_mxcsr,
    NULL,
    avx_zeroupper,
    ORC_AVX_REG_SIZE,
    X86_YMM0,
    ORC_AVX_REG_AMOUNT,
    16,
  };
  // clang-format on
  static OrcTarget t;

  orc_x86_register_extension (&t, &target);
  orc_compiler_avx_register_rules (&t);
}
