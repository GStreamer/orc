#ifndef ORC_AVX_INSN_H_
#define ORC_AVX_INSN_H_

#include <orc/orcsseinsn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

/* This extends OrcSSEInsnOperandFlag */
typedef enum _OrcAVXInsnOperandFlag {
  ORC_AVX_INSN_OPERAND_OP1_YMM = (1 << (ORC_SSE_INSN_OPERAND_FLAG_LAST + 1)), 
  ORC_AVX_INSN_OPERAND_OP2_YMM = (1 << (ORC_SSE_INSN_OPERAND_FLAG_LAST + 2)),
  ORC_AVX_INSN_OPERAND_OP3_YMM = (1 << (ORC_SSE_INSN_OPERAND_FLAG_LAST + 3)),
  ORC_AVX_INSN_OPERAND_OP4_YMM = (1 << (ORC_SSE_INSN_OPERAND_FLAG_LAST + 4)),
  ORC_AVX_INSN_OPERAND_OP3_XMM = (1 << (ORC_SSE_INSN_OPERAND_FLAG_LAST + 5)),
  ORC_AVX_INSN_OPERAND_OP4_XMM = (1 << (ORC_SSE_INSN_OPERAND_FLAG_LAST + 6)),
} OrcAVXInsnOperandFlag;

/* We need to differentiate the SSE (VEX.128) from the AVX (VEX.256)
 * instructions because the type of the opcodes are not independent.
 * In SSE compatibility mode both operands must be XMM, but on AVX
 * both should be YMM. We can assume that but there are instructions
 * like cvtdq2ps that work with a YMM and XMM registers. The other
 * criteria to duplicate instructions is because some are part of
 * AVX (the SSE ones) and others part of AVX2 (the AVX ones).
 */
typedef enum _OrcAVXInsnOpcodeIdx {
  ORC_AVX_vperm2f128,
  ORC_AVX_blendpd,
  ORC_AVX_vextractf128,
  ORC_AVX_vperm2i128,
  ORC_AVX_vblendvpd,
  ORC_AVX_SSE_vblendvpd,
  ORC_AVX_sqrtps,
  ORC_AVX_SSE_sqrtps,
  ORC_AVX_andps,
  ORC_AVX_SSE_andps,
  /* 10 */
  ORC_AVX_orps,
  ORC_AVX_SSE_orps,
  ORC_AVX_addps,
  ORC_AVX_SSE_addps,
  ORC_AVX_mulps,
  ORC_AVX_SSE_mulps,
  ORC_AVX_subps,
  ORC_AVX_SSE_subps,
  ORC_AVX_minps,
  ORC_AVX_SSE_minps,
  /* 20 */
  ORC_AVX_divps,
  ORC_AVX_SSE_divps,
  ORC_AVX_maxps,
  ORC_AVX_SSE_maxps,
  ORC_AVX_ldmxcsr,
  ORC_AVX_stmxcsr,
  ORC_AVX_sqrtpd,
  ORC_AVX_SSE_sqrtpd,
  ORC_AVX_addpd,
  ORC_AVX_SSE_addpd,
  /* 30 */
  ORC_AVX_mulpd,
  ORC_AVX_SSE_mulpd,
  ORC_AVX_cvtps2pd,
  ORC_AVX_SSE_cvtps2pd,
  ORC_AVX_cvtpd2ps, /* An example of [ymm, xmm], xmm */
  ORC_AVX_SSE_cvtpd2ps,
  ORC_AVX_cvtdq2ps,
  ORC_AVX_SSE_cvtdq2ps,
  ORC_AVX_cvttps2dq,
  ORC_AVX_SSE_cvttps2dq,
  /* 40 */
  ORC_AVX_subpd,
  ORC_AVX_SSE_subpd,
  ORC_AVX_minpd,
  ORC_AVX_SSE_minpd,
  ORC_AVX_divpd,
  ORC_AVX_SSE_divpd,
  ORC_AVX_maxpd,
  ORC_AVX_SSE_maxpd,
  ORC_AVX_cmpeqpd,
  ORC_AVX_SSE_cmpeqpd,
  /* 50 */
  ORC_AVX_cmpltpd,
  ORC_AVX_SSE_cmpltpd,
  ORC_AVX_cmplepd,
  ORC_AVX_SSE_cmplepd,
  ORC_AVX_cmpeqps,
  ORC_AVX_SSE_cmpeqps,
  ORC_AVX_cmpltps,
  ORC_AVX_SSE_cmpltps,
  ORC_AVX_cmpleps,
  ORC_AVX_SSE_cmpleps,
  /* 60 */
  ORC_AVX_cvtdq2pd,
  ORC_AVX_SSE_cvtdq2pd,
  ORC_AVX_cvttpd2dq,
  ORC_AVX_SSE_cvttpd2dq,
  ORC_AVX_SSE_punpcklbw,
  ORC_AVX_SSE_punpcklwd,
  ORC_AVX_SSE_punpckldq,
  ORC_AVX_SSE_packsswb,
  ORC_AVX_SSE_pcmpgtb, 
  ORC_AVX_SSE_pcmpgtw,
  /* 70 */
  ORC_AVX_SSE_pcmpgtd,
  ORC_AVX_SSE_packuswb,
  ORC_AVX_SSE_punpckhbw,
  ORC_AVX_SSE_punpckhwd,
  ORC_AVX_SSE_punpckhdq,
  ORC_AVX_SSE_packssdw,
  ORC_AVX_SSE_movd_load,
  ORC_AVX_SSE_movq_rm_r,
  ORC_AVX_SSE_psrlw_imm,
  ORC_AVX_SSE_psraw_imm,
  /* 80 */
  ORC_AVX_SSE_psllw_imm,
  ORC_AVX_SSE_psrld_imm,
  ORC_AVX_SSE_psrad_imm,
  ORC_AVX_SSE_pslld_imm,
  ORC_AVX_SSE_psrlq_imm,
  ORC_AVX_SSE_psllq_imm,
  ORC_AVX_SSE_pcmpeqb,
  ORC_AVX_SSE_pcmpeqw,
  ORC_AVX_SSE_pcmpeqd,
  ORC_AVX_vzeroupper,
  /* 90 */
  ORC_AVX_SSE_movd_store,
  ORC_AVX_SSE_movq_r_rm,
  ORC_AVX_SSE_movq_load,
  ORC_AVX_SSE_pinsrw,
  ORC_AVX_SSE_pextrw,
  ORC_AVX_SSE_psrlw,
  ORC_AVX_SSE_psrld,
  ORC_AVX_SSE_psrlq,
  ORC_AVX_SSE_paddq,
  ORC_AVX_SSE_pmullw,
  /* 100 */
  ORC_AVX_SSE_movq_store,
  ORC_AVX_SSE_psubusb,
  ORC_AVX_SSE_psubusw,
  ORC_AVX_SSE_pminub,
  ORC_AVX_SSE_pand,
  ORC_AVX_SSE_paddusb,
  ORC_AVX_SSE_paddusw,
  ORC_AVX_SSE_pmaxub,
  ORC_AVX_SSE_pandn,
  ORC_AVX_SSE_pavgb,
  /* 110 */
  ORC_AVX_SSE_psraw,
  ORC_AVX_SSE_psrad,
  ORC_AVX_SSE_pavgw,
  ORC_AVX_SSE_pmulhuw,
  ORC_AVX_SSE_pmulhw,
  ORC_AVX_SSE_psubsb,
  ORC_AVX_SSE_psubsw,
  ORC_AVX_SSE_pminsw,
  ORC_AVX_SSE_por,
  ORC_AVX_SSE_paddsb,
  /* 120 */
  ORC_AVX_SSE_paddsw,
  ORC_AVX_SSE_pmaxsw,
  ORC_AVX_SSE_pxor,
  ORC_AVX_SSE_psllw,
  ORC_AVX_SSE_pslld,
  ORC_AVX_SSE_psllq,
  ORC_AVX_SSE_pmuludq,
  ORC_AVX_SSE_psadbw,
  ORC_AVX_SSE_psubb,
  ORC_AVX_SSE_psubw,
  /* 130 */
  ORC_AVX_SSE_psubd,
  ORC_AVX_SSE_psubq,
  ORC_AVX_SSE_paddb,
  ORC_AVX_SSE_paddw,
  ORC_AVX_SSE_paddd,
  ORC_AVX_SSE_punpcklqdq,
  ORC_AVX_movdqa_load,
  ORC_AVX_SSE_movdqa_load,
  ORC_AVX_SSE_pshufd,
  ORC_AVX_movdqa_store,
  /* 140 */
  ORC_AVX_SSE_movdqa_store,
  ORC_AVX_SSE_pslldq_imm,
  ORC_AVX_movntdq_store,
  ORC_AVX_SSE_movntdq_store,
  ORC_AVX_SSE_pshuflw,
  ORC_AVX_movdqu_load,
  ORC_AVX_SSE_movdqu_load,
  ORC_AVX_movdqu_store,
  ORC_AVX_SSE_movdqu_store,
  ORC_AVX_SSE_pshufhw,
  /* 150 */
  ORC_AVX_SSE_pshufb,
  ORC_AVX_SSE_psignb,
  ORC_AVX_SSE_psignw,
  ORC_AVX_SSE_psignd,
  ORC_AVX_SSE_pabsb,
  ORC_AVX_SSE_pabsw,
  ORC_AVX_SSE_pabsd,
  ORC_AVX_SSE_pextrb,
  ORC_AVX_SSE_pextrw_mem,
  ORC_AVX_SSE_pinsrb,
  /* 160 */
  ORC_AVX_SSE_pmovsxbw,
  ORC_AVX_SSE_pinsrd,
  ORC_AVX_SSE_pmovsxwd,
  ORC_AVX_SSE_pmovsxdq,
  ORC_AVX_SSE_pmuldq,
  ORC_AVX_SSE_pcmpeqq,
  ORC_AVX_packusdw,
  ORC_AVX_SSE_packusdw,
  ORC_AVX_SSE_pmovzxbw,
  ORC_AVX_SSE_pmovzxwd,
  /* 170 */
  ORC_AVX_SSE_pmovzxdq,
  ORC_AVX_SSE_pminsb,
  ORC_AVX_SSE_pminsd,
  ORC_AVX_SSE_pminuw,
  ORC_AVX_SSE_pminud,
  ORC_AVX_SSE_pmaxsb,
  ORC_AVX_SSE_pmaxsd,
  ORC_AVX_SSE_pmaxuw,
  ORC_AVX_SSE_pmaxud,
  ORC_AVX_SSE_pmulld,
  /* 180 */
  ORC_AVX_SSE_pcmpgtq,
  /* AVX2 only */
  ORC_AVX_vpermq,
  ORC_AVX_vpblendd,
  ORC_AVX_vpbroadcastd,
  ORC_AVX_vpbroadcastq,
  ORC_AVX_vpbroadcastb,
  ORC_AVX_vpbroadcastw,
  ORC_AVX_punpcklbw,
  ORC_AVX_punpcklwd,
  ORC_AVX_punpckldq,
  /* 190 */
  ORC_AVX_packsswb,
  ORC_AVX_pcmpgtb,
  ORC_AVX_pcmpgtw,
  ORC_AVX_pcmpgtd,
  ORC_AVX_packuswb,
  ORC_AVX_punpckhbw,
  ORC_AVX_punpckhwd,
  ORC_AVX_punpckhdq,
  ORC_AVX_packssdw,
  ORC_AVX_psrlw_imm,
  /* 200 */
  ORC_AVX_psraw_imm,
  ORC_AVX_psllw_imm,
  ORC_AVX_psrld_imm,
  ORC_AVX_psrad_imm,
  ORC_AVX_pslld_imm,
  ORC_AVX_psrlq_imm,
  ORC_AVX_psllq_imm,
  ORC_AVX_pcmpeqb,
  ORC_AVX_pcmpeqw,
  ORC_AVX_pcmpeqd,
  /* 210 */
  ORC_AVX_psrlw,
  ORC_AVX_psrld,
  ORC_AVX_psrlq,
  ORC_AVX_paddq,
  ORC_AVX_pmullw,
  ORC_AVX_psubusb,
  ORC_AVX_psubusw,
  ORC_AVX_pminub,
  ORC_AVX_pand,
  ORC_AVX_paddusb,
  /* 220 */
  ORC_AVX_paddusw,
  ORC_AVX_pmaxub,
  ORC_AVX_pandn,
  ORC_AVX_pavgb,
  ORC_AVX_psraw,
  ORC_AVX_psrad,
  ORC_AVX_pavgw,
  ORC_AVX_pmulhuw,
  ORC_AVX_pmulhw,
  ORC_AVX_psubsb,
  /* 230 */
  ORC_AVX_psubsw,
  ORC_AVX_pminsw,
  ORC_AVX_por,
  ORC_AVX_paddsb,
  ORC_AVX_paddsw,
  ORC_AVX_pmaxsw,
  ORC_AVX_pxor,
  ORC_AVX_psllw,
  ORC_AVX_pslld,
  ORC_AVX_psllq,
  /* 240 */
  ORC_AVX_pmuludq,
  ORC_AVX_pmaddwd,
  ORC_AVX_psadbw,
  ORC_AVX_psubb,
  ORC_AVX_psubw,
  ORC_AVX_psubd,
  ORC_AVX_psubq,
  ORC_AVX_paddb,
  ORC_AVX_paddw,
  ORC_AVX_paddd,
  /* 250 */
  ORC_AVX_punpcklqdq,
  ORC_AVX_punpckhqdq,
  ORC_AVX_pshufd,
  ORC_AVX_psrldq_imm,
  ORC_AVX_pslldq_imm,
  ORC_AVX_pshuflw,
  ORC_AVX_pshufhw,
  ORC_AVX_pshufb,
  ORC_AVX_psignb,
  ORC_AVX_psignw,
  /* 260 */
  ORC_AVX_psignd,
  ORC_AVX_pabsb,
  ORC_AVX_pabsw,
  ORC_AVX_pabsd,
  ORC_AVX_pmovsxbw,
  ORC_AVX_pmovsxbd,
  ORC_AVX_pmovsxbq,
  ORC_AVX_pmovsxwd,
  ORC_AVX_pmovsxwq,
  ORC_AVX_pmovsxdq,
  /* 270 */
  ORC_AVX_pmuldq,
  ORC_AVX_pcmpeqq,
  ORC_AVX_pmovzxbw,
  ORC_AVX_pmovzxbd,
  ORC_AVX_pmovzxbq,
  ORC_AVX_pmovzxwd,
  ORC_AVX_pmovzxwq,
  ORC_AVX_pmovzxdq,
  ORC_AVX_pminsb,
  ORC_AVX_pminsd,
  /* 280 */
  ORC_AVX_pminuw,
  ORC_AVX_pminud,
  ORC_AVX_pmaxsb,
  ORC_AVX_pmaxsd,
  ORC_AVX_pmaxuw,
  ORC_AVX_pmaxud,
  ORC_AVX_pmulld,
  ORC_AVX_pcmpgtq,
} OrcAVXInsnOpcodeIdx;

ORC_API void orc_vex_emit_cpuinsn_none (OrcCompiler *p, const int index);
ORC_API void orc_vex_emit_cpuinsn_avx (OrcCompiler *const p, const int index,
    const int src0, const int src1, const int src2, const int dest);
ORC_API void orc_vex_emit_cpuinsn_size (OrcCompiler *const p, const int index,
    const int size, const int src0, const int src1, const int src2,
    const int dest);
ORC_API void orc_vex_emit_cpuinsn_imm (OrcCompiler *const p, const int index,
    const int size, const int imm, const int src0, const int src1,
    const int dest);
ORC_API void orc_vex_emit_cpuinsn_load_memoffset (OrcCompiler *const p,
    const int index, const int imm, const int offset,
    const int src0, const int src1, const int dest);
ORC_API void orc_vex_emit_cpuinsn_store_memoffset (OrcCompiler *const p,
    const int index, const int imm, const int offset,
    const int src, const int dest);
ORC_API void orc_vex_emit_cpuinsn_load_memindex (OrcCompiler *const p,
    const int index, const int imm, const int offset, const int src,
    const int src_index, const int shift, int dest);

#define orc_avx_sse_emit_punpcklbw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpcklbw, s1, s2, 0, d)
#define orc_avx_emit_punpcklbw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpcklbw, s1, s2, 0, d)
#define orc_avx_sse_emit_punpcklwd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpcklwd, s1, s2, 0, d)
#define orc_avx_emit_punpcklwd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpcklwd, s1, s2, 0, d)
#define orc_avx_sse_emit_punpckldq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpckldq, s1, s2, 0, d)
#define orc_avx_emit_punpckldq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpckldq, s1, s2, 0, d)
#define orc_avx_sse_emit_packsswb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_packsswb, s1, s2, 0, d)
#define orc_avx_emit_packsswb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_packsswb, s1, s2, 0, d)

/* NOT USED */
#define orc_avx_sse_emit_pcmpgtb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpgtb, s1, s2, 0, d)
/* NOT USED */
#define orc_avx_emit_pcmpgtb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpgtb, s1, s2, 0, d)
#define orc_avx_sse_emit_pcmpgtw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpgtw, s1, s2, 0, d)
#define orc_avx_emit_pcmpgtw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpgtw, s1, s2, 0, d)
/* NOT USED */
#define orc_avx_sse_emit_pcmpgtd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpgtd, s1, s2, 0, d)
#define orc_avx_emit_pcmpgtd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpgtd, s1, s2, 0, d)
#define orc_avx_sse_emit_packuswb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_packuswb, s1, s2, 0, d)
#define orc_avx_emit_packuswb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_packuswb, s1, s2, 0, d)
#define orc_avx_sse_emit_punpckhbw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpckhbw, s1, s2, 0, d)
#define orc_avx_emit_punpckhbw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpckhbw, s1, s2, 0, d)
#define orc_avx_sse_emit_punpckhwd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpckhwd, s1, s2, 0, d)
#define orc_avx_emit_punpckhwd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpckhwd, s1, s2, 0, d)
#define orc_avx_sse_emit_punpckhdq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpckhdq, s1, s2, 0, d)
#define orc_avx_emit_punpckhdq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpckhdq, s1, s2, 0, d)
#define orc_avx_emit_packssdw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_packssdw, s1, s2, 0, d)
#define orc_avx_sse_emit_packssdw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_packssdw, s1, s2, 0, d)
#define orc_avx_sse_emit_punpcklqdq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_punpcklqdq, s1, s2, 0, d)
#define orc_avx_emit_punpcklqdq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpcklqdq, s1, s2, 0, d)
#define orc_avx_emit_punpckhqdq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_punpckhqdq, s1, s2, 0, d)
#define orc_avx_sse_emit_movdqa(p,a,b) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_movdqa_load, a, 0, 0, b)
#define orc_avx_emit_movdqa(p,a,b) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_movdqa_load, a, 0, 0, b)
#define orc_avx_sse_emit_pcmpeqb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpeqb, s1, s2, 0, d)
#define orc_avx_emit_pcmpeqb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpeqb, s1, s2, 0, d)
#define orc_avx_sse_emit_pcmpeqw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpeqw, s1, s2, 0, d)
#define orc_avx_emit_pcmpeqw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpeqw, s1, s2, 0, d)
#define orc_avx_sse_emit_pcmpeqd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpeqd, s1, s2, 0, d)
#define orc_avx_emit_pcmpeqd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpeqd, s1, s2, 0, d)
#define orc_avx_sse_emit_paddq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddq, s1, s2, 0, d)
#define orc_avx_emit_paddq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddq, s1, s2, 0, d)
#define orc_avx_sse_emit_pmullw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmullw, s1, s2, 0, d)
#define orc_avx_emit_pmullw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmullw, s1, s2, 0, d)
#define orc_avx_sse_emit_psubusb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubusb, s1, s2, 0, d)
#define orc_avx_emit_psubusb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubusb, s1, s2, 0, d)
#define orc_avx_sse_emit_psubusw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubusw, s1, s2, 0, d)
#define orc_avx_emit_psubusw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubusw, s1, s2, 0, d)
#define orc_avx_sse_emit_pminub(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pminub, s1, s2, 0, d)
#define orc_avx_emit_pminub(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pminub, s1, s2, 0, d)
#define orc_avx_sse_emit_pand(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pand, s1, s2, 0, d)
#define orc_avx_emit_pand(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pand, s1, s2, 0, d)
#define orc_avx_sse_emit_paddusb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddusb, s1, s2, 0, d)
#define orc_avx_emit_paddusb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddusb, s1, s2, 0, d)
#define orc_avx_sse_emit_paddusw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddusw, s1, s2, 0, d)
#define orc_avx_emit_paddusw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddusw, s1, s2, 0, d)
#define orc_avx_sse_emit_pmaxub(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmaxub, s1, s2, 0, d)
#define orc_avx_emit_pmaxub(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmaxub, s1, s2, 0, d)
#define orc_avx_sse_emit_pandn(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pandn, s1, s2, 0, d)
#define orc_avx_emit_pandn(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pandn, s1, s2, 0, d)
#define orc_avx_sse_emit_pavgb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pavgb, s1, s2, 0, d)
#define orc_avx_emit_pavgb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pavgb, s1, s2, 0, d)
#define orc_avx_sse_emit_pavgw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pavgw, s1, s2, 0, d)
#define orc_avx_emit_pavgw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pavgw, s1, s2, 0, d)
#define orc_avx_sse_emit_pmulhuw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmulhuw, s1, s2, 0, d)
#define orc_avx_emit_pmulhuw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmulhuw, s1, s2, 0, d)
#define orc_avx_sse_emit_pmulhw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmulhw, s1, s2, 0, d)
#define orc_avx_emit_pmulhw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmulhw, s1, s2, 0, d)
#define orc_avx_sse_emit_psubsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubsb, s1, s2, 0, d)
#define orc_avx_emit_psubsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubsb, s1, s2, 0, d)
#define orc_avx_sse_emit_psubsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubsw, s1, s2, 0, d)
#define orc_avx_emit_psubsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubsw, s1, s2, 0, d)
#define orc_avx_sse_emit_pminsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pminsw, s1, s2, 0, d)
#define orc_avx_emit_pminsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pminsw, s1, s2, 0, d)
#define orc_avx_sse_emit_por(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_por, s1, s2, 0, d)
#define orc_avx_emit_por(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_por, s1, s2, 0, d)
#define orc_avx_sse_emit_paddsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddsb, s1, s2, 0, d)
#define orc_avx_emit_paddsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddsb, s1, s2, 0, d)
#define orc_avx_sse_emit_paddsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddsw, s1, s2, 0, d)
#define orc_avx_emit_paddsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddsw, s1, s2, 0, d)
#define orc_avx_sse_emit_pmaxsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmaxsw, s1, s2, 0, d)
#define orc_avx_emit_pmaxsw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmaxsw, s1, s2, 0, d)
#define orc_avx_sse_emit_pxor(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pxor, s1, s2, 0, d)
#define orc_avx_emit_pxor(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pxor, s1, s2, 0, d)
#define orc_avx_sse_emit_pmuludq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmuludq, s1, s2, 0, d)
#define orc_avx_emit_pmuludq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmuludq, s1, s2, 0, d)
#define orc_avx_sse_emit_psadbw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psadbw, s1, s2, 0, d)
#define orc_avx_emit_psadbw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psadbw, s1, s2, 0, d)
#define orc_avx_sse_emit_psubb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubb, s1, s2, 0, d)
#define orc_avx_emit_psubb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubb, s1, s2, 0, d)
#define orc_avx_sse_emit_psubw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubw, s1, s2, 0, d)
#define orc_avx_emit_psubw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubw, s1, s2, 0, d)
#define orc_avx_sse_emit_psubd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubd, s1, s2, 0, d)
#define orc_avx_emit_psubd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubd, s1, s2, 0, d)
#define orc_avx_sse_emit_psubq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_psubq, s1, s2, 0, d)
#define orc_avx_emit_psubq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_psubq, s1, s2, 0, d)
#define orc_avx_sse_emit_paddb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddb, s1, s2, 0, d)
#define orc_avx_emit_paddb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddb, s1, s2, 0, d)
#define orc_avx_sse_emit_paddw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddw, s1, s2, 0, d)
#define orc_avx_emit_paddw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddw, s1, s2, 0, d)
#define orc_avx_sse_emit_paddd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_paddd, s1, s2, 0, d)
#define orc_avx_emit_paddd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_paddd, s1, s2, 0, d)
#define orc_avx_sse_emit_pshufb(p,mask,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pshufb, mask, s1, 0, d)
#define orc_avx_emit_pshufb(p,mask,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pshufb, mask, s1, 0, d)
#define orc_avx_sse_emit_pabsb(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pabsb, s1, 0, 0, d)
#define orc_avx_emit_pabsb(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pabsb, s1, 0, 0, d)
#define orc_avx_sse_emit_pabsw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pabsw, s1, 0, 0, d)
#define orc_avx_emit_pabsw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pabsw, s1, 0, 0, d)
#define orc_avx_sse_emit_pabsd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pabsd, s1, 0, 0, d)
#define orc_avx_emit_pabsd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pabsd, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovsxbw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovsxbw, s1, 0, 0, d)
#define orc_avx_emit_pmovsxbw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovsxbw, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovsxbd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovsxbd, s1, 0, 0, d)
#define orc_avx_emit_pmovsxbd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovsxbd, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovsxbq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovsxbq, s1, 0, 0, d)
#define orc_avx_emit_pmovsxbq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovsxbq, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovsxwd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovsxwd, s1, 0, 0, d)
#define orc_avx_emit_pmovsxwd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovsxwd, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovsxwq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovsxwq, s1, 0, 0, d)
#define orc_avx_emit_pmovsxwq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovsxwq, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovsxdq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovsxdq, s1, 0, 0, d)
#define orc_avx_emit_pmovsxdq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovsxdq, s1, 0, 0, d)
#define orc_avx_sse_emit_pmuldq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmuldq, s1, s2, 0, d)
#define orc_avx_emit_pmuldq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmuldq, s1, s2, 0, d)
#define orc_avx_sse_emit_pcmpeqq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpeqq, s1, s2, 0, d)
#define orc_avx_emit_pcmpeqq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpeqq, s1, s2, 0, d)
#define orc_avx_sse_emit_packusdw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_packusdw, s1, s2, 0, d)
#define orc_avx_emit_packusdw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_packusdw, s1, s2, 0, d)
#define orc_avx_sse_emit_pmovzxbw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovzxbw, s1, 0, 0, d)
#define orc_avx_emit_pmovzxbw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovzxbw, s1, 0, 0, d)
#define orc_avx_emit_pmovzxbd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovzxbd, s1, 0, 0, d)
#define orc_avx_emit_pmovzxbq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovzxbq, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovzxwd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovzxwd, s1, 0, 0, d)
#define orc_avx_emit_pmovzxwd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovzxwd, s1, 0, 0, d)
#define orc_avx_emit_pmovzxwq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovzxwq, s1, 0, 0, d)
#define orc_avx_sse_emit_pmovzxdq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmovzxdq, s1, 0, 0, d)
#define orc_avx_emit_pmovzxdq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmovzxdq, s1, 0, 0, d)
#define orc_avx_sse_emit_pmulld(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmulld, s1, s2, 0, d)
#define orc_avx_emit_pmulld(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmulld, s1, s2, 0, d)
#define orc_avx_sse_emit_pminsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pminsb, s1, s2, 0, d)
#define orc_avx_emit_pminsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pminsb, s1, s2, 0, d)
#define orc_avx_sse_emit_pminsd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pminsd, s1, s2, 0, d)
#define orc_avx_emit_pminsd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pminsd, s1, s2, 0, d)
#define orc_avx_sse_emit_pminuw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pminuw, s1, s2, 0, d)
#define orc_avx_emit_pminuw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pminuw, s1, s2, 0, d)
#define orc_avx_sse_emit_pminud(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pminud, s1, s2, 0, d)
#define orc_avx_emit_pminud(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pminud, s1, s2, 0, d)
#define orc_avx_sse_emit_pmaxsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmaxsb, s1, s2, 0, d)
#define orc_avx_emit_pmaxsb(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmaxsb, s1, s2, 0, d)
#define orc_avx_sse_emit_pmaxsd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmaxsd, s1, s2, 0, d)
#define orc_avx_emit_pmaxsd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmaxsd, s1, s2, 0, d)
#define orc_avx_sse_emit_pmaxuw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmaxuw, s1, s2, 0, d)
#define orc_avx_emit_pmaxuw(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmaxuw, s1, s2, 0, d)
#define orc_avx_sse_emit_pmaxud(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pmaxud, s1, s2, 0, d)
#define orc_avx_emit_pmaxud(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pmaxud, s1, s2, 0, d)
#define orc_avx_sse_emit_pcmpgtq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_pcmpgtq, s1, s2, 0, d)
#define orc_avx_emit_pcmpgtq(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_pcmpgtq, s1, s2, 0, d)

#define orc_avx_sse_emit_addps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_addps, s1, s2, 0, d)
#define orc_avx_emit_addps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_addps, s1, s2, 0, d)
#define orc_avx_sse_emit_subps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_subps, s1, s2, 0, d)
#define orc_avx_emit_subps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_subps, s1, s2, 0, d)
#define orc_avx_sse_emit_mulps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_mulps, s1, s2, 0, d)
#define orc_avx_emit_mulps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_mulps, s1, s2, 0, d)
#define orc_avx_sse_emit_divps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_divps, s1, s2, 0, d)
#define orc_avx_emit_divps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_divps, s1, s2, 0, d)
#define orc_avx_sse_emit_sqrtps(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_sqrtps, s1, 0, 0, d)
#define orc_avx_emit_sqrtps(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_sqrtps, s1, 0, 0, d)
#define orc_avx_sse_emit_andps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_andps, s1, s2, 0, d)
#define orc_avx_emit_andps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_andps, s1, s2, 0, d)
#define orc_avx_sse_emit_orps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_orps, s1, s2, 0, d)
#define orc_avx_emit_orps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_orps, s1, s2, 0, d)

#define orc_avx_sse_emit_addpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_addpd, s1, s2, 0, d)
#define orc_avx_emit_addpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_addpd, s1, s2, 0, d)
#define orc_avx_sse_emit_subpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_subpd, s1, s2, 0, d)
#define orc_avx_emit_subpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_subpd, s1, s2, 0, d)
#define orc_avx_sse_emit_mulpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_mulpd, s1, s2, 0, d)
#define orc_avx_emit_mulpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_mulpd, s1, s2, 0, d)
#define orc_avx_sse_emit_divpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_divpd, s1, s2, 0, d)
#define orc_avx_emit_divpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_divpd, s1, s2, 0, d)
#define orc_avx_sse_emit_sqrtpd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_sqrtpd, s1, 0, 0, d)
#define orc_avx_emit_sqrtpd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_sqrtpd, s1, 0, 0, d)
#define orc_avx_sse_emit_cmpeqps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cmpeqps, s1, s2, 0, d)
#define orc_avx_emit_cmpeqps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cmpeqps, s1, s2, 0, d)
#define orc_avx_sse_emit_cmpeqpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cmpeqpd, s1, s2, 0, d)
#define orc_avx_emit_cmpeqpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cmpeqpd, s1, s2, 0, d)
#define orc_avx_sse_emit_cmpltps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cmpltps, s1, s2, 0, d)
#define orc_avx_emit_cmpltps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cmpltps, s1, s2, 0, d)
#define orc_avx_sse_emit_cmpltpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cmpltpd, s1, s2, 0, d)
#define orc_avx_emit_cmpltpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cmpltpd, s1, s2, 0, d)
#define orc_avx_sse_emit_cmpleps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cmpleps, s1, s2, 0, d)
#define orc_avx_emit_cmpleps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cmpleps, s1, s2, 0, d)
#define orc_avx_sse_emit_cmplepd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cmplepd, s1, s2, 0, d)
#define orc_avx_emit_cmplepd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cmplepd, s1, s2, 0, d)
#define orc_avx_sse_emit_cvttps2dq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cvttps2dq, s1, 0, 0, d)
#define orc_avx_emit_cvttps2dq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cvttps2dq, s1, 0, 0, d)
#define orc_avx_sse_emit_cvttpd2dq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cvttpd2dq, s1, 0, 0, d)
#define orc_avx_emit_cvttpd2dq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cvttpd2dq, s1, 0, 0, d)
#define orc_avx_sse_emit_cvtdq2ps(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cvtdq2ps, s1, 0, 0, d)
#define orc_avx_emit_cvtdq2ps(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cvtdq2ps, s1, 0, 0, d)
#define orc_avx_sse_emit_cvtdq2pd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cvtdq2pd, s1, 0, 0, d)
#define orc_avx_emit_cvtdq2pd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cvtdq2pd, s1, 0, 0, d)
#define orc_avx_sse_emit_cvtps2pd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cvtps2pd, s1, 0, 0, d)
#define orc_avx_emit_cvtps2pd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cvtps2pd, s1, 0, 0, d)
#define orc_avx_sse_emit_cvtpd2ps(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_cvtpd2ps, s1, 0, 0, d)
#define orc_avx_emit_cvtpd2ps(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_cvtpd2ps, s1, 0, 0, d)
#define orc_avx_sse_emit_minps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_minps, s1, s2, 0, d)
#define orc_avx_emit_minps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_minps, s1, s2, 0, d)
#define orc_avx_sse_emit_minpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_minpd, s1, s2, 0, d)
#define orc_avx_emit_minpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_minpd, s1, s2, 0, d)
#define orc_avx_sse_emit_maxps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_maxps, s1, s2, 0, d)
#define orc_avx_emit_maxps(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_maxps, s1, s2, 0, d)
#define orc_avx_sse_emit_maxpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_SSE_maxpd, s1, s2, 0, d)
#define orc_avx_emit_maxpd(p,s1,s2,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_maxpd, s1, s2, 0, d)

#define orc_avx_sse_emit_psraw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psraw_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psraw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psraw_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_psrlw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psrlw_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psrlw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psrlw_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_psllw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psllw_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psllw_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psllw_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psrad_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psrad_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_psrad_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psrad_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_psrld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psrld_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psrld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psrld_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_pslld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_pslld_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_pslld_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_pslld_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_psrlq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psrlq_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psrlq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psrlq_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_psllq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_psllq_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_psllq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_psllq_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_pslldq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_pslldq_imm, 0, imm, s1, 0, d)
#define orc_avx_emit_pslldq_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_pslldq_imm, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_pshufd(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_pshufd, 0, imm, s1, 0, d)
#define orc_avx_emit_pshufd(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_pshufd, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_pshuflw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_pshuflw, 0, imm, s1, 0, d)
#define orc_avx_emit_pshuflw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_pshuflw, 0, imm, s1, 0, d)
#define orc_avx_sse_emit_pshufhw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_pshufhw, 0, imm, s1, 0, d)
#define orc_avx_emit_pshufhw(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_pshufhw, 0, imm, s1, 0, d)

#define orc_avx_sse_emit_pinsrb_memoffset(p,imm,offset,s1,s2,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_pinsrb, imm, offset, s1, s2, d)
#define orc_avx_sse_emit_pinsrw_memoffset(p,imm,offset,s1,s2,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_pinsrw, imm, offset, s1, s2, d)
#define orc_avx_sse_emit_movd_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_movd_load, 0, offset, s1, 0, d)
#define orc_avx_sse_emit_pinsrd_memoffset(p,imm,offset,s1,s2,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_pinsrd, imm, offset, s1, s2, d)
#define orc_avx_sse_emit_movq_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_movq_load, 0, offset, s1, 0, d)
#define orc_avx_sse_emit_movdqa_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_movdqa_load, 0, offset, s1, 0, d)
#define orc_avx_emit_movdqa_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_movdqa_load, 0, offset, s1, 0, d)
#define orc_avx_sse_emit_movdqu_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_SSE_movdqu_load, 0, offset, s1, 0, d)
#define orc_avx_emit_movdqu_load_memoffset(p,offset,s1,d) orc_vex_emit_cpuinsn_load_memoffset(p, ORC_AVX_movdqu_load, 0, offset, s1, 0, d)

#define orc_avx_sse_emit_pextrb_memoffset(p,imm,offset,a,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_pextrb, imm, offset, a, b)
#define orc_avx_sse_emit_pextrw_memoffset(p,imm,offset,a,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_pextrw_mem, imm, offset, a, b)
#define orc_avx_sse_emit_movd_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_movd_store, 0, a, offset, b)
#define orc_avx_sse_emit_movq_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_movq_store, 0, a, offset, b)
#define orc_avx_sse_emit_movdqa_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_movdqa_store, 0, a, offset, b)
#define orc_avx_emit_movdqa_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_movdqa_store, 0, a, offset, b)
#define orc_avx_sse_emit_movdqu_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_movdqu_store, 0, a, offset, b)
#define orc_avx_emit_movdqu_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_movdqu_store, 0, a, offset, b)
#define orc_avx_sse_emit_movntdq_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_SSE_movntdq_store, 0, a, offset, b)
#define orc_avx_emit_movntdq_store_memoffset(p,a,offset,b) orc_vex_emit_cpuinsn_store_memoffset(p, ORC_AVX_movntdq_store, 0, a, offset, b)

#define orc_avx_sse_emit_movd_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_AVX_SSE_movd_load, 0, offset, a, a_index, shift, b)
#define orc_avx_sse_emit_movq_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_AVX_SSE_movq_load, 0, offset, a, a_index, shift, b)
#define orc_avx_sse_emit_movdqa_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_AVX_SSE_movdqa_load, 0, offset, a, a_index, shift, b)
#define orc_avx_emit_movdqa_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_AVX_movdqa_load, 0, offset, a, a_index, shift, b)
#define orc_avx_sse_emit_movdqu_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_AVX_SSE_movdqu_load, 0, offset, a, a_index, shift, b)
#define orc_avx_emit_movdqu_load_memindex(p,offset,a,a_index,shift,b) orc_vex_emit_cpuinsn_load_memindex(p, ORC_AVX_movdqu_load, 0, offset, a, a_index, shift, b)

#define orc_avx_sse_emit_pinsrw_register(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_pinsrw, 4, imm, s1, s2, d)
#define orc_avx_sse_emit_movd_load_register(p,a,b) orc_vex_emit_cpuinsn_size(p, ORC_AVX_SSE_movd_load, 4, a, 0, 0, b)


#define orc_avx_emit_permute2f128(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_vperm2f128, 0, imm, s1, s2, d)
#define orc_avx_emit_permute2i128(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_vperm2i128, 0, imm, s1, s2, d)

#define orc_avx_emit_pbroadcastb(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_vpbroadcastb, s1, 0, 0, d)
#define orc_avx_emit_pbroadcastw(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_vpbroadcastw, s1, 0, 0, d)
#define orc_avx_emit_pbroadcastd(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_vpbroadcastd, s1, 0, 0, d)
#define orc_avx_emit_pbroadcastq(p,s1,d) orc_vex_emit_cpuinsn_avx(p, ORC_AVX_vpbroadcastq, s1, 0, 0, d)

#define orc_avx_sse_emit_shufps_imm(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_SSE_shufps_imm, 0, imm, s1, s2, d)
#define orc_avx_emit_insertf128_si256(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_insertf128_avx, 0, imm, s1, s2, d)
#define orc_avx_emit_extractf128_si256(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_vextractf128, 0, imm, s1, 0, d)

#define orc_avx_emit_permute4x64_imm(p,imm,s1,d) orc_vex_emit_cpuinsn_imm(p,  ORC_AVX_vpermq, 0, imm, s1, 0, d)
#define orc_avx_emit_blendpd(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_blendpd, 0, imm, s1, s2, d)
#define orc_avx_emit_pblendd(p,imm,s1,s2,d) orc_vex_emit_cpuinsn_imm(p, ORC_AVX_vpblendd, 0, imm, s1, s2, d)

#define orc_avx_sse_emit_blendvpd(p, s1, s2, mask, d) \
    orc_vex_emit_cpuinsn_avx (p, ORC_AVX_SSE_vblendvpd, s1, s2, mask, d)
#define orc_avx_emit_blendvpd(p, s1, s2, mask, d) \
    orc_vex_emit_cpuinsn_avx (p, ORC_AVX_vblendvpd, s1, s2, mask, d)

#define orc_avx_sse_emit_pinsrd_register(p, imm, s1, s2, d) \
  orc_vex_emit_cpuinsn_imm (p, ORC_AVX_SSE_pinsrd, 4, imm, s1, s2, d)

#define orc_avx_sse_emit_movq_load_register(p, s1, d) orc_vex_emit_cpuinsn_size (p, ORC_AVX_SSE_movq_rm_r, 8, s1, 0, 0, d)
#endif

ORC_END_DECLS

#endif
