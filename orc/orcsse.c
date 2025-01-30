
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
#include <orc/orcsse.h>
#include <orc/orcmmx.h>
#include <orc/orcx86insn.h>

/**
 * SECTION:orcsse
 * @title: SSE
 * @short_description: code generation for SSE
 */


const char *
orc_x86_get_regname_sse(int i)
{
  static const char *x86_regs[] = {
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
  };

  if (i>=X86_XMM0 && i<X86_XMM0 + 16) return x86_regs[i - X86_XMM0];
  if (i>=X86_MM0 && i<X86_MM0 + 8) return "ERROR_MMX";
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
orc_sse_get_regname_by_size (int reg, int size)
{
  if (reg >= X86_XMM0 && reg < X86_XMM0 + 16)
    return orc_x86_get_regname_sse (reg);
  else
    return orc_x86_get_regname_size (reg, size);
}

unsigned int
orc_sse_get_cpu_flags (void)
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

    if (ecx & (1<<6)) {
      flags |= ORC_TARGET_SSE_SSE4A;
    }
    if (ecx & (1<<11)) {
      flags |= ORC_TARGET_SSE_SSE5;
    }
  }
  orc_x86_cpu_get_cpuid (0x00000001, &eax, &ebx, &ecx, &edx);

  if (edx & (1<<25)) {
    flags |= ORC_TARGET_SSE_SSE;
  }
  if (edx & (1<<26)) {
    flags |= ORC_TARGET_SSE_SSE2;
  }
  if (ecx & (1<<0)) {
    flags |= ORC_TARGET_SSE_SSE3;
  }
  if (ecx & (1<<9)) {
    flags |= ORC_TARGET_SSE_SSSE3;
  }
  if (ecx & (1<<19)) {
    flags |= ORC_TARGET_SSE_SSE4_1;
  }
  if (ecx & (1<<20)) {
    flags |= ORC_TARGET_SSE_SSE4_2;
  }

  /* unset the flags */
  if (orc_compiler_flag_check ("-sse")) {
    flags &= ~ORC_TARGET_SSE_SSE;
  }
  if (orc_compiler_flag_check ("-sse2")) {
    flags &= ~ORC_TARGET_SSE_SSE2;
  }
  if (orc_compiler_flag_check ("-sse3")) {
    flags &= ~ORC_TARGET_SSE_SSE3;
  }
  if (orc_compiler_flag_check ("-ssse3")) {
    flags &= ~ORC_TARGET_SSE_SSSE3;
  }
  if (orc_compiler_flag_check ("-sse41")) {
    flags &= ~ORC_TARGET_SSE_SSE4_1;
  }
  if (orc_compiler_flag_check ("-sse42")) {
    flags &= ~ORC_TARGET_SSE_SSE4_2;
  }
  if (orc_compiler_flag_check ("-sse4a")) {
    flags &= ~ORC_TARGET_SSE_SSE4A;
  }
  if (orc_compiler_flag_check ("-sse5")) {
    flags &= ~ORC_TARGET_SSE_SSE5;
  }
#endif
  return flags;
}
