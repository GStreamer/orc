#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcx86.h>
#include <orc/orcsse.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>

extern int orc_x86_sse_flags;

/* TODO To be placed in a common header for private stuff */
void orc_compiler_sse_register_rules (OrcTarget *target);

/* X86 specific */
static unsigned int
sse_get_default_flags (void)
{
  unsigned int flags = 0;

#if defined(HAVE_AMD64)
  flags |= ORC_TARGET_SSE_64BIT;
#endif
  if (_orc_compiler_flag_debug) {
    flags |= ORC_TARGET_SSE_FRAME_POINTER;
  }
  
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  flags |= orc_x86_sse_flags;
#else
  flags |= ORC_TARGET_SSE_SSE2;
  flags |= ORC_TARGET_SSE_SSE3;
  flags |= ORC_TARGET_SSE_SSSE3;
#endif

  return flags;
}

static const char *
sse_get_flag_name (int shift)
{
  static const char *flags[] = {
    "sse2", "sse3", "ssse3", "sse41", "sse42", "sse4a", "sse5",
    "frame_pointer", "short_jumps", "64bit"
  };

  if (shift >= 0 && shift < sizeof(flags)/sizeof(flags[0])) {
    return flags[shift];
  }

  return NULL;
}

static int
sse_is_executable (void)
{
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  /* initializes cache information */
  const int flags = orc_sse_get_cpu_flags ();

  if (flags & ORC_TARGET_SSE_SSE2) {
    return TRUE;
  }
#endif
  return FALSE;
}

static void
sse_validate_registers (int *regs, int is_64bit)
{
  int i;

  if (is_64bit) {
    for(i = X86_XMM0;i < X86_XMM0 + 16; i++){
      regs[i] = 1;
    }
  } else {
    for(i = X86_XMM0; i < X86_XMM0 + 8; i++){
      regs[i] = 1;
    }
  }
}

static void
sse_saveable_registers (int *regs, int is_64bit)
{
#ifdef HAVE_OS_WIN32
  if (is_64bit) {
    int i;
    for(i = X86_XMM0 + 6; i < X86_XMM0 + 16; i++){
      regs[i] = 1;
    }
  }
#endif
}

static int
sse_is_64bit (int flags)
{
  if (flags & ORC_TARGET_SSE_64BIT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
sse_use_frame_pointer (int flags)
{
  if (flags & ORC_TARGET_SSE_FRAME_POINTER) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
sse_use_long_jumps (int flags)
{
  if (!(flags & ORC_TARGET_SSE_SHORT_JUMPS)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
sse_loop_shift (int max_var_size)
{
  switch (max_var_size) {
    case 1:
      return 4;
    case 2:
      return 3;
    case 4:
      return 2;
    case 8:
      return 1;
    default:
      ORC_ERROR ("unhandled max var size %d", max_var_size);
      break;
  }

  return -1;
}

static void
sse_init_accumulator (OrcCompiler *compiler, OrcVariable *var)
{
  orc_sse_emit_pxor (compiler, var->alloc, var->alloc);
}

static void
sse_reduce_accumulator (OrcCompiler *compiler, int i, OrcVariable *var) {
  const int src = var->alloc;
  const int tmp = orc_compiler_get_temp_reg (compiler);

        orc_sse_emit_pshufd (compiler, ORC_SSE_SHUF(3,2,3,2), src, tmp);
        if (var->size == 2) {
          orc_sse_emit_paddw (compiler, tmp, src);
        } else {
          orc_sse_emit_paddd (compiler, tmp, src);
        }

        orc_sse_emit_pshufd (compiler, ORC_SSE_SHUF(1,1,1,1), src, tmp);

        if (var->size == 2) {
          orc_sse_emit_paddw (compiler, tmp, src);
        } else {
          orc_sse_emit_paddd (compiler, tmp, src);
        }

        if (var->size == 2) {
          orc_sse_emit_pshuflw (compiler, ORC_SSE_SHUF(1,1,1,1), src, tmp);
          orc_sse_emit_paddw (compiler, tmp, src);
        }

        if (var->size == 2) {
          orc_sse_emit_movd_store_register (compiler, src, compiler->gp_tmpreg);
          orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, compiler->gp_tmpreg);
          orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]),
              compiler->exec_reg);
        } else {
          orc_x86_emit_mov_sse_memoffset (compiler, 4, src,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]),
              compiler->exec_reg,
              var->is_aligned, var->is_uncached);
        }
}


void
orc_sse_load_constant (OrcCompiler *compiler, int reg, int size, orc_uint64 value)
{
  int i;

  if (size == 8) {
    int offset = ORC_STRUCT_OFFSET(OrcExecutor,arrays[ORC_VAR_T1]);

    /* FIXME how ugly and slow! */
    orc_x86_emit_mov_imm_reg (compiler, 4, value>>0,
        compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        offset + 0, compiler->exec_reg);

    orc_x86_emit_mov_imm_reg (compiler, 4, value>>32,
        compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        offset + 4, compiler->exec_reg);

    orc_x86_emit_mov_memoffset_sse (compiler, 8, offset, compiler->exec_reg,
        reg, FALSE);
    orc_sse_emit_pshufd (compiler, ORC_SSE_SHUF(1,0,1,0), reg, reg);
    return;
  }

  if (size == 1) {
    value &= 0xff;
    value |= (value << 8);
    value |= (value << 16);
  }
  if (size == 2) {
    value &= 0xffff;
    value |= (value << 16);
  }

  ORC_ASM_CODE(compiler, "# loading constant %d 0x%08x\n", (int)value, (int)value);
  if (value == 0) {
    orc_sse_emit_pxor(compiler, reg, reg);
    return;
  }
  if (value == 0xffffffff) {
    orc_sse_emit_pcmpeqb (compiler, reg, reg);
    return;
  }
  if (compiler->target_flags & ORC_TARGET_SSE_SSSE3) {
    if (value == 0x01010101) {
      orc_sse_emit_pcmpeqb (compiler, reg, reg);
      orc_sse_emit_pabsb (compiler, reg, reg);
      return;
    }
  }

  for(i=1;i<32;i++){
    orc_uint32 v;
    v = (0xffffffff<<i);
    if (value == v) {
      orc_sse_emit_pcmpeqb (compiler, reg, reg);
      orc_sse_emit_pslld_imm (compiler, i, reg);
      return;
    }
    v = (0xffffffff>>i);
    if (value == v) {
      orc_sse_emit_pcmpeqb (compiler, reg, reg);
      orc_sse_emit_psrld_imm (compiler, i, reg);
      return;
    }
  }
  for(i=1;i<16;i++){
    orc_uint32 v;
    v = (0xffff & (0xffff<<i)) | (0xffff0000 & (0xffff0000<<i));
    if (value == v) {
      orc_sse_emit_pcmpeqb (compiler, reg, reg);
      orc_sse_emit_psllw_imm (compiler, i, reg);
      return;
    }
    v = (0xffff & (0xffff>>i)) | (0xffff0000 & (0xffff0000>>i));
    if (value == v) {
      orc_sse_emit_pcmpeqb (compiler, reg, reg);
      orc_sse_emit_psrlw_imm (compiler, i, reg);
      return;
    }
  }

  orc_x86_emit_mov_imm_reg (compiler, 4, value, compiler->gp_tmpreg);
  orc_sse_emit_movd_load_register (compiler, compiler->gp_tmpreg, reg);
  orc_sse_emit_pshufd (compiler, ORC_SSE_SHUF(0,0,0,0), reg, reg);
}

void
sse_load_constant_long (OrcCompiler *compiler, int reg,
    OrcConstant *constant)
{
  int i;
  int offset = ORC_STRUCT_OFFSET(OrcExecutor,arrays[ORC_VAR_T1]);

  /* FIXME this is slower than it could be */

  ORC_ASM_CODE(compiler, "# loading constant %08x %08x %08x %08x\n",
      constant->full_value[0], constant->full_value[1],
      constant->full_value[2], constant->full_value[3]);

  for(i=0;i<4;i++){
    orc_x86_emit_mov_imm_reg (compiler, 4, constant->full_value[i],
        compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        offset + 4*i, compiler->exec_reg);
  }
  orc_x86_emit_mov_memoffset_sse (compiler, 16, offset, compiler->exec_reg,
      reg, FALSE);

}

static void
sse_move_register_to_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2, int aligned, int uncached)
{
  orc_x86_emit_mov_sse_memoffset (compiler, size, reg1, offset, reg2, aligned, uncached);
}

static void
sse_move_memoffset_to_register (OrcCompiler *compiler, int size, int offset, int reg1, int reg2, int is_aligned)
{
  orc_x86_emit_mov_memoffset_sse (compiler, size, reg1, offset, reg2, is_aligned);
}

static int
sse_get_shift (int size)
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
    default:
      ORC_ERROR ("bad size %d", size);
  }
  return -1;
}

static void
sse_set_mxcsr (OrcCompiler *c)
{
  orc_sse_set_mxcsr (c);
}

static void
sse_restore_mxcsr(OrcCompiler *c)
{
  orc_sse_restore_mxcsr (c);
}

void
orc_sse_init (void)
{
  // clang-format off
  static OrcX86Target target = {
    "sse",
    sse_get_default_flags,
    sse_get_flag_name,
    sse_is_executable,
    sse_validate_registers,
    sse_saveable_registers,
    sse_is_64bit,
    sse_use_frame_pointer,
    sse_use_long_jumps,
    sse_loop_shift,
    sse_init_accumulator,
    sse_reduce_accumulator,
    orc_sse_load_constant,
    sse_load_constant_long,
    sse_move_register_to_memoffset,
    sse_move_memoffset_to_register,
    sse_get_shift,
    sse_set_mxcsr,
    sse_restore_mxcsr,
    NULL,
    16,
    X86_XMM0,
    16,
    13,
  };
  // clang-format on
  OrcTarget *t;

  t = orc_x86_register_target (&target);
  orc_compiler_sse_register_rules (t);
}
