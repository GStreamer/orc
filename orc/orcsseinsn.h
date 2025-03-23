#ifndef ORC_SSE_INSN_H_
#define ORC_SSE_INSN_H_

#include <orc/orcmmxinsn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

/* This extends OrcMMXInsnOperandFlag */
typedef enum _OrcSSEInsnOperandFlag {
  /* Size embedded on the type of register */
  ORC_SSE_INSN_OPERAND_OP1_XMM = (1 << (ORC_MMX_INSN_OPERAND_FLAG_LAST + 1)), 
  ORC_SSE_INSN_OPERAND_OP2_XMM = (1 << (ORC_MMX_INSN_OPERAND_FLAG_LAST + 2))
} OrcSSEInsnOperandFlag;

#define ORC_SSE_INSN_OPERAND_FLAG_LAST (ORC_MMX_INSN_OPERAND_FLAG_LAST + 2)

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSE_REG32_IMM8 (\
  ORC_X86_INSN_OPERAND_REG_REGM_IMM |     \
  ORC_X86_INSN_OPERAND_OP2_32 |           \
  ORC_X86_INSN_OPERAND_OP3_8              \
), ORC_SSE_INSN_OPERAND_OP1_XMM

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSE_IMM8 (\
  ORC_X86_INSN_OPERAND_REG_IMM |    \
  ORC_X86_INSN_OPERAND_OP3_8        \
), ORC_SSE_INSN_OPERAND_OP1_XMM

/* For CMPPS example xmm1, xmm2/m128, imm8 */
/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSE_SSEM_IMM8 (\
  ORC_X86_INSN_OPERAND_REG_REGM_IMM |    \
  ORC_X86_INSN_OPERAND_OP3_8             \
), (                                     \
  ORC_SSE_INSN_OPERAND_OP1_XMM |         \
  ORC_SSE_INSN_OPERAND_OP2_XMM           \
)

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSE_SSEM (\
  ORC_X86_INSN_OPERAND_REG_REGM     \
), (                                \
  ORC_SSE_INSN_OPERAND_OP1_XMM |    \
  ORC_SSE_INSN_OPERAND_OP2_XMM      \
)

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSE_REGM32 (\
  ORC_X86_INSN_OPERAND_REG_REGM |     \
  ORC_X86_INSN_OPERAND_OP2_32         \
), ORC_SSE_INSN_OPERAND_OP1_XMM

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_REGM32_SSE (\
  ORC_X86_INSN_OPERAND_REGM_REG |     \
  ORC_X86_INSN_OPERAND_OP1_32         \
), ORC_SSE_INSN_OPERAND_OP2_XMM

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_REG32TO64_SSE_IMM8 (\
  ORC_X86_INSN_OPERAND_REG_REG_IMM |          \
  ORC_X86_INSN_OPERAND_OP1_32 |               \
  ORC_X86_INSN_OPERAND_OP1_64 |               \
  ORC_X86_INSN_OPERAND_OP3_8                  \
), ORC_SSE_INSN_OPERAND_OP2_XMM

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_REGM32TO64_SSE_IMM8 (\
  ORC_X86_INSN_OPERAND_REGM_REG_IMM |          \
  ORC_X86_INSN_OPERAND_OP1_32 |                \
  ORC_X86_INSN_OPERAND_OP1_64 |                \
  ORC_X86_INSN_OPERAND_OP3_8                   \
), ORC_SSE_INSN_OPERAND_OP2_XMM

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSEM_SSE (\
  ORC_X86_INSN_OPERAND_REGM_REG     \
), (                                \
  ORC_SSE_INSN_OPERAND_OP1_XMM |    \
  ORC_SSE_INSN_OPERAND_OP2_XMM      \
)

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_SSE_REGM64 (\
  ORC_X86_INSN_OPERAND_REG_REGM |     \
  ORC_X86_INSN_OPERAND_OP2_64         \
), ORC_SSE_INSN_OPERAND_OP1_XMM

/* FIXME don't export this */
#define ORC_SSE_INSN_TYPE_REGM64_SSE (\
  ORC_X86_INSN_OPERAND_REGM_REG |     \
  ORC_X86_INSN_OPERAND_OP1_64         \
), ORC_SSE_INSN_OPERAND_OP2_XMM

/* FIXME don't export this */
ORC_INTERNAL orc_bool orc_sse_insn_validate_operand1_sse (int reg, unsigned int sse_operands);
ORC_INTERNAL orc_bool orc_sse_insn_validate_operand2_sse (int reg, unsigned int sse_operands);
ORC_INTERNAL orc_bool orc_sse_insn_validate_reg (int reg);

typedef enum _OrcSSEInsnIdx {
  ORC_SSE_movhps_load,
  ORC_SSE_sqrtps,
  ORC_SSE_andps,
  ORC_SSE_orps,
  ORC_SSE_addps,
  ORC_SSE_mulps,
  ORC_SSE_subps,
  ORC_SSE_minps,
  ORC_SSE_divps,
  ORC_SSE_maxps,
  /* 10 */
  ORC_SSE_ldmxcsr,
  ORC_SSE_stmxcsr,
  ORC_SSE_sqrtpd,
  ORC_SSE_addpd,
  ORC_SSE_mulpd,
  ORC_SSE_cvtps2pd,
  ORC_SSE_cvtpd2ps,
  ORC_SSE_cvtdq2ps,
  ORC_SSE_cvttps2dq,
  ORC_SSE_subpd,
  /* 20 */
  ORC_SSE_minpd,
  ORC_SSE_divpd,
  ORC_SSE_maxpd,
  ORC_SSE_cmpeqpd,
  ORC_SSE_cmpltpd,
  ORC_SSE_cmplepd,
  ORC_SSE_cmpeqps,
  ORC_SSE_cmpltps,
  ORC_SSE_cmpleps,
  ORC_SSE_cvtdq2pd,
  /* 30 */
  ORC_SSE_cvttpd2dq,
  ORC_SSE_punpcklbw,
  ORC_SSE_punpcklwd,
  ORC_SSE_punpckldq,
  ORC_SSE_packsswb,
  ORC_SSE_pcmpgtb,
  ORC_SSE_pcmpgtw,
  ORC_SSE_pcmpgtd,
  ORC_SSE_packuswb,
  ORC_SSE_punpckhbw,
  /* 40 */
  ORC_SSE_punpckhwd,
  ORC_SSE_punpckhdq,
  ORC_SSE_packssdw,
  ORC_SSE_movq_rm_r,
  ORC_SSE_movd_load,
  ORC_SSE_psrlw_imm,
  ORC_SSE_psraw_imm,
  ORC_SSE_psllw_imm,
  ORC_SSE_psrld_imm,
  /* 50 */
  ORC_SSE_psrad_imm,
  ORC_SSE_pslld_imm,
  ORC_SSE_psrlq_imm,
  ORC_SSE_psllq_imm,
  ORC_SSE_pcmpeqb,
  ORC_SSE_pcmpeqw,
  ORC_SSE_pcmpeqd,
  ORC_SSE_movd_store,
  ORC_SSE_movq_sse_load,
  ORC_SSE_movq_r_rm,
  ORC_SSE_pinsrw,
  ORC_SSE_pextrw,
  /* 60 */
  ORC_SSE_psrlw,
  ORC_SSE_psrld,
  ORC_SSE_psrlq,
  ORC_SSE_paddq,
  ORC_SSE_pmullw,
  ORC_SSE_movq_sse_store,
  ORC_SSE_psubusb,
  ORC_SSE_psubusw,
  ORC_SSE_pminub,
  /* 70 */
  ORC_SSE_pand,
  ORC_SSE_paddusb,
  ORC_SSE_paddusw,
  ORC_SSE_pmaxub,
  ORC_SSE_pandn,
  ORC_SSE_pavgb,
  ORC_SSE_psraw,
  ORC_SSE_psrad,
  ORC_SSE_pavgw,
  ORC_SSE_pmulhuw,
  ORC_SSE_pmulhw,
  /* 80 */
  ORC_SSE_psubsb,
  ORC_SSE_psubsw,
  ORC_SSE_pminsw,
  ORC_SSE_por,
  ORC_SSE_paddsb,
  ORC_SSE_paddsw,
  ORC_SSE_pmaxsw,
  ORC_SSE_pxor,
  ORC_SSE_psllw,
  ORC_SSE_pslld,
  /* 90 */
  ORC_SSE_psllq,
  ORC_SSE_pmuludq,
  ORC_SSE_pmaddwd,
  ORC_SSE_psadbw,
  ORC_SSE_psubb,
  ORC_SSE_psubw,
  ORC_SSE_psubd,
  ORC_SSE_psubq,
  ORC_SSE_paddb,
  ORC_SSE_paddw,
  /* 100 */
  ORC_SSE_paddd,
  ORC_SSE_punpcklqdq,
  ORC_SSE_punpckhqdq,
  ORC_SSE_movdqa_load,
  ORC_SSE_pshufd,
  ORC_SSE_movdqa_store,
  ORC_SSE_psrldq_imm,
  ORC_SSE_pslldq_imm,
  ORC_SSE_movntdq_store,
  ORC_SSE_pshuflw,
  /* 110 */
  ORC_SSE_movdqu_load,
  ORC_SSE_movdqu_store,
  ORC_SSE_pshufhw,
  ORC_SSE_pshufb,
  ORC_SSE_phaddw,
  ORC_SSE_phaddd,
  ORC_SSE_phaddsw,
  ORC_SSE_pmaddubsw,
  ORC_SSE_phsubw,
  ORC_SSE_phsubd,
  /* 120 */
  ORC_SSE_phsubsw,
  ORC_SSE_psignb,
  ORC_SSE_psignw,
  ORC_SSE_psignd,
  ORC_SSE_pmulhrsw,
  ORC_SSE_palignr,
  ORC_SSE_pabsb,
  ORC_SSE_pabsw,
  ORC_SSE_pabsd,
  ORC_SSE_pextrb,
  /* 130 */
  ORC_SSE_blendvpd_sse,
  ORC_SSE_pextrw_mem,
  ORC_SSE_pinsrb,
  ORC_SSE_pmovsxbw,
  ORC_SSE_pmovsxbd,
  ORC_SSE_pinsrd,
  ORC_SSE_pmovsxbq,
  ORC_SSE_pmovsxwd,
  ORC_SSE_pmovsxwq,
  ORC_SSE_pmovsxdq,
  /* 140 */
  ORC_SSE_pmuldq,
  ORC_SSE_pcmpeqq,
  ORC_SSE_packusdw,
  ORC_SSE_pmovzxbw,
  ORC_SSE_pmovzxbd,
  ORC_SSE_pmovzxbq,
  ORC_SSE_pmovzxwd,
  ORC_SSE_pmovzxwq,
  ORC_SSE_pmovzxdq,
  ORC_SSE_pminsb,
  /* 150 */
  ORC_SSE_pminsd,
  ORC_SSE_pminuw,
  ORC_SSE_pminud,
  ORC_SSE_pmaxsb,
  ORC_SSE_pmaxsd,
  ORC_SSE_pmaxuw,
  ORC_SSE_pmaxud,
  ORC_SSE_pmulld,
  ORC_SSE_phminposuw,
  /* 160 */
  ORC_SSE_pcmpgtq,
} OrcSSEInsnIdx;

ORC_API void orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_memindex_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned);
ORC_API void orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);

ORC_API void orc_sse_set_mxcsr (OrcCompiler *compiler);
ORC_API void orc_sse_restore_mxcsr (OrcCompiler *compiler);

ORC_API void orc_sse_emit_cpuinsn_sse (OrcCompiler *p, int index, int src, int dest);
ORC_API void orc_sse_emit_cpuinsn_size (OrcCompiler *p, int index, int size, int src, int dest);
ORC_API void orc_sse_emit_cpuinsn_imm (OrcCompiler *p, int index, int size, int imm, int src, int dest);
ORC_API void orc_sse_emit_cpuinsn_load_memoffset (OrcCompiler *p, int index, int imm,
    int offset, int src, int dest);
ORC_API void orc_sse_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int imm,
    int offset, int src, int src_index, int shift, int dest);
ORC_API void orc_sse_emit_cpuinsn_store_memoffset (OrcCompiler *p, int index, int imm,
    int offset, int src, int dest);

#define ORC_SSE_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

#define orc_sse_emit_movhps_load_memoffset(p,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_movhps_load, 0, offset, a, b)
#define orc_sse_emit_movhps_load_memindex(p,offset,a,a_index,shift,b) orc_sse_emit_cpuinsn_load_memindex(p, ORC_SSE_movhps_load, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_sqrtps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_sqrtps, a, b)
#define orc_sse_emit_andps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_andps, a, b)
#define orc_sse_emit_orps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_orps, a, b)
#define orc_sse_emit_addps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_addps, a, b)
#define orc_sse_emit_mulps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_mulps, a, b)
#define orc_sse_emit_subps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_subps, a, b)
#define orc_sse_emit_minps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_minps, a, b)
#define orc_sse_emit_divps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_divps, a, b)
/* 10 */
#define orc_sse_emit_maxps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_maxps, a, b)
#define orc_sse_emit_sqrtpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_sqrtpd, a, b)
#define orc_sse_emit_addpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_addpd, a, b)
#define orc_sse_emit_mulpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_mulpd, a, b)
#define orc_sse_emit_cvtps2pd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cvtps2pd, a, b)
#define orc_sse_emit_cvtpd2ps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cvtpd2ps, a, b)
#define orc_sse_emit_cvtdq2ps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cvtdq2ps, a, b)
#define orc_sse_emit_cvttps2dq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cvttps2dq, a, b)
#define orc_sse_emit_subpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_subpd, a, b)
/* 20 */
#define orc_sse_emit_minpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_minpd, a, b)
#define orc_sse_emit_divpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_divpd, a, b)
#define orc_sse_emit_maxpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_maxpd, a, b)
#define orc_sse_emit_cmpeqpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cmpeqpd, a, b)
#define orc_sse_emit_cmpltpd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cmpltpd, a, b)
#define orc_sse_emit_cmplepd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cmplepd, a, b)
#define orc_sse_emit_cmpeqps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cmpeqps, a, b)
#define orc_sse_emit_cmpltps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cmpltps, a, b)
#define orc_sse_emit_cmpleps(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cmpleps, a, b)
#define orc_sse_emit_cvtdq2pd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cvtdq2pd, a, b)
/* 30 */
#define orc_sse_emit_cvttpd2dq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_cvttpd2dq, a, b)
#define orc_sse_emit_punpcklbw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpcklbw, a, b)
#define orc_sse_emit_punpcklwd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpcklwd, a, b)
#define orc_sse_emit_punpckldq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpckldq, a, b)
#define orc_sse_emit_packsswb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_packsswb, a, b)
#define orc_sse_emit_pcmpgtb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpgtb, a, b)
#define orc_sse_emit_pcmpgtw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpgtw, a, b)
#define orc_sse_emit_pcmpgtd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpgtd, a, b)
#define orc_sse_emit_packuswb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_packuswb, a, b)
#define orc_sse_emit_punpckhbw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpckhbw, a, b)
/* 40 */
#define orc_sse_emit_punpckhwd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpckhwd, a, b)
#define orc_sse_emit_punpckhdq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpckhdq, a, b)
#define orc_sse_emit_packssdw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_packssdw, a, b)
#define orc_sse_emit_movd_load_memindex(p,offset,a,a_index,shift,b) orc_sse_emit_cpuinsn_load_memindex(p, ORC_SSE_movd_load, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movd_load_register(p,a,b) orc_sse_emit_cpuinsn_size(p, ORC_SSE_movd_load, 4, a, b)
#define orc_sse_emit_movd_load_memoffset(p,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_movd_load, 0, offset, a, b)
#define orc_sse_emit_movq_load_memoffset(p,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_movq_sse_load, 0, offset, a, b)
#define orc_sse_emit_movq_load_memindex(p,offset,a,a_index,shift,b) orc_sse_emit_cpuinsn_load_memindex(p, ORC_SSE_movq_sse_load, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movq_load_register(p,a,b) orc_sse_emit_cpuinsn_size(p, ORC_SSE_movq_rm_r, 8, a, b)
#define orc_sse_emit_psrlw_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psrlw_imm, 0, imm, 0, b)
#define orc_sse_emit_psraw_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psraw_imm, 0, imm, 0, b)
#define orc_sse_emit_psllw_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psllw_imm, 0, imm, 0, b)
#define orc_sse_emit_psrld_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psrld_imm, 0, imm, 0, b)
#define orc_sse_emit_psrad_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psrad_imm, 0, imm, 0, b)
/* 50 */
#define orc_sse_emit_pslld_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pslld_imm, 0, imm, 0, b)
#define orc_sse_emit_psrlq_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psrlq_imm, 0, imm, 0, b)
#define orc_sse_emit_psllq_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psllq_imm, 0, imm, 0, b)
#define orc_sse_emit_pcmpeqb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpeqb, a, b)
#define orc_sse_emit_pcmpeqw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpeqw, a, b)
#define orc_sse_emit_pcmpeqd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpeqd, a, b)
#define orc_sse_emit_movd_store_memoffset(p,a,offset,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_movd_store, 0, a, offset, b)
#define orc_sse_emit_movd_store_memindex(p,a,offset,b,b_index,shift) orc_sse_emit_cpuinsn_store_memindex(p, ORC_SSE_movd_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_movd_store_register(p,a,b) orc_sse_emit_cpuinsn_size(p, ORC_SSE_movd_store, 4, a, b)
#define orc_sse_emit_movq_store_memoffset(p,a,offset,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_movq_sse_store, 0, a, offset, b)
#define orc_sse_emit_movq_store_memindex(p,a,offset,b,b_index,shift) orc_sse_emit_cpuinsn_store_memindex(p, ORC_SSE_movq_sse_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_movq_store_register(p,a,b) orc_sse_emit_cpuinsn_size(p, ORC_SSE_movq_r_rm, 8, a, b)
#define orc_sse_emit_pinsrw_memoffset(p,imm,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_pinsrw, imm, offset, a, b)
#define orc_sse_emit_pinsrw_memindex(p,imm,offset,a,a_index,shift,b) orc_sse_emit_cpuinsn_load_memindex(p, ORC_SSE_pinsrw, imm, offset, a, a_index, shift, b)
#define orc_sse_emit_pinsrd_register(p,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pinsrd, 4, imm, a, b)
#define orc_sse_emit_pinsrw_register(p,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pinsrw, 4, imm, a, b)
#define orc_sse_emit_pextrw_register(p,s,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pextrw, s, imm, a, b)
#define orc_sse_emit_psrlw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psrlw, a, b)
/* 60 */
#define orc_sse_emit_psrld(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psrld, a, b)
#define orc_sse_emit_psrlq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psrlq, a, b)
#define orc_sse_emit_paddq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddq, a, b)
#define orc_sse_emit_pmullw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmullw, a, b)
#define orc_sse_emit_psubusb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubusb, a, b)
#define orc_sse_emit_psubusw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubusw, a, b)
#define orc_sse_emit_pminub(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pminub, a, b)
#define orc_sse_emit_pand(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pand, a, b)
#define orc_sse_emit_paddusb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddusb, a, b)
#define orc_sse_emit_paddusw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddusw, a, b)
/* 70 */
#define orc_sse_emit_pmaxub(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaxub, a, b)
#define orc_sse_emit_pandn(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pandn, a, b)
#define orc_sse_emit_pavgb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pavgb, a, b)
#define orc_sse_emit_psraw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psraw, a, b)
#define orc_sse_emit_psrad(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psrad, a, b)
#define orc_sse_emit_pavgw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pavgw, a, b)
#define orc_sse_emit_pmulhuw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmulhuw, a, b)
#define orc_sse_emit_pmulhw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmulhw, a, b)
#define orc_sse_emit_psubsb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubsb, a, b)
#define orc_sse_emit_psubsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubsw, a, b)
/* 80 */
#define orc_sse_emit_pminsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pminsw, a, b)
#define orc_sse_emit_por(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_por, a, b)
#define orc_sse_emit_paddsb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddsb, a, b)
#define orc_sse_emit_paddsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddsw, a, b)
#define orc_sse_emit_pmaxsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaxsw, a, b)
#define orc_sse_emit_pxor(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pxor, a, b)
#define orc_sse_emit_psllw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psllw, a, b)
#define orc_sse_emit_pslld(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pslld, a, b)
#define orc_sse_emit_psllq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psllq, a, b)
#define orc_sse_emit_pmuludq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmuludq, a, b)
/* 90 */
#define orc_sse_emit_pmaddwd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaddwd, a, b)
#define orc_sse_emit_psadbw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psadbw, a, b)
#define orc_sse_emit_psubb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubb, a, b)
#define orc_sse_emit_psubw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubw, a, b)
#define orc_sse_emit_psubd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubd, a, b)
#define orc_sse_emit_psubq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psubq, a, b)
#define orc_sse_emit_paddb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddb, a, b)
#define orc_sse_emit_paddw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddw, a, b)
#define orc_sse_emit_paddd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_paddd, a, b)
#define orc_sse_emit_punpcklqdq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpcklqdq, a, b)
/* 100 */
#define orc_sse_emit_punpckhqdq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_punpckhqdq, a, b)
#define orc_sse_emit_movdqa(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_movdqa_load, a, b)
#define orc_sse_emit_movdqa_load_memoffset(p,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_movdqa_load, 0, offset, a, b)
#define orc_sse_emit_movdqa_load_memindex(p,offset,a,a_index,shift,b) orc_sse_emit_cpuinsn_load_memindex(p, ORC_SSE_movdqa_load, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_pshufd(p,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pshufd, 0, imm, a, b)
#define orc_sse_emit_movdqa_store_memoffset(p,a,offset,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_movdqa_store, 0, a, offset, b)
#define orc_sse_emit_movdqa_store_memindex(p,a,offset,b,b_index,shift) orc_sse_emit_cpuinsn_store_memindex(p, ORC_SSE_movdqa_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_psrldq_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_psrldq_imm, 0, imm, 0, b)
#define orc_sse_emit_pslldq_imm(p,imm,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pslldq_imm, 0, imm, 0, b)
#define orc_sse_emit_movntdq_store_memoffset(p,a,offset,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_movntdq_store, 0, a, offset, b)
#define orc_sse_emit_movntdq_store_memindex(p,a,offset,b,b_index,shift) orc_sse_emit_cpuinsn_store_memindex(p, ORC_SSE_movntdq_store, 0, a, offset, b, b_index, shift)
#define orc_sse_emit_pshuflw(p,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pshuflw, 0, imm, a, b)
#define orc_sse_emit_movdqu(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_movdqu_load, a, b)
#define orc_sse_emit_movdqu_load_memoffset(p,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_movdqu_load, 0, offset, a, b)
#define orc_sse_emit_movdqu_load_memindex(p,offset,a,a_index,shift,b) orc_sse_emit_cpuinsn_load_memindex(p, ORC_SSE_movdqu_load, 0, offset, a, a_index, shift, b)
#define orc_sse_emit_movdqu_store_memoffset(p,a,offset,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_movdqu_store, 0, a, offset, b)
#define orc_sse_emit_movdqu_store_memindex(p,a,offset,b,b_index,shift) orc_sse_emit_cpuinsn_store_memindex(p, ORC_SSE_movdqu_store, 0, a, offset, b, b_index, shift)
/* 110 */
#define orc_sse_emit_pshufhw(p,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_pshufhw, 0, imm, a, b)
#define orc_sse_emit_pshufb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pshufb, a, b)
#define orc_sse_emit_phaddw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phaddw, a, b)
#define orc_sse_emit_phaddd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phaddd, a, b)
#define orc_sse_emit_phaddsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phaddsw, a, b)
#define orc_sse_emit_pmaddubsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaddubsw, a, b)
#define orc_sse_emit_phsubw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phsubw, a, b)
#define orc_sse_emit_phsubd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phsubd, a, b)
#define orc_sse_emit_phsubsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phsubsw, a, b)
#define orc_sse_emit_psignb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psignb, a, b)
/* 120 */
#define orc_sse_emit_psignw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psignw, a, b)
#define orc_sse_emit_psignd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_psignd, a, b)
#define orc_sse_emit_pmulhrsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmulhrsw, a, b)
#define orc_sse_emit_palignr(p,imm,a,b) orc_sse_emit_cpuinsn_imm(p, ORC_SSE_palignr, 0, imm, a, b)
#define orc_sse_emit_pabsb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pabsb, a, b)
#define orc_sse_emit_pabsw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pabsw, a, b)
#define orc_sse_emit_pabsd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pabsd, a, b)
#define orc_sse_emit_pextrb_memoffset(p,imm,offset,a,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_pextrb, imm, offset, a,b)
#define orc_sse_emit_blendvpd(p, s1, d) orc_sse_emit_cpuinsn_sse (p, ORC_SSE_blendvpd_sse, s1, d)
#define orc_sse_emit_pextrw_memoffset(p,imm,offset,a,b) orc_sse_emit_cpuinsn_store_memoffset(p, ORC_SSE_pextrw_mem, imm, offset, a, b)
#define orc_sse_emit_pextrw_memindex(p,imm,a,offset,b,b_index,shift) orc_sse_emit_cpuinsn_store_memindex(p, ORC_SSE_pextrw_mem, imm, a, offset, b, b_index, shift)
/* 130 */
#define orc_sse_emit_pinsrb_memoffset(p,imm,offset,a,b) orc_sse_emit_cpuinsn_load_memoffset(p, ORC_SSE_pinsrb, imm, offset, a, b)
#define orc_sse_emit_pmovsxbw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovsxbw, a, b)
#define orc_sse_emit_pmovsxbd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovsxbd, a, b)
#define orc_sse_emit_pmovsxbq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovsxbq, a, b)
#define orc_sse_emit_pmovsxwd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovsxwd, a, b)
#define orc_sse_emit_pmovsxwq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovsxwq, a, b)
#define orc_sse_emit_pmovsxdq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovsxdq, a, b)
#define orc_sse_emit_pmuldq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmuldq, a, b)
#define orc_sse_emit_pcmpeqq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpeqq, a, b)
#define orc_sse_emit_packusdw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_packusdw, a, b)
/* 140 */
#define orc_sse_emit_pmovzxbw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovzxbw, a, b)
#define orc_sse_emit_pmovzxbd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovzxbd, a, b)
#define orc_sse_emit_pmovzxbq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovzxbq, a, b)
#define orc_sse_emit_pmovzxwd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovzxwd, a, b)
#define orc_sse_emit_pmovzxwq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovzxwq, a, b)
#define orc_sse_emit_pmovzxdq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmovzxdq, a, b)
#define orc_sse_emit_pminsb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pminsb, a, b)
#define orc_sse_emit_pminsd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pminsd, a, b)
#define orc_sse_emit_pminuw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pminuw, a, b)
#define orc_sse_emit_pminud(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pminud, a, b)
/* 150 */
#define orc_sse_emit_pmaxsb(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaxsb, a, b)
#define orc_sse_emit_pmaxsd(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaxsd, a, b)
#define orc_sse_emit_pmaxuw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaxuw, a, b)
#define orc_sse_emit_pmaxud(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmaxud, a, b)
#define orc_sse_emit_pmulld(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pmulld, a, b)
#define orc_sse_emit_phminposuw(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_phminposuw, a, b)
#define orc_sse_emit_pcmpgtq(p,a,b) orc_sse_emit_cpuinsn_sse(p, ORC_SSE_pcmpgtq, a, b)

#endif

#endif
