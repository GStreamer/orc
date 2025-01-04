
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>

#include <orc/orcavx.h>
#include <orc/orcdebug.h>
#include <orc/orcprogram.h>
#include <orc/orcsse.h>
#include <orc/orcx86insn.h>


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
