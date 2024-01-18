
#ifndef ORC_ORC_X86_INSN_H_
#define ORC_ORC_X86_INSN_H_

#include <orc/orc.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  ORC_X86_INSN_TYPE_MMXM_MMX,        	/* mem/mmx, mmx */
  ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT,	/* $shift, mmx.  opcode in src */
  ORC_X86_INSN_TYPE_SSEM_SSE,		/* mem/mmx, sse */
  ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, /* uses both a mmx and an imm */
  ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV, /* uses both an imm and a mem/mmx destination */
  ORC_X86_INSN_TYPE_MMXM_MMX_REV,
  ORC_X86_INSN_TYPE_SSEM_SSE_REV,
  ORC_X86_INSN_TYPE_REGM_MMX,
  ORC_X86_INSN_TYPE_MMX_REGM_REV,
  ORC_X86_INSN_TYPE_REGM,
  ORC_X86_INSN_TYPE_MEM,
  ORC_X86_INSN_TYPE_IMM8_REGM,
  ORC_X86_INSN_TYPE_IMM32_REGM,
  ORC_X86_INSN_TYPE_REGM_REG,
  ORC_X86_INSN_TYPE_REG_REGM,
  ORC_X86_INSN_TYPE_LABEL,
  ORC_X86_INSN_TYPE_ALIGN,
  ORC_X86_INSN_TYPE_BRANCH,
  ORC_X86_INSN_TYPE_NONE,
  ORC_X86_INSN_TYPE_STACK,
  ORC_X86_INSN_TYPE_IMM32_REGM_MOV,
  ORC_X86_INSN_TYPE_REG8_REGM,
  ORC_X86_INSN_TYPE_REG16_REGM,
  ORC_X86_INSN_TYPE_IMM32_A, /* scalar register AND imm */
  ORC_X86_INSN_TYPE_IMM8_REGM_MMX, /* uses mmx register and imm */

  ORC_X86_INSN_TYPE_SSEM_AVX, /* avx, sse or memory register */
  ORC_X86_INSN_TYPE_IMM8_SSEM_AVX,  /* avx <- avx + (sse/m, imm) */
  ORC_X86_INSN_TYPE_IMM8_AVX_SSEM, /* sse/m <- avx, imm */
} OrcX86InsnType;

typedef enum
{
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
  ORC_X86_pinsrb,
  ORC_X86_pinsrw,
  ORC_X86_movd_load,
  ORC_X86_movq_sse_load,
  ORC_X86_movdqa_load,
  ORC_X86_movdqu_load,
  ORC_X86_movhps_load,
  ORC_X86_pextrb,
  ORC_X86_pextrw,
  ORC_X86_movd_store,
  ORC_X86_movq_sse_store,
  ORC_X86_movdqa_store,
  ORC_X86_movdqu_store,
  ORC_X86_movntdq_store,
  ORC_X86_ldmxcsr,
  ORC_X86_stmxcsr,
  ORC_X86_add_imm8_rm,
  ORC_X86_add_imm32_rm,
  ORC_X86_add_rm_r,
  ORC_X86_add_r_rm,
  ORC_X86_or_imm8_rm,
  ORC_X86_or_imm32_rm,
  ORC_X86_or_rm_r,
  ORC_X86_or_r_rm,
  ORC_X86_adc_imm8_rm,
  ORC_X86_adc_imm32_rm,
  ORC_X86_adc_rm_r,
  ORC_X86_adc_r_rm,
  ORC_X86_sbb_imm8_rm,
  ORC_X86_sbb_imm32_rm,
  ORC_X86_sbb_rm_r,
  ORC_X86_sbb_r_rm,
  ORC_X86_and_imm8_rm,
  ORC_X86_and_imm32_rm,
  ORC_X86_and_rm_r,
  ORC_X86_and_r_rm,
  ORC_X86_sub_imm8_rm,
  ORC_X86_sub_imm32_rm,
  ORC_X86_sub_rm_r,
  ORC_X86_sub_r_rm,
  ORC_X86_xor_imm8_rm,
  ORC_X86_xor_imm32_rm,
  ORC_X86_xor_rm_r,
  ORC_X86_xor_r_rm,
  ORC_X86_cmp_imm8_rm,
  ORC_X86_cmp_imm32_rm,
  ORC_X86_cmp_rm_r,
  ORC_X86_cmp_r_rm,
  ORC_X86_jo,
  ORC_X86_jno,
  ORC_X86_jc,
  ORC_X86_jnc,
  ORC_X86_jz,
  ORC_X86_jnz,
  ORC_X86_jbe,
  ORC_X86_ja,
  ORC_X86_js,
  ORC_X86_jns,
  ORC_X86_jp,
  ORC_X86_jnp,
  ORC_X86_jl,
  ORC_X86_jge,
  ORC_X86_jle,
  ORC_X86_jg,
  ORC_X86_jmp,
  ORC_X86_LABEL,
  ORC_X86_ret,
  ORC_X86_retq,
  ORC_X86_emms,
  ORC_X86_rdtsc,
  ORC_X86_nop,
  ORC_X86_rep_movsb,
  ORC_X86_rep_movsw,
  ORC_X86_rep_movsl,
  ORC_X86_push,
  ORC_X86_pop,
  ORC_X86_movzx_rm_r,
  ORC_X86_movw_rm_r,
  ORC_X86_movl_rm_r,
  ORC_X86_mov_rm_r,
  ORC_X86_mov_imm32_r,
  ORC_X86_movb_r_rm,
  ORC_X86_movw_r_rm,
  ORC_X86_movl_r_rm,
  ORC_X86_mov_r_rm,
  ORC_X86_test,
  ORC_X86_test_imm,
  ORC_X86_leal,
  ORC_X86_leaq,
  ORC_X86_imul_rm_r,
  ORC_X86_imul_rm,
  ORC_X86_inc,
  ORC_X86_dec,
  ORC_X86_sar_imm,
  ORC_X86_sar,
  ORC_X86_and_imm32_a,
  ORC_X86_ALIGN,
  ORC_X86_pshufw,
  ORC_X86_movq_mmx_load,
  ORC_X86_movq_mmx_store,
  ORC_X86_endbr32,
  ORC_X86_endbr64,

  ORC_X86_shufps_imm,
  ORC_X86_insertf128_avx,
  ORC_X86_extractf128_avx,
  ORC_X86_permute2f128_avx,
  ORC_X86_pbroadcastb_avx,
  ORC_X86_pbroadcastw_avx,
  ORC_X86_pbroadcastd_avx,
  ORC_X86_pbroadcastq_avx,
  ORC_X86_zeroupper_avx,
  ORC_X86_permute4x64_imm_avx,
  ORC_X86_blendpd_avx,
  ORC_X86_pinsrd,
  ORC_X86_permute2i128_avx,
  ORC_X86_pblendd_avx,
  ORC_X86_blendvpd_avx,
  ORC_X86_andps,
  ORC_X86_orps,
  ORC_X86_blendvpd_sse,
} OrcX86OpcodeIdx;

typedef enum {
  ORC_X86_RM_REG, /* operand can be register or memory */
  ORC_X86_RM_MEMOFFSET, /* operand can be register or memory, allows offset */
  ORC_X86_RM_MEMINDEX  /* operand can be register or memory, allows index */
} OrcX86OperandType;

typedef enum {
  ORC_X86_NO_PREFIX,
  ORC_X86_SSE_PREFIX,
  ORC_X86_AVX_VEX128_PREFIX,
  ORC_X86_AVX_VEX256_PREFIX,
} OrcX86OpcodePrefix;

struct _OrcX86Opcode {
  // Mnemonic
  char name[16];
  // Type of instruction (source instruction set, operation type)
  int type;
  // (x86 only) Escape prefix (0F) 3A or 38
  // See ss. 3.1.1.2 Intel® 64 and IA-32 Architectures Software Developer’s
  // Manual
  int flags;
  // (x86 only) Instruction set prefix (EVEX, VEX, 2-byte SSE...)
  // See ss. 2.3.5 Intel® 64 and IA-32 Architectures Software Developer’s
  // Manual
  orc_uint8 prefix;
  // Opcode (may include the last byte of the prefix as an opcode extension)
  // For x86, see ss. 2.1.2 Intel® 64 and IA-32 Architectures Software
  // Developer’s Manual
  orc_uint32 code;
  // Additional opcode (if any)
  int code2;
};

typedef struct _OrcX86Opcode OrcX86Opcode;

#define ORC_SYS_OPCODE_FLAG_FIXED (1<<0)

typedef struct _OrcX86Insn OrcX86Insn;
struct _OrcX86Insn {
  // Index into the opcode table
  OrcX86OpcodeIdx opcode_index;
  // The opcode the index points to
  const OrcX86Opcode *opcode;
  // What version should we encode?
  // SSE (without), VEX with SSE vectors,
  // VEX with AVX vectors...
  OrcX86OpcodePrefix prefix;
  // Immediate mode operand
  int imm;
  // Source register(s)/address
  int src[3];
  // Destination
  int dest;
  // Operand size
  int size;
  // Label for the function (if this is a preamble)
  int label;
  // Operation type
  OrcX86OperandType type;
  // Memory offset
  int offset;
  // Index register (if applicable)
  int index_reg;
  // Shift left (if applicable)
  int shift;
  // Offset from instruction pointer (for loads)
  int code_offset;
};

ORC_API OrcX86Insn * orc_x86_get_output_insn (OrcCompiler *p);
ORC_API void orc_x86_output_insns (OrcCompiler *p);
ORC_API void orc_x86_calculate_offsets (OrcCompiler *p);

ORC_API void orc_vex_emit_cpuinsn_none (OrcCompiler *p, int index, OrcX86OpcodePrefix prefix);
ORC_API void orc_vex_emit_cpuinsn_size (OrcCompiler *p, int opcode, int size,
    int src0, int src1, int dest, OrcX86OpcodePrefix prefix);
ORC_API void orc_vex_emit_cpuinsn_imm (OrcCompiler *p, int opcode, int imm,
    int src0, int src1, int dest, OrcX86OpcodePrefix prefix);
ORC_API void orc_vex_emit_cpuinsn_load_memoffset (OrcCompiler *p, int index,
    int size, int imm, int offset, int src0, int src1, int dest,
    OrcX86OpcodePrefix prefix);
ORC_API void orc_vex_emit_cpuinsn_store_memoffset (OrcCompiler *p, int index,
    int size, int imm, int offset, int src,
    int dest, OrcX86OpcodePrefix prefix);
ORC_API void orc_vex_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int src_index, int shift, int dest, OrcX86OpcodePrefix prefix);

ORC_API void orc_vex_emit_blend_size (OrcCompiler *p, int opcode, int size,
    int src0, int src1, int src2, int dest, OrcX86OpcodePrefix prefix);

#endif

ORC_END_DECLS

#endif

