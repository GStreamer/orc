
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>

#include <orc/orcavx.h>
#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>
#include <orc/orcx86-private.h>
#include <orc/orcsse.h>


/**
 * SECTION:orcavx
 * @title: AVX
 * @short_description: code generation for AVX/AVX2
 */

const char *
orc_x86_get_regname_avx (int i)
{
  static const char *x86_regs[] = { "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
    "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13",
    "ymm14", "ymm15" };

  if (i >= X86_YMM0 && i <= X86_YMM15) return x86_regs[i - X86_YMM0];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

#define XCR0_SUPPORT_XMM 1U << 1
#define XCR0_SUPPORT_YMM 1U << 2
// On a kernel with Gather Data Sampling mitigation, the former will
// be disabled -- both bits must be enabled, otherwise it'll hit an
// undefined opcode trap.
// See https://docs.kernel.org/admin-guide/hw-vuln/gather_data_sampling.html
#define XCR0_SUPPORT_AVX (XCR0_SUPPORT_YMM | XCR0_SUPPORT_XMM)

unsigned int
orc_avx_get_cpu_flags (void)
{
  unsigned int flags = 0;
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  orc_uint32 eax, ebx, ecx, edx;

  orc_x86_cpu_detect (NULL, NULL);
  if (!orc_x86_cpu_is_xsave_enabled ())
    goto done;

  // Checks if XMM and YMM state are enabled in XCR0.
  // See 14.3 DETECTION OF INTEL® AVX INSTRUCTIONS on the
  // Intel® 64 and IA-32 Architectures Software Developer’s Manual
  const orc_uint32 xcr0 = orc_x86_cpu_get_xcr0 ();
  if (!((xcr0 & XCR0_SUPPORT_AVX) == XCR0_SUPPORT_AVX))
    goto done;

  orc_x86_cpu_get_cpuid (0x00000001, &eax, &ebx, &ecx, &edx);
  const orc_bool avx_instructions_supported = (ecx & (1 << 28)) != 0;

  orc_x86_cpu_get_cpuid (0x00000007, &eax, &ebx, &ecx, &edx);
  const orc_bool avx2_instructions_supported = (ebx & (1 << 5)) != 0;

  if (avx_instructions_supported) {
    flags |= ORC_TARGET_AVX_AVX;
  }

  if (avx_instructions_supported && avx2_instructions_supported) {
    flags |= ORC_TARGET_AVX_AVX2;
  }

  /* clear the flags based on the compiler flags */
  if (orc_compiler_flag_check ("-avx")) {
    flags &= ~ORC_TARGET_AVX_AVX;
  }
  if (orc_compiler_flag_check ("-avx2")) {
    flags &= ~ORC_TARGET_AVX_AVX2;
  }

done:
#endif
  return flags;
}
