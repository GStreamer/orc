#ifndef ORC_MMX_INSN_H_
#define ORC_MMX_INSN_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum _OrcMMXInsnOperandFlag {
  /* Size embedded on the type of register */
  ORC_MMX_INSN_OPERAND_OP1_MM = (1 << 0),
  ORC_MMX_INSN_OPERAND_OP2_MM = (1 << 1),
} OrcMMXInsnOperandFlag;

#define ORC_MMX_INSN_OPERAND_FLAG_LAST 1

typedef enum _OrcMMXInsnIdx {
  ORC_MMX_punpcklbw,
  ORC_MMX_punpcklwd,
  ORC_MMX_punpckldq,
  ORC_MMX_packsswb,
  ORC_MMX_pcmpgtb,
  ORC_MMX_pcmpgtw,
  ORC_MMX_pcmpgtd,
  ORC_MMX_packuswb,
  ORC_MMX_punpckhbw,
  ORC_MMX_punpckhwd,
  /* 10 */
  ORC_MMX_punpckhdq,
  ORC_MMX_packssdw,
  ORC_MMX_movd_load,
  ORC_MMX_movq,
  ORC_MMX_movq_mmx_load, /* FIXME change the name */
  ORC_MMX_psrlw_imm,
  ORC_MMX_psraw_imm,
  ORC_MMX_psllw_imm,
  ORC_MMX_psrld_imm,
  ORC_MMX_psrad_imm,
  /* 20 */
  ORC_MMX_pslld_imm,
  ORC_MMX_psrlq_imm,
  ORC_MMX_psllq_imm,
  ORC_MMX_pcmpeqb,
  ORC_MMX_pcmpeqw,
  ORC_MMX_pcmpeqd,
  ORC_MMX_emms,
  ORC_MMX_movd_store,
  ORC_MMX_movq_mmx_store,
  ORC_MMX_psrlw,
  /* 30 */
  ORC_MMX_psrld,
  ORC_MMX_psrlq,
  ORC_MMX_paddq,
  ORC_MMX_pmullw,
  ORC_MMX_psubusb,
  ORC_MMX_psubusw,
  ORC_MMX_pand,
  ORC_MMX_paddusb,
  ORC_MMX_paddusw,
  ORC_MMX_pandn,
  /* 40 */
  ORC_MMX_psraw,
  ORC_MMX_psrad,
  ORC_MMX_pmulhw,
  ORC_MMX_psubsb,
  ORC_MMX_psubsw,
  ORC_MMX_por,
  ORC_MMX_paddsb,
  ORC_MMX_paddsw,
  ORC_MMX_pxor,
  ORC_MMX_psllw,
  /* 50 */
  ORC_MMX_pslld,
  ORC_MMX_psllq,
  ORC_MMX_pmaddwd,
  ORC_MMX_psubb,
  ORC_MMX_psubw,
  ORC_MMX_psubd,
  ORC_MMX_paddb,
  ORC_MMX_paddw,
  ORC_MMX_paddd,
  ORC_MMX_pshufw,
  /* 60 */
  ORC_MMX_pinsrw,
  ORC_MMX_pminub,
  ORC_MMX_pmaxub,
  ORC_MMX_pavgb,
  ORC_MMX_pavgw,
  ORC_MMX_pmulhuw,
  ORC_MMX_pminsw,
  ORC_MMX_pmaxsw,
  ORC_MMX_psadbw,
  ORC_MMX_psubq,
  /* 70 */
  ORC_MMX_pmuludq,
  ORC_MMX_pshufb,
  ORC_MMX_phaddw,
  ORC_MMX_phaddd,
  ORC_MMX_phaddsw,
  ORC_MMX_pmaddubsw,
  ORC_MMX_phsubw,
  ORC_MMX_phsubd,
  ORC_MMX_phsubsw,
  ORC_MMX_psignb,
  /* 80 */
  ORC_MMX_psignw,
  ORC_MMX_psignd,
  ORC_MMX_pmulhrsw,
  ORC_MMX_pabsb,
  ORC_MMX_pabsw,
  ORC_MMX_pabsd,
  ORC_MMX_palignr,
} OrcMMXInsnIdx;

ORC_API void orc_mmx_emit_cpuinsn_none (OrcCompiler *p, int index);
ORC_API void orc_mmx_emit_cpuinsn_mmx (OrcCompiler *p, int index, int src, int dest);
ORC_API void orc_mmx_emit_cpuinsn_size (OrcCompiler *p, int index, int size, int src, int dest);
ORC_API void orc_mmx_emit_cpuinsn_imm (OrcCompiler *p, int index, int size, int imm, int src, int dest);
ORC_API void orc_mmx_emit_cpuinsn_load_memoffset (OrcCompiler *p, int index, int imm,
    int offset, int src, int dest);
ORC_API void orc_mmx_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int imm,
    int offset, int src, int src_index, int shift, int dest);
ORC_API void orc_mmx_emit_cpuinsn_store_memoffset (OrcCompiler *p, int index, int imm,
    int offset, int src, int dest);

/* 0 */
#define orc_mmx_emit_punpcklbw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_punpcklbw, a, b)
#define orc_mmx_emit_punpcklwd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_punpcklwd, a, b)
#define orc_mmx_emit_punpckldq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_punpckldq, a, b)
#define orc_mmx_emit_packsswb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_packsswb, a, b)
#define orc_mmx_emit_pcmpgtb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pcmpgtb, a, b)
#define orc_mmx_emit_pcmpgtw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pcmpgtw, a, b)
#define orc_mmx_emit_pcmpgtd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pcmpgtd, a, b)
#define orc_mmx_emit_packuswb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_packuswb, a, b)
#define orc_mmx_emit_punpckhbw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_punpckhbw, a, b)
#define orc_mmx_emit_punpckhwd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_punpckhwd, a, b)
/* 10 */
#define orc_mmx_emit_punpckhdq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_punpckhdq, a, b)
#define orc_mmx_emit_packssdw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_packssdw, a, b)
#define orc_mmx_emit_movd_load_register(p,a,b) orc_mmx_emit_cpuinsn_size(p, ORC_MMX_movd_load, 4, a, b)
#define orc_mmx_emit_movd_load_memoffset(p,offset,a,b) orc_mmx_emit_cpuinsn_load_memoffset(p, ORC_MMX_movd_load, 0, offset, a, b)
#define orc_mmx_emit_movd_load_memindex(p,offset,a,a_index,shift,b) orc_mmx_emit_cpuinsn_load_memindex(p, ORC_MMX_movd_load, 0, offset, a, a_index, shift, b)
#define orc_mmx_emit_movq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_movq_mmx_load, a, b)
#define orc_mmx_emit_movq_load_register(p,a,b) orc_mmx_emit_cpuinsn_size(p, ORC_MMX_movq, 8, a, b)
#define orc_mmx_emit_movq_load_memoffset(p,offset,a,b) orc_mmx_emit_cpuinsn_load_memoffset(p, ORC_MMX_movq_mmx_load, 0, offset, a, b)
#define orc_mmx_emit_movq_load_memindex(p,offset,a,a_index,shift,b) orc_mmx_emit_cpuinsn_load_memindex(p, ORC_MMX_movq_mmx_load, 0, offset, a, a_index, shift, b)
#define orc_mmx_emit_psrlw_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psrlw_imm, 0, imm, 0, b)
#define orc_mmx_emit_psraw_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psraw_imm, 0, imm, 0, b)
#define orc_mmx_emit_psllw_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psllw_imm, 0, imm, 0, b)
#define orc_mmx_emit_psrld_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psrld_imm, 0, imm, 0, b)
#define orc_mmx_emit_psrad_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psrad_imm, 0, imm, 0, b)
/* 20 */
#define orc_mmx_emit_pslld_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_pslld_imm, 0, imm, 0, b)
#define orc_mmx_emit_psrlq_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psrlq_imm, 0, imm, 0, b)
#define orc_mmx_emit_psllq_imm(p,imm,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_psllq_imm, 0, imm, 0, b)
#define orc_mmx_emit_pcmpeqb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pcmpeqb, a, b)
#define orc_mmx_emit_pcmpeqw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pcmpeqw, a, b)
#define orc_mmx_emit_pcmpeqd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pcmpeqd, a, b)
#define orc_mmx_emit_emms(p) orc_mmx_emit_cpuinsn_none (p, ORC_MMX_emms)
#define orc_mmx_emit_movd_store_memoffset(p,a,offset,b) orc_mmx_emit_cpuinsn_store_memoffset(p, ORC_MMX_movd_store, 0, a, offset, b)
#define orc_mmx_emit_movq_store_memoffset(p,a,offset,b) orc_mmx_emit_cpuinsn_store_memoffset(p, ORC_MMX_movq_mmx_store, 0, a, offset, b)
#define orc_mmx_emit_movd_store_memindex(p,a,offset,b,b_index,shift) orc_mmx_emit_cpuinsn_store_memindex(p, ORC_MMX_movd_store, 0, a, offset, b, b_index, shift)
#define orc_mmx_emit_movq_store_memindex(p,a,offset,b,b_index,shift) orc_mmx_emit_cpuinsn_store_memindex(p, ORC_MMX_movq_mmx_store, 0, a, offset, b, b_index, shift)
#define orc_mmx_emit_movd_store_register(p,a,b) orc_mmx_emit_cpuinsn_size(p, ORC_MMX_movd_store, 4, a, b)
#define orc_mmx_emit_movq_store_register(p,a,b) orc_mmx_emit_cpuinsn_size(p, ORC_MMX_movq_mmx_store, 8, a, b)
#define orc_mmx_emit_psrlw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psrlw, a, b)
/* 30 */
#define orc_mmx_emit_psrld(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psrld, a, b)
#define orc_mmx_emit_psrlq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psrlq, a, b)
#define orc_mmx_emit_paddq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddq, a, b)
#define orc_mmx_emit_pmullw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmullw, a, b)
#define orc_mmx_emit_psubusb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubusb, a, b)
#define orc_mmx_emit_psubusw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubusw, a, b)
#define orc_mmx_emit_pand(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pand, a, b)
#define orc_mmx_emit_paddusb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddusb, a, b)
#define orc_mmx_emit_paddusw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddusw, a, b)
#define orc_mmx_emit_pandn(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pandn, a, b)
/* 40 */
#define orc_mmx_emit_psraw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psraw, a, b)
#define orc_mmx_emit_psrad(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psrad, a, b)
#define orc_mmx_emit_pmulhw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmulhw, a, b)
#define orc_mmx_emit_psubsb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubsb, a, b)
#define orc_mmx_emit_psubsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubsw, a, b)
#define orc_mmx_emit_por(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_por, a, b)
#define orc_mmx_emit_paddsb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddsb, a, b)
#define orc_mmx_emit_paddsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddsw, a, b)
#define orc_mmx_emit_pxor(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pxor, a, b)
#define orc_mmx_emit_psllw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psllw, a, b)
/* 50 */
#define orc_mmx_emit_pslld(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pslld, a, b)
#define orc_mmx_emit_psllq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psllq, a, b)
#define orc_mmx_emit_pmaddwd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmaddwd, a, b)
#define orc_mmx_emit_psubb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubb, a, b)
#define orc_mmx_emit_psubw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubw, a, b)
#define orc_mmx_emit_psubd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubd, a, b)
#define orc_mmx_emit_paddb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddb, a, b)
#define orc_mmx_emit_paddw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddw, a, b)
#define orc_mmx_emit_paddd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_paddd, a, b)
#define orc_mmx_emit_pshufw(p,imm,a,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_pshufw, 0, imm, a, b)
#define orc_mmx_emit_pinsrw_memoffset(p,imm,offset,a,b) orc_mmx_emit_cpuinsn_load_memoffset(p, ORC_MMX_pinsrw, imm, offset, a, b)
#define orc_mmx_emit_pinsrw_memindex(p,imm,offset,a,a_index,shift,b) orc_mmx_emit_cpuinsn_load_memindex(p, ORC_MMX_pinsrw, 4, imm, offset, a, a_index, shift, b)
/* 60 */
#define orc_mmx_emit_pinsrw_register(p,s,imm,a,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_pinsrw, s, imm, a, b)
#define orc_mmx_emit_pminub(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pminub, a, b)
#define orc_mmx_emit_pmaxub(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmaxub, a, b)
#define orc_mmx_emit_pavgb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pavgb, a, b)
#define orc_mmx_emit_pavgw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pavgw, a, b)
#define orc_mmx_emit_pmulhuw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmulhuw, a, b)
#define orc_mmx_emit_pminsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pminsw, a, b)
#define orc_mmx_emit_pmaxsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmaxsw, a, b)
#define orc_mmx_emit_psadbw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psadbw, a, b)
#define orc_mmx_emit_psubq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psubq, a, b)
/* 70 */
#define orc_mmx_emit_pmuludq(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmuludq, a, b)
#define orc_mmx_emit_pshufb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pshufb, a, b)
#define orc_mmx_emit_phaddw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_phaddw, a, b)
#define orc_mmx_emit_phaddd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_phaddd, a, b)
#define orc_mmx_emit_phaddsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_phaddsw, a, b)
#define orc_mmx_emit_pmaddubsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmaddubsw, a, b)
#define orc_mmx_emit_phsubw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_phsubw, a, b)
#define orc_mmx_emit_phsubd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_phsubd, a, b)
#define orc_mmx_emit_phsubsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_phsubsw, a, b)
#define orc_mmx_emit_psignb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psignb, a, b)
/* 80 */
#define orc_mmx_emit_psignw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psignw, a, b)
#define orc_mmx_emit_psignd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_psignd, a, b)
#define orc_mmx_emit_pmulhrsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pmulhrsw, a, b)
#define orc_mmx_emit_pabsb(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pabsb, a, b)
#define orc_mmx_emit_pabsw(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pabsw, a, b)
#define orc_mmx_emit_pabsd(p,a,b) orc_mmx_emit_cpuinsn_mmx(p, ORC_MMX_pabsd, a, b)
#define orc_mmx_emit_palignr(p,s,imm,a,b) orc_mmx_emit_cpuinsn_imm(p, ORC_MMX_salignr, s, imm, a, b)

#endif

#endif
