

static OrcX86OpcodeNew sse_opcodes = {
  /* missing MOVUPS 0x0f10 */
  /* missing MOVSS 0xf3 0x0f10 */
  /* missing MOVUPS 0x0f11 */
  /* missing MOVLPS 0x0f12 */
  /* missing MOVHLPS 0x0f12 */
  /* missing MOVLPS 0x0f13 */
  /* missing UNPCKLPS 0x0f14 */
  /* missing UNPCKHPS 0x0f15 */
  { "movhps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f16 },
  /* missing MOVLHPS 0x0f15 */
  /* missing MOVHPS 0x0f17 */
  /* missing MOVAPS 0x0f28 */ 
  /* missing MOVAPS 0x0f29 */
  /* missing CVTPI2PS 0x0f2a */
  /* missing CVTSI2SS 0xf3 0x0f2a */
  /* missing CVTSI2SS 0xf3 REX.W 0x0f2a */
  /* missing MOVNTPS 0x0f2b */
  /* missing CVTTPS2PI 0x0f2c */
  /* missing CVTTSS2SI 0xf3 0x0f2c */
  /* missing CVTTSS2SI 0xf3 REX.W 0x0f2c */
  /* missing CVTPS2PI 0x0f2d */
  /* missing CVTSS2SI 0xf3 0x0f2d */
  /* missing CVTSS2SI 0xf3 REX.W 0x0f2d */
  /* missing UCOMISS 0xf3 0x0f2e */
  /* missing COMISS 0xf3 0x0f2f */
  /* missing MOVMSKPS 0x0f50 */
  { "sqrtps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f51 },
  /* missing SQRTSS 0xf3 0x0f51 */
  /* missing RSQRTSS 0xf3 0x0f52 */
  /* missing RSQRTPS 0x0f52 */
  /* missing RCPPS 0x0f53 */
  /* missing RCPSS 0xf3 0x0f53 */
  { "andps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f54 },
  /* missing ANDNPS 0x0f55 */
  { "orps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f56 },
  /* missing XORPS 0x0f57 */
  { "addps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f58 },
  /* missing ADDSS 0xf3 0x0f58 */
  { "mulps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f59 },
  /* missing MULSS 0xf3 0x0f59 */
  { "subps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f5c },
  /* missing SUBSS 0xf3 0x0f5c */
  { "minps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f5d },
  /* missing MINSS 0xf3 0x0f5d */
  { "divps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f5e },
  /* missing DIVSS 0xf3 0x0f5e */
  { "maxps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x0f5f },
  /* missing MAXSS 0xf3 0x0f5f */
  { "ldmxcsr", ORC_X86_INSN_TYPE_MEM, 0, 0x0fae, 2 },
  { "stmxcsr", ORC_X86_INSN_TYPE_MEM, 0, 0x0fae, 3 },
  /* missing CMPPS 0x0fc2 */
  /* missing CMPSS 0xf3 0x0fc2 */
  /* missing SHUFPS 0x0fc6 */

  /* SSE with SSE2 */
  /* TODO SSE2 data movement instructions */
  /* TODO SSE2 packed arithmetic instructions */
  { "minpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f5d },
  /* TODO SSE2 logical instructions */
  /* TODO SSE2 compare instructions */
  /* TODO SSE2 shuffle and unpack instructions */
  /* TODO SSE2 conversion instructions */
  /* MMX ones, not all */
  { "punpcklbw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f60 },
  { "punpcklwd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f61 },
  { "punpckldq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f62 },
  { "packsswb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f63 },
  { "pcmpgtb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f64 },
  { "pcmpgtw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f65 },
  { "pcmpgtd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f66 },
  { "packuswb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f67 },
  { "punpckhbw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f68 },
  { "punpckhwd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f69 },
  { "punpckhdq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f6a },
  { "packssdw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f6b },
  { "movd", ORC_X86_INSN_TYPE_REGM_MMX, ORC_VEX_W0, 0x66, 0x0f6e },
  { "psrlw", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, 0x66, 0x0f71, 2 },
  { "psraw", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, 0x66, 0x0f71, 4 },
  { "psllw", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, 0x66, 0x0f71, 6 },
  { "psrld", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, 0x66, 0x0f72, 2 },
  { "psrad", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, 0x66, 0x0f72, 4 },
  { "pslld", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, 0x66, 0x0f72, 6 },
  { "psrlq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f73, 2 },
  { "psllq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f73 6 },
  { "pcmpeqb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f74 },
  { "pcmpeqw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f75 },
  { "pcmpeqd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0f76 },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0xf3, 0x0f7e },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_X86_REX_W, 0x66, 0x0f7e },
  /* missing MOVQ 0x66 REX.W 0x6e */
  { "pinsrw", ORC_X86_INSN_TYPE_IMM8_REGM_MMX, ORC_VEX_W0, 0x66, 0x0fc4 },
  /* missing PEXTRW 0x66 0x0fc5 */
  { "psrlw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd1 },
  { "psrld", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd2 },
  { "psrlq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd3 },
  { "paddq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd4 },
  { "pmullw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd5 },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, 0x66, 0x0fd6 },
  /* missing PMOVMSKB 0x66 0x0fd7 */
  { "psubusb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd8 },
  { "psubusw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fd9 },
  { "pminub", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fda },
  { "pand", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fdb },
  { "paddusb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fdc },
  { "paddusw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fdd },
  { "pmaxub", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fde },
  { "pandn", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fdf },
  { "pavgb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe0 },
  { "psraw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe1 },
  { "psrad", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe2 },
  { "pavgw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe3 },
  { "pmulhuw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe4 },
  { "pmulhw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe5 },
  { "psubsb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe8 },
  { "psubsw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fe9 },
  { "pminsw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fea },
  { "por", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0feb },
  { "paddsb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fec },
  { "pxor", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0fef },
  { "psllw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff1 },
  { "pslld", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff2 },
  { "psllq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff3 },
  { "pmuludq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff4 },
  { "pmaddwd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff5 },
  { "psadbw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff6 },
  { "psubb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff8 },
  { "psubw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ff9 },
  { "psubd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ffa },
  { "paddb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ffc },
  { "paddw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ffd },
  { "paddd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, 0x66, 0x0ffe },
  /* SSE2 for SSE only */
  { "punpcklqdq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6c },
  { "punpckhqdq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6d },
  { "movdqa", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6f },
  { "movdqa", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x6f },
  { "pshufd", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x70 },
  { "movdqa", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_66, 0x7f },
  { "psrldq", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x73, 3 },
  { "pslldq", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x73, 7 },
  { "movntdq", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_66, 0xe7 },
  /* missing MASKMOVDQU 0x66 0x0FF7 */
  { "pshuflw", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F2, 0x70 },
  /* missing MOVDQ2D 0xF2 0x0FD6 */
  { "movdqu", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0x6f },
  { "movdqu", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_F3, 0x7f },
  { "pshufhw", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0x70 },
  /* missing MOVDQ2D 0xF3 0x0FD6 */
  
  /* SSE with SSE3 */
  /* SSE with SSE4.1 */
  { "packusdw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x2b },
  /* SSE with SSE4a */
  /* SSE with SSE4.2 */
};
