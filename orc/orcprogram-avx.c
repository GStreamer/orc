
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <orc/orcavx.h>
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
  if (_orc_compiler_flag_debug) {
    flags |= ORC_TARGET_SSE_FRAME_POINTER;
  }

#if defined(HAVE_I386) || defined(HAVE_AMD64)
  flags |= orc_sse_get_cpu_flags ();
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
    "sse2",
    "sse3",
    "ssse3",
    "sse41",
    "sse42",
    "frame_pointer",
    "short_jumps",
    "64bit",
    "avx",
    "avx2",
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
  const int flags = orc_sse_get_cpu_flags ();

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
#ifdef HAVE_OS_WIN32
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
  orc_avx_emit_extractf128_si256 (compiler, 1, src, tmp);

  // Pairwise summation
  if (var->size == 2) {
    orc_avx_sse_emit_paddw (compiler, src, tmp, src);
  } else {
    orc_avx_sse_emit_paddd (compiler, src, tmp, src);
  }

  // Duplicate the high half now
  orc_avx_sse_emit_pshufd (compiler, ORC_AVX_SSE_SHUF (3, 2, 3, 2), src,
      tmp);

  // Pairwise summation
  if (var->size == 2) {
    orc_avx_sse_emit_paddw (compiler, src, tmp, src);
  } else {
    orc_avx_sse_emit_paddd (compiler, src, tmp, src);
  }

  // Combine the remaining two pairs in the low half
  orc_avx_sse_emit_pshufd (compiler, ORC_AVX_SSE_SHUF (1, 1, 1, 1), src,
      tmp);

  // Pairwise summation
  if (var->size == 2) {
    orc_avx_sse_emit_paddw (compiler, src, tmp, src);
  } else {
    orc_avx_sse_emit_paddd (compiler, src, tmp, src);
  }

  // Reduce the last pair if it's 16-bit
  if (var->size == 2) {
    orc_avx_sse_emit_pshuflw (compiler, ORC_AVX_SSE_SHUF (1, 1, 1, 1),
        src, tmp);
    orc_avx_sse_emit_paddw (compiler, src, tmp, src);
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

void
orc_avx_load_constant (OrcCompiler *compiler, int reg, int size,
    orc_uint64 value)
{
  if (size == 8) {
    if (value == 0) {
      orc_avx_emit_pxor (compiler, reg, reg, reg);
      return;
    } else if (value == UINT64_MAX) {
      orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
      return;
    }

    // Store the upper half
    if (value >> 32) {
      orc_x86_emit_mov_imm_reg (compiler, 4, value >> 32, compiler->gp_tmpreg);
      orc_avx_sse_emit_pinsrd_register (compiler, 1,  reg, compiler->gp_tmpreg, reg);
    } else {
      orc_avx_emit_pxor (compiler, reg, reg, reg);
    }

    // Store the lower half
    orc_x86_emit_mov_imm_reg (compiler, 4, value >> 0, compiler->gp_tmpreg);
    orc_avx_sse_emit_pinsrd_register (compiler, 0,  reg, compiler->gp_tmpreg, reg);

    // broadcast mm0 to the rest
    orc_avx_emit_broadcast (compiler, reg, reg, size);
    return;
  } else if (size == 1) {
    // Synthesize a MMX "vector"
    value &= 0xff;
    value |= (value << 8);
    value |= (value << 16);
  } else if (size == 2) {
    // Same as above
    value &= 0xffff;
    value |= (value << 16);
  }

  ORC_ASM_CODE (compiler, "# loading constant %" PRIu64 " 0x%16" PRIx64 "\n",
      value, value);

  if (value == 0) {
    orc_avx_emit_pxor (compiler, reg, reg, reg);
    return;
  } else if (value == UINT32_MAX) {
    orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
    return;
  } else if (value == 0x01010101) {
    orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
    // Force two-complement wraparound
    orc_avx_emit_pabsb (compiler, reg, reg);
    return;
  } else if (value == 0x00010001) {
    orc_avx_emit_pcmpeqw (compiler, reg, reg, reg);
    // Force two-complement wraparound
    orc_avx_emit_pabsw (compiler, reg, reg);
    return;
  } else if (value == 0x00000001) {
    orc_avx_emit_pcmpeqd (compiler, reg, reg, reg);
    // Force two-complement wraparound
    orc_avx_emit_pabsd (compiler, reg, reg);
    return;
  }

  // Shifted UINT32_MAX masks
  for (orc_uint32 i = 1; i < 32; i++) {
    const orc_uint32 v = (0xffffffff << i);
    if (value == v) {
      orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
      orc_avx_emit_pslld_imm (compiler, i, reg, reg);
      return;
    }
    const orc_uint32 v2 = (0xffffffff >> i);
    if (value == v2) {
      orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
      orc_avx_emit_psrld_imm (compiler, i, reg, reg);
      return;
    }
  }

  // Shifted UINT16_MAX masks
  for (orc_uint32 i = 1; i < 16; i++) {
    const orc_uint32 v1
        = (0xffff & (0xffff << i)) | (0xffff0000 & (0xffff0000 << i));
    if (value == v1) {
      orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
      orc_avx_emit_psllw_imm (compiler, i, reg, reg);
      return;
    }
    const orc_uint32 v2
        = (0xffff & (0xffff >> i)) | (0xffff0000 & (0xffff0000 >> i));
    if (value == v2) {
      orc_avx_emit_pcmpeqb (compiler, reg, reg, reg);
      orc_avx_emit_psrlw_imm (compiler, i, reg, reg);
      return;
    }
  }

  orc_x86_emit_mov_imm_reg (compiler, 4, value, compiler->gp_tmpreg);
  orc_avx_sse_emit_movd_load_register (compiler, compiler->gp_tmpreg, reg);
  orc_avx_emit_broadcast (compiler, reg, reg, 4);
}


static void
avx_load_constant_long (OrcCompiler *compiler, int reg, OrcConstant *constant)
{
  ORC_ASM_CODE (compiler, "# loading constant %08x %08x %08x %08x\n",
      constant->full_value[0], constant->full_value[1], constant->full_value[2],
      constant->full_value[3]);

  for (int i = 0; i < 4; i++) {
    orc_x86_emit_mov_imm_reg (compiler, 4, constant->full_value[i],
        compiler->gp_tmpreg);
    orc_avx_sse_emit_pinsrd_register (compiler, i, reg, compiler->gp_tmpreg, reg);
  }

  orc_avx_emit_broadcast (compiler, reg, reg, 16);
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
    orc_avx_load_constant,
    avx_load_constant_long,
    avx_move_register_to_memoffset,
    avx_move_memoffset_to_register,
    avx_get_shift,
    avx_set_mxcsr,
    avx_restore_mxcsr,
    ORC_AVX_REG_SIZE,
    X86_YMM0,
    ORC_AVX_REG_AMOUNT,
    16,
  };
  // clang-format on
  OrcTarget *t;

  t = orc_x86_register_target (&target);
  orc_compiler_avx_register_rules (t);
}

/*
 * The following code was ported from the MIPS backend,
 * and extended to allow for store reordering and the
 * multi-operand VEX syntax.
 */
static int
uses_in_destination_register (const OrcCompiler *const compiler,
               const OrcInstruction *const insn,
               int reg)
{
  for (int i=0; i<ORC_STATIC_OPCODE_N_DEST; i++) {
    const OrcVariable *const var = compiler->vars + insn->dest_args[i];
    if (var->alloc == reg || var->ptr_register == reg)
      return TRUE;
  }

  return FALSE;
}

static int uses_in_source_register(const OrcCompiler *const compiler,
               const OrcInstruction *const insn,
               int reg) {
  for (int i=0; i<ORC_STATIC_OPCODE_N_SRC; i++) {
    const OrcVariable *const var = compiler->vars + insn->src_args[i];
    if (var->alloc == reg || var->ptr_register == reg)
      return TRUE;
  }

  return FALSE;
}

static void
do_swap (int *tab, int i, int j)
{
  int tmp = tab[i];
  tab[i] = tab[j];
  tab[j] = tmp;
}

/* Assumes that the instruction at indexes[i] is a load instruction */
static int
can_raise (const OrcCompiler *const compiler, const int *const indexes, int i)
{
  if (i==0)
    return FALSE;

  const OrcInstruction *const insn = compiler->insns + indexes[i];
  const OrcInstruction *const previous_insn = compiler->insns + indexes[i-1];

  /* Register where the load operation will put the data */
  const int reg = compiler->vars[insn->dest_args[0]].alloc;
  if (uses_in_source_register(compiler, previous_insn, reg) || uses_in_destination_register(compiler, previous_insn, reg)) {
    return FALSE;
  }

  for (int j = 0; j < ORC_STATIC_OPCODE_N_SRC; j++) {
    const OrcVariable *const var = compiler->vars + insn->src_args[j];
    // If the previous instruction touches anything RIP
    if (uses_in_destination_register(compiler, previous_insn, var->alloc) || uses_in_destination_register(compiler, previous_insn, var->ptr_register))
      return FALSE;
  }

  return TRUE;
}

/* Recursive. */
static void
try_raise (OrcCompiler *compiler, int *indexes, int i)
{
  if (can_raise (compiler, indexes, i)) {
    do_swap (indexes, i-1, i);
    try_raise (compiler, indexes, i-1);
  }
}


/* Assumes that the instruction at indexes[i] is a load instruction */
static int
can_lower (const OrcCompiler *const compiler, const int *const indexes, int i)
{
  if (i >= compiler->n_insns - 1)
    return FALSE;

  const OrcInstruction *const insn = compiler->insns + indexes[i];
  const OrcInstruction *const next_insn = compiler->insns + indexes[i+1];

  /* Register where the store operation will put the data */
  const int reg = compiler->vars[insn->dest_args[0]].ptr_register;
  if (uses_in_source_register(compiler, next_insn, reg)) {
    return FALSE;
  }

  for (int j = 0; j < ORC_STATIC_OPCODE_N_SRC; j++) {
    const OrcVariable *const var = compiler->vars + insn->src_args[j];
    // If the next instruction touches anything RIP
    if (uses_in_destination_register(compiler, next_insn, var->alloc) || uses_in_destination_register(compiler, next_insn, var->ptr_register))
      return FALSE;
  }

  return TRUE;
}

static void
try_lower (OrcCompiler *compiler, int *indexes, int i)
{
  if (can_lower (compiler, indexes, i)) {
    do_swap (indexes, i-1, i);
    try_lower (compiler, indexes, i+1);
  }
}

/*
   Do a kind of bubble sort, though it might not exactly be a sort. It only
   moves load instructions up until they reach an operation above which they
   cannot go.

   FIXME: also push store instructions down.
 */
static void
optimise_order (OrcCompiler *compiler, int *const indexes)
{
  for (int i=0; i<compiler->n_insns; i++) {
    const OrcInstruction *const insn = compiler->insns + indexes[i];
    if (insn->opcode->flags & ORC_STATIC_OPCODE_LOAD) {
      try_raise(compiler, indexes, i);
    }
    else if (insn->opcode->flags & ORC_STATIC_OPCODE_STORE) {
      try_lower(compiler, indexes, i);
    }
  }
}

static int *
get_optimised_instruction_order (OrcCompiler *compiler)
{
  if (compiler->n_insns == 0)
    return NULL;

  int *const instruction_idx = malloc (compiler->n_insns * sizeof(int));
  for (int i=0; i<compiler->n_insns; i++)
    instruction_idx[i] = i;

  optimise_order (compiler, instruction_idx);

  return instruction_idx;
}

