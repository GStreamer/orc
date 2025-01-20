
#ifndef _ORC_SSE_H_
#define _ORC_SSE_H_

#include <orc/orc.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  X86_XMM0 = ORC_VEC_REG_BASE + 16,
  X86_XMM1,
  X86_XMM2,
  X86_XMM3,
  X86_XMM4,
  X86_XMM5,
  X86_XMM6,
  X86_XMM7,
  X86_XMM8,
  X86_XMM9,
  X86_XMM10,
  X86_XMM11,
  X86_XMM12,
  X86_XMM13,
  X86_XMM14,
  X86_XMM15
} OrcSSERegister;

#define ORC_SSE_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

ORC_API const char * orc_x86_get_regname_sse(int i);
ORC_API void orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_memindex_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);

ORC_API void orc_sse_set_mxcsr (OrcCompiler *compiler);
ORC_API void orc_sse_restore_mxcsr (OrcCompiler *compiler);

ORC_API void orc_sse_load_constant (OrcCompiler *compiler, int reg, int size,
    orc_uint64 value);

#define orc_sse_emit_punpcklbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpcklbw, 16, a, b)
#define orc_sse_emit_punpcklwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpcklwd, 16, a, b)
#define orc_sse_emit_punpckldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckldq, 16, a, b)
#define orc_sse_emit_packsswb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packsswb, 16, a, b)
#define orc_sse_emit_pcmpgtb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtb, 16, a, b)
#define orc_sse_emit_pcmpgtw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtw, 16, a, b)
#define orc_sse_emit_pcmpgtd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtd, 16, a, b)
#define orc_sse_emit_packuswb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packuswb, 16, a, b)
#define orc_sse_emit_punpckhbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhbw, 16, a, b)
#define orc_sse_emit_punpckhwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhwd, 16, a, b)
#define orc_sse_emit_punpckhdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhdq, 16, a, b)
#define orc_sse_emit_packssdw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packssdw, 16, a, b)
#define orc_sse_emit_punpcklqdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpcklqdq, 16, a, b)
#define orc_sse_emit_punpckhqdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhqdq, 16, a, b)
#define orc_sse_emit_movdqa(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movdqa, 16, a, b)
#define orc_sse_emit_psraw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psraw, 16, a, b)
#define orc_sse_emit_psrlw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrlw, 16, a, b)
#define orc_sse_emit_psllw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psllw, 16, a, b)
#define orc_sse_emit_psrad(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrad, 16, a, b)
#define orc_sse_emit_psrld(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrld, 16, a, b)
#define orc_sse_emit_pslld(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pslld, 16, a, b)
#define orc_sse_emit_psrlq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrlq, 16, a, b)
#define orc_sse_emit_psllq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psllq, 16, a, b)
#define orc_sse_emit_psrldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrldq, 16, a, b)
#define orc_sse_emit_pslldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pslldq, 16, a, b)
#define orc_sse_emit_psrlq_reg(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrlq_reg, 16, a, b)
#define orc_sse_emit_pcmpeqb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqb, 16, a, b)
#define orc_sse_emit_pcmpeqw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqw, 16, a, b)
#define orc_sse_emit_pcmpeqd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqd, 16, a, b)
#define orc_sse_emit_paddq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddq, 16, a, b)
#define orc_sse_emit_pmullw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmullw, 16, a, b)
#define orc_sse_emit_psubusb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubusb, 16, a, b)
#define orc_sse_emit_psubusw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubusw, 16, a, b)
#define orc_sse_emit_pminub(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminub, 16, a, b)
#define orc_sse_emit_pand(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pand, 16, a, b)
#define orc_sse_emit_paddusb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddusb, 16, a, b)
#define orc_sse_emit_paddusw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddusw, 16, a, b)
#define orc_sse_emit_pmaxub(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxub, 16, a, b)
#define orc_sse_emit_pandn(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pandn, 16, a, b)
#define orc_sse_emit_pavgb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pavgb, 16, a, b)
#define orc_sse_emit_pavgw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pavgw, 16, a, b)
#define orc_sse_emit_pmulhuw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulhuw, 16, a, b)
#define orc_sse_emit_pmulhw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulhw, 16, a, b)
#define orc_sse_emit_psubsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubsb, 16, a, b)
#define orc_sse_emit_psubsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubsw, 16, a, b)
#define orc_sse_emit_pminsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminsw, 16, a, b)
#define orc_sse_emit_por(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_por, 16, a, b)
#define orc_sse_emit_paddsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddsb, 16, a, b)
#define orc_sse_emit_paddsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddsw, 16, a, b)
#define orc_sse_emit_pmaxsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxsw, 16, a, b)
#define orc_sse_emit_pxor(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pxor, 16, a, b)
#define orc_sse_emit_pmuludq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmuludq, 16, a, b)
#define orc_sse_emit_pmaddwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaddwd, 16, a, b)
#define orc_sse_emit_psadbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psadbw, 16, a, b)
#define orc_sse_emit_psubb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubb, 16, a, b)
#define orc_sse_emit_psubw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubw, 16, a, b)
#define orc_sse_emit_psubd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubd, 16, a, b)
#define orc_sse_emit_psubq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubq, 16, a, b)
#define orc_sse_emit_paddb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddb, 16, a, b)
#define orc_sse_emit_paddw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddw, 16, a, b)
#define orc_sse_emit_paddd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddd, 16, a, b)
#define orc_sse_emit_pshufb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pshufb, 16, a, b)
#define orc_sse_emit_phaddw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phaddw, 16, a, b)
#define orc_sse_emit_phaddd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phaddd, 16, a, b)
#define orc_sse_emit_phaddsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phaddsw, 16, a, b)
#define orc_sse_emit_pmaddubsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaddubsw, 16, a, b)
#define orc_sse_emit_phsubw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phsubw, 16, a, b)
#define orc_sse_emit_phsubd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phsubd, 16, a, b)
#define orc_sse_emit_phsubsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phsubsw, 16, a, b)
#define orc_sse_emit_psignb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psignb, 16, a, b)
#define orc_sse_emit_psignw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psignw, 16, a, b)
#define orc_sse_emit_psignd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psignd, 16, a, b)
#define orc_sse_emit_pmulhrsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulhrsw, 16, a, b)
#define orc_sse_emit_pabsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pabsb, 16, a, b)
#define orc_sse_emit_pabsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pabsw, 16, a, b)
#define orc_sse_emit_pabsd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pabsd, 16, a, b)
#define orc_sse_emit_pmovsxbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovsxbw, 16, a, b)
#define orc_sse_emit_pmovsxbd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovsxbd, 16, a, b)
#define orc_sse_emit_pmovsxbq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovsxbq, 16, a, b)
#define orc_sse_emit_pmovsxwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovsxwd, 16, a, b)
#define orc_sse_emit_pmovsxwq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovsxwq, 16, a, b)
#define orc_sse_emit_pmovsxdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovsxdq, 16, a, b)
#define orc_sse_emit_pmuldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmuldq, 16, a, b)
#define orc_sse_emit_pcmpeqq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqq, 16, a, b)
#define orc_sse_emit_packusdw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packusdw, 16, a, b)
#define orc_sse_emit_pmovzxbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovzxbw, 16, a, b)
#define orc_sse_emit_pmovzxbd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovzxbd, 16, a, b)
#define orc_sse_emit_pmovzxbq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovzxbq, 16, a, b)
#define orc_sse_emit_pmovzxwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovzxwd, 16, a, b)
#define orc_sse_emit_pmovzxwq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovzxwq, 16, a, b)
#define orc_sse_emit_pmovzxdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmovzxdq, 16, a, b)
#define orc_sse_emit_pmulld(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulld, 16, a, b)
#define orc_sse_emit_phminposuw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phminposuw, 16, a, b)
#define orc_sse_emit_pminsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminsb, 16, a, b)
#define orc_sse_emit_pminsd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminsd, 16, a, b)
#define orc_sse_emit_pminuw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminuw, 16, a, b)
#define orc_sse_emit_pminud(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminud, 16, a, b)
#define orc_sse_emit_pmaxsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxsb, 16, a, b)
#define orc_sse_emit_pmaxsd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxsd, 16, a, b)
#define orc_sse_emit_pmaxuw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxuw, 16, a, b)
#define orc_sse_emit_pmaxud(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxud, 16, a, b)
#define orc_sse_emit_pcmpgtq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtq, 16, a, b)

/* Single Precision Floating-Point Instructions */
#define orc_sse_emit_addps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_addps, 16, a, b)
#define orc_sse_emit_subps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_subps, 16, a, b)
#define orc_sse_emit_mulps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_mulps, 16, a, b)
#define orc_sse_emit_divps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_divps, 16, a, b)
#define orc_sse_emit_sqrtps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_sqrtps, 16, a, b)
#define orc_sse_emit_andps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_andps, 16, a, b)
#define orc_sse_emit_orps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_orps, 16, a, b)

/* Double Precision Floating-Point Instructions */
#define orc_sse_emit_addpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_addpd, 16, a, b)
#define orc_sse_emit_subpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_subpd, 16, a, b)
#define orc_sse_emit_mulpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_mulpd, 16, a, b)
#define orc_sse_emit_divpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_divpd, 16, a, b)
#define orc_sse_emit_sqrtpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_sqrtpd, 16, a, b)
#define orc_sse_emit_cmpeqps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpeqps, 16, a, b)
#define orc_sse_emit_cmpeqpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpeqpd, 16, a, b)
#define orc_sse_emit_cmpltps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpltps, 16, a, b)
#define orc_sse_emit_cmpltpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpltpd, 16, a, b)
#define orc_sse_emit_cmpleps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpleps, 16, a, b)
#define orc_sse_emit_cmplepd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmplepd, 16, a, b)
#define orc_sse_emit_cvttps2dq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvttps2dq, 16, a, b)

#define orc_sse_emit_cvttpd2dq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvttpd2dq, 16, a, b)
#define orc_sse_emit_cvtdq2ps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtdq2ps, 16, a, b)
#define orc_sse_emit_cvtdq2pd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtdq2pd, 16, a, b)
#define orc_sse_emit_cvtps2pd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtps2pd, 16, a, b)
#define orc_sse_emit_cvtpd2ps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtpd2ps, 16, a, b)
#define orc_sse_emit_minps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_minps, 16, a, b)
#define orc_sse_emit_minpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_minpd, 16, a, b)
#define orc_sse_emit_maxps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_maxps, 16, a, b)
#define orc_sse_emit_maxpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_maxpd, 16, a, b)
#define orc_sse_emit_psraw_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psraw_imm, imm, 0, b)
#define orc_sse_emit_psrlw_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrlw_imm, imm, 0, b)
#define orc_sse_emit_psllw_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psllw_imm, imm, 0, b)
#define orc_sse_emit_psrad_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrad_imm, imm, 0, b)
#define orc_sse_emit_psrld_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrld_imm, imm, 0, b)
#define orc_sse_emit_pslld_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pslld_imm, imm, 0, b)
#define orc_sse_emit_psrlq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrlq_imm, imm, 0, b)
#define orc_sse_emit_psllq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psllq_imm, imm, 0, b)
#define orc_sse_emit_psrldq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrldq_imm, imm, 0, b)
#define orc_sse_emit_pslldq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pslldq_imm, imm, 0, b)
#define orc_sse_emit_pshufd(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshufd, imm, a, b)
#define orc_sse_emit_pshuflw(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshuflw, imm, a, b)
#define orc_sse_emit_pshufhw(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshufhw, imm, a, b)
#define orc_sse_emit_palignr(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psalignr, imm, a, b)
#define orc_sse_emit_movdqu(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movdqu_load, 16, a, b)

#define orc_sse_emit_pinsrb_memoffset(p,imm,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_pinsrb, 4, imm, offset, a, b)
#define orc_sse_emit_pinsrw_memoffset(p,imm,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_pinsrw, 4, imm, offset, a, b)
#define orc_sse_emit_movd_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movd_load, 4, 0, offset, a, b)
#define orc_sse_emit_movq_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movq_sse_load, 4, 0, offset, a, b)
#define orc_sse_emit_movdqa_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movdqa_load, 4, 0, offset, a, b)
#define orc_sse_emit_movdqu_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movdqu_load, 4, 0, offset, a, b)
#define orc_sse_emit_movhps_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movhps_load, 4, 0, offset, a, b)
#define orc_sse_emit_pextrb_memoffset(p,imm,offset,a,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_pextrb, 8, imm, offset, a,b)
#define orc_sse_emit_pextrw_memoffset(p,imm,offset,a,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_pextrw, 16, imm, offset, a, b)
#define orc_sse_emit_movd_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movd_store, 4, 0, a, offset, b)
#define orc_sse_emit_movq_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movq_sse_store, 16, 0, a, offset, b)
#define orc_sse_emit_movdqa_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movdqa_store, 16, 0, a, offset, b)
#define orc_sse_emit_movdqu_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movdqu_store, 16, 0, a, offset, b)
#define orc_sse_emit_movntdq_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movntdq_store, 16, 0, a, offset, b)
#define orc_sse_emit_pinsrw_memindex(p,imm,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_pinsrw, 4, imm, offset, a, a_index, shift, b)
#define orc_sse_emit_movd_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movd_load, 4, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movq_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movq_sse_load, 4, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movdqa_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movdqa_load, 4, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movdqu_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movdqu_load, 4, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movhps_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movhps_load, 4, 0, offset, a, a_index, shift, b)

#define orc_sse_emit_pextrw_memindex(p,imm,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_pextrw, imm, a, offset, b, b_index, shift)
#define orc_sse_emit_movd_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movd_store, 4, a, offset, b, b_index, shift)
#define orc_sse_emit_movq_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movq_sse_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_movdqa_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movdqa_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_movdqu_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movdqu_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_movntdq_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movntdq_store, 0, a, offset, b, b_index, shift)

#define orc_sse_emit_pinsrw_register(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pinsrw, imm, a, b)
#define orc_sse_emit_movd_load_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movd_load, 4, a, b)
#define orc_sse_emit_movq_load_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movq_sse_load, 8, a, b)
#define orc_sse_emit_pextrw_register(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pextrw, imm, a, b)
#define orc_sse_emit_movd_store_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movd_store, 4, a, b)
#define orc_sse_emit_movq_store_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movq_sse_store, 8, a, b)

#define orc_sse_emit_blendvpd(p, s1, d) orc_x86_emit_cpuinsn_size (p, ORC_X86_blendvpd_sse, 1, s1, d)

#endif

ORC_API unsigned int orc_sse_get_cpu_flags (void);

ORC_END_DECLS

#endif

