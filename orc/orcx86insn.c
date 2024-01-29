
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <orc/orc.h>
#include <orc/orccpuinsn.h>
#include <orc/orcx86.h>
#include <orc/orcavx.h>
#include <orc/orcsse.h>
#include <orc/orcmmx.h>

#include <orc/orcx86insn.h>


#define ORC_VEX_3_BIT 0xC4
#define ORC_VEX_2_BIT 0xC5

#define ORC_VEX_SIMD_PREFIX_NONE 0x0U
#define ORC_VEX_SIMD_PREFIX_66 0x1U
#define ORC_VEX_SIMD_PREFIX_F3 0x2U
#define ORC_VEX_SIMD_PREFIX_F2 0x3U
// Encodes a SSE instruction coming from MMX
#define ORC_SIMD_PREFIX_MMX 0xFEU
// Encodes a two-byte instruction
#define ORC_SIMD_PREFIX_ESCAPE_ONLY 0xFFU

// Escapes that require 3-byte VEX
#define ORC_VEX_ESCAPE_38 (1U << 0)
#define ORC_VEX_ESCAPE_3A (1U << 1)
// Escapes that are fake, two-byte opcode
#define ORC_USES_TWO_BYTES_OPCODE (1U << 2)
// Specifies the following REX.W bytes forcefully
#define ORC_VEX_WIG 0U
#define ORC_VEX_W0 (1U << 3)
#define ORC_VEX_W1 (1U << 5)
// Special instruction that uses 66 but not the escape 0F
#define ORC_SKIP_ESCAPE (1U << 6)

static const OrcSysOpcode orc_x86_opcodes[] = {
  { "punpcklbw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x60 },
  { "punpcklwd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x61 },
  { "punpckldq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x62 },
  { "packsswb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x63 },
  { "pcmpgtb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x64 },
  { "pcmpgtw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x65 },
  { "pcmpgtd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x66 },
  { "packuswb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x67 },
  { "punpckhbw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x68 },
  { "punpckhwd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x69 },
  { "punpckhdq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6a },
  { "packssdw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6b },
  { "punpcklqdq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6c },
  { "punpckhqdq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6d },
  { "movdqa", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x6f },
  { "psraw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe1 },
  { "psrlw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd1 },
  { "psllw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf1 },
  { "psrad", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe2 },
  { "psrld", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd2 },
  { "pslld", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf2 },
  { "psrlq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd3 },
  { "psllq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf3 },
  { "psrldq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x73 },
  { "pslldq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x73 },
  { "psrlq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd3 },
  { "pcmpeqb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x74 },
  { "pcmpeqw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x75 },
  { "pcmpeqd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0x76 },
  { "paddq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd4 },
  { "pmullw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd5 },
  { "psubusb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd8 },
  { "psubusw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xd9 },
  { "pminub", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xda },
  { "pand", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xdb },
  { "paddusb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xdc },
  { "paddusw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xdd },
  { "pmaxub", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xde },
  { "pandn", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xdf },
  { "pavgb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe0 },
  { "pavgw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe3 },
  { "pmulhuw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe4 },
  { "pmulhw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe5 },
  { "psubsb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe8 },
  { "psubsw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xe9 },
  { "pminsw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xea },
  { "por", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xeb },
  { "paddsb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xec },
  { "paddsw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xed },
  { "pmaxsw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xee },
  { "pxor", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xef },
  { "pmuludq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf4 },
  { "pmaddwd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf5 },
  { "psadbw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf6 },
  { "psubb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf8 },
  { "psubw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xf9 },
  { "psubd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xfa },
  { "psubq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xfb },
  { "paddb", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xfc },
  { "paddw", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xfd },
  { "paddd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_MMX, 0xfe },
  { "pshufb", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x00 },
  { "phaddw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x01 },
  { "phaddd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x02 },
  { "phaddsw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x03 },
  { "pmaddubsw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x04 },
  { "phsubw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x05 },
  { "phsubd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x06 },
  { "phsubsw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x07 },
  { "psignb", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x08 },
  { "psignw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x09 },
  { "psignd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x0a },
  { "pmulhrsw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x0b },
  { "pabsb", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x1c },
  { "pabsw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x1d },
  { "pabsd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x1e },
  { "pmovsxbw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x20 },
  { "pmovsxbd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x21 },
  { "pmovsxbq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x22 },
  { "pmovsxwd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x23 },
  { "pmovsxwq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x24 },
  { "pmovsxdq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x25 },
  { "pmuldq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x28 },
  { "pcmpeqq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x29 },
  { "packusdw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x2b },
  { "pmovzxbw", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x30 },
  { "pmovzxbd", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x31 },
  { "pmovzxbq", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x32 },
  { "pmovzxwd", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x33 },
  { "pmovzxwq", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x34 },
  { "pmovzxdq", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x35 },
  { "pmulld", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x40 },
  { "phminposuw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x41 },
  { "pminsb", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x38 },
  { "pminsd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x39 },
  { "pminuw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x3a },
  { "pminud", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x3b },
  { "pmaxsb", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x3c },
  { "pmaxsd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x3d },
  { "pmaxuw", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x3e },
  { "pmaxud", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x3f },
  { "pcmpgtq", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_SIMD_PREFIX_MMX, 0x37 },
  { "addps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x58 },
  { "subps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x5c },
  { "mulps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x59 },
  { "divps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x5e },
  { "sqrtps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x51 },
  { "addpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x58 },
  { "subpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x5c },
  { "mulpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x59 },
  { "divpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x5e },
  { "sqrtpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x51 },
  { "cmpeqps", ORC_X86_INSN_TYPE_SSEM_SSE, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xc2, 0 },
  { "cmpeqpd", ORC_X86_INSN_TYPE_SSEM_SSE, 0, ORC_VEX_SIMD_PREFIX_66, 0xc2, 0 },
  { "cmpltps", ORC_X86_INSN_TYPE_SSEM_SSE, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xc2, 1 },
  { "cmpltpd", ORC_X86_INSN_TYPE_SSEM_SSE, 0, ORC_VEX_SIMD_PREFIX_66, 0xc2, 1 },
  { "cmpleps", ORC_X86_INSN_TYPE_SSEM_SSE, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xc2, 2 },
  { "cmplepd", ORC_X86_INSN_TYPE_SSEM_SSE, 0, ORC_VEX_SIMD_PREFIX_66, 0xc2, 2 },
  { "cvttps2dq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0x5b },
  { "cvttpd2dq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0xe6 },
  { "cvtdq2ps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x5b },
  { "cvtdq2pd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0xe6 },
  { "cvtps2pd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x5a },
  { "cvtpd2ps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x5a },
  { "minps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x5d },
  { "minpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x5d },
  { "maxps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x5f },
  { "maxpd", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x5f },
  { "psraw", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x71, 4 },
  { "psrlw", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x71, 2 },
  { "psllw", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x71, 6 },
  { "psrad", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x72, 4 },
  { "psrld", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x72, 2 },
  { "pslld", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x72, 6 },
  { "psrlq", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x73, 2 },
  { "psllq", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x73, 6 },
  { "psrldq", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x73, 3 },
  { "pslldq", ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT, 0, ORC_SIMD_PREFIX_MMX, 0x73, 7 },
  { "pshufd", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x70 },
  { "pshuflw", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F2, 0x70 },
  { "pshufhw", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0x70 },
  { "palignr", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x0f },
  { "pinsrb", ORC_X86_INSN_TYPE_IMM8_REGM_MMX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x20 },
  { "pinsrw", ORC_X86_INSN_TYPE_IMM8_REGM_MMX, ORC_VEX_W0, ORC_SIMD_PREFIX_MMX, 0xc4 },
  { "movd", ORC_X86_INSN_TYPE_REGM_MMX, ORC_VEX_W0, ORC_SIMD_PREFIX_MMX, 0x6e },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0x7e },
  { "movdqa", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_66, 0x6f },
  { "movdqu", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_VEX_SIMD_PREFIX_F3, 0x6f },
  { "movhps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x16 },
  { "pextrb", ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_SIMD_PREFIX_MMX, 0x14 },
  { "pextrw", ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_SIMD_PREFIX_MMX, 0x15 },
  { "movd", ORC_X86_INSN_TYPE_MMX_REGM_REV, ORC_VEX_W0, ORC_SIMD_PREFIX_MMX, 0x7e },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_66, 0xd6 },
  { "movdqa", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_66, 0x7f },
  { "movdqu", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_F3, 0x7f },
  { "movntdq", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_VEX_SIMD_PREFIX_66, 0xe7 },
  { "ldmxcsr", ORC_X86_INSN_TYPE_MEM, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xae, 2 },
  { "stmxcsr", ORC_X86_INSN_TYPE_MEM, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xae, 3 },
  { "add", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 0 },
  { "add", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 0 },
  { "add", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x03 },
  { "add", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x01 },
  { "or", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 1 },
  { "or", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 1 },
  { "or", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x0b },
  { "or", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x09 },
  { "adc", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 2 },
  { "adc", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 2 },
  { "adc", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x13 },
  { "adc", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x11 },
  { "sbb", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 3 },
  { "sbb", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 3 },
  { "sbb", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x1b },
  { "sbb", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x19 },
  { "and", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 4 },
  { "and", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 4 },
  { "and", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x23 },
  { "and", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x21 },
  { "sub", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 5 },
  { "sub", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 5 },
  { "sub", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x2b },
  { "sub", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x29 },
  { "xor", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 6 },
  { "xor", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 6 },
  { "xor", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x33 },
  { "xor", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x31 },
  { "cmpb", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x83, 7 },
  { "cmpd", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x81, 7 },
  { "cmp", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x3b },
  { "cmp", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x39 },
  { "jo", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x70 },
  { "jno", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x71 },
  { "jc", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x72 },
  { "jnc", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x73 },
  { "jz", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x74 },
  { "jnz", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x75 },
  { "jbe", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x76 },
  { "ja", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x77 },
  { "js", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x78 },
  { "jns", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x79 },
  { "jp", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x7a },
  { "jnp", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x7b },
  { "jl", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x7c },
  { "jge", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x7d },
  { "jle", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x7e },
  { "jg", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x7f },
  { "jmp", ORC_X86_INSN_TYPE_BRANCH, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xeb },
  { "", ORC_X86_INSN_TYPE_LABEL, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x00 },
  { "ret", ORC_X86_INSN_TYPE_NONE, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xc3 },
  { "retq", ORC_X86_INSN_TYPE_NONE, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xc3 },
  { "emms", ORC_X86_INSN_TYPE_NONE, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x77 },
  { "rdtsc", ORC_X86_INSN_TYPE_NONE, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x31 },
  { "nop", ORC_X86_INSN_TYPE_NONE, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x90 },
  { "rep movsb", ORC_X86_INSN_TYPE_NONE, 0, ORC_VEX_SIMD_PREFIX_F3, 0xa4 },
  { "rep movsw", ORC_X86_INSN_TYPE_NONE, 0, ORC_VEX_SIMD_PREFIX_66, 0xf3a5 },
  { "rep movsl", ORC_X86_INSN_TYPE_NONE, 0, ORC_VEX_SIMD_PREFIX_F3, 0xa5 },
  { "push", ORC_X86_INSN_TYPE_STACK, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x50 },
  { "pop", ORC_X86_INSN_TYPE_STACK, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x58 },
  { "movzx", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xb6 },
  { "movw", ORC_X86_INSN_TYPE_REGM_REG, ORC_SKIP_ESCAPE, ORC_VEX_SIMD_PREFIX_66, 0x8b },
  { "movl", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x8b },
  { "mov", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x8b },
  { "mov", ORC_X86_INSN_TYPE_IMM32_REGM_MOV, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xb8 },
  { "movb", ORC_X86_INSN_TYPE_REG8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x88 },
  { "movw", ORC_X86_INSN_TYPE_REG16_REGM, ORC_SKIP_ESCAPE, ORC_VEX_SIMD_PREFIX_66, 0x89 },
  { "movl", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x89 },
  { "mov", ORC_X86_INSN_TYPE_REG_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x89 },
  { "test", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x85 },
  { "testl", ORC_X86_INSN_TYPE_IMM32_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xf7, 0 },
  { "leal", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x8d },
  { "leaq", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x8d },
  { "imul", ORC_X86_INSN_TYPE_REGM_REG, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xaf },
  { "imull", ORC_X86_INSN_TYPE_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xf7, 5 },
  { "incl", ORC_X86_INSN_TYPE_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xff, 0 },
  { "decl", ORC_X86_INSN_TYPE_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xff, 1 },
  { "sar", ORC_X86_INSN_TYPE_IMM8_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xc1, 7 },
  { "sar", ORC_X86_INSN_TYPE_REGM, 0, ORC_VEX_SIMD_PREFIX_NONE, 0xd1, 7 },
  { "and", ORC_X86_INSN_TYPE_IMM32_A, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x25, 4 },
  { "", ORC_X86_INSN_TYPE_ALIGN, 0, ORC_VEX_SIMD_PREFIX_NONE, 0x00 },
  { "pshufw", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x70 },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x6f },
  { "movq", ORC_X86_INSN_TYPE_MMXM_MMX_REV, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x7f },
  { "endbr32", ORC_X86_INSN_TYPE_NONE, ORC_USES_TWO_BYTES_OPCODE, ORC_VEX_SIMD_PREFIX_F3, 0x1efb },
  { "endbr64", ORC_X86_INSN_TYPE_NONE, ORC_USES_TWO_BYTES_OPCODE, ORC_VEX_SIMD_PREFIX_F3, 0x1efa },

  { "shufps", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0xC6 },
  { "insertf128", ORC_X86_INSN_TYPE_IMM8_SSEM_AVX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x18 },
  { "extractf128", ORC_X86_INSN_TYPE_IMM8_AVX_SSEM, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x19 },
  { "perm2f128", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x06 },
  { "pbroadcastb", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_W0 | ORC_VEX_ESCAPE_38, ORC_VEX_SIMD_PREFIX_66, 0x78 },
  { "pbroadcastw", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_W0 | ORC_VEX_ESCAPE_38, ORC_VEX_SIMD_PREFIX_66, 0x79 },
  { "pbroadcastd", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_W0 | ORC_VEX_ESCAPE_38, ORC_VEX_SIMD_PREFIX_66, 0x58 },
  { "pbroadcastq", ORC_X86_INSN_TYPE_SSEM_AVX, ORC_VEX_W0 | ORC_VEX_ESCAPE_38, ORC_VEX_SIMD_PREFIX_66, 0x59 },
  { "zeroupper", ORC_X86_INSN_TYPE_NONE, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x77 },
  { "permq", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, ORC_VEX_W1 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x00 },
  { "blendpd", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x0d },
  { "pinsrd", ORC_X86_INSN_TYPE_IMM8_REGM_MMX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x22 },
  { "perm2i128", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x46 },
  { "pblendd", ORC_X86_INSN_TYPE_IMM8_MMXM_MMX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x02 },
  { "blendvpd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_W0 | ORC_VEX_ESCAPE_3A, ORC_VEX_SIMD_PREFIX_66, 0x4b },
  { "andps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x54 },
  { "orps", ORC_X86_INSN_TYPE_MMXM_MMX, 0, ORC_SIMD_PREFIX_ESCAPE_ONLY, 0x56 },
  { "blendvpd", ORC_X86_INSN_TYPE_MMXM_MMX, ORC_VEX_ESCAPE_38, ORC_VEX_SIMD_PREFIX_66, 0x15 },
};

static void
output_opcode (OrcCompiler *p, const OrcSysOpcode *opcode, int size,
    int src, int dest, OrcX86OpcodePrefix reg_type)
{
  ORC_ASSERT(opcode->code != 0 || opcode->prefix != 0 || opcode->flags != 0);

  switch (opcode->prefix) {
    case ORC_SIMD_PREFIX_MMX:
      if (reg_type == ORC_X86_SSE_PREFIX) {
        *p->codeptr++ = 0x66;
      }
      break;
    case ORC_VEX_SIMD_PREFIX_F2:
      *p->codeptr++ = 0xF2;
      break;
    case ORC_VEX_SIMD_PREFIX_F3:
      *p->codeptr++ = 0xF3;
      break;
    case ORC_VEX_SIMD_PREFIX_66:
      *p->codeptr++ = 0x66;
      break;
    case ORC_VEX_SIMD_PREFIX_NONE:
      break;
    case ORC_SIMD_PREFIX_ESCAPE_ONLY:
      break;
    default:
      if (opcode->prefix != 0) {
        *p->codeptr++ = opcode->prefix;
      }
      ORC_ERROR("unhandled x86 opcode prefix: %x", opcode->prefix);
      ORC_ASSERT(0);
      break;
  }
  // Emit REX prefix -- Vol. 2A 2-7
  orc_x86_emit_rex (p, size, dest, 0, src);
  // Emit opcode in big endian format -- max 3 bytes
  switch (opcode->prefix) {
    case ORC_VEX_SIMD_PREFIX_66:
      if (!(opcode->flags & ORC_SKIP_ESCAPE))
        *p->codeptr++ = 0x0F;
      break;
    case ORC_VEX_SIMD_PREFIX_F2:
    case ORC_VEX_SIMD_PREFIX_F3:
    case ORC_SIMD_PREFIX_ESCAPE_ONLY:
    case ORC_SIMD_PREFIX_MMX:
      *p->codeptr++ = 0x0F;
      break;
    case ORC_VEX_SIMD_PREFIX_NONE:
      break;
    default:
      ORC_ERROR("unhandled x86 opcode prefix: %x", opcode->prefix);
      ORC_ASSERT(0);
      break;
  }
  if (opcode->flags & ORC_VEX_ESCAPE_38) {
    *p->codeptr++ = 0x38;
  } else if (opcode->flags & ORC_VEX_ESCAPE_3A) {
    *p->codeptr++ = 0x3a;
  } else if (opcode->flags & ORC_USES_TWO_BYTES_OPCODE) {
    *p->codeptr++ = (opcode->code >> 8) & 0xff;
  }
  *p->codeptr++ = (opcode->code >> 0) & 0xff;
}

static const char *
orc_x86_get_simd_regname (int reg, OrcX86OpcodePrefix prefix)
{
  switch (prefix) {
#if ENABLE_BACKEND_AVX
    case ORC_X86_AVX_VEX256_PREFIX:
    case ORC_X86_AVX_VEX128_PREFIX:
      return orc_x86_get_regname_avx (reg, prefix);
#endif
    case ORC_X86_SSE_PREFIX:
      return orc_x86_get_regname_sse (reg);
    default:
      return orc_x86_get_regname_mmx (reg);
  }
}

static OrcX86OpcodePrefix
get_common_reg_type (OrcX86Insn *xinsn)
{
  if (xinsn->prefix != ORC_X86_NO_PREFIX) {
    return xinsn->prefix;
  }

  const int reg1 = xinsn->src[0];
  const int reg2 = xinsn->dest;

  if ((reg1 >= X86_YMM0) && (reg1 <= X86_YMM15)) {
    return ORC_X86_AVX_VEX256_PREFIX;
  } else if ((reg1 >= X86_XMM0) && (reg1 <= X86_XMM15)) {
    return ORC_X86_SSE_PREFIX;
  }

  if ((reg2 >= X86_YMM0) && (reg2 <= X86_YMM15)) {
    return ORC_X86_AVX_VEX256_PREFIX;
  } else if ((reg2 >= X86_XMM0) && (reg2 <= X86_XMM15)) {
    return ORC_X86_SSE_PREFIX;
  }

  return ORC_X86_NO_PREFIX;
}

static void
/* Output assembler code in AT&T style (opcode src, dest)*/
orc_x86_insn_output_asm (OrcCompiler *p, OrcX86Insn *xinsn)
{
  char imm_str[40] = { 0 };
  char src_op[40] = { 0 };
  char dst_op[40] = { 0 };
  char src_2nd_op[40] = { 0 };
  char src_3rd_op[40] = { 0 };

  const OrcX86OpcodePrefix is_sse = get_common_reg_type(xinsn);

  if (xinsn->opcode->type == ORC_X86_INSN_TYPE_ALIGN) {
    if (xinsn->size > 0) ORC_ASM_CODE(p,".p2align %d\n", xinsn->size);
    return;
  }
  if (xinsn->opcode->type == ORC_X86_INSN_TYPE_LABEL) {
    ORC_ASM_CODE(p,"%d:\n", xinsn->label);
    return;
  }

  // Parse immediate operand
  switch (xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_REGM_REG:
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_STACK:
    case ORC_X86_INSN_TYPE_MEM:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_NONE:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
      imm_str[0] = 0;
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM32_A:
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      sprintf(imm_str, "$%d, ", xinsn->imm);
      break;
    default:
      ORC_ERROR("Unhandled immediate operand for instruction type %d", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }

  const int operand1 = (xinsn->src[0] != 0 && xinsn->src[1] != 0) ? xinsn->src[1] : xinsn->src[0];

  // Parse source operand (if any)
  switch (xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
      if (xinsn->type == ORC_X86_RM_REG) {
        sprintf(src_op, "%%%s, ",
            orc_x86_get_simd_regname (operand1, is_sse));
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        sprintf(src_op, "%d(%%%s), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, operand1));
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        sprintf(src_op, "%d(%%%s,%%%s,%d), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, operand1),
            orc_x86_get_regname_ptr (p, xinsn->index_reg),
            1 << xinsn->shift);
      } else {
        ORC_COMPILER_ERROR(p, "Unhandled instruction type %d for source operand", xinsn->type);
        ORC_ASSERT(0);
        return;
      }
      break;
    case ORC_X86_INSN_TYPE_SSEM_AVX:
      if (xinsn->type == ORC_X86_RM_REG) {
        // This one in particular enforces SSE for the source operand
        sprintf(src_op, "%%%s, ",
            orc_x86_get_simd_regname (operand1, ORC_X86_AVX_VEX128_PREFIX));
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        sprintf(src_op, "%d(%%%s), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, operand1));
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        sprintf(src_op, "%d(%%%s,%%%s,%d), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, operand1),
            orc_x86_get_regname_ptr (p, xinsn->index_reg),
            1 << xinsn->shift);
      } else {
        ORC_COMPILER_ERROR(p, "Unhandled instruction type %d for source operand", xinsn->type);
        ORC_ASSERT(0);
        return;
      }
      break;
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV: /* FIXME misnamed */
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      if (operand1 != 0) {
        sprintf (src_op, "%%%s, ", orc_x86_get_simd_regname (operand1, is_sse));
      }
      break;
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
      if (operand1 != 0) {
        sprintf (src_op, "%%%s, ", orc_x86_get_simd_regname (operand1, ORC_X86_AVX_VEX128_PREFIX));
      }
      break;
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_REGM_REG:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
      if (xinsn->type == ORC_X86_RM_REG) {
        sprintf(src_op, "%%%s, ", orc_x86_get_regname_size (operand1,
            xinsn->size));
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        sprintf(src_op, "%d(%%%s), ", xinsn->offset,
          orc_x86_get_regname_ptr (p, operand1));
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        sprintf(src_op, "%d(%%%s,%%%s,%d), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, operand1),
            orc_x86_get_regname_ptr (p, xinsn->index_reg),
            1 << xinsn->shift);
      }  else {
        ORC_COMPILER_ERROR(p, "Unhandled instruction type %d for source operand", xinsn->type);
        ORC_ASSERT(0);
        return;
      }
      break;
    case ORC_X86_INSN_TYPE_MEM:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_STACK:
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_NONE:
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_IMM32_A:
      src_op[0] = 0;
      break;
    case ORC_X86_INSN_TYPE_REG_REGM:
      sprintf(src_op, "%%%s, ", orc_x86_get_regname (operand1));
      break;
    case ORC_X86_INSN_TYPE_REG8_REGM:
      sprintf(src_op, "%%%s, ", orc_x86_get_regname_8 (operand1));
      break;
    case ORC_X86_INSN_TYPE_REG16_REGM:
      sprintf(src_op, "%%%s, ", orc_x86_get_regname_16 (operand1));
      break;
    default:
      ORC_COMPILER_ERROR(p, "Unhandled opcode type %d for source operand", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }

  // Handle second operand
  // These must come first, if available, because GAS...
  // See https://stackoverflow.com/a/7548587 and https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=372528
  const unsigned int may_have_avx_operand = (xinsn->prefix == ORC_X86_AVX_VEX128_PREFIX || xinsn->prefix == ORC_X86_AVX_VEX256_PREFIX);

  const int operand2 = xinsn->src[0];

  if (may_have_avx_operand && xinsn->src[1] != 0) {
    switch (xinsn->opcode->type) {
      case ORC_X86_INSN_TYPE_SSEM_SSE:
      case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
      case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
      case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
        sprintf(src_2nd_op, "%%%s, ",
            orc_x86_get_simd_regname (operand2, is_sse));
        break;
      case ORC_X86_INSN_TYPE_SSEM_AVX:
      case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
        sprintf(src_2nd_op, "%%%s, ",
              orc_x86_get_simd_regname (operand2, ORC_X86_AVX_VEX128_PREFIX));
        break;
      case ORC_X86_INSN_TYPE_MMXM_MMX:
        // In all cases it can be either a pointer to, or a XMM/YMM register
        // Intel Intrinsics Manual s.2.3.9
        if (xinsn->type == ORC_X86_RM_REG) {
          sprintf(src_2nd_op, "%%%s, ",
              orc_x86_get_simd_regname (operand2, is_sse));
        } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
          sprintf(src_2nd_op, "%d(%%%s), ", xinsn->offset,
              orc_x86_get_regname_ptr (p, xinsn->src[2]));
        } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
          sprintf(src_2nd_op, "%d(%%%s,%%%s,%d), ", xinsn->offset,
              orc_x86_get_regname_ptr (p, xinsn->src[2]),
              orc_x86_get_regname_ptr (p, xinsn->index_reg),
              1 << xinsn->shift);
        } else {
          ORC_COMPILER_ERROR(p, "Unhandled instruction type %d for 4th operand", xinsn->type);
          ORC_ASSERT(0);
          return;
        }
        break;
      case ORC_X86_INSN_TYPE_REGM_MMX:
      case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
      case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
      case ORC_X86_INSN_TYPE_MMX_REGM_REV:
      case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
      case ORC_X86_INSN_TYPE_IMM8_REGM:
      case ORC_X86_INSN_TYPE_IMM32_REGM:
      case ORC_X86_INSN_TYPE_REGM:
      case ORC_X86_INSN_TYPE_REG8_REGM:
      case ORC_X86_INSN_TYPE_REG16_REGM:
      case ORC_X86_INSN_TYPE_REG_REGM:
      case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
      case ORC_X86_INSN_TYPE_REGM_REG:
      case ORC_X86_INSN_TYPE_STACK:
      case ORC_X86_INSN_TYPE_MEM:
      case ORC_X86_INSN_TYPE_BRANCH:
      case ORC_X86_INSN_TYPE_LABEL:
      case ORC_X86_INSN_TYPE_ALIGN:
      case ORC_X86_INSN_TYPE_NONE:
      case ORC_X86_INSN_TYPE_IMM32_A:
        src_2nd_op[0] = '\0';
        break;
      default:
        ORC_ERROR("%d", xinsn->opcode->type);
        ORC_ASSERT(0);
        break;
    }
  }

  if (xinsn->src[2] != 0) {
    switch (xinsn->opcode->type) {
      case ORC_X86_INSN_TYPE_MMXM_MMX:
        if (may_have_avx_operand) {
          sprintf(src_3rd_op, "%%%s, ",
                orc_x86_get_simd_regname (xinsn->src[2], is_sse));
        } else {
          ORC_COMPILER_ERROR(p, "Blends on SSE require XMM0 as the mask, this cannot be guaranteed");
          ORC_ASSERT(0);
          return;
        }
        break;
      default:
        ORC_COMPILER_ERROR(p, "Unhandled instruction type %d for 4th operand", xinsn->type);
        ORC_ASSERT(0);
        return;
    }
  }

  // Handle destinations
  switch (xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
      sprintf(dst_op, "%%%s",
            orc_x86_get_simd_regname (xinsn->dest, is_sse));
      break;
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
      if (xinsn->type == ORC_X86_RM_REG) {
        sprintf(dst_op, "%%%s",
            orc_x86_get_simd_regname (xinsn->dest, is_sse));
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        sprintf(dst_op, "%d(%%%s)", xinsn->offset,
            orc_x86_get_regname_ptr (p, xinsn->dest));
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        sprintf(dst_op, "%d(%%%s,%%%s,%d), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, xinsn->dest),
            orc_x86_get_regname_ptr (p, xinsn->index_reg),
            1 << xinsn->shift);
      } else {
        ORC_ERROR("%d", xinsn->opcode->type);
	      ORC_ASSERT(0);
      }
      break;
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      sprintf(dst_op, "%%%s",
          orc_x86_get_simd_regname (xinsn->dest, ORC_X86_AVX_VEX128_PREFIX));
      break;
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
      if (xinsn->type == ORC_X86_RM_REG) {
        sprintf(dst_op, "%%%s", orc_x86_get_regname (xinsn->dest));
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        sprintf(dst_op, "%d(%%%s)", xinsn->offset,
            orc_x86_get_regname_ptr (p, xinsn->dest));
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        sprintf(dst_op, "%d(%%%s,%%%s,%d), ", xinsn->offset,
            orc_x86_get_regname_ptr (p, xinsn->dest),
            orc_x86_get_regname_ptr (p, xinsn->index_reg),
            1 << xinsn->shift);
      } else {
        ORC_ERROR("%d", xinsn->opcode->type);
	ORC_ASSERT(0);
      }
      break;
    case ORC_X86_INSN_TYPE_REGM_REG:
    case ORC_X86_INSN_TYPE_STACK:
      sprintf(dst_op, "%%%s", orc_x86_get_regname_size (xinsn->dest,
	  xinsn->size));
      break;
    case ORC_X86_INSN_TYPE_MEM:
      if (xinsn->type == ORC_X86_RM_REG) {
        ORC_ERROR("register operand on memory instruction");
        sprintf(dst_op, "ERROR");
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
	/* FIXME: this uses xinsn->src[0] */
        sprintf(dst_op, "%d(%%%s)", xinsn->offset,
            orc_x86_get_regname_ptr (p, xinsn->src[0]));
      } else {
        ORC_ERROR("%d", xinsn->opcode->type);
	ORC_ASSERT(0);
      }
      break;
    case ORC_X86_INSN_TYPE_BRANCH:
      sprintf (dst_op, "%d%c", xinsn->label,
          (p->labels_int[xinsn->label] <
           xinsn - ((OrcX86Insn *)p->output_insns)) ? 'b' : 'f');
      break;
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_NONE:
      dst_op[0] = 0;
      break;
    case ORC_X86_INSN_TYPE_IMM32_A:
      sprintf(dst_op, "%%%s", orc_x86_get_regname_size (X86_EAX, xinsn->size));
      break;
    default:
      ORC_ERROR("%d", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }

  if (xinsn->prefix == ORC_X86_AVX_VEX128_PREFIX || xinsn->prefix == ORC_X86_AVX_VEX256_PREFIX) {
    ORC_ASM_CODE(p,"  v%s %s%s%s%s%s\n", xinsn->opcode->name,
        imm_str, src_op, src_2nd_op, src_3rd_op, dst_op);
  } else {
    ORC_ASM_CODE(p,"  %s %s%s%s\n", xinsn->opcode->name,
        imm_str, src_op, dst_op);
  }
}

static const orc_uint8 nop_codes[][16] = {
  { 0 /* MSVC wants something here */ },
  { 0x90 },
  { 0x66, 0x90 }, /* xchg %ax,%ax */
#if 0
  { 0x0f, 0x1f, 0x00 }, /*  nopl (%rax) */
  { 0x0f, 0x1f, 0x40, 0x00 }, /* nopl 0x0(%rax) */
  { 0x0f, 0x1f, 0x44, 0x00, 0x00 }, /* nopl 0x0(%rax,%rax,1) */
  { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 }, /* nopw 0x0(%rax,%rax,1) */
  { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 }, /* nopl 0x0(%rax) */
  { 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* nopl 0x0(%rax,%rax,1) */
  { 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* nopw   0x0(%rax,%rax,1) */
  /* Forms of nopw %cs:0x0(%rax,%rax,1) */
  { 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x66, 0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,
    0x00 },
  { 0x66, 0x66, 0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
    0x00, 0x00 },
  { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00,
    0x00, 0x00, 0x00 },
#else
  { 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, },
  { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, },
#endif
};

static void
orc_x86_insn_output_opcode (OrcCompiler *p, OrcX86Insn *xinsn)
{
  const OrcX86OpcodePrefix is_sse = get_common_reg_type (xinsn);

  switch (xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
      output_opcode (p, xinsn->opcode, 4, xinsn->dest, 0, is_sse);
      break;
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
      output_opcode (p, xinsn->opcode, 4, xinsn->dest, xinsn->src[0], is_sse);
      break;
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
      output_opcode (p, xinsn->opcode, xinsn->size, xinsn->dest, xinsn->src[0], ORC_X86_NO_PREFIX);
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
      output_opcode (p, xinsn->opcode, 4, xinsn->src[0], xinsn->dest, is_sse);
      break;
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
      output_opcode (p, xinsn->opcode, xinsn->size, xinsn->src[0], xinsn->dest,
          is_sse);
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
      output_opcode (p, xinsn->opcode, 4, xinsn->dest, xinsn->src[0],
          is_sse);
      break;
    case ORC_X86_INSN_TYPE_MEM:
    case ORC_X86_INSN_TYPE_REGM_REG:
      output_opcode (p, xinsn->opcode, xinsn->size, xinsn->src[0], xinsn->dest, ORC_X86_NO_PREFIX);
      break;
    case ORC_X86_INSN_TYPE_REGM:
      output_opcode (p, xinsn->opcode, xinsn->size, xinsn->src[0], xinsn->dest, ORC_X86_NO_PREFIX);
      break;
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
      orc_x86_emit_rex (p, xinsn->size, 0, 0, xinsn->dest);
      *p->codeptr++ = xinsn->opcode->code + (xinsn->dest&7);
      break;
    case ORC_X86_INSN_TYPE_NONE:
      output_opcode (p, xinsn->opcode, 4, 0, 0, FALSE);
      break;
    case ORC_X86_INSN_TYPE_IMM32_A:
      output_opcode (p, xinsn->opcode, xinsn->size, 0, 0, FALSE);
      break;
    case ORC_X86_INSN_TYPE_ALIGN:
      {
        const int diff = (p->code - p->codeptr) & ((1 << xinsn->size) - 1);
        for (int i = 0; i < diff; i++) {
          *p->codeptr++ = nop_codes[diff][i];
        }
      }
      break;
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_STACK:
      break;
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      ORC_COMPILER_ERROR (p, "AVX-only instruction type %d cannot be codegen'd without VEX", xinsn->opcode->type);
      break;
    default:
      ORC_ERROR("Unhandled opcode type %d for machine language codegen", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }
}

static void
orc_x86_insn_output_modrm (OrcCompiler *const p, const OrcX86Insn *const xinsn)
{
  switch (xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_REGM_REG:
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
      if (xinsn->type == ORC_X86_RM_REG) {
        orc_x86_emit_modrm_reg (p, xinsn->src[0], xinsn->dest);
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[0],
            xinsn->dest);
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[0],
            xinsn->index_reg, xinsn->shift, xinsn->dest);
      } else {
        ORC_COMPILER_ERROR (p, "Unhandled op type %d for modr/m", xinsn->type);
        ORC_ASSERT (0);
        break;
      }
      break;
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      if (xinsn->type == ORC_X86_RM_REG) {
        orc_x86_emit_modrm_reg (p, xinsn->dest, xinsn->src[0]);
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->dest,
            xinsn->src[0]);
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->dest,
            xinsn->index_reg, xinsn->shift, xinsn->src[0]);
      } else {
        ORC_COMPILER_ERROR (p, "Unhandled op type %d for modr/m", xinsn->type);
        ORC_ASSERT (0);
        break;
      }
      break;
    case ORC_X86_INSN_TYPE_MEM:
      if (xinsn->type == ORC_X86_RM_REG) {
        orc_x86_emit_modrm_reg (p, xinsn->src[0], xinsn->opcode->code2);
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[0],
            xinsn->opcode->code2);
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[0],
            xinsn->index_reg, xinsn->shift, xinsn->opcode->code2);
      } else {
        ORC_COMPILER_ERROR (p, "Unhandled op type %d for modr/m", xinsn->type);
        ORC_ASSERT (0);
        break;
      }
      break;
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM32_A:
    case ORC_X86_INSN_TYPE_NONE:
    case ORC_X86_INSN_TYPE_ALIGN:
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_REGM:
      if (xinsn->type == ORC_X86_RM_REG) {
        orc_x86_emit_modrm_reg (p, xinsn->dest, xinsn->opcode->code2);
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->dest,
            xinsn->opcode->code2);
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->dest,
            xinsn->index_reg, xinsn->shift, xinsn->opcode->code2);
      } else {
        ORC_COMPILER_ERROR (p, "Unhandled op type %d for modr/m", xinsn->type);
        ORC_ASSERT (0);
        break;
      }
      break;
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
      if (xinsn->type == ORC_X86_RM_REG) {
        orc_x86_emit_modrm_reg (p, xinsn->src[0], xinsn->dest);
      } else if (xinsn->type == ORC_X86_RM_MEMOFFSET) {
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[0],
            xinsn->dest);
      } else if (xinsn->type == ORC_X86_RM_MEMINDEX) {
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[0],
            xinsn->index_reg, xinsn->shift, xinsn->dest);
      } else {
        ORC_COMPILER_ERROR (p, "Unhandled op type %d for modr/m", xinsn->type);
        ORC_ASSERT (0);
        break;
      }
      *p->codeptr++ = xinsn->opcode->code2;
      break;
    case ORC_X86_INSN_TYPE_STACK:
      *p->codeptr++ = xinsn->opcode->code + (xinsn->dest&0x7);
      break;
    case ORC_X86_INSN_TYPE_BRANCH:
      if (xinsn->size == 4) {
        if (xinsn->opcode_index == ORC_X86_jmp) {
          *p->codeptr++ = 0xe9;
        } else {
          *p->codeptr++ = 0x0f;
          *p->codeptr++ = xinsn->opcode->code + 0x10;
        }
      } else {
        *p->codeptr++ = xinsn->opcode->code;
      }

      if (xinsn->size == 4) {
        x86_add_fixup (p, p->codeptr, xinsn->label, 1);
        *p->codeptr++ = 0xfc;
        *p->codeptr++ = 0xff;
        *p->codeptr++ = 0xff;
        *p->codeptr++ = 0xff;
      } else {
        x86_add_fixup (p, p->codeptr, xinsn->label, 0);
        *p->codeptr++ = 0xff;
      }
      break;
    case ORC_X86_INSN_TYPE_LABEL:
      x86_add_label (p, p->codeptr, xinsn->label);
      break;
    default:
      ORC_COMPILER_ERROR (p, "Unhandled machine language codegen for opcode type %d", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }
}

static void
orc_x86_insn_output_immediate (OrcCompiler *const p, const OrcX86Insn *const xinsn)
{
  switch (xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_REGM_REG:
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
      *p->codeptr++ = xinsn->imm;
      break;
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_IMM32_A:
      *p->codeptr++ = xinsn->imm&0xff;
      *p->codeptr++ = (xinsn->imm>>8)&0xff;
      *p->codeptr++ = (xinsn->imm>>16)&0xff;
      *p->codeptr++ = (xinsn->imm>>24)&0xff;
      break;
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_STACK:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_MEM:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_NONE:
      break;
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      ORC_COMPILER_ERROR (p, "AVX-only instruction type %d cannot be codegen'd without VEX", xinsn->opcode->type);
      break;
    default:
      ORC_COMPILER_ERROR (p, "FIXME: unhandled immediate operand codegen for opcode type %i", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }
}

OrcX86Insn *
orc_x86_get_output_insn (OrcCompiler *p)
{
  OrcX86Insn *xinsn;
  if (p->n_output_insns >= p->n_output_insns_alloc) {
    p->n_output_insns_alloc += 10;
    p->output_insns = realloc (p->output_insns,
        sizeof(OrcX86Insn) * p->n_output_insns_alloc);
  }

  xinsn = ((OrcX86Insn *)p->output_insns) + p->n_output_insns;
  memset (xinsn, 0, sizeof(OrcX86Insn));
  p->n_output_insns++;
  return xinsn;
}

static void
orc_vex_insn_output_modrm (OrcCompiler *const p, const OrcX86Insn *const xinsn)
{
  if (xinsn->src[0] != 0 && xinsn->src[1] != 0) {
    switch ((OrcX86InsnType)xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
      switch(xinsn->type) {
        case ORC_X86_RM_REG:
          orc_x86_emit_modrm_reg (p, xinsn->src[1], xinsn->dest);
          break;
        case ORC_X86_RM_MEMOFFSET:
          orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[1],
            xinsn->dest);
          break;
        case ORC_X86_RM_MEMINDEX:
          orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[1],
              xinsn->index_reg, xinsn->shift, xinsn->dest);
          break;
        default:
          ORC_COMPILER_ERROR (p, "Unhandled operand type %d for modr/m", xinsn->type);
          ORC_ASSERT (0);
          break;
      }
      break;
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      switch(xinsn->type) {
        case ORC_X86_RM_REG:
          orc_x86_emit_modrm_reg (p, xinsn->dest, xinsn->src[1]);
          break;
        case ORC_X86_RM_MEMOFFSET:
          orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->dest,
            xinsn->src[1]);
          break;
        case ORC_X86_RM_MEMINDEX:
          orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->dest,
            xinsn->index_reg, xinsn->shift, xinsn->src[1]);
          break;
        default:
          ORC_COMPILER_ERROR (p, "Unhandled operand type %d for modr/m", xinsn->type);
          ORC_ASSERT (0);
          break;
      }
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
      switch(xinsn->type) {
        case ORC_X86_RM_REG:
          orc_x86_emit_modrm_reg (p, xinsn->src[0], xinsn->opcode->code2);
          break;
        case ORC_X86_RM_MEMOFFSET:
          orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[0],
            xinsn->opcode->code2);
          break;
        case ORC_X86_RM_MEMINDEX:
          orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[0],
            xinsn->index_reg, xinsn->shift, xinsn->opcode->code2);
          break;
        default:
          ORC_COMPILER_ERROR (p, "Unhandled operand type %d for modr/m", xinsn->type);
          ORC_ASSERT (0);
          break;
      }
      break;
    case ORC_X86_INSN_TYPE_SSEM_SSE:
      switch(xinsn->type) {
        case ORC_X86_RM_REG:
          orc_x86_emit_modrm_reg (p, xinsn->src[1], xinsn->dest);
          break;
        case ORC_X86_RM_MEMOFFSET:
          orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[1],
            xinsn->dest);
          break;
        case ORC_X86_RM_MEMINDEX:
          orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[1],
            xinsn->index_reg, xinsn->shift, xinsn->dest);
          break;
        default:
          ORC_COMPILER_ERROR (p, "Unhandled operand type %d for modr/m", xinsn->type);
          ORC_ASSERT (0);
          break;
      };
      *p->codeptr++ = xinsn->opcode->code2;
      break;
    case ORC_X86_INSN_TYPE_NONE:
      break;
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
    case ORC_X86_INSN_TYPE_MEM:
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM32_A:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_STACK:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_REGM_REG:
      ORC_COMPILER_ERROR (p,
          "Instruction type %d cannot be codegen'd with VEX",
          xinsn->opcode->type);
      ORC_ASSERT (0);
      break;
    default:
      ORC_COMPILER_ERROR (p,
          "Unhandled machine language codegen for opcode type %d",
          xinsn->opcode->type);
      ORC_ASSERT (0);
      break;
    }
  } else if (xinsn->opcode->type == ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT) {
    switch(xinsn->type) {
      case ORC_X86_RM_REG:
        orc_x86_emit_modrm_reg (p, xinsn->src[0], xinsn->opcode->code2);
        break;
      case ORC_X86_RM_MEMOFFSET:
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->src[0],
          xinsn->opcode->code2);
        break;
      case ORC_X86_RM_MEMINDEX:
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->src[0],
          xinsn->index_reg, xinsn->shift, xinsn->opcode->code2);
        break;
      default:
        ORC_COMPILER_ERROR (p, "Unhandled operand type %d for shift extended modr/m", xinsn->type);
        ORC_ASSERT (0);
        break;
    }
  } else {
    orc_x86_insn_output_modrm (p, xinsn);
    return;
  }
}

static void
orc_vex_insn_output_immediate (OrcCompiler *const p, const OrcX86Insn *const xinsn)
{
  switch ((OrcX86InsnType)xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_MMXM_MMX:
      switch (xinsn->opcode_index) {
        // Complete here with VPBLENDVB and VBLENDVPS when implemented
        // Intel Intrinsics Manual s.2.3.9
        case ORC_X86_blendvpd_avx:
          *p->codeptr++ = (xinsn->src[2] & 0xF) << 4;
        default:
          break;
      }
      break;
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
      break;
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
      *p->codeptr++ = xinsn->imm;
      break;
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_MEM:
    case ORC_X86_INSN_TYPE_NONE:
      break;
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM32_A:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_STACK:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_REGM_REG:
      ORC_COMPILER_ERROR (p,
          "Instruction type %d cannot be codegen'd with VEX",
          xinsn->opcode->type);
      ORC_ASSERT (0);
      break;
    default:
      ORC_COMPILER_ERROR (p,
          "Unhandled immediate operand codegen for opcode type %i",
          xinsn->opcode->type);
      ORC_ASSERT (0);
      break;
  }
}

static unsigned char
orc_vex_get_rex (OrcCompiler *compiler, int reg1, int reg2, int reg3)
{
  // In protected and compatibility modes the bit must be set to ‘1’
  // otherwise the instruction is LES or LDS.
  unsigned char rex = 0x80 | 0x40 | 0x20;

  if (compiler->is_64bit) {
    if (reg1 & 8) rex &= ~0x80; // modr/m.reg needs access to 64-bit mode operands
    if (reg2 & 8) rex &= ~0x40; // SIB index field extension
    if (reg3 & 8) rex &= ~0x20; // ModR/M r/m field, SIB base field, or Opcode reg field
  }

  return rex;
}

#define ORC_VEX_vvvv_UNUSED 0xF << 3
#define ORC_VEX_vvvv_MASK 0xF << 3

static unsigned char
get_vex_vvvv (OrcCompiler *p, const OrcX86Insn *const xinsn)
{
  // Section 2.3.5.6, 2.3.6
  // Instruction Operand Encoding and VEX.vvvv, ModR/M
  if (xinsn->src[0] != 0 && xinsn->src[1] == 0) {
    // VEX.vvvv is not used by instructions with one source (except
    // certain shifts, see below);
    switch (xinsn->opcode_index) {
      case ORC_X86_pslldq_imm:
      case ORC_X86_psrldq_imm:
      case ORC_X86_psrlq_imm:
      case ORC_X86_psrlw_imm:
      case ORC_X86_psrld_imm:
      case ORC_X86_psraw_imm:
      case ORC_X86_psrad_imm:
      case ORC_X86_psllw_imm:
      case ORC_X86_pslld_imm:
      case ORC_X86_psllq_imm:
        return (~xinsn->dest & 0xF) << 3 & ORC_VEX_vvvv_MASK;
      default:
        // If an instruction does not use VEX.vvvv then it should be set to
        // 1111b otherwise instruction will #UD.
        return ORC_VEX_vvvv_UNUSED;
    }
  } else if (xinsn->src[0] != 0 && xinsn->src[1] != 0 /* && (is_sse_destination || uses_memory) */) {
    // Get the 1-complement of the register (we always use the lower four bits)
    // And mask it appropriately
    return (~xinsn->src[0] & 0xF) << 3 & ORC_VEX_vvvv_MASK;
  } else if (xinsn->opcode->type == ORC_X86_INSN_TYPE_NONE) {
    return ORC_VEX_vvvv_UNUSED;
  } else {
    ORC_COMPILER_ERROR (p, "Inconsistent operand state for instruction (%i, %i) -> %i", xinsn->src[0], xinsn->src[1], xinsn->dest);
    ORC_ASSERT (0);
    return ORC_VEX_vvvv_UNUSED;
  }
}

static void
output_3byte_vex_opcode (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  // ORC_DEBUG("generating 3-byte %x code for %s (%i, %i) -> %i", xinsn->prefix, xinsn->opcode->name, xinsn->src[0], xinsn->src[1], xinsn->dest);

  // In protected and compatibility modes the bit must be set to ‘1’ otherwise
  // the instruction is LES or LDS.
  unsigned char byte2 = 0;

  // Handle flags
  if (xinsn->opcode->flags & ORC_VEX_ESCAPE_38) {
    byte2 |= 0x2;
  } else if (xinsn->opcode->flags & ORC_VEX_ESCAPE_3A) {
    byte2 |= 0x3;
  } else {
    switch (xinsn->opcode->prefix) {
      case ORC_VEX_SIMD_PREFIX_F2:
      case ORC_VEX_SIMD_PREFIX_F3:
      case ORC_VEX_SIMD_PREFIX_66:
      case ORC_SIMD_PREFIX_MMX:
      case ORC_SIMD_PREFIX_ESCAPE_ONLY:
        byte2 |= 0x1; 
        break;
      case ORC_VEX_SIMD_PREFIX_NONE:
        break;
      default:
        ORC_COMPILER_ERROR(p, "unhandled VEX.mmmm for instruction type %x", xinsn->opcode->prefix);
        ORC_ASSERT(0);
        return;
    }
  }

  if (p->is_64bit) {
    // As I understand it, Orc does not use SIB
    // because it disallows ESP indexing.
    if (xinsn->src[0] != 0 && xinsn->src[1] == 0) {
      switch (xinsn->opcode->type) {
        case ORC_X86_INSN_TYPE_REGM_REG:
        case ORC_X86_INSN_TYPE_REGM_MMX:
        case ORC_X86_INSN_TYPE_MMXM_MMX:
        case ORC_X86_INSN_TYPE_SSEM_AVX:
        case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
        case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
          byte2 |= orc_vex_get_rex (p, xinsn->dest, 0, xinsn->src[0]);
          break;
        case ORC_X86_INSN_TYPE_REG_REGM:
        case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
        case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
        case ORC_X86_INSN_TYPE_MMX_REGM_REV:
        case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
        case ORC_X86_INSN_TYPE_REG8_REGM:
        case ORC_X86_INSN_TYPE_REG16_REGM:
          byte2 |= orc_vex_get_rex (p, xinsn->src[0], 0, xinsn->dest);
          break;
        case ORC_X86_INSN_TYPE_MEM:
          byte2 |= orc_vex_get_rex (p, xinsn->src[0], 0, 0);
          break;
        case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
        case ORC_X86_INSN_TYPE_IMM32_A:
        case ORC_X86_INSN_TYPE_NONE:
        case ORC_X86_INSN_TYPE_ALIGN:
          break;
        case ORC_X86_INSN_TYPE_IMM8_REGM:
        case ORC_X86_INSN_TYPE_IMM32_REGM:
        case ORC_X86_INSN_TYPE_REGM:
          byte2 |= orc_vex_get_rex (p, xinsn->dest, 0, 0);
          break;
        case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
          byte2 |= orc_vex_get_rex (p, 0, 0, xinsn->src[0]);
          break;
        case ORC_X86_INSN_TYPE_SSEM_SSE:
        case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
          byte2 |= orc_vex_get_rex (p, xinsn->dest, 0, xinsn->src[0]);
          break;
        case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
          ORC_COMPILER_ERROR (p, "Invalid VEX.RXB language codegen for opcode type %d", xinsn->opcode->type);
          ORC_ASSERT(0);
          break;
        case ORC_X86_INSN_TYPE_STACK:
        case ORC_X86_INSN_TYPE_BRANCH:
        case ORC_X86_INSN_TYPE_LABEL:
          break;
        default:
          ORC_COMPILER_ERROR (p, "Unhandled VEX.RXB language codegen for opcode type %d", xinsn->opcode->type);
          ORC_ASSERT(0);
          break;
      }
    } else if (xinsn->src[0] != 0 && xinsn->src[1] != 0) {
      switch ((OrcX86InsnType)xinsn->opcode->type) {
      case ORC_X86_INSN_TYPE_REGM_MMX:
      case ORC_X86_INSN_TYPE_MMXM_MMX:
      case ORC_X86_INSN_TYPE_SSEM_AVX:
      case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
      case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
      case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
        switch(xinsn->type) {
          case ORC_X86_RM_REG:
          case ORC_X86_RM_MEMINDEX:
            byte2 |= orc_vex_get_rex (p, xinsn->dest, 0, xinsn->src[1]);
            break;
          case ORC_X86_RM_MEMOFFSET:
            byte2 |= orc_vex_get_rex (p, xinsn->dest, 0, xinsn->src[1]);
            break;
          default:
            ORC_COMPILER_ERROR (p, "Unhandled operand type %d for modr/m", xinsn->type);
            ORC_ASSERT (0);
            break;
        }
        break;
      case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
      case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
      case ORC_X86_INSN_TYPE_MMX_REGM_REV:
      case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
      case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
        byte2 |= orc_vex_get_rex (p, xinsn->dest, 0, xinsn->src[1]);
        break;
      case ORC_X86_INSN_TYPE_SSEM_SSE:
        byte2 |= orc_vex_get_rex (p, xinsn->src[1], 0, xinsn->dest);
        break;
      case ORC_X86_INSN_TYPE_NONE:
        break;
      case ORC_X86_INSN_TYPE_REG_REGM:
      case ORC_X86_INSN_TYPE_REG8_REGM:
      case ORC_X86_INSN_TYPE_REG16_REGM:
      case ORC_X86_INSN_TYPE_MEM:
      case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
      case ORC_X86_INSN_TYPE_IMM32_A:
      case ORC_X86_INSN_TYPE_ALIGN:
      case ORC_X86_INSN_TYPE_IMM8_REGM:
      case ORC_X86_INSN_TYPE_IMM32_REGM:
      case ORC_X86_INSN_TYPE_REGM:
      case ORC_X86_INSN_TYPE_STACK:
      case ORC_X86_INSN_TYPE_BRANCH:
      case ORC_X86_INSN_TYPE_LABEL:
      case ORC_X86_INSN_TYPE_REGM_REG:
        ORC_COMPILER_ERROR (p,
            "Instruction type %d cannot be codegen'd with VEX",
            xinsn->opcode->type);
        ORC_ASSERT (0);
        break;
      default:
        ORC_COMPILER_ERROR (p, "Unhandled VEX.RXB language codegen for opcode type %d", xinsn->opcode->type);
        ORC_ASSERT (0);
        break;
      }
    } else {
      ORC_COMPILER_ERROR (p, "Unhandled operand type %d for VEX.RXB", xinsn->type);
      ORC_ASSERT (0);
    }
  } else {
    // REX.R,X must be force set in 32-bit
    byte2 |= 1 << 7;
    byte2 |= 1 << 6;
  }

  unsigned char byte3 = 0;

  // Section 2.3.5.6, 2.3.6
  // Instruction Operand Encoding and VEX.vvvv, ModR/M
  byte3 |= get_vex_vvvv (p, xinsn);

  // Set REX.W if required
  if (xinsn->opcode->flags & ORC_VEX_W1) {
    byte3 |= 1 << 7;
  }

  // Set vector length
  switch (xinsn->prefix) {
    case ORC_X86_AVX_VEX256_PREFIX:
      byte3 |= 0x4;
      break;
    default:
      break;
  }

  // Handle flags
  switch (xinsn->opcode->prefix) {
    case ORC_VEX_SIMD_PREFIX_F2:
    case ORC_VEX_SIMD_PREFIX_F3:
      byte3 |= 0x2; 
      break;
    case ORC_VEX_SIMD_PREFIX_66:
    case ORC_SIMD_PREFIX_MMX:
    case ORC_SIMD_PREFIX_ESCAPE_ONLY:
      byte3 |= 0x1; 
      break;
    case ORC_VEX_SIMD_PREFIX_NONE:
      break;
    default:
      ORC_COMPILER_ERROR(p, "unhandled VEX.pp for instruction type %x", xinsn->opcode->prefix);
      ORC_ASSERT(0);
      return;
  }

  *p->codeptr++ = ORC_VEX_3_BIT;

  *p->codeptr++ = byte2;

  *p->codeptr++ = byte3;

  // Emit opcode
  *p->codeptr++ = (xinsn->opcode->code >> 0) & 0xff;

  // ModR/M
  orc_vex_insn_output_modrm (p, xinsn);

  // imm
  orc_vex_insn_output_immediate (p, xinsn);
}

static void
output_2byte_vex_opcode (OrcCompiler *p, const OrcX86Insn *const xinsn)
{
  // ORC_DEBUG("generating 2-byte %x code for %s (%i, %i) -> %i", xinsn->prefix, xinsn->opcode->name, xinsn->src[0], xinsn->src[1], xinsn->dest);

  ORC_ASSERT((xinsn->opcode->flags & (ORC_VEX_ESCAPE_38 | ORC_VEX_ESCAPE_3A | ORC_VEX_W1)) == 0);

  // In protected and compatibility modes the REX.R bit must be set to ‘1’
  // otherwise the instruction is LES or LDS.
  unsigned char byte2 = 0x80;

  // Section 2.3.5.6, 2.3.6
  // Instruction Operand Encoding and VEX.vvvv, ModR/M
  byte2 |= get_vex_vvvv (p, xinsn);

  // Set vector length
  switch (xinsn->prefix) {
    case ORC_X86_AVX_VEX256_PREFIX:
      byte2 |= 0x4;
      break;
    default:
      break;
  }

  // Escape prefix
  switch (xinsn->opcode->prefix) {
    case ORC_VEX_SIMD_PREFIX_F2:
      byte2 |= 0x3; 
      break;
    case ORC_VEX_SIMD_PREFIX_F3:
      byte2 |= 0x2; 
      break;
    case ORC_VEX_SIMD_PREFIX_66:
    case ORC_SIMD_PREFIX_MMX:
      byte2 |= 0x1; 
      break;
    case ORC_VEX_SIMD_PREFIX_NONE:
    case ORC_SIMD_PREFIX_ESCAPE_ONLY:
      break;
    default:
      ORC_COMPILER_ERROR(p, "unhandled VEX opcode escape prefix: %x", xinsn->opcode->prefix);
      ORC_ASSERT(0);
      return;
  }

  *p->codeptr++ = ORC_VEX_2_BIT;

  *p->codeptr++ = byte2;

  // Emit opcode
  *p->codeptr++ = (xinsn->opcode->code >> 0) & 0xff;

  // ModR/M
  orc_vex_insn_output_modrm (p, xinsn);

  // imm
  orc_vex_insn_output_immediate (p, xinsn);
}

static void
output_vex_opcode (OrcCompiler *p, const OrcX86Insn *const xinsn)
{
  ORC_ASSERT((xinsn->opcode->code & 0xFF00) == 0);

  if (xinsn->opcode->flags & (ORC_VEX_ESCAPE_38 | ORC_VEX_ESCAPE_3A | ORC_VEX_W1)) {
    output_3byte_vex_opcode (p, xinsn);
  } else if (xinsn->src[0] & 8 || xinsn->src[1] & 8 || xinsn->dest & 8) {
    // it's a hard error if ever someone uses REX registers without x64
    // See Table 2-8. VEX.vvvv to register name mapping -- Vol. 2A 2-17
    ORC_ASSERT (p->is_64bit);
    output_3byte_vex_opcode (p, xinsn);
  } else {
    output_2byte_vex_opcode (p, xinsn);
  }
}

static void
orc_vex_insn_codegen (OrcCompiler *const p, OrcX86Insn *const xinsn)
{
  switch ((OrcX86InsnType) xinsn->opcode->type) {
    case ORC_X86_INSN_TYPE_IMM8_MMX_SHIFT:
    case ORC_X86_INSN_TYPE_MMX_REGM_REV:
    case ORC_X86_INSN_TYPE_MMXM_MMX_REV:
    case ORC_X86_INSN_TYPE_SSEM_SSE_REV:
    case ORC_X86_INSN_TYPE_IMM8_MMXM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_AVX_SSEM:
    case ORC_X86_INSN_TYPE_MMXM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_SSE:
    case ORC_X86_INSN_TYPE_REGM_MMX:
    case ORC_X86_INSN_TYPE_SSEM_AVX:
    case ORC_X86_INSN_TYPE_IMM8_REGM_MMX:
    case ORC_X86_INSN_TYPE_IMM8_MMX_REG_REV:
    case ORC_X86_INSN_TYPE_NONE:
    case ORC_X86_INSN_TYPE_MEM:
      output_vex_opcode (p, xinsn);
      break;
    case ORC_X86_INSN_TYPE_REG_REGM:
    case ORC_X86_INSN_TYPE_IMM8_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM:
    case ORC_X86_INSN_TYPE_REG8_REGM:
    case ORC_X86_INSN_TYPE_REG16_REGM:
    case ORC_X86_INSN_TYPE_REGM_REG:
    case ORC_X86_INSN_TYPE_REGM:
    case ORC_X86_INSN_TYPE_IMM32_REGM_MOV:
    case ORC_X86_INSN_TYPE_IMM32_A:
    case ORC_X86_INSN_TYPE_ALIGN:
    case ORC_X86_INSN_TYPE_LABEL:
    case ORC_X86_INSN_TYPE_BRANCH:
    case ORC_X86_INSN_TYPE_STACK:
      ORC_COMPILER_ERROR (p, "Opcode type %i cannot be VEX encoded", xinsn->opcode->type);
      break;
    default:
      ORC_ERROR("Unhandled opcode type %d for assembly codegen", xinsn->opcode->type);
      ORC_ASSERT(0);
      break;
  }
}

static void
orc_x86_recalc_offsets (OrcCompiler *p)
{
  OrcX86Insn *xinsn;
  int i;
  unsigned char *minptr;

  minptr = p->code;
  p->codeptr = p->code;
  for(i=0;i<p->n_output_insns;i++){
    unsigned char *ptr;

    xinsn = ((OrcX86Insn *)p->output_insns) + i;

    xinsn->code_offset = p->codeptr - p->code;

    ptr = p->codeptr;

    switch (xinsn->prefix) {
      case ORC_X86_NO_PREFIX:
      case ORC_X86_SSE_PREFIX:
        orc_x86_insn_output_opcode (p, xinsn);
        orc_x86_insn_output_modrm (p, xinsn);
        orc_x86_insn_output_immediate (p, xinsn);
        break;
      case ORC_X86_AVX_VEX128_PREFIX:
      case ORC_X86_AVX_VEX256_PREFIX:
        orc_vex_insn_codegen (p, xinsn);
        break;
      default:
        ORC_COMPILER_ERROR (p, "Unimplemented codegen encoding %i", xinsn->prefix);
        return;
    }

    if (xinsn->opcode->type == ORC_X86_INSN_TYPE_ALIGN) {
      if (xinsn->size > 0) {
        minptr += ((p->code - minptr)&((1<<xinsn->size) - 1));
      }
    } else {
      minptr += p->codeptr - ptr;
      if (xinsn->opcode->type == ORC_X86_INSN_TYPE_BRANCH) {
        if (xinsn->size == 4) minptr -= 4;
      }
    }

  }

  p->codeptr = p->code;
  p->n_fixups = 0;
}

void
orc_x86_calculate_offsets (OrcCompiler *p)
{
  OrcX86Insn *xinsn;
  int i;
  int j;

  orc_x86_recalc_offsets (p);

  for(j=0;j<3;j++){
    int change = FALSE;

    for(i=0;i<p->n_output_insns;i++){
      OrcX86Insn *dinsn;
      int diff;

      xinsn = ((OrcX86Insn *)p->output_insns) + i;
      if (xinsn->opcode->type != ORC_X86_INSN_TYPE_BRANCH) {
        continue;
      }

      dinsn = ((OrcX86Insn *)p->output_insns) + p->labels_int[xinsn->label];

      if (xinsn->size == 1) {
        diff = dinsn->code_offset - (xinsn->code_offset + 2);
        if (diff < -128 || diff > 127) {
          xinsn->size = 4;
          ORC_DEBUG("%d: relaxing at %d,%04x diff %d",
              j, i, xinsn->code_offset, diff);
          change = TRUE;
        } else {
        }
      } else {
        diff = dinsn->code_offset - (xinsn->code_offset + 2);
        if (diff >= -128 && diff <= 127) {
          ORC_DEBUG("%d: unrelaxing at %d,%04x diff %d",
              j, i, xinsn->code_offset, diff);
          xinsn->size = 1;
          change = TRUE;
        }
      }
    }

    if (!change) break;

    orc_x86_recalc_offsets (p);
  }
}

void
orc_x86_output_insns (OrcCompiler *p)
{
  OrcX86Insn *xinsn;
  int i;

  for(i=0;i<p->n_output_insns;i++){
    xinsn = ((OrcX86Insn *)p->output_insns) + i;

    orc_x86_insn_output_asm (p, xinsn);

    switch (xinsn->prefix) {
      case ORC_X86_NO_PREFIX:
      case ORC_X86_SSE_PREFIX:
        orc_x86_insn_output_opcode (p, xinsn);
        orc_x86_insn_output_modrm (p, xinsn);
        orc_x86_insn_output_immediate (p, xinsn);
        break;
      case ORC_X86_AVX_VEX128_PREFIX:
      case ORC_X86_AVX_VEX256_PREFIX:
        orc_vex_insn_codegen (p, xinsn);
        break;
      default:
        ORC_COMPILER_ERROR (p, "Unimplemented codegen encoding %i", xinsn->prefix);
        return;
    }
  }
}

void
orc_x86_emit_cpuinsn_size (OrcCompiler *p, int index, int size, int src, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_REG;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_imm (OrcCompiler *p, int index, int imm, int src, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_REG;
  xinsn->size = 4;
}

void
orc_x86_emit_cpuinsn_load_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_store_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int src_index, int shift, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMINDEX;
  xinsn->offset = offset;
  xinsn->index_reg = src_index;
  xinsn->shift = shift;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_imm_reg (OrcCompiler *p, int index, int size, int imm,
    int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = 0;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_REG;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_imm_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = 0;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_reg_memoffset (OrcCompiler *p, int index, int src,
    int offset, int dest)
{
  orc_x86_emit_cpuinsn_reg_memoffset_s (p, index, 4, src, offset, dest);
}

void
orc_x86_emit_cpuinsn_reg_memoffset_8 (OrcCompiler *p, int index, int src,
    int offset, int dest)
{
  orc_x86_emit_cpuinsn_reg_memoffset_s (p, index, 8, src, offset, dest);
}

void
orc_x86_emit_cpuinsn_reg_memoffset_s (OrcCompiler *p, int index, int size,
    int src, int offset, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_memoffset_reg (OrcCompiler *p, int index, int size,
    int offset, int src, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_branch (OrcCompiler *p, int index, int label)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->label = label;
  xinsn->size = 1;
}

void
orc_x86_emit_cpuinsn_align (OrcCompiler *p, int index, int align_shift)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->size = align_shift;
}

void
orc_x86_emit_cpuinsn_label (OrcCompiler *p, int index, int label)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->label = label;
  x86_add_label2 (p, p->n_output_insns - 1, label);
}

void
orc_x86_emit_cpuinsn_none (OrcCompiler *p, int index)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;
  int size = 4;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->size = size;
}

void
orc_x86_emit_cpuinsn_memoffset (OrcCompiler *p, int index, int size,
    int offset, int srcdest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->src[0] = srcdest;
  xinsn->dest = srcdest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_vex_emit_cpuinsn_none (OrcCompiler *p, const int index,
    const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;
  int size = 4;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->prefix = prefix;
  xinsn->size = size;
}

void
orc_vex_emit_cpuinsn_size (OrcCompiler *const p, const int index,
    const int size, const int src0, const int src1, const int dest,
    const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *const opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->prefix = prefix;
  xinsn->src[0] = src0;
  xinsn->src[1] = src1;
  xinsn->src[2] = 0;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_REG;
  xinsn->size = size;
}

void
orc_vex_emit_cpuinsn_imm (OrcCompiler *const p, const int index, const int imm, const int src0, const int src1, const int dest, const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->prefix = prefix;
  xinsn->imm = imm;
  xinsn->src[0] = src0;
  xinsn->src[1] = src1;
  xinsn->src[2] = 0;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_REG;
  xinsn->size = 4;
}

void
orc_vex_emit_cpuinsn_load_memoffset (OrcCompiler *const p, const int index,
    const int size, const int imm, const int offset, const int src0,
    const int src1, const int dest, const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->prefix = prefix;
  xinsn->imm = imm;
  xinsn->src[0] = src0;
  xinsn->src[1] = src1;
  xinsn->src[2] = 0;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_vex_emit_cpuinsn_store_memoffset (OrcCompiler *const p, const int index,
    const int size, const int imm, const int offset, const int src,
    const int dest, const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->imm = imm;
  xinsn->src[0] = src;
  xinsn->prefix = prefix;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMOFFSET;
  xinsn->offset = offset;
  xinsn->size = size;
}

void
orc_vex_emit_cpuinsn_load_memindex (OrcCompiler *const p, const int index, const int size,
    const int imm, const int offset, const int src, const int src_index, const int shift, int dest, const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->prefix = prefix;
  xinsn->imm = imm;
  xinsn->src[0] = src;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_MEMINDEX;
  xinsn->offset = offset;
  xinsn->index_reg = src_index;
  xinsn->shift = shift;
  xinsn->size = size;
}

void
orc_vex_emit_blend_size (OrcCompiler *const p, const int index,
    const int size, const int src0, const int src1, const int src2,
    const int dest, const OrcX86OpcodePrefix prefix)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcSysOpcode *const opcode = orc_x86_opcodes + index;

  xinsn->opcode_index = index;
  xinsn->opcode = opcode;
  xinsn->prefix = prefix;
  xinsn->src[0] = src0;
  xinsn->src[1] = src1;
  xinsn->src[2] = src2;
  xinsn->dest = dest;
  xinsn->type = ORC_X86_RM_REG;
  xinsn->size = size;
}
