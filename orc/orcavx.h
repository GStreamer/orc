#ifndef _ORC_AVX_H_
#define _ORC_AVX_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>

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

#define ORC_AVX_SSE_SHUF(a, b, c, d) \
  ((((a)&3) << 6) | (((b)&3) << 4) | (((c)&3) << 2) | (((d)&3) << 0))

#define ORC_AVX_ZERO_LANE 0x8
#define ORC_AVX_PERMUTE(upper_lane, lower_lane) ((upper_lane & 0xF) << 4) \
    | (lower_lane & 0xF)

#define ORC_AVX_PERMUTE_QUAD(upper_highquad, upper_lowquad, highquad, lowquad) \
    ORC_AVX_SSE_SHUF(upper_highquad, upper_lowquad, highquad, lowquad)

#define ORC_AVX_REG_AMOUNT X86_YMM15 - X86_YMM0 + 1
#define ORC_AVX_REG_SIZE 32

ORC_API const char *orc_x86_get_regname_avx (int i, OrcX86OpcodePrefix prefix);
ORC_API void orc_x86_emit_mov_memoffset_avx (OrcCompiler *compiler, int size,
    int offset, int reg1, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_memindex_avx (OrcCompiler *compiler, int size,
    int offset, int reg1, int regindex, int shift, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_avx_memoffset (OrcCompiler *compiler, int size,
    int reg1, int offset, int reg2, int aligned, int uncached);

ORC_API void orc_avx_load_constant (OrcCompiler *compiler, int reg, int size,
    orc_uint64 value);

ORC_API void orc_avx_emit_broadcast (OrcCompiler *compiler, int s1, int d,
    int size);

ORC_API void orc_avx_set_mxcsr (OrcCompiler *compiler);
ORC_API void orc_avx_restore_mxcsr (OrcCompiler *compiler);

#define orc_avx_sse_emit_punpcklbw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpcklbw, 16, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpcklbw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpcklbw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_punpcklwd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpcklwd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpcklwd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpcklwd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_punpckldq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckldq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpckldq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckldq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_packsswb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packsswb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_packsswb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packsswb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpgtb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpgtb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpgtw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpgtw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpgtd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpgtd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_packuswb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packuswb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_packuswb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packuswb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_punpckhbw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhbw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpckhbw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhbw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_punpckhwd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhwd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpckhwd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhwd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_punpckhdq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhdq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpckhdq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhdq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_packssdw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packssdw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_packssdw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packssdw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_punpcklqdq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpcklqdq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_punpcklqdq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpcklqdq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_punpckhqdq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_punpckhqdq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_movdqa(p,a,b) orc_vex_emit_cpuinsn_size(p, ORC_X86_movdqa, 32, a, 0, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqa(p,a,b) orc_vex_emit_cpuinsn_size(p, ORC_X86_movdqa, 32, a, 0, b, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpeqb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpeqb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpeqw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpeqw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpeqd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqd, 16, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpeqd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmullw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmullw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmullw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmullw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubusb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubusb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubusb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubusb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubusw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubusw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubusw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubusw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pminub(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminub, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pminub(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminub, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pand(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pand, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pand(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pand, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddusb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddusb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddusb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddusb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddusw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddusw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddusw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddusw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmaxub(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxub, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmaxub(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxub, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pandn(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pandn, 16, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pandn(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pandn, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pavgb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pavgb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pavgb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pavgb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pavgw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pavgw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pavgw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pavgw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmulhuw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmulhuw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmulhuw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmulhuw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmulhw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmulhw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmulhw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmulhw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubsb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubsb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubsw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubsw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pminsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminsw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pminsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminsw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_por(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_por, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_por(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_por, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddsb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddsb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddsw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddsw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmaxsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxsw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmaxsw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxsw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pxor(p, s1, s2, d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pxor, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pxor(p, s1, s2, d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pxor, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmuludq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmuludq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmuludq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmuludq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psadbw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psadbw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psadbw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psadbw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psubq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psubq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_psubq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_paddd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddd, 16, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_paddd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_paddd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pshufb(p,mask,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pshufb, 32, mask, s1, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pshufb(p,mask,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pshufb, 32, mask, s1, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pabsb(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pabsb, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pabsb(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pabsb, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pabsw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pabsw, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pabsw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pabsw, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pabsd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pabsd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pabsd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pabsd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovsxbw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxbw, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovsxbw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxbw, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovsxbd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxbd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovsxbd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxbd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovsxbq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxbq, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovsxbq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxbq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovsxwd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxwd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovsxwd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxwd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovsxwq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxwq, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovsxwq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxwq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovsxdq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxdq, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovsxdq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovsxdq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmuldq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmuldq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmuldq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmuldq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpeqq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpeqq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpeqq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_packusdw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packusdw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_packusdw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_packusdw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovzxbw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxbw, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovzxbw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxbw, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pmovzxbd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxbd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pmovzxbq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxbq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovzxwd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxwd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovzxwd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxwd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pmovzxwq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxwq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmovzxdq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxdq, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmovzxdq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmovzxdq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmulld(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmulld, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmulld(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmulld, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pminsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminsb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pminsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminsb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pminsd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminsd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pminsd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminsd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pminuw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminuw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pminuw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminuw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pminud(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminud, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pminud(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pminud, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmaxsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxsb, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmaxsb(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxsb, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmaxsd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxsd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmaxsd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxsd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmaxuw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxuw, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmaxuw(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxuw, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pmaxud(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxud, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pmaxud(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pmaxud, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pcmpgtq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtq, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pcmpgtq(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pcmpgtq, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_addps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_addps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_addps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_addps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_subps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_subps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_subps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_subps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_mulps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_mulps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_mulps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_mulps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_divps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_divps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_divps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_divps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_sqrtps(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_sqrtps, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_sqrtps(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_sqrtps, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_andps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_andps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_andps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_andps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_orps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_orps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_orps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_orps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_addpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_addpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_addpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_addpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_subpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_subpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_subpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_subpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_mulpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_mulpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_mulpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_mulpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_divpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_divpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_divpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_divpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_sqrtpd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_sqrtpd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_sqrtpd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_sqrtpd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cmpeqps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpeqps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cmpeqps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpeqps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cmpeqpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpeqpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cmpeqpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpeqpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cmpltps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpltps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cmpltps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpltps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cmpltpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpltpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cmpltpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpltpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cmpleps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpleps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cmpleps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmpleps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cmplepd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmplepd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cmplepd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cmplepd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cvttps2dq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvttps2dq, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cvttps2dq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvttps2dq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cvttpd2dq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvttpd2dq, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cvttpd2dq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvttpd2dq, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cvtdq2ps(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtdq2ps, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cvtdq2ps(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtdq2ps, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cvtdq2pd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtdq2pd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cvtdq2pd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtdq2pd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cvtps2pd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtps2pd, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cvtps2pd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtps2pd, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_cvtpd2ps(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtpd2ps, 32, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_cvtpd2ps(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_cvtpd2ps, 32, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_minps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_minps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_minps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_minps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_minpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_minpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_minpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_minpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_maxps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_maxps, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_maxps(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_maxps, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_maxpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_maxpd, 32, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_maxpd(p,s1,s2,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_maxpd, 32, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psraw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psraw_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psraw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psraw_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psrlw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrlw_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psrlw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrlw_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psllw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psllw_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psllw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psllw_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_psrad_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrad_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psrad_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrad_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_psrld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrld_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psrld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrld_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pslld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pslld_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pslld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pslld_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psrlq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrlq_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psrlq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psrlq_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_psllq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psllq_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_psllq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_psllq_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pslldq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pslldq_imm, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pslldq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pslldq_imm, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pshufd(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pshufd, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pshufd(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pshufd, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pshuflw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pshuflw, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pshuflw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pshuflw, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_pshufhw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pshufhw, imm, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_pshufhw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pshufhw, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_pinsrb_memoffset(p,imm,offset,s1,s2,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_pinsrb, 4, imm, offset, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_pinsrw_memoffset(p,imm,offset,s1,s2,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_pinsrw, 4, imm, offset, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movd_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_movd_load, 4, 0, offset, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_pinsrd_memoffset(p,imm,offset,s1,s2,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_pinsrd, 4, imm, offset, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movq_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_movq_sse_load, 4, 0, offset, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movdqa_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_movdqa_load, 4, 0, offset, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqa_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_movdqa_load, 4, 0, offset, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_movdqu_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_movdqu_load, 4, 0, offset, s1, 0, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqu_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_X86_movdqu_load, 4, 0, offset, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_pextrb_memoffset(p,imm,offset,a,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_pextrb, 8, imm, offset, a, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_pextrw_memoffset(p,imm,offset,a,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_pextrw, 16, imm, offset, a, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movd_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movd_store, 4, 0, a, offset, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movq_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movq_sse_store, 16, 0, a, offset, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movdqa_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movdqa_store, 16, 0, a, offset, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqa_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movdqa_store, 32, 0, a, offset, b, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_movdqu_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movdqu_store, 16, 0, a, offset, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqu_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movdqu_store, 32, 0, a, offset, b, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_movntdq_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movntdq_store, 16, 0, a, offset, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movntdq_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_X86_movntdq_store, 32, 0, a, offset, b, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_movd_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_X86_movd_load, 4, 0, offset, a, a_index, shift, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movq_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_X86_movq_sse_load, 4, 0, offset, a, a_index, shift, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movdqa_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_X86_movdqa_load, 4, 0, offset, a, a_index, shift, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqa_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_X86_movdqa_load, 4, 0, offset, a, a_index, shift, b, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_sse_emit_movdqu_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_X86_movdqu_load, 4, 0, offset, a, a_index, shift, b, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_movdqu_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_X86_movdqu_load, 4, 0, offset, a, a_index, shift, b, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_pinsrw_register(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pinsrw, imm, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_sse_emit_movd_load_register(p,a,b) orc_vex_emit_cpuinsn_size(p, ORC_X86_movd_load, 4, a, 0, b, ORC_X86_AVX_VEX128_PREFIX)


#define orc_avx_emit_permute2f128(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_permute2f128_avx, imm, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_permute2i128(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_permute2i128_avx, imm, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_emit_pbroadcastb(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pbroadcastb_avx, 1, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pbroadcastw(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pbroadcastw_avx, 2, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pbroadcastd(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pbroadcastd_avx, 4, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pbroadcastq(p,s1,d) orc_vex_emit_cpuinsn_size(p, ORC_X86_pbroadcastq_avx, 8, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_shufps_imm(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_shufps_imm, imm, s1, s2, d, ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_insertf128_si256(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_insertf128_avx, imm, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_extractf128_si256(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_extractf128_avx, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_emit_permute4x64_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_permute4x64_imm_avx, imm, s1, 0, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_blendpd(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_blendpd_avx, imm, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)
#define orc_avx_emit_pblendd(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_X86_pblendd_avx, imm, s1, s2, d, ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_blendvpd(p, s1, s2, mask, d) \
    orc_vex_emit_blend_size (p, ORC_X86_blendvpd_avx, 1, s1, s2, mask, d, \
        ORC_X86_AVX_VEX128_PREFIX)
#define orc_avx_emit_blendvpd(p, s1, s2, mask, d) \
    orc_vex_emit_blend_size (p, ORC_X86_blendvpd_avx, 1, s1, s2, mask, d, \
        ORC_X86_AVX_VEX256_PREFIX)

#define orc_avx_sse_emit_pinsrd_register(p, imm, s1, s2, d) \
  orc_vex_emit_cpuinsn_imm (p, ORC_X86_pinsrd, imm, s1, s2, d, \
      ORC_X86_AVX_VEX128_PREFIX)


#endif

ORC_END_DECLS

#endif
