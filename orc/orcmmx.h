
#ifndef _ORC_MMX_H_
#define _ORC_MMX_H_

#include <orc/orcx86.h>
#include <orc/orcx86insn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  X86_MM0 = ORC_VEC_REG_BASE,
  X86_MM1,
  X86_MM2,
  X86_MM3,
  X86_MM4,
  X86_MM5,
  X86_MM6,
  X86_MM7
} OrcMMXRegister;

#define ORC_MMX_SHUF(a,b,c,d) ((((a)&3)<<6)|(((b)&3)<<4)|(((c)&3)<<2)|(((d)&3)<<0))

ORC_API
const char * orc_x86_get_regname_mmx(int i);

ORC_API
void orc_x86_emit_mov_memoffset_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned);

ORC_API
void orc_x86_emit_mov_memindex_mmx (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned);

ORC_API
void orc_x86_emit_mov_mmx_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached);
#if 0
void orc_x86_emit_mov_mmx_reg_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_reg_mmx (OrcCompiler *compiler, int reg1, int reg2);
void orc_x86_emit_mov_mmx_reg (OrcCompiler *compiler, int reg1, int reg2);
void orc_mmx_emit_loadib (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadiw (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadil (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpb (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpw (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpl (OrcCompiler *p, int reg, int value);
void orc_mmx_emit_loadpq (OrcCompiler *p, int reg, int value);

void orc_mmx_emit_660f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_f20f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_f30f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_0f (OrcCompiler *p, const char *insn_name, int code,
    int src, int dest);
void orc_mmx_emit_pshufw (OrcCompiler *p, int shuf, int src, int dest);
void orc_mmx_emit_palignr (OrcCompiler *p, int align, int src, int dest);
void orc_mmx_emit_pinsrw_memoffset (OrcCompiler *p, int imm, int offset,
    int src, int dest);
void orc_mmx_emit_pextrw_memoffset (OrcCompiler *p, int imm, int src,
    int offset, int dest);
void orc_mmx_emit_shiftimm (OrcCompiler *p, const char *insn_name,
    int code, int modrm_code, int shift, int reg);
#endif

ORC_API unsigned int orc_mmx_get_cpu_flags (void);

ORC_API void orc_mmx_load_constant (OrcCompiler *compiler, int reg, int size,
    orc_uint64 value);

#define orc_mmx_emit_punpcklbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpcklbw, 8, a, b)
#define orc_mmx_emit_punpcklwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpcklwd, 8, a, b)
#define orc_mmx_emit_punpckldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckldq, 8, a, b)
#define orc_mmx_emit_packsswb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packsswb, 8, a, b)
#define orc_mmx_emit_pcmpgtb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtb, 8, a, b)
#define orc_mmx_emit_pcmpgtw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtw, 8, a, b)
#define orc_mmx_emit_pcmpgtd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtd, 8, a, b)
#define orc_mmx_emit_packuswb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packuswb, 8, a, b)
#define orc_mmx_emit_punpckhbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhbw, 8, a, b)
#define orc_mmx_emit_punpckhwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhwd, 8, a, b)
#define orc_mmx_emit_punpckhdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhdq, 8, a, b)
#define orc_mmx_emit_packssdw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_packssdw, 8, a, b)
#define orc_mmx_emit_punpcklqdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpcklqdq, 8, a, b)
#define orc_mmx_emit_punpckhqdq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_punpckhqdq, 8, a, b)
#define orc_mmx_emit_psraw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psraw, 8, a, b)
#define orc_mmx_emit_psrlw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrlw, 8, a, b)
#define orc_mmx_emit_psllw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psllw, 8, a, b)
#define orc_mmx_emit_psrad(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrad, 8, a, b)
#define orc_mmx_emit_psrld(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrld, 8, a, b)
#define orc_mmx_emit_pslld(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pslld, 8, a, b)
#define orc_mmx_emit_psrlq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrlq, 8, a, b)
#define orc_mmx_emit_psllq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psllq, 8, a, b)
#define orc_mmx_emit_psrldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrldq, 8, a, b)
#define orc_mmx_emit_pslldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pslldq, 8, a, b)
#define orc_mmx_emit_psrlq_reg(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psrlq_reg, 8, a, b)
#define orc_mmx_emit_pcmpeqb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqb, 8, a, b)
#define orc_mmx_emit_pcmpeqw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqw, 8, a, b)
#define orc_mmx_emit_pcmpeqd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpeqd, 8, a, b)
#define orc_mmx_emit_paddq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddq, 8, a, b)
#define orc_mmx_emit_pmullw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmullw, 8, a, b)
#define orc_mmx_emit_psubusb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubusb, 8, a, b)
#define orc_mmx_emit_psubusw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubusw, 8, a, b)
#define orc_mmx_emit_pminub(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminub, 8, a, b)
#define orc_mmx_emit_pand(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pand, 8, a, b)
#define orc_mmx_emit_paddusb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddusb, 8, a, b)
#define orc_mmx_emit_paddusw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddusw, 8, a, b)
#define orc_mmx_emit_pmaxub(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxub, 8, a, b)
#define orc_mmx_emit_pandn(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pandn, 8, a, b)
#define orc_mmx_emit_pavgb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pavgb, 8, a, b)
#define orc_mmx_emit_pavgw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pavgw, 8, a, b)
#define orc_mmx_emit_pmulhuw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulhuw, 8, a, b)
#define orc_mmx_emit_pmulhw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulhw, 8, a, b)
#define orc_mmx_emit_psubsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubsb, 8, a, b)
#define orc_mmx_emit_psubsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubsw, 8, a, b)
#define orc_mmx_emit_pminsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pminsw, 8, a, b)
#define orc_mmx_emit_por(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_por, 8, a, b)
#define orc_mmx_emit_paddsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddsb, 8, a, b)
#define orc_mmx_emit_paddsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddsw, 8, a, b)
#define orc_mmx_emit_pmaxsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaxsw, 8, a, b)
#define orc_mmx_emit_pxor(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pxor, 8, a, b)
#define orc_mmx_emit_pmuludq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmuludq, 8, a, b)
#define orc_mmx_emit_pmaddwd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaddwd, 8, a, b)
#define orc_mmx_emit_psadbw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psadbw, 8, a, b)
#define orc_mmx_emit_psubb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubb, 8, a, b)
#define orc_mmx_emit_psubw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubw, 8, a, b)
#define orc_mmx_emit_psubd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubd, 8, a, b)
#define orc_mmx_emit_psubq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psubq, 8, a, b)
#define orc_mmx_emit_paddb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddb, 8, a, b)
#define orc_mmx_emit_paddw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddw, 8, a, b)
#define orc_mmx_emit_paddd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_paddd, 8, a, b)
#define orc_mmx_emit_pshufb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pshufb, 8, a, b)
#define orc_mmx_emit_phaddw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phaddw, 8, a, b)
#define orc_mmx_emit_phaddd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phaddd, 8, a, b)
#define orc_mmx_emit_phaddsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phaddsw, 8, a, b)
#define orc_mmx_emit_pmaddubsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmaddubsw, 8, a, b)
#define orc_mmx_emit_phsubw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phsubw, 8, a, b)
#define orc_mmx_emit_phsubd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phsubd, 8, a, b)
#define orc_mmx_emit_phsubsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phsubsw, 8, a, b)
#define orc_mmx_emit_psignb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psignb, 8, a, b)
#define orc_mmx_emit_psignw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psignw, 8, a, b)
#define orc_mmx_emit_psignd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_psignd, 8, a, b)
#define orc_mmx_emit_pmulhrsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmulhrsw, 8, a, b)
#define orc_mmx_emit_pabsb(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pabsb, 8, a, b)
#define orc_mmx_emit_pabsw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pabsw, 8, a, b)
#define orc_mmx_emit_pabsd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pabsd, 8, a, b)
#define orc_mmx_emit_pmuldq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pmuldq, 8, a, b)
#define orc_mmx_emit_phminposuw(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_phminposuw, 8, a, b)
#define orc_mmx_emit_pcmpgtq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_pcmpgtq, 8, a, b)
#define orc_mmx_emit_addps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_addps, 8, a, b)
#define orc_mmx_emit_subps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_subps, 8, a, b)
#define orc_mmx_emit_mulps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_mulps, 8, a, b)
#define orc_mmx_emit_divps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_divps, 8, a, b)
#define orc_mmx_emit_sqrtps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_sqrtps, 8, a, b)
#define orc_mmx_emit_addpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_addpd, 8, a, b)
#define orc_mmx_emit_subpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_subpd, 8, a, b)
#define orc_mmx_emit_mulpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_mulpd, 8, a, b)
#define orc_mmx_emit_divpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_divpd, 8, a, b)
#define orc_mmx_emit_sqrtpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_sqrtpd, 8, a, b)
#define orc_mmx_emit_cmpeqps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpeqps, 8, a, b)
#define orc_mmx_emit_cmpeqpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpeqpd, 8, a, b)
#define orc_mmx_emit_cmpltps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpltps, 8, a, b)
#define orc_mmx_emit_cmpltpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpltpd, 8, a, b)
#define orc_mmx_emit_cmpleps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmpleps, 8, a, b)
#define orc_mmx_emit_cmplepd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cmplepd, 8, a, b)
#define orc_mmx_emit_cvttps2dq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvttps2dq, 8, a, b)
#define orc_mmx_emit_cvttpd2dq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvttpd2dq, 8, a, b)
#define orc_mmx_emit_cvtdq2ps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtdq2ps, 8, a, b)
#define orc_mmx_emit_cvtdq2pd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtdq2pd, 8, a, b)
#define orc_mmx_emit_cvtps2pd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtps2pd, 8, a, b)
#define orc_mmx_emit_cvtpd2ps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_cvtpd2ps, 8, a, b)
#define orc_mmx_emit_minps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_minps, 8, a, b)
#define orc_mmx_emit_minpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_minpd, 8, a, b)
#define orc_mmx_emit_maxps(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_maxps, 8, a, b)
#define orc_mmx_emit_maxpd(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_maxpd, 8, a, b)
#define orc_mmx_emit_psraw_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psraw_imm, imm, 0, b)
#define orc_mmx_emit_psrlw_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrlw_imm, imm, 0, b)
#define orc_mmx_emit_psllw_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psllw_imm, imm, 0, b)
#define orc_mmx_emit_psrad_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrad_imm, imm, 0, b)
#define orc_mmx_emit_psrld_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrld_imm, imm, 0, b)
#define orc_mmx_emit_pslld_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pslld_imm, imm, 0, b)
#define orc_mmx_emit_psrlq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrlq_imm, imm, 0, b)
#define orc_mmx_emit_psllq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psllq_imm, imm, 0, b)
#define orc_mmx_emit_psrldq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psrldq_imm, imm, 0, b)
#define orc_mmx_emit_pslldq_imm(p,imm,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pslldq_imm, imm, 0, b)
#define orc_mmx_emit_pshufd(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshufd, imm, a, b)
#define orc_mmx_emit_pshuflw(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshuflw, imm, a, b)
#define orc_mmx_emit_pshufhw(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshufhw, imm, a, b)
#define orc_mmx_emit_palignr(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_psalignr, imm, a, b)

#define orc_mmx_emit_pinsrw_memoffset(p,imm,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_pinsrw, 4, imm, offset, a, b)
#define orc_mmx_emit_movd_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movd_load, 4, 0, offset, a, b)
#define orc_mmx_emit_movq_load_memoffset(p,offset,a,b) orc_x86_emit_cpuinsn_load_memoffset(p, ORC_X86_movq_mmx_load, 4, 0, offset, a, b)

#define orc_mmx_emit_pextrw_memoffset(p,imm,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_pextrw, 8, imm, a, offset, b)
#define orc_mmx_emit_movd_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movd_store, 4, 0, a, offset, b)
#define orc_mmx_emit_movq_store_memoffset(p,a,offset,b) orc_x86_emit_cpuinsn_store_memoffset(p, ORC_X86_movq_mmx_store, 8, 0, a, offset, b)

#define orc_mmx_emit_pinsrw_memindex(p,imm,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_pinsrw, 4, imm, offset, a, a_index, shift, b)
#define orc_mmx_emit_movd_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movd_load, 4, 0, offset, a, a_index, shift, b)
#define orc_mmx_emit_movq_load_memindex(p,offset,a,a_index,shift,b) orc_x86_emit_cpuinsn_load_memindex(p, ORC_X86_movq_mmx_load, 8, 0, offset, a, a_index, shift, b)

#define orc_mmx_emit_pextrw_memindex(p,imm,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_pextrw, imm, a, offset, b, b_index, shift)
#define orc_mmx_emit_movd_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movd_store, 4, a, offset, b, b_index, shift)
#define orc_mmx_emit_movq_store_memindex(p,a,offset,b,b_index,shift) orc_x86_emit_cpuinsn_store_memindex(p, ORC_X86_movq_mmx_store, 0, a, offset, b, b_index, shift)

#define orc_mmx_emit_pinsrw_register(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pinsrw, imm, a, b)
#define orc_mmx_emit_movd_load_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movd_load, 4, a, b)
#define orc_mmx_emit_movq_load_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movq_mmx_load, 8, a, b)

#define orc_mmx_emit_pextrw_register(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pextrw, imm, a, b)
#define orc_mmx_emit_movd_store_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movd_store, 4, a, b)
#define orc_mmx_emit_movq_store_register(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movq_mmx_store, 8, a, b)


#define orc_mmx_emit_pshufw(p,imm,a,b) orc_x86_emit_cpuinsn_imm(p, ORC_X86_pshufw, imm, a, b)
#define orc_mmx_emit_movq(p,a,b) orc_x86_emit_cpuinsn_size(p, ORC_X86_movq_mmx_load, 8, a, b)

#endif

ORC_END_DECLS

#endif

