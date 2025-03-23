#ifndef _ORC_AVX_H_
#define _ORC_AVX_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>
#include <orc/orcsse.h> /* To know the X86_XMM0 register */

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum
{
  X86_YMM0 = ORC_VEC_REG_BASE + 32,
  X86_YMM1,
  X86_YMM2,
  X86_YMM3,
  X86_YMM4,
  X86_YMM5,
  X86_YMM6,
  X86_YMM7,
  X86_YMM8,
  X86_YMM9,
  X86_YMM10,
  X86_YMM11,
  X86_YMM12,
  X86_YMM13,
  X86_YMM14,
  X86_YMM15
} OrcAVXRegister;

/* The X86 target uses the size argument to differentiate between
 * halves, like rax, eax, ax, al, etc ... as they use the same register id.
 * In this case, AVX registers have different id than their SSE halves, so the
 * size is irrelevant. Use the proper id.
 */
#define ORC_AVX_SSE_REG(i) (X86_XMM0 + (i - X86_YMM0))

#define ORC_AVX_SSE_SHUF(a, b, c, d) \
  ((((a)&3) << 6) | (((b)&3) << 4) | (((c)&3) << 2) | (((d)&3) << 0))

#define ORC_AVX_ZERO_LANE 0x8
#define ORC_AVX_PERMUTE(upper_lane, lower_lane) ((upper_lane & 0xF) << 4) \
    | (lower_lane & 0xF)

#define ORC_AVX_PERMUTE_QUAD(upper_highquad, upper_lowquad, highquad, lowquad) \
    ORC_AVX_SSE_SHUF(upper_highquad, upper_lowquad, highquad, lowquad)

#define ORC_AVX_REG_AMOUNT X86_YMM15 - X86_YMM0 + 1
#define ORC_AVX_REG_SIZE 32

ORC_API const char *orc_x86_get_regname_avx (int i);
ORC_API void orc_x86_emit_mov_memoffset_avx (OrcCompiler *compiler, int size,
    int offset, int reg1, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_memindex_avx (OrcCompiler *compiler, int size,
    int offset, int reg1, int regindex, int shift, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_avx_memoffset (OrcCompiler *compiler, int size,
    int reg1, int offset, int reg2, int aligned, int uncached);

ORC_API void orc_avx_emit_broadcast (OrcCompiler *compiler, int s1, int d,
    int size);

ORC_API void orc_avx_set_mxcsr (OrcCompiler *compiler);
ORC_API void orc_avx_restore_mxcsr (OrcCompiler *compiler);

ORC_API unsigned int orc_avx_get_cpu_flags (void);

#endif

ORC_END_DECLS

#endif
