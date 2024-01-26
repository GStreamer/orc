
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

#define SIZE 65536

#define ORC_SSE_ALIGNED_DEST_CUTOFF 64

static void orc_avx_emit_loop (OrcCompiler *compiler, int offset, int update);
static void orc_compiler_avx_init (OrcCompiler *const compiler);
static unsigned int orc_compiler_avx_get_default_flags (void);
static void orc_compiler_avx_assemble (OrcCompiler *compiler);
static void avx_load_constant (OrcCompiler *compiler, int reg, int size,
    int value);
static void avx_load_constant_long (OrcCompiler *compiler, int reg,
    OrcConstant *constant);
static const char *avx_get_flag_name (const int shift);

void
orc_avx_init (void)
{
  // clang-format off
  static OrcTarget target = {
    "avx",
  #if defined(HAVE_I386) || defined(HAVE_AMD64)
    TRUE,
  #else
    FALSE,
  #endif
    ORC_VEC_REG_BASE,
    orc_compiler_avx_get_default_flags,
    orc_compiler_avx_init,
    orc_compiler_avx_assemble,
    { { 0 } },
    0,
    NULL,
    avx_load_constant,
    avx_get_flag_name,
    NULL,
    avx_load_constant_long
  };
  // clang-format on

#if defined(HAVE_I386) || defined(HAVE_AMD64)
  /* initializes cache information */
  const int flags = orc_sse_get_cpu_flags ();

  if (!(flags & ORC_TARGET_AVX_AVX) || !(flags & ORC_TARGET_AVX_AVX2))
    target.executable = FALSE;
#endif

  orc_target_register (&target);

  orc_compiler_avx_register_rules (&target);
}

static unsigned int
orc_compiler_avx_get_default_flags (void)
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

static void
orc_compiler_avx_init (OrcCompiler *const compiler)
{
  int i;

  if (compiler->target_flags & ORC_TARGET_SSE_64BIT) {
    compiler->is_64bit = TRUE;
  }
  if (compiler->target_flags & ORC_TARGET_SSE_FRAME_POINTER) {
    compiler->use_frame_pointer = TRUE;
  }
  if (!(compiler->target_flags & ORC_TARGET_SSE_SHORT_JUMPS)) {
    compiler->long_jumps = TRUE;
  }

  if (compiler->is_64bit) {
    for (i = ORC_GP_REG_BASE; i < ORC_GP_REG_BASE + 16; i++) {
      compiler->valid_regs[i] = 1;
    }
    compiler->valid_regs[X86_ESP] = 0;
    for (i = 0; i < ORC_AVX_REG_AMOUNT; i++) {
      compiler->valid_regs[X86_YMM0 + i] = 1;
    }

    compiler->save_regs[X86_EBX] = 1;
    compiler->save_regs[X86_EBP] = 1;
    compiler->save_regs[X86_R12] = 1;
    compiler->save_regs[X86_R13] = 1;
    compiler->save_regs[X86_R14] = 1;
    compiler->save_regs[X86_R15] = 1;
#ifdef HAVE_OS_WIN32
    compiler->save_regs[X86_EDI] = 1;
    compiler->save_regs[X86_ESI] = 1;
    // When present, the upper portions of YMM0-YMM15 and ZMM0-ZMM15 are also
    // volatile
    // https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170#callercallee-saved-registers
    for (i = 6; i < ORC_AVX_REG_AMOUNT; i++) {
      compiler->save_regs[X86_YMM0 + i] = 1;
    }
#endif
  } else {
    for (i = ORC_GP_REG_BASE; i < ORC_GP_REG_BASE + 8; i++) {
      compiler->valid_regs[i] = 1;
    }
    compiler->valid_regs[X86_ESP] = 0;
    if (compiler->use_frame_pointer) {
      compiler->valid_regs[X86_EBP] = 0;
    }
    for (i = 0; i < ORC_AVX_REG_AMOUNT - 8; i++) {
      compiler->valid_regs[X86_YMM0 + i] = 1;
    }
    compiler->save_regs[X86_EBX] = 1;
    compiler->save_regs[X86_EDI] = 1;
    compiler->save_regs[X86_EBP] = 1;
  }
  for (i = 0; i < 128; i++) {
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  if (compiler->is_64bit) {
#ifdef HAVE_OS_WIN32
    compiler->exec_reg = X86_ECX;
    compiler->gp_tmpreg = X86_EDX;
#else
    compiler->exec_reg = X86_EDI;
    compiler->gp_tmpreg = X86_ECX;
#endif
  } else {
    compiler->gp_tmpreg = X86_ECX;
    if (compiler->use_frame_pointer) {
      compiler->exec_reg = X86_EBX;
    } else {
      compiler->exec_reg = X86_EBP;
    }
  }
  compiler->valid_regs[compiler->gp_tmpreg] = 0;
  compiler->valid_regs[compiler->exec_reg] = 0;

  switch (compiler->max_var_size) {
    case 1:
      compiler->loop_shift = 5;
      break;
    case 2:
      compiler->loop_shift = 4;
      break;
    case 4:
      compiler->loop_shift = 3;
      break;
    case 8:
      compiler->loop_shift = 2;
      break;
    default:
      ORC_ERROR ("unhandled max var size %d", compiler->max_var_size);
      break;
  }

  /* This limit is arbitrary, but some large functions run slightly
     slower when unrolled (ginger Core2 6,15,6), and only some small
     functions run faster when unrolled.  Most are the same speed. */
  /* Also don't enable unrolling with loop_shift == 0, this enables
     double reading in the hot loop. */
  if (compiler->n_insns <= 10 && compiler->loop_shift > 0) {
    compiler->unroll_shift = 1;
  }
  if (!compiler->long_jumps) {
    compiler->unroll_shift = 0;
  }
  compiler->alloc_loop_counter = TRUE;
  compiler->allow_gp_on_stack = TRUE;

  {
    for (i = 0; i < compiler->n_insns; i++) {
      OrcInstruction *insn = compiler->insns + i;
      OrcStaticOpcode *opcode = insn->opcode;

      if (strcmp (opcode->name, "ldreslinb") == 0
          || strcmp (opcode->name, "ldreslinl") == 0
          || strcmp (opcode->name, "ldresnearb") == 0
          || strcmp (opcode->name, "ldresnearl") == 0) {
        compiler->vars[insn->src_args[0]].need_offset_reg = TRUE;
      }
    }
  }
}

void
avx_save_accumulators (OrcCompiler *compiler)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    OrcVariable *var = compiler->vars + i;

    if (var->name == NULL)
      continue;
    switch (var->vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
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
        break;
      default:
        break;
    }
  }
}

static void
avx_load_constant (OrcCompiler *compiler, int reg, int size, int value)
{
  orc_avx_load_constant (compiler, reg, size, value);
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

void
avx_load_constants_outer (OrcCompiler *compiler)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (compiler->vars[i].name == NULL)
      continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_ACCUMULATOR:
        orc_avx_emit_pxor (compiler, compiler->vars[i].alloc,
            compiler->vars[i].alloc, compiler->vars[i].alloc);
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error (compiler, "bad vartype");
        break;
    }
  }

  orc_compiler_emit_invariants (compiler);

  /* FIXME move to a better place */
  for (int i = 0; i < compiler->n_constants; i++) {
    compiler->constants[i].alloc_reg = orc_compiler_get_constant_reg (compiler);
  }

  for (int i = 0; i < compiler->n_constants; i++) {
    if (compiler->constants[i].alloc_reg) {
      if (compiler->constants[i].is_long) {
        avx_load_constant_long (compiler, compiler->constants[i].alloc_reg,
            compiler->constants + i);
      } else {
        avx_load_constant (compiler, compiler->constants[i].alloc_reg, 4,
            compiler->constants[i].value);
      }
    }
  }

  {
    for (int i = 0; i < compiler->n_insns; i++) {
      OrcInstruction *insn = compiler->insns + i;
      OrcStaticOpcode *opcode = insn->opcode;

      if (strcmp (opcode->name, "ldreslinb") == 0
          || strcmp (opcode->name, "ldreslinl") == 0
          || strcmp (opcode->name, "ldresnearb") == 0
          || strcmp (opcode->name, "ldresnearl") == 0) {
        if (compiler->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
          orc_x86_emit_mov_memoffset_reg (compiler, 4,
              (int)ORC_STRUCT_OFFSET (OrcExecutor, params[insn->src_args[1]]),
              compiler->exec_reg, compiler->vars[insn->src_args[0]].ptr_offset);
        } else {
          orc_x86_emit_mov_imm_reg (compiler, 4,
              compiler->vars[insn->src_args[1]].value.i,
              compiler->vars[insn->src_args[0]].ptr_offset);
        }
      }
    }
  }
}

void
avx_load_constants_inner (OrcCompiler *compiler)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (compiler->vars[i].name == NULL)
      continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
              (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]),
              compiler->exec_reg, compiler->vars[i].ptr_register);
        }
        break;
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error (compiler, "bad vartype");
        break;
    }
  }
}

void
avx_add_strides (OrcCompiler *compiler)
{
  for (int i = 0; i < ORC_N_COMPILER_VARIABLES; i++) {
    if (compiler->vars[i].name == NULL)
      continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        orc_x86_emit_mov_memoffset_reg (compiler, 4,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, params[i]), compiler->exec_reg,
            compiler->gp_tmpreg);
        orc_x86_emit_add_reg_memoffset (compiler, compiler->is_64bit ? 8 : 4,
            compiler->gp_tmpreg,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[i]),
            compiler->exec_reg);

        if (compiler->vars[i].ptr_register == 0) {
          orc_compiler_error (compiler,
              "unimplemented: stride on pointer stored in memory");
        }
        break;
      case ORC_VAR_TYPE_CONST:
      case ORC_VAR_TYPE_PARAM:
      case ORC_VAR_TYPE_ACCUMULATOR:
      case ORC_VAR_TYPE_TEMP:
        break;
      default:
        orc_compiler_error (compiler, "bad vartype");
        break;
    }
  }
}

static int
get_align_var (OrcCompiler *compiler)
{
  for (int i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (compiler->vars[i].size == 0)
      continue;
    if ((compiler->vars[i].size << compiler->loop_shift) >= 32) {
      return i;
    }
  }
  for (int i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (compiler->vars[i].size == 0)
      continue;
    if ((compiler->vars[i].size << compiler->loop_shift) >= 16) {
      return i;
    }
  }
  for (int i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (compiler->vars[i].size == 0)
      continue;
    if ((compiler->vars[i].size << compiler->loop_shift) >= 8) {
      return i;
    }
  }
  for (int i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (compiler->vars[i].size == 0)
      continue;
    return i;
  }

  orc_compiler_error (compiler, "could not find alignment variable");

  return -1;
}

static int
get_shift (int size)
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
orc_emit_split_3_regions (OrcCompiler *compiler)
{
  int align_var;
  int align_shift;
  int var_size_shift;

  align_var = get_align_var (compiler);
  if (align_var < 0)
    return;
  var_size_shift = get_shift (compiler->vars[align_var].size);
  align_shift = var_size_shift + compiler->loop_shift;

  /* determine how many iterations until align array is aligned (n1) */
  orc_x86_emit_mov_imm_reg (compiler, 4, 32, X86_EAX);
  // Get the address of the array in question
  // and eax <- eax - addressof(alignment variable)
  orc_x86_emit_sub_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[align_var]),
      compiler->exec_reg, X86_EAX);
  // How many bytes are needed for alignment? (mask wise)
  orc_x86_emit_and_imm_reg (compiler, 4, (1 << align_shift) - 1, X86_EAX);
  // Undo the shift to determine number of ELEMENTS
  orc_x86_emit_sar_imm_reg (compiler, 4, var_size_shift, X86_EAX);

  /* check if n1 is greater than n. */
  orc_x86_emit_cmp_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg);

  orc_x86_emit_jle (compiler, 6);

  /* If so, we have a standard 3-region split. */
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter1), compiler->exec_reg);

  /* Calculate n2 */
  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg,
      compiler->gp_tmpreg);
  orc_x86_emit_sub_reg_reg (compiler, 4, X86_EAX, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_reg (compiler, 4, compiler->gp_tmpreg, X86_EAX);

  orc_x86_emit_sar_imm_reg (compiler, 4,
      compiler->loop_shift + compiler->unroll_shift, compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);

  /* Calculate n3 */
  orc_x86_emit_and_imm_reg (compiler, 4,
      (1 << (compiler->loop_shift + compiler->unroll_shift)) - 1, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);

  orc_x86_emit_jmp (compiler, 7);

  /* else, iterations are all unaligned: n1=n, n2=0, n3=0 */
  orc_x86_emit_label (compiler, 6);

  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter1), compiler->exec_reg);
  orc_x86_emit_mov_imm_reg (compiler, 4, 0, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);

  orc_x86_emit_label (compiler, 7);
}

static void
orc_emit_split_2_regions (OrcCompiler *compiler)
{
  int align_var;
  int align_shift ORC_GNUC_UNUSED;
  int var_size_shift;

  align_var = get_align_var (compiler);
  if (align_var < 0)
    return;
  var_size_shift = get_shift (compiler->vars[align_var].size);
  align_shift = var_size_shift + compiler->loop_shift;

  /* Calculate n2 */
  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg,
      compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_reg (compiler, 4, compiler->gp_tmpreg, X86_EAX);
  orc_x86_emit_sar_imm_reg (compiler, 4,
      compiler->loop_shift + compiler->unroll_shift, compiler->gp_tmpreg);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);

  /* Calculate n3 */
  orc_x86_emit_and_imm_reg (compiler, 4,
      (1 << (compiler->loop_shift + compiler->unroll_shift)) - 1, X86_EAX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
      (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);
}

#define LABEL_REGION1_SKIP 1
#define LABEL_INNER_LOOP_START 2
#define LABEL_REGION2_SKIP 3
#define LABEL_OUTER_LOOP 4
#define LABEL_OUTER_LOOP_SKIP 5
// XXX: For AVX-512 onwards, check that this range doesn't overlap
// with the region 1 labels (LABEL_STEP_UP)
#define LABEL_STEP_DOWN(x) (8 + (x))
#define LABEL_STEP_UP(x) (16 + (x))

static void
orc_compiler_avx_save_registers (OrcCompiler *compiler)
{
  orc_uint16 saved = 0;
  for (orc_uint16 i = 0; i < ORC_AVX_REG_AMOUNT; ++i) {
    if (compiler->save_regs[X86_YMM0 + i] == 1) {
      ++saved;
    }
  }
  if (saved > 0) {
    orc_x86_emit_mov_imm_reg (compiler, 4, ORC_AVX_REG_SIZE * saved,
        compiler->gp_tmpreg);
    orc_x86_emit_sub_reg_reg (compiler, compiler->is_64bit ? 8 : 4,
        compiler->gp_tmpreg, X86_ESP);
    saved = 0;
    for (orc_uint16 i = 0; i < ORC_AVX_REG_AMOUNT; ++i) {
      if (compiler->save_regs[X86_YMM0 + i] == 1) {
        orc_x86_emit_mov_avx_memoffset (compiler, ORC_AVX_REG_SIZE,
            X86_YMM0 + i, saved * ORC_AVX_REG_SIZE, X86_ESP, FALSE, FALSE);
        ++saved;
      }
    }
  }
}

static void
orc_compiler_avx_restore_registers (OrcCompiler *compiler)
{
  orc_uint16 saved = 0;
  for (orc_uint16 i = 0; i < ORC_AVX_REG_AMOUNT; ++i) {
    if (compiler->save_regs[X86_YMM0 + i] == 1) {
      orc_x86_emit_mov_memoffset_avx (compiler, ORC_AVX_REG_SIZE,
          saved * ORC_AVX_REG_SIZE, X86_ESP, X86_YMM0 + i, FALSE);
      ++saved;
    }
  }
  if (saved > 0) {
    orc_x86_emit_mov_imm_reg (compiler, 4, ORC_AVX_REG_SIZE * saved,
        compiler->gp_tmpreg);
    orc_x86_emit_add_reg_reg (compiler, compiler->is_64bit ? 8 : 4,
        compiler->gp_tmpreg, X86_ESP);
  }
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

static void
orc_compiler_avx_assemble (OrcCompiler *compiler)
{
  int set_mxcsr = FALSE;

  // Adjust alignment of variables -- AVX requires 32-byte
  for (int i = ORC_VAR_D1; i <= ORC_VAR_S8; i++) {
    if (compiler->vars[i].size == 0)
      continue;
    if (compiler->vars[i].alignment >= 32) {
      compiler->vars[i].is_aligned = TRUE;
    } else {
      compiler->vars[i].is_aligned = FALSE;
    }
  }

  const int align_var = get_align_var (compiler);
  if (align_var < 0) {
    orc_x86_assemble_copy (compiler);
    return;
  }
  const int is_aligned = compiler->vars[align_var].is_aligned;

  {
    orc_avx_emit_loop (compiler, 0, 0);

    compiler->codeptr = compiler->code;
    free (compiler->asm_code);
    compiler->asm_code = NULL;
    compiler->asm_code_len = 0;
    memset (compiler->labels, 0, sizeof (compiler->labels));
    memset (compiler->labels_int, 0, sizeof (compiler->labels_int));
    compiler->n_fixups = 0;
    compiler->n_output_insns = 0;
  }

  if (compiler->error)
    return;

  orc_x86_emit_prologue (compiler);

  orc_compiler_avx_save_registers (compiler);

  if (orc_program_has_float (compiler)) {
    set_mxcsr = TRUE;
    orc_avx_set_mxcsr (compiler);
  }

  avx_load_constants_outer (compiler);

  if (compiler->program->is_2d) {
    if (compiler->program->constant_m > 0) {
      orc_x86_emit_mov_imm_reg (compiler, 4, compiler->program->constant_m,
          X86_EAX);
      orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A2]),
          compiler->exec_reg);
    } else {
      orc_x86_emit_mov_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A1]),
          compiler->exec_reg, X86_EAX);
      orc_x86_emit_test_reg_reg (compiler, 4, X86_EAX, X86_EAX);
      orc_x86_emit_jle (compiler, LABEL_OUTER_LOOP_SKIP);
      orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A2]),
          compiler->exec_reg);
    }

    orc_x86_emit_label (compiler, LABEL_OUTER_LOOP);
  }

  if (compiler->program->constant_n > 0
      && compiler->program->constant_n <= ORC_SSE_ALIGNED_DEST_CUTOFF) {
    /* don't need to load n */
  } else if (compiler->loop_shift > 0) {
    if (compiler->has_iterator_opcode || is_aligned) {
      orc_emit_split_2_regions (compiler);
    } else {
      /* split n into three regions, with center region being aligned */
      orc_emit_split_3_regions (compiler);
    }
  } else {
    /* loop shift is 0, no need to split */
    orc_x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, n), compiler->exec_reg,
        compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
  }

  avx_load_constants_inner (compiler);

  if (compiler->program->constant_n > 0
      && compiler->program->constant_n <= ORC_SSE_ALIGNED_DEST_CUTOFF) {
    int n_left = compiler->program->constant_n;
    int save_loop_shift;
    int loop_shift;

    compiler->offset = 0;

    save_loop_shift = compiler->loop_shift;
    while (n_left >= (1 << compiler->loop_shift)) {
      ORC_ASM_CODE (compiler, "# AVX LOOP SHIFT %d\n", compiler->loop_shift);
      orc_avx_emit_loop (compiler, compiler->offset, 0);

      n_left -= 1 << compiler->loop_shift;
      compiler->offset += 1 << compiler->loop_shift;
    }
    for (loop_shift = compiler->loop_shift - 1; loop_shift >= 0; loop_shift--) {
      if (n_left >= (1 << loop_shift)) {
        compiler->loop_shift = loop_shift;
        ORC_ASM_CODE (compiler, "# AVX LOOP SHIFT %d\n", loop_shift);
        orc_avx_emit_loop (compiler, compiler->offset, 0);
        n_left -= 1 << loop_shift;
        compiler->offset += 1 << loop_shift;
      }
    }
    compiler->loop_shift = save_loop_shift;

  } else {
    int ui, ui_max;
    int emit_region1 = TRUE;
    int emit_region3 = TRUE;

    if (compiler->has_iterator_opcode || is_aligned) {
      emit_region1 = FALSE;
    }
    if (compiler->loop_shift == 0) {
      emit_region1 = FALSE;
      emit_region3 = FALSE;
    }

    if (emit_region1) {
      int save_loop_shift;
      int l;

      save_loop_shift = compiler->loop_shift;
      compiler->vars[align_var].is_aligned = FALSE;

      for (l = 0; l < save_loop_shift; l++) {
        compiler->loop_shift = l;
        ORC_ASM_CODE (compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);

        orc_x86_emit_test_imm_memoffset (compiler, 4, 1 << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, counter1), compiler->exec_reg);
        orc_x86_emit_je (compiler, LABEL_STEP_UP (compiler->loop_shift));
        orc_avx_emit_loop (compiler, 0, 1 << compiler->loop_shift);
        orc_x86_emit_label (compiler, LABEL_STEP_UP (compiler->loop_shift));
      }

      compiler->loop_shift = save_loop_shift;
      compiler->vars[align_var].is_aligned = TRUE;
    }

    orc_x86_emit_label (compiler, LABEL_REGION1_SKIP);

    orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
    orc_x86_emit_je (compiler, LABEL_REGION2_SKIP);

    if (compiler->loop_counter != ORC_REG_INVALID) {
      orc_x86_emit_mov_memoffset_reg (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg,
          compiler->loop_counter);
    }

    ORC_ASM_CODE (compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);
    // Instruction fetch windows are 16-byte aligned
    // https://easyperf.net/blog/2018/01/18/Code_alignment_issues
    orc_x86_emit_align (compiler, 4);
    orc_x86_emit_label (compiler, LABEL_INNER_LOOP_START);
    ui_max = 1 << compiler->unroll_shift;
    for (ui = 0; ui < ui_max; ui++) {
      compiler->offset = ui << compiler->loop_shift;
      orc_avx_emit_loop (compiler, compiler->offset,
          (ui == ui_max - 1)
              << (compiler->loop_shift + compiler->unroll_shift));
    }
    compiler->offset = 0;
    if (compiler->loop_counter != ORC_REG_INVALID) {
      orc_x86_emit_add_imm_reg (compiler, 4, -1, compiler->loop_counter, TRUE);
    } else {
      orc_x86_emit_dec_memoffset (compiler, 4,
          (int)ORC_STRUCT_OFFSET (OrcExecutor, counter2), compiler->exec_reg);
    }
    orc_x86_emit_jne (compiler, LABEL_INNER_LOOP_START);
    orc_x86_emit_label (compiler, LABEL_REGION2_SKIP);

    if (emit_region3) {
      int save_loop_shift;
      int l;

      save_loop_shift = compiler->loop_shift + compiler->unroll_shift;
      compiler->vars[align_var].is_aligned = FALSE;

      for (l = save_loop_shift - 1; l >= 0; l--) {
        compiler->loop_shift = l;
        ORC_ASM_CODE (compiler, "# LOOP SHIFT %d\n", compiler->loop_shift);

        orc_x86_emit_test_imm_memoffset (compiler, 4, 1 << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET (OrcExecutor, counter3), compiler->exec_reg);
        orc_x86_emit_je (compiler, LABEL_STEP_DOWN (compiler->loop_shift));
        orc_avx_emit_loop (compiler, 0, 1 << compiler->loop_shift);
        orc_x86_emit_label (compiler, LABEL_STEP_DOWN (compiler->loop_shift));
      }

      compiler->loop_shift = save_loop_shift;
    }
  }

  if (compiler->program->is_2d && compiler->program->constant_m != 1) {
    avx_add_strides (compiler);

    orc_x86_emit_add_imm_memoffset (compiler, 4, -1,
        (int)ORC_STRUCT_OFFSET (OrcExecutor, params[ORC_VAR_A2]),
        compiler->exec_reg);
    orc_x86_emit_jne (compiler, LABEL_OUTER_LOOP);
    orc_x86_emit_label (compiler, LABEL_OUTER_LOOP_SKIP);
  }

  avx_save_accumulators (compiler);

  if (set_mxcsr) {
    orc_avx_restore_mxcsr (compiler);
  }

  orc_compiler_avx_restore_registers (compiler);

  orc_x86_emit_epilogue (compiler);

  orc_x86_calculate_offsets (compiler);
  orc_x86_output_insns (compiler);

  orc_x86_do_fixups (compiler);
}

static void
orc_avx_emit_loop (OrcCompiler *compiler, int offset, int update)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  OrcRule *rule;

  int *const insn_idx = get_optimised_instruction_order (compiler);

  for (j = 0; j < compiler->n_insns; j++) {
    insn = compiler->insns + insn_idx[j];
    opcode = insn->opcode;

    compiler->insn_index = j;

    if (insn->flags & ORC_INSN_FLAG_INVARIANT)
      continue;

    ORC_ASM_CODE (compiler, "# %d: %s\n", j, insn->opcode->name);

    compiler->min_temp_reg = ORC_VEC_REG_BASE;

    compiler->insn_shift = compiler->loop_shift;
    if (insn->flags & ORC_INSTRUCTION_FLAG_X2) {
      compiler->insn_shift += 1;
    }
    if (insn->flags & ORC_INSTRUCTION_FLAG_X4) {
      compiler->insn_shift += 2;
    }

    rule = insn->rule;
    if (rule && rule->emit) {
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_error (compiler, "no code generation rule for %s",
          opcode->name);
    }
  }

  if (update) {
    for (k = 0; k < ORC_N_COMPILER_VARIABLES; k++) {
      OrcVariable *var = compiler->vars + k;

      if (var->name == NULL)
        continue;
      if (var->vartype == ORC_VAR_TYPE_SRC
          || var->vartype == ORC_VAR_TYPE_DEST) {
        int offset;
        if (var->update_type == 0) {
          offset = 0;
        } else if (var->update_type == 1) {
          offset = (var->size * update) >> 1;
        } else {
          offset = var->size * update;
        }

        if (offset != 0) {
          if (compiler->vars[k].ptr_register) {
            orc_x86_emit_add_imm_reg (compiler, compiler->is_64bit ? 8 : 4,
                offset, compiler->vars[k].ptr_register, FALSE);
          } else {
            orc_x86_emit_add_imm_memoffset (compiler,
                compiler->is_64bit ? 8 : 4, offset,
                (int)ORC_STRUCT_OFFSET (OrcExecutor, arrays[k]),
                compiler->exec_reg);
          }
        }
      }
    }
  }

  free (insn_idx);
}
