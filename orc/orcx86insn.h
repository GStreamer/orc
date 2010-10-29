
#ifndef ORC_ORC_X86_INSN_H_
#define ORC_ORC_X86_INSN_H_

#define ORC_X86_INSN_TYPE_SD 0
#define ORC_X86_INSN_TYPE_SHIFTIMM 1
#define ORC_X86_INSN_TYPE_SD2 2
#define ORC_X86_INSN_TYPE_SDI 3

enum {
  ORC_X86_punpcklbw,
  ORC_X86_punpcklwd,
  ORC_X86_punpckldq,
  ORC_X86_packsswb,
  ORC_X86_pcmpgtb,
  ORC_X86_pcmpgtw,
  ORC_X86_pcmpgtd,
  ORC_X86_packuswb,
  ORC_X86_punpckhbw,
  ORC_X86_punpckhwd,
  ORC_X86_punpckhdq,
  ORC_X86_packssdw,
  ORC_X86_punpcklqdq,
  ORC_X86_punpckhqdq,
  ORC_X86_movdqa,
  ORC_X86_psraw,
  ORC_X86_psrlw,
  ORC_X86_psllw,
  ORC_X86_psrad,
  ORC_X86_psrld,
  ORC_X86_pslld,
  ORC_X86_psrlq,
  ORC_X86_psllq,
  ORC_X86_psrldq,
  ORC_X86_pslldq,
  ORC_X86_psrlq_reg,
  ORC_X86_pcmpeqb,
  ORC_X86_pcmpeqw,
  ORC_X86_pcmpeqd,
  ORC_X86_paddq,
  ORC_X86_pmullw,
  ORC_X86_psubusb,
  ORC_X86_psubusw,
  ORC_X86_pminub,
  ORC_X86_pand,
  ORC_X86_paddusb,
  ORC_X86_paddusw,
  ORC_X86_pmaxub,
  ORC_X86_pandn,
  ORC_X86_pavgb,
  ORC_X86_pavgw,
  ORC_X86_pmulhuw,
  ORC_X86_pmulhw,
  ORC_X86_psubsb,
  ORC_X86_psubsw,
  ORC_X86_pminsw,
  ORC_X86_por,
  ORC_X86_paddsb,
  ORC_X86_paddsw,
  ORC_X86_pmaxsw,
  ORC_X86_pxor,
  ORC_X86_pmuludq,
  ORC_X86_pmaddwd,
  ORC_X86_psadbw,
  ORC_X86_psubb,
  ORC_X86_psubw,
  ORC_X86_psubd,
  ORC_X86_psubq,
  ORC_X86_paddb,
  ORC_X86_paddw,
  ORC_X86_paddd,
  ORC_X86_pshufb,
  ORC_X86_phaddw,
  ORC_X86_phaddd,
  ORC_X86_phaddsw,
  ORC_X86_pmaddubsw,
  ORC_X86_phsubw,
  ORC_X86_phsubd,
  ORC_X86_phsubsw,
  ORC_X86_psignb,
  ORC_X86_psignw,
  ORC_X86_psignd,
  ORC_X86_pmulhrsw,
  ORC_X86_pabsb,
  ORC_X86_pabsw,
  ORC_X86_pabsd,
  ORC_X86_pmovsxbw,
  ORC_X86_pmovsxbd,
  ORC_X86_pmovsxbq,
  ORC_X86_pmovsxwd,
  ORC_X86_pmovsxwq,
  ORC_X86_pmovsxdq,
  ORC_X86_pmuldq,
  ORC_X86_pcmpeqq,
  ORC_X86_packusdw,
  ORC_X86_pmovzxbw,
  ORC_X86_pmovzxbd,
  ORC_X86_pmovzxbq,
  ORC_X86_pmovzxwd,
  ORC_X86_pmovzxwq,
  ORC_X86_pmovzxdq,
  ORC_X86_pmulld,
  ORC_X86_phminposuw,
  ORC_X86_pminsb,
  ORC_X86_pminsd,
  ORC_X86_pminuw,
  ORC_X86_pminud,
  ORC_X86_pmaxsb,
  ORC_X86_pmaxsd,
  ORC_X86_pmaxuw,
  ORC_X86_pmaxud,
  ORC_X86_pcmpgtq,
  ORC_X86_addps,
  ORC_X86_subps,
  ORC_X86_mulps,
  ORC_X86_divps,
  ORC_X86_sqrtps,
  ORC_X86_addpd,
  ORC_X86_subpd,
  ORC_X86_mulpd,
  ORC_X86_divpd,
  ORC_X86_sqrtpd,
  ORC_X86_cmpeqps,
  ORC_X86_cmpeqpd,
  ORC_X86_cmpltps,
  ORC_X86_cmpltpd,
  ORC_X86_cmpleps,
  ORC_X86_cmplepd,
  ORC_X86_cvttps2dq,
  ORC_X86_cvttpd2dq,
  ORC_X86_cvtdq2ps,
  ORC_X86_cvtdq2pd,
  ORC_X86_cvtps2pd,
  ORC_X86_cvtpd2ps,
  ORC_X86_minps,
  ORC_X86_minpd,
  ORC_X86_maxps,
  ORC_X86_maxpd,
  ORC_X86_psraw_imm,
  ORC_X86_psrlw_imm,
  ORC_X86_psllw_imm,
  ORC_X86_psrad_imm,
  ORC_X86_psrld_imm,
  ORC_X86_pslld_imm,
  ORC_X86_psrlq_imm,
  ORC_X86_psllq_imm,
  ORC_X86_psrldq_imm,
  ORC_X86_pslldq_imm,
  ORC_X86_pshufd,
  ORC_X86_pshuflw,
  ORC_X86_pshufhw,
  ORC_X86_palignr,
};



#define orc_sse_emit_punpcklbw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpcklbw, a, b, 0)
#define orc_sse_emit_punpcklwd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpcklwd, a, b, 0)
#define orc_sse_emit_punpckldq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpckldq, a, b, 0)
#define orc_sse_emit_packsswb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_packsswb, a, b, 0)
#define orc_sse_emit_pcmpgtb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpgtb, a, b, 0)
#define orc_sse_emit_pcmpgtw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpgtw, a, b, 0)
#define orc_sse_emit_pcmpgtd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpgtd, a, b, 0)
#define orc_sse_emit_packuswb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_packuswb, a, b, 0)
#define orc_sse_emit_punpckhbw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpckhbw, a, b, 0)
#define orc_sse_emit_punpckhwd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpckhwd, a, b, 0)
#define orc_sse_emit_punpckhdq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpckhdq, a, b, 0)
#define orc_sse_emit_packssdw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_packssdw, a, b, 0)
#define orc_sse_emit_punpcklqdq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpcklqdq, a, b, 0)
#define orc_sse_emit_punpckhqdq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_punpckhqdq, a, b, 0)
#define orc_sse_emit_movdqa(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_movdqa, a, b, 0)
#define orc_sse_emit_psraw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psraw, a, b, 0)
#define orc_sse_emit_psrlw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrlw, a, b, 0)
#define orc_sse_emit_psllw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psllw, a, b, 0)
#define orc_sse_emit_psrad(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrad, a, b, 0)
#define orc_sse_emit_psrld(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrld, a, b, 0)
#define orc_sse_emit_pslld(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pslld, a, b, 0)
#define orc_sse_emit_psrlq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrlq, a, b, 0)
#define orc_sse_emit_psllq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psllq, a, b, 0)
#define orc_sse_emit_psrldq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrldq, a, b, 0)
#define orc_sse_emit_pslldq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pslldq, a, b, 0)
#define orc_sse_emit_psrlq_reg(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrlq_reg, a, b, 0)
#define orc_sse_emit_pcmpeqb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpeqb, a, b, 0)
#define orc_sse_emit_pcmpeqw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpeqw, a, b, 0)
#define orc_sse_emit_pcmpeqd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpeqd, a, b, 0)
#define orc_sse_emit_paddq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddq, a, b, 0)
#define orc_sse_emit_pmullw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmullw, a, b, 0)
#define orc_sse_emit_psubusb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubusb, a, b, 0)
#define orc_sse_emit_psubusw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubusw, a, b, 0)
#define orc_sse_emit_pminub(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pminub, a, b, 0)
#define orc_sse_emit_pand(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pand, a, b, 0)
#define orc_sse_emit_paddusb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddusb, a, b, 0)
#define orc_sse_emit_paddusw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddusw, a, b, 0)
#define orc_sse_emit_pmaxub(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaxub, a, b, 0)
#define orc_sse_emit_pandn(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pandn, a, b, 0)
#define orc_sse_emit_pavgb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pavgb, a, b, 0)
#define orc_sse_emit_pavgw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pavgw, a, b, 0)
#define orc_sse_emit_pmulhuw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmulhuw, a, b, 0)
#define orc_sse_emit_pmulhw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmulhw, a, b, 0)
#define orc_sse_emit_psubsb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubsb, a, b, 0)
#define orc_sse_emit_psubsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubsw, a, b, 0)
#define orc_sse_emit_pminsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pminsw, a, b, 0)
#define orc_sse_emit_por(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_por, a, b, 0)
#define orc_sse_emit_paddsb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddsb, a, b, 0)
#define orc_sse_emit_paddsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddsw, a, b, 0)
#define orc_sse_emit_pmaxsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaxsw, a, b, 0)
#define orc_sse_emit_pxor(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pxor, a, b, 0)
#define orc_sse_emit_pmuludq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmuludq, a, b, 0)
#define orc_sse_emit_pmaddwd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaddwd, a, b, 0)
#define orc_sse_emit_psadbw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psadbw, a, b, 0)
#define orc_sse_emit_psubb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubb, a, b, 0)
#define orc_sse_emit_psubw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubw, a, b, 0)
#define orc_sse_emit_psubd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubd, a, b, 0)
#define orc_sse_emit_psubq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psubq, a, b, 0)
#define orc_sse_emit_paddb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddb, a, b, 0)
#define orc_sse_emit_paddw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddw, a, b, 0)
#define orc_sse_emit_paddd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_paddd, a, b, 0)
#define orc_sse_emit_pshufb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pshufb, a, b, 0)
#define orc_sse_emit_phaddw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phaddw, a, b, 0)
#define orc_sse_emit_phaddd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phaddd, a, b, 0)
#define orc_sse_emit_phaddsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phaddsw, a, b, 0)
#define orc_sse_emit_pmaddubsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaddubsw, a, b, 0)
#define orc_sse_emit_phsubw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phsubw, a, b, 0)
#define orc_sse_emit_phsubd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phsubd, a, b, 0)
#define orc_sse_emit_phsubsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phsubsw, a, b, 0)
#define orc_sse_emit_psignb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psignb, a, b, 0)
#define orc_sse_emit_psignw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psignw, a, b, 0)
#define orc_sse_emit_psignd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psignd, a, b, 0)
#define orc_sse_emit_pmulhrsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmulhrsw, a, b, 0)
#define orc_sse_emit_pabsb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pabsb, a, b, 0)
#define orc_sse_emit_pabsw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pabsw, a, b, 0)
#define orc_sse_emit_pabsd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pabsd, a, b, 0)
#define orc_sse_emit_pmovsxbw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovsxbw, a, b, 0)
#define orc_sse_emit_pmovsxbd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovsxbd, a, b, 0)
#define orc_sse_emit_pmovsxbq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovsxbq, a, b, 0)
#define orc_sse_emit_pmovsxwd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovsxwd, a, b, 0)
#define orc_sse_emit_pmovsxwq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovsxwq, a, b, 0)
#define orc_sse_emit_pmovsxdq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovsxdq, a, b, 0)
#define orc_sse_emit_pmuldq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmuldq, a, b, 0)
#define orc_sse_emit_pcmpeqq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpeqq, a, b, 0)
#define orc_sse_emit_packusdw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_packusdw, a, b, 0)
#define orc_sse_emit_pmovzxbw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovzxbw, a, b, 0)
#define orc_sse_emit_pmovzxbd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovzxbd, a, b, 0)
#define orc_sse_emit_pmovzxbq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovzxbq, a, b, 0)
#define orc_sse_emit_pmovzxwd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovzxwd, a, b, 0)
#define orc_sse_emit_pmovzxwq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovzxwq, a, b, 0)
#define orc_sse_emit_pmovzxdq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmovzxdq, a, b, 0)
#define orc_sse_emit_pmulld(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmulld, a, b, 0)
#define orc_sse_emit_phminposuw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_phminposuw, a, b, 0)
#define orc_sse_emit_pminsb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pminsb, a, b, 0)
#define orc_sse_emit_pminsd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pminsd, a, b, 0)
#define orc_sse_emit_pminuw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pminuw, a, b, 0)
#define orc_sse_emit_pminud(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pminud, a, b, 0)
#define orc_sse_emit_pmaxsb(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaxsb, a, b, 0)
#define orc_sse_emit_pmaxsd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaxsd, a, b, 0)
#define orc_sse_emit_pmaxuw(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaxuw, a, b, 0)
#define orc_sse_emit_pmaxud(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pmaxud, a, b, 0)
#define orc_sse_emit_pcmpgtq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pcmpgtq, a, b, 0)
#define orc_sse_emit_addps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_addps, a, b, 0)
#define orc_sse_emit_subps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_subps, a, b, 0)
#define orc_sse_emit_mulps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_mulps, a, b, 0)
#define orc_sse_emit_divps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_divps, a, b, 0)
#define orc_sse_emit_sqrtps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_sqrtps, a, b, 0)
#define orc_sse_emit_addpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_addpd, a, b, 0)
#define orc_sse_emit_subpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_subpd, a, b, 0)
#define orc_sse_emit_mulpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_mulpd, a, b, 0)
#define orc_sse_emit_divpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_divpd, a, b, 0)
#define orc_sse_emit_sqrtpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_sqrtpd, a, b, 0)
#define orc_sse_emit_cmpeqps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cmpeqps, a, b, 0)
#define orc_sse_emit_cmpeqpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cmpeqpd, a, b, 0)
#define orc_sse_emit_cmpltps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cmpltps, a, b, 0)
#define orc_sse_emit_cmpltpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cmpltpd, a, b, 0)
#define orc_sse_emit_cmpleps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cmpleps, a, b, 0)
#define orc_sse_emit_cmplepd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cmplepd, a, b, 0)
#define orc_sse_emit_cvttps2dq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cvttps2dq, a, b, 0)
#define orc_sse_emit_cvttpd2dq(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cvttpd2dq, a, b, 0)
#define orc_sse_emit_cvtdq2ps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cvtdq2ps, a, b, 0)
#define orc_sse_emit_cvtdq2pd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cvtdq2pd, a, b, 0)
#define orc_sse_emit_cvtps2pd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cvtps2pd, a, b, 0)
#define orc_sse_emit_cvtpd2ps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_cvtpd2ps, a, b, 0)
#define orc_sse_emit_minps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_minps, a, b, 0)
#define orc_sse_emit_minpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_minpd, a, b, 0)
#define orc_sse_emit_maxps(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_maxps, a, b, 0)
#define orc_sse_emit_maxpd(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_maxpd, a, b, 0)
#define orc_sse_emit_psraw_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psraw_imm, 0, b, a)
#define orc_sse_emit_psrlw_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrlw_imm, 0, b, a)
#define orc_sse_emit_psllw_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psllw_imm, 0, b, a)
#define orc_sse_emit_psrad_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrad_imm, 0, b, a)
#define orc_sse_emit_psrld_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrld_imm, 0, b, a)
#define orc_sse_emit_pslld_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pslld_imm, 0, b, a)
#define orc_sse_emit_psrlq_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrlq_imm, 0, b, a)
#define orc_sse_emit_psllq_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psllq_imm, 0, b, a)
#define orc_sse_emit_psrldq_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psrldq_imm, 0, b, a)
#define orc_sse_emit_pslldq_imm(p,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pslldq_imm, 0, b, a)
#define orc_sse_emit_pshufd(p,imm,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pshufd, a, b, imm)
#define orc_sse_emit_pshuflw(p,imm,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pshuflw, a, b, imm)
#define orc_sse_emit_pshufhw(p,imm,a,b) orc_sse_emit_sysinsn(p, ORC_X86_pshufhw, a, b, imm)
#define orc_sse_emit_palignr(p,imm,a,b) orc_sse_emit_sysinsn(p, ORC_X86_psalignr, a, b, imm)

#endif

