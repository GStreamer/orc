#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <orc/orcx86.h>
#include <orc/orcx86-private.h>
#include <orc/avx512/orcavx512.h>
#include <orc/avx512/orcavx512insn.h>
#include <orc/avx512/orcavx512-private.h>
#include <orc/orcinternal.h>

#define ORC_AVX512_REG_SIZE 64
#define ORC_AVX512_REG_AMOUNT 32
/* TODO Foundation (F) instruction set should be the minimum set to make
 * the target runnable
 */
#define ORC_AVX512_MIN_FLAGS (\
  ORC_TARGET_AVX512_F |       \
  ORC_TARGET_AVX512_VL |      \
  ORC_TARGET_AVX512_BW |      \
  ORC_TARGET_AVX512_VBMI      \
)

static unsigned int
orc_avx512_target_get_default_flags (void)
{
  unsigned int flags = 0;

#if defined(HAVE_AMD64)
  flags |= ORC_TARGET_AVX512_64BIT;
#endif
  if (orc_compiler_is_debug ()) {
    flags |= ORC_TARGET_AVX512_FRAME_POINTER;
  }

#if defined(HAVE_I386) || defined(HAVE_AMD64)
  flags |= orc_avx512_get_cpu_flags ();
#else
  flags |= ORC_AVX512_MIN_FLAGS;
#endif

  return flags;
}

static const char *
orc_avx512_target_get_flag_name (const int shift)
{
  return orc_avx512_get_flag_name (1 << shift);
}

static int
orc_avx512_target_is_executable (void)
{
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  /* initializes cache information */
  const int flags = orc_avx512_get_cpu_flags ();

  if ((flags & ORC_AVX512_MIN_FLAGS) == ORC_AVX512_MIN_FLAGS) {
    return TRUE;
  }
#endif
  return FALSE;
}

static void
orc_avx512_target_validate_registers (int *regs, int is_64bit)
{
  int i;

  if (is_64bit) {
    for (i = 0; i < ORC_AVX512_REG_AMOUNT; i++) {
      regs[ORC_AVX512_ZMM0 + i] = 1;
    }
  } else {
    for (i = 0; i < 8; i++) {
      regs[ORC_AVX512_ZMM0 + i] = 1;
    }
  }
}

static void
orc_avx512_target_saveable_registers (int *regs, int is_64bit)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  if (is_64bit) {
    int i;
    for (i = 6; i < ORC_AVX_REG_AMOUNT; i++) {
      regs[ORC_AVX512_ZMM0 + i] = 1;
    }
  }
#endif
}

static int
orc_avx512_target_is_64bit (int flags)
{
  if (flags & ORC_TARGET_AVX512_64BIT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
orc_avx512_target_use_frame_pointer (int flags)
{
  if (flags & ORC_TARGET_AVX512_FRAME_POINTER) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
orc_avx512_target_use_long_jumps (int flags)
{
  if (!(flags & ORC_TARGET_AVX512_SHORT_JUMPS)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static void
orc_avx512_target_init_accumulator (OrcCompiler *c, OrcVariable *var)
{
  orc_avx512_insn_emit_vpxord (c, var->alloc, var->alloc, var->alloc,
      ORC_REG_INVALID, FALSE);
}

/* Duplicate higer half_size bytes at the less significant half_size bytes */
static void
orc_avx512_compiler_dup_high_half (OrcCompiler *c, int half_size, int src,
    int dest)
{
  switch (half_size) {
    case 32:
      orc_avx512_insn_emit_vextracti64x4 (c, 1, src, ORC_AVX512_AVX_REG (dest),
          ORC_REG_INVALID, FALSE);
      break;
    case 16:
      orc_avx512_insn_avx_emit_vextracti32x4 (c, 1, ORC_AVX512_AVX_REG (src),
          ORC_AVX512_SSE_REG (dest), ORC_REG_INVALID, FALSE);
      break;
    case 8:
      orc_avx512_insn_sse_emit_vpshufd (c, ORC_AVX_SSE_SHUF (3, 2, 3, 2),
          ORC_AVX512_SSE_REG (src), ORC_AVX512_SSE_REG (dest), ORC_REG_INVALID,
          FALSE);
      break;
    case 4:
      orc_avx512_insn_sse_emit_vpshufd (c, ORC_AVX_SSE_SHUF (1, 1, 1, 1),
          ORC_AVX512_SSE_REG (src), ORC_AVX512_SSE_REG (dest), ORC_REG_INVALID,
          FALSE);
      break;
    case 2:
      orc_avx512_insn_sse_emit_vpshuflw (c, ORC_AVX_SSE_SHUF (1, 1, 1, 1),
          ORC_AVX512_SSE_REG (src), ORC_AVX512_SSE_REG (dest), ORC_REG_INVALID,
          FALSE);
      break;
    default:
      ORC_ERROR ("Unsupported size %d", half_size);
      break;
  }
}

/* TODO use the proper reg size */
static void
orc_avx512_insn_emit_add (OrcCompiler *c, int size, int a, int b)
{
  switch (size) {
    case 4:
      orc_avx512_insn_emit_vpaddd (c, a, b, a, ORC_REG_INVALID, FALSE);
      break;

    case 2:
      orc_avx512_insn_emit_vpaddw (c, a, b, a, ORC_REG_INVALID, FALSE);
      break;
  }
}

/* The register needs to be reduced horizontally by adding the values
 * For that we swap halves and add
 */ 
static void
orc_avx512_target_reduce_accumulator (OrcCompiler *c, int i, OrcVariable *var)
{
  int reg = var->alloc;
  int tmp = orc_compiler_get_temp_reg (c);
  int half_size;

  for (half_size = 32; half_size >= var->size; half_size = half_size / 2) {
    orc_avx512_compiler_dup_high_half (c, half_size, reg, tmp);
    /* Do the operation on reg, tmp -> reg */
    orc_avx512_insn_emit_add (c, var->size, reg, tmp);
  }
  orc_avx512_insn_emit_mov_reg_memoffset (c, var->size, reg,
      ORC_STRUCT_OFFSET (OrcExecutor, accumulators[i - ORC_VAR_A1]),
      c->exec_reg, var->is_aligned, var->is_uncached);
  orc_compiler_release_temp_reg (c, reg);
}

static void
orc_avx512_target_load_constant_full (OrcCompiler *c, int reg,
    OrcConstant *cnst, int size)
{
  int i;
  int n;
  int mask;
  int mask_value = 1;

  switch (size) {
    case 32:
      mask_value = 17; // 00010001b
      n = 4;
      break;
    case 16:
      mask_value = 85; // 01010101b
      n = 2;
      break;
    case 0:
      if (cnst->v[7].i || cnst->v[6].i || cnst->v[5].i || cnst->v[4].i) {
        /* 512-bits */
        n = 8;
      } else if (cnst->v[3].i || cnst->v[2].i) {
        /* 256-bits */
        n = 4;
      } else {
        /* 128-bits */
        n = 2;
      }
    break;

    default:
      ORC_ERROR ("Unsupported size %d", size);
      return;
  }

  mask = orc_avx512_compiler_get_mask_reg (c, TRUE);
  if (mask == ORC_REG_INVALID) {
    ORC_ERROR ("No mask register available to load constant");
    return;
  }

  /* Initialize the mask */
  orc_x86_emit_mov_imm_reg (c, 4, mask_value, c->gp_tmpreg);
  orc_avx512_insn_emit_size (c, ORC_AVX512_INSN_kmovw_k_r, 4, c->gp_tmpreg,
      ORC_REG_INVALID, ORC_REG_INVALID, mask, ORC_REG_INVALID, FALSE);


  if (!c->is_64bit) {
    ORC_ERROR ("Unsupported constant loading in 32-bits");
    goto done;
  }

  for (i = 0; i < n; i++) {
    orc_x86_emit_mov_imm_reg64 (c, 8, cnst->v[i].i, c->gp_tmpreg);
    orc_avx512_insn_emit_x86_vpbroadcastq (c, c->gp_tmpreg, reg, mask, FALSE);
    orc_avx512_insn_emit_imm (c, ORC_AVX512_INSN_kshiftlb,
        ORC_X86_INSN_OPERAND_SIZE_NONE, 1, mask, ORC_REG_INVALID, mask,
        ORC_REG_INVALID, FALSE);
  }
done:
  orc_avx512_compiler_release_mask_reg (c, mask);
}

/* Nice source of constants loading
 * http://0x80.pl/notesen/2023-01-19-avx512-consts.html
 */
static void
orc_avx512_target_load_constant_long (OrcCompiler *c, int reg, OrcConstant *cnst)
{
  switch (cnst->type) {
    case ORC_CONST_ZERO:
      orc_avx512_insn_emit_vpxord (c, reg, reg, reg, ORC_REG_INVALID, FALSE);
      break;
    case ORC_CONST_SPLAT_B:
      orc_x86_emit_mov_imm_reg (c, 4, cnst->v[0].x8[0], c->gp_tmpreg);
      orc_avx512_insn_emit_x86_vpbroadcastb (c, c->gp_tmpreg, reg, ORC_REG_INVALID, FALSE);
      break;
    case ORC_CONST_SPLAT_W:
      orc_x86_emit_mov_imm_reg (c, 4, cnst->v[0].x4[0], c->gp_tmpreg);
      orc_avx512_insn_emit_x86_vpbroadcastw (c, c->gp_tmpreg, reg, ORC_REG_INVALID, FALSE);
      break;
    case ORC_CONST_SPLAT_L:
      orc_x86_emit_mov_imm_reg (c, 4, cnst->v[0].x2[0], c->gp_tmpreg);
      orc_avx512_insn_emit_x86_vpbroadcastd (c, c->gp_tmpreg, reg, ORC_REG_INVALID, FALSE);
      break;
    case ORC_CONST_SPLAT_Q:
      /* FIXME until vpbroadcastq */
      {
        OrcConstant r;
        orc_constant_resolve (cnst, &r, 64);
        orc_avx512_target_load_constant_full (c, reg, &r, 0);
      }
      break;
    case ORC_CONST_SPLAT_DQ:
      orc_avx512_target_load_constant_full (c, reg, cnst, 16);
      break;

    case ORC_CONST_SPLAT_QQ:
      orc_avx512_target_load_constant_full (c, reg, cnst, 32);
      break;
    case ORC_CONST_FULL:
      orc_avx512_target_load_constant_full (c, reg, cnst, 0);
      break;
  }
}

static void
orc_avx512_target_move_register_to_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2, int aligned, int uncached)
{
  orc_avx512_insn_emit_mov_reg_memoffset (compiler, size, reg1, offset, reg2, aligned, uncached);
}

static void
orc_avx512_target_move_memoffset_to_register (OrcCompiler *compiler, int size, int offset, int reg1, int reg2, int is_aligned)
{
  orc_avx512_insn_emit_mov_memoffset_reg (compiler, size, offset, reg1, reg2, is_aligned);
}

static void
orc_avx512_target_set_mxcsr (OrcCompiler *c)
{
  orc_avx512_insn_emit_stmxcsr (c,
      ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      c->exec_reg);

  orc_x86_emit_mov_memoffset_reg (c, 4,
      ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      c->exec_reg, c->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (c, 4, c->gp_tmpreg,
      ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_C1]),
      c->exec_reg);

  orc_x86_emit_cpuinsn_imm_reg (c, ORC_X86_or_imm32_rm, 4, 0x8040,
      c->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (c, 4, c->gp_tmpreg,
      ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      c->exec_reg);

  orc_avx512_insn_emit_ldmxcsr (c,
      ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      c->exec_reg);
}

static void
orc_avx512_target_restore_mxcsr (OrcCompiler *c)
{
  orc_avx512_insn_emit_ldmxcsr (c,
      ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A4]),
      c->exec_reg);
}

OrcTarget *
orc_avx512_target_init (void)
{
  /* clang-format off */
  static OrcX86Target target = {
    "avx512",
    orc_avx512_target_get_default_flags,
    orc_avx512_target_get_flag_name,
    orc_avx512_target_is_executable,
    orc_avx512_target_validate_registers,
    orc_avx512_target_saveable_registers,
    orc_avx512_target_is_64bit,
    orc_avx512_target_use_frame_pointer,
    orc_avx512_target_use_long_jumps,
    orc_avx512_target_init_accumulator,
    orc_avx512_target_reduce_accumulator,
    NULL,
    orc_avx512_target_load_constant_long,
    orc_avx512_target_move_register_to_memoffset,
    orc_avx512_target_move_memoffset_to_register,
    orc_avx512_target_set_mxcsr,
    orc_avx512_target_restore_mxcsr,
    NULL,
    NULL,
    ORC_AVX512_REG_SIZE,
    ORC_AVX512_ZMM0,
    ORC_AVX512_REG_AMOUNT,
    16,
  };
  /* clang-format on */
  static OrcTarget t;
  orc_x86_register_extension (&t, &target);
  return &t;
}
