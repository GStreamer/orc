
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>
#include <orc/orcx86-private.h>
#include <orc/orcmmx.h>

/**
 * SECTION:orcmmx
 * @title: MMX
 * @short_description: code generation for MMX
 */

/* FIXME rename it to orc_mmx prefix */
const char *
orc_x86_get_regname_mmx(int i)
{
  static const char *x86_regs[] = {
    "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
    "mm8", "mm9", "mm10", "mm11", "mm12", "mm13", "mm14", "mm15"
  };

  if (i>=X86_MM0 && i<X86_MM0 + 16) return x86_regs[i - X86_MM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
orc_mmx_get_regname_by_size (int reg, int size)
{
  if (reg >= X86_MM0 && reg < X86_MM0 + 16)
    return orc_x86_get_regname_mmx (reg);
  else
    return orc_x86_get_regname_size (reg, size);
}

#if 0
void
orc_x86_emit_mov_memindex_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %d(%%%s,%%%s,%d), %%%s\n", offset,
          orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %d(%%%s,%%%s,%d), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_ptr(compiler, regindex), 1<<shift,
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
  orc_x86_emit_modrm_memindex (compiler, reg2, offset, reg1, regindex, shift);
}

void
orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %d(%%%s), %%%s\n", offset, orc_x86_get_regname_ptr(compiler, reg1),
          orc_x86_get_regname_mmx(reg2));
      orc_x86_emit_rex(compiler, 0, reg2, 0, reg1);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x6f;
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
  orc_x86_emit_modrm_memoffset (compiler, offset, reg1, reg2);
}

void
orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      ORC_ASM_CODE(compiler,"  movd %%%s, %d(%%%s)\n", orc_x86_get_regname_mmx(reg1), offset,
          orc_x86_get_regname_ptr(compiler, reg2));
      orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7e;
      break;
    case 8:
      ORC_ASM_CODE(compiler,"  movq %%%s, %d(%%%s)\n", orc_x86_get_regname_mmx(reg1), offset,
          orc_x86_get_regname_ptr(compiler, reg2));
      orc_x86_emit_rex(compiler, 0, reg1, 0, reg2);
      *compiler->codeptr++ = 0x0f;
      *compiler->codeptr++ = 0x7f;
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }

  orc_x86_emit_modrm_memoffset (compiler, offset, reg2, reg1);
}
#endif

void
orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      orc_mmx_emit_movd_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_mmx_emit_movq_load_memoffset (compiler, offset, reg1, reg2);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_mov_memindex_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      orc_mmx_emit_movd_load_memindex (compiler, offset,
          reg1, regindex, shift, reg2);
      break;
    case 8:
      orc_mmx_emit_movq_load_memindex (compiler, offset,
          reg1, regindex, shift, reg2);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1,
    int offset, int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      orc_mmx_emit_movd_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_mmx_emit_movq_store_memoffset (compiler, offset, reg1, reg2);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

unsigned int
orc_mmx_get_cpu_flags (void)
{
  unsigned int flags = 0;
#if defined(HAVE_AMD64) || defined(HAVE_I386)
  OrcX86CPUVendor vendor = 0;
  orc_uint32 level = 0;
  orc_uint32 eax, ebx, ecx, edx;

  orc_x86_cpu_detect (&level, &vendor);
  /* The AMD specific case */
  if (vendor == ORC_X86_CPU_VENDOR_AMD && level >= 1) {
    orc_x86_cpu_get_cpuid (0x80000001, &eax, &ebx, &ecx, &edx);

    if (edx & (1<<22)) {
      flags |= ORC_TARGET_MMX_MMXEXT;
    }
    if (edx & (1U<<31)) {
      flags |= ORC_TARGET_MMX_3DNOW;
    }
    if (edx & (1U<<30)) {
      flags |= ORC_TARGET_MMX_3DNOWEXT;
    }
  }
  orc_x86_cpu_get_cpuid (0x00000001, &eax, &ebx, &ecx, &edx);

  if (edx & (1<<23)) {
    flags |= ORC_TARGET_MMX_MMX;
  }
  if (edx & (1<<26)) {
    flags |= ORC_TARGET_MMX_SSE2;
  }
  if (ecx & (1<<0)) {
    flags |= ORC_TARGET_MMX_SSE3;
  }
  if (ecx & (1<<9)) {
    flags |= ORC_TARGET_MMX_SSSE3;
  }
  if (ecx & (1<<19)) {
    flags |= ORC_TARGET_MMX_SSE4_1;
  }
  if (ecx & (1<<20)) {
    flags |= ORC_TARGET_MMX_SSE4_2;
  }

  /* unset the flags */
  if (orc_compiler_flag_check ("-mmx")) {
    flags &= ~ORC_TARGET_MMX_MMX;
  }
  if (orc_compiler_flag_check ("-mmxext")) {
    flags &= ~ORC_TARGET_MMX_MMXEXT;
  }
  if (orc_compiler_flag_check ("-3dnow")) {
    flags &= ~ORC_TARGET_MMX_3DNOW;
  }
  if (orc_compiler_flag_check ("-3dnowext")) {
    flags &= ~ORC_TARGET_MMX_3DNOWEXT;
  }
  if (orc_compiler_flag_check ("-sse2")) {
    flags &= ~ORC_TARGET_MMX_SSE2;
  }
  if (orc_compiler_flag_check ("-sse3")) {
    flags &= ~ORC_TARGET_MMX_SSE3;
  }
  if (orc_compiler_flag_check ("-ssse3")) {
    flags &= ~ORC_TARGET_MMX_SSSE3;
  }
  if (orc_compiler_flag_check ("-sse41")) {
    flags &= ~ORC_TARGET_MMX_SSE4_1;
  }
  if (orc_compiler_flag_check ("-sse42")) {
    flags &= ~ORC_TARGET_MMX_SSE4_2;
  }
#endif
  return flags;
}


