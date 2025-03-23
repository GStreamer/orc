#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcx86.h>
#include <orc/orcx86-private.h>
#include <orc/orcmmx.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>
#include <orc/orcinternal.h>

#define ORC_REG_SIZE 8

/* TODO To be placed in a common header for private stuff */
void orc_compiler_mmx_register_rules (OrcTarget *target);

/* X86 specific */
static unsigned int
mmx_get_default_flags (void)
{
  unsigned int flags = 0;

#if defined(HAVE_AMD64)
  flags |= ORC_TARGET_MMX_64BIT;
#endif
  if (orc_compiler_is_debug ()) {
    flags |= ORC_TARGET_MMX_FRAME_POINTER;
  }
  
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  flags |= orc_mmx_get_cpu_flags ();
#else
  flags |= ORC_TARGET_MMX_MMX;
  flags |= ORC_TARGET_MMX_3DNOW;
#endif

  return flags;
}

static const char *
mmx_get_flag_name (int shift)
{
  static const char *flags[] = {
    "mmx",
    "mmxext",
    "3dnow",
    "3dnowext",
    "ssse3",
    "sse41",
    "sse42",
    "frame_pointer",
    "short_jumps",
    "64bit"
    "sse2",
    "sse3",
  };

  if (shift >= 0 && shift < sizeof(flags)/sizeof(flags[0])) {
    return flags[shift];
  }

  return NULL;
}

static int
mmx_is_executable (void)
{
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  /* initializes cache information */
  const int flags = orc_mmx_get_cpu_flags ();

  if (flags & ORC_TARGET_MMX_MMX) {
    return TRUE;
  }
#endif
  return FALSE;
}

static void
mmx_validate_registers (int *regs, int is_64bit)
{
  int i;

  if (is_64bit) {
    for (i = X86_MM0; i < X86_MM0 + ORC_REG_SIZE; i++) {
      regs[i] = 1;
    }
  } else {
    for (i = X86_MM0; i < X86_MM0 + ORC_REG_SIZE; i++) {
      regs[i] = 1;
    }
  }
}

static void
mmx_saveable_registers (int *regs, int is_64bit)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  if (is_64bit) {
    int i;
    for(i = X86_MM0 + 6; i < X86_MM0 + ORC_REG_SIZE; i++){
      regs[i] = 1;
    }
  }
#endif
}

static int
mmx_is_64bit (int flags)
{
  if (flags & ORC_TARGET_SSE_64BIT) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
mmx_use_frame_pointer (int flags)
{
  if (flags & ORC_TARGET_SSE_FRAME_POINTER) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
mmx_use_long_jumps (int flags)
{
  if (!(flags & ORC_TARGET_SSE_SHORT_JUMPS)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static int
mmx_loop_shift (int max_var_size)
{
  switch (max_var_size) {
    case 1:
      return 3;
    case 2:
      return 2;
    case 4:
      return 1;
    case 8:
      return 0;
    default:
      ORC_ERROR ("unhandled max var size %d", max_var_size);
      break;
  }

  return -1;
}

static void
mmx_init_accumulator (OrcCompiler *compiler, OrcVariable *var)
{
  orc_mmx_emit_pxor (compiler, var->alloc, var->alloc);
}

static void
mmx_reduce_accumulator (OrcCompiler *compiler, int i, OrcVariable *var)
{
  const int src = var->alloc;
  const int tmp = orc_compiler_get_temp_reg (compiler);

  orc_mmx_emit_pshufw (compiler, ORC_MMX_SHUF(3,2,3,2), src, tmp);

  if (var->size == 2) {
    orc_mmx_emit_paddw (compiler, tmp, src);
  } else {
    orc_mmx_emit_paddd (compiler, tmp, src);
  }

  if (var->size == 2) {
    orc_mmx_emit_pshufw (compiler, ORC_MMX_SHUF(1,1,1,1), src, tmp);
    orc_mmx_emit_paddw (compiler, tmp, src);
  }
  if (var->size == 2) {
    orc_mmx_emit_movd_store_register (compiler, src, compiler->gp_tmpreg);
    orc_x86_emit_and_imm_reg (compiler, 4, 0xffff, compiler->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]),
        compiler->exec_reg);
  } else {
    orc_x86_emit_mov_mmx_memoffset (compiler, 4, src,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, accumulators[i-ORC_VAR_A1]),
        compiler->exec_reg,
        var->is_aligned, var->is_uncached);
  }
}

static void
orc_mmx_load_constant_full_u64 (OrcCompiler *c, int reg, orc_uint64 value)
{
  orc_x86_emit_cpuinsn_comment (c, "# loading full u64 constant %" PRIu64
      " 0x%16" PRIx64, value, value);
  if (c->is_64bit) {
    orc_x86_emit_mov_imm_reg64 (c, 8, value, c->gp_tmpreg);
    orc_mmx_emit_movq_load_register (c, c->gp_tmpreg, reg);
  } else {
    int offset = ORC_STRUCT_OFFSET(OrcExecutor,arrays[ORC_VAR_T1]);

    orc_x86_emit_mov_imm_reg (c, 4, value & UINT32_MAX,
        c->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (c, 4, c->gp_tmpreg,
        offset + 0, c->exec_reg);

    orc_x86_emit_mov_imm_reg (c, 4, value>>32,
        c->gp_tmpreg);
    orc_x86_emit_mov_reg_memoffset (c, 4, c->gp_tmpreg,
        offset + 4, c->exec_reg);

    orc_x86_emit_mov_memoffset_mmx (c, 8, offset, c->exec_reg,
        reg, FALSE);
  }
}

static void
orc_mmx_load_constant_full_u32 (OrcCompiler *c, int reg, orc_uint32 value)
{
  orc_x86_emit_cpuinsn_comment (c, "# loading full u32 constant %d 0x%08x", value, value);
  orc_x86_emit_mov_imm_reg (c, 4, value, c->gp_tmpreg);
  orc_mmx_emit_movd_load_register (c, c->gp_tmpreg, reg);
  orc_mmx_emit_pshufw (c, ORC_MMX_SHUF(1,0,1,0), reg, reg);
}

static orc_bool
orc_mmx_load_constant_u8 (OrcCompiler *c, int reg, orc_uint8 value)
{
  orc_x86_emit_cpuinsn_comment (c, "# loading u8 constant %d 0x%02x", value, value);
  if (value == 0) {
    orc_mmx_emit_pxor (c, reg, reg);
    return TRUE;
  } else if (value == UINT8_MAX) {
    orc_mmx_emit_pcmpeqb (c, reg, reg);
    return TRUE;
  } else if (c->target_flags & ORC_TARGET_MMX_SSSE3) {
    if (value == 1) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_pabsb (c, reg, reg);
      return TRUE;
    }
  }

  return FALSE;
}

static orc_bool
orc_mmx_load_constant_u16 (OrcCompiler *c, int reg, orc_uint16 value)
{
  int i;

  orc_x86_emit_cpuinsn_comment (c, "# loading u16 constant %d 0x%04x", value, value);
  if (value == 0) {
    orc_mmx_emit_pxor (c, reg, reg);
    return TRUE;
  } else if (value == UINT16_MAX) {
    orc_mmx_emit_pcmpeqb (c, reg, reg);
    return TRUE;
  } else if (c->target_flags & ORC_TARGET_MMX_SSSE3) {
    if (value == 1) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_pabsw (c, reg, reg);
      return TRUE;
    }
  }

  for (i = 1; i < 16; i++) {
    orc_uint16 v;
    v = 0xffff << i;
    if (value == v) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_psllw_imm (c, i, reg);
      return TRUE;
    }
    v = 0xffff >> i;
    if (value == v) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_psrlw_imm (c, i, reg);
      return TRUE;
    }
  }
  return FALSE;
}


static orc_bool
orc_mmx_load_constant_u32 (OrcCompiler *c, int reg, orc_uint32 value)
{
  int i;

  orc_x86_emit_cpuinsn_comment (c, "# loading u32 constant %d 0x%08x", value, value);
  if (value == 0) {
    orc_mmx_emit_pxor (c, reg, reg);
    return TRUE;
  } else if (value == UINT32_MAX) {
    orc_mmx_emit_pcmpeqb (c, reg, reg);
    return TRUE;
  } else if (c->target_flags & ORC_TARGET_MMX_SSSE3) {
    if (value == 1) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_pabsd (c, reg, reg);
      return TRUE;
    }
  }

  for (i = 1; i < 32; i++) {
    orc_uint32 v;
    v = 0xffffffff << i;
    if (value == v) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_pslld_imm (c, i, reg);
      return TRUE;
    }
    v = 0xffffffff >> i;
    if (value == v) {
      orc_mmx_emit_pcmpeqb (c, reg, reg);
      orc_mmx_emit_psrld_imm (c, i, reg);
      return TRUE;
    }
  }

  return FALSE;
}

static void
orc_mmx_load_constant_long (OrcCompiler *c, int reg, OrcConstant *cnst)
{
  switch (cnst->type) {
    case ORC_CONST_ZERO:
      orc_mmx_emit_pxor (c, reg, reg);
      break;

    case ORC_CONST_SPLAT_B:
      if (!orc_mmx_load_constant_u8 (c, reg, cnst->v[0].x8[0])) {
        OrcConstant r;
        orc_constant_resolve (cnst, &r, 8);
        orc_mmx_load_constant_full_u32 (c, reg, r.v[0].x2[0]);
      }
      break;

    case ORC_CONST_SPLAT_W:
      if (!orc_mmx_load_constant_u16 (c, reg, cnst->v[0].x4[0])) {
        OrcConstant r;
        orc_constant_resolve (cnst, &r, 8);
        orc_mmx_load_constant_full_u32 (c, reg, r.v[0].x2[0]);
      }
      break;
    case ORC_CONST_SPLAT_L:
      if (!orc_mmx_load_constant_u32 (c, reg, cnst->v[0].x2[0])) {
        orc_mmx_load_constant_full_u32 (c, reg, cnst->v[0].x2[0]);
      }
      break;

    case ORC_CONST_FULL:
    case ORC_CONST_SPLAT_Q:
      orc_mmx_load_constant_full_u64 (c, reg, cnst->v[0].i);
      break;

    default:
      ORC_COMPILER_ERROR (c, "Unsupported constant type %d", cnst->type);
      break;
  }
}

static void
mmx_move_register_to_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2, int aligned, int uncached)
{
  orc_x86_emit_mov_mmx_memoffset (compiler, size, reg1, offset, reg2, aligned, uncached);
}

static void
mmx_move_memoffset_to_register (OrcCompiler *compiler, int size, int offset, int reg1, int reg2, int is_aligned)
{
  orc_x86_emit_mov_memoffset_mmx (compiler, size, offset, reg1, reg2, is_aligned);
}

static int
mmx_get_shift (int size)
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
mmx_clear_emms(OrcCompiler *c)
{
  orc_mmx_emit_emms (c);
}

void
orc_mmx_init (void)
{
  // clang-format off
  static OrcX86Target target = {
    "mmx",
    mmx_get_default_flags,
    mmx_get_flag_name,
    mmx_is_executable,
    mmx_validate_registers,
    mmx_saveable_registers,
    mmx_is_64bit,
    mmx_use_frame_pointer,
    mmx_use_long_jumps,
    mmx_loop_shift,
    mmx_init_accumulator,
    mmx_reduce_accumulator,
    NULL,
    orc_mmx_load_constant_long,
    mmx_move_register_to_memoffset,
    mmx_move_memoffset_to_register,
    mmx_get_shift,
    NULL,
    NULL,
    mmx_clear_emms,
    NULL,
    8,
    X86_MM0,
    ORC_REG_SIZE,
    13,
  };
  // clang-format on
  static OrcTarget t;

  orc_x86_register_extension (&t, &target);
  orc_compiler_mmx_register_rules (&t);
}
