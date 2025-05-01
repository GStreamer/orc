#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>

#include <orc/orc.h>
#include <orc/orcinternal.h>
#include <orc/orcx86.h>
#include <orc/orcx86-private.h>
#include <orc/orcx86insn.h>
#include <orc/orcsse.h>
#include <orc/orcsseinsn.h>

#define ORC_SSE_INSN_TYPE_SSE_REG32M_IMM8 (\
  ORC_X86_INSN_OPERAND_REG_REGM_IMM |        \
  ORC_X86_INSN_OPERAND_OP2_32 |              \
  ORC_X86_INSN_OPERAND_OP3_8                 \
), ORC_SSE_INSN_OPERAND_OP1_XMM

#define ORC_SSE_INSN_TYPE_SSE_REGM64 (\
  ORC_X86_INSN_OPERAND_REG_REGM |     \
  ORC_X86_INSN_OPERAND_OP2_64         \
), ORC_SSE_INSN_OPERAND_OP1_XMM

#define ORC_SSE_INSN_TYPE_SSE_REGM (\
  ORC_X86_INSN_OPERAND_REG_REGM |   \
  ORC_X86_INSN_OPERAND_OP2_32 |     \
  ORC_X86_INSN_OPERAND_OP2_64       \
), ORC_SSE_INSN_OPERAND_OP1_XMM

typedef struct _OrcSSEInsnOp {
  char name[16];
  unsigned int target_flags;
  unsigned int operands;
  unsigned int sse_operands;
  OrcX86InsnOpcodePrefix prefix;
  OrcX86InsnOpcodeEscapeSequence escape;
  orc_uint32 opcode;
  orc_uint8 extension;
} OrcSSEInsnOp;

/* clang-format off */
static OrcSSEInsnOp orc_sse_ops[] = {
  /* Missing MOVUPS */
  /* Missing MOVSS */
  /* Missing MOVUPS */
  /* Missing MOVLPS */
  /* Missing MOVHLPS */
  /* Missing MOVLPS */
  /* Missing UNPCKLPS */
  /* Missing UNPCKHPS */
  /* Missing MOVLHPS */
  { "movhps"    , 0, ORC_SSE_INSN_TYPE_SSE_REGM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x16 },
  /* Missing MOVHPS */
  /* Missing MOVAPS */ 
  /* Missing MOVAPS */
  /* Missing CVTPI2PS */
  /* Missing CVTSI2SS */
  /* Missing CVTSI2SS */
  /* Missing MOVNTPS */
  /* Missing CVTTPS2PI */
  /* Missing CVTTSS2SI */
  /* Missing CVTTSS2SI */
  /* Missing CVTPS2PI  */
  /* Missing CVTSS2SI */
  /* Missing CVTSS2SI */
  /* Missing UCOMISS */
  /* Missing COMISS */
  /* Missing MOVMSKPS */
  { "sqrtps"    , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x51 },
  /* Missing SQRTSS */
  /* Missing RSQRTSS */
  /* Missing RSQRTPS */
  /* Missing RCPPS */
  /* Missing RCPSS */
  { "andps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x54 },
  /* Missing ANDNPS */
  { "orps"      , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x56 },
  /* Missing XORPS */
  { "addps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x58 },
  /* Missing ADDSS */
  { "mulps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x59 },
  /* Missing MULSS */
  { "subps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5c },
  /* Missing SUBSS */
  { "minps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5d },
  /* Missing MINSS */
  { "divps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5e },
  /* Missing DIVSS */
  { "maxps"     , 0, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5f },
  /* 10 */
  /* Missing MAXSS */
  { "ldmxcsr"   , 0, ORC_X86_INSN_TYPE_MEM32, 0, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xae, 2 },
  { "stmxcsr"   , 0, ORC_X86_INSN_TYPE_MEM32, 0, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xae, 3 },
  /* Missing CMPPS */
  /* Missing CMPSS */
  /* Missing SHUFPS */

  /* SSE with SSE2 */
  /* Missing MOVUPD */
  /* Missing MOVSD  */
  /* Missing MOVUPD */
  /* Missing MOVSD  */
  /* Missing MOVLPD */
  /* Missing MOVLPD */
  /* Missing UNPCKLPD */
  /* Missing UNPCKHPD */
  /* Missing MOVHPD */
  /* Missing MOVHPD */
  /* Missing MOVAPD */
  /* Missing MOVAPD */
  /* Missing CVTSI2SD */
  /* Missing CVTPI2PD */
  /* Missing CVTSI2SD */
  /* Missing MOVNTPD  */
  /* Missing CVTTSD2SI  */
  /* Missing CVTTPD2PI  */
  /* Missing CVTTSD2SI  */
  /* Missing CVTSD2SI */
  /* Missing CVTPD2PI */
  /* Missing CVTSD2SI */
  /* Missing UCOMISD  */
  /* Missing COMISD */
  /* Missing MOVMSKPD */
  { "sqrtpd"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x51 },
  /* Missing SQRTSD */
  /* Missing ANDPD */
  /* Missing ANDNPD */
  /* Missing ORPD */
  /* Missing XORPD */
  { "addpd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x58 },
  /* Missing ADDSD */ 
  { "mulpd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x59 },
  /* Missing MULSD */
  { "cvtps2pd"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5a },
  { "cvtpd2ps"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5a },
  /* Missing CVTSD2SS */
  /* Missing CVTSS2SD */
  { "cvtdq2ps"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5b },
  /* Missing CVTPS2DQ */ 
  { "cvttps2dq" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5b },
  { "subpd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5c },
  /* 20 */
  /* Missing SUBSD */
  { "minpd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5d },
  /* Missing MINSD */
  { "divpd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5e },
  /* Missing DIVSD */
  { "maxpd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x5f },
  /* Missing MAXSD */
  { "cmpeqpd"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc2, 0 },
  { "cmpltpd"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc2, 1 },
  { "cmplepd"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc2, 2 },
  { "cmpeqps"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc2, 0 },
  { "cmpltps"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc2, 1 },
  { "cmpleps"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc2, 2 },
 
  /* Missing SHUFPD */
  { "cvtdq2pd"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe6 },
  /* Missing CVTPD2DQ */
  /* 30 */
  { "cvttpd2dq" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe6 },

  /* MMX ones, not all */
  { "punpcklbw" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x60 },
  { "punpcklwd" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x61 },
  { "punpckldq" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x62 },
  { "packsswb"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x63 },
  { "pcmpgtb"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x64 },
  { "pcmpgtw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x65 },
  { "pcmpgtd"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x66 },
  { "packuswb"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x67 },
  { "punpckhbw" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x68 },
  /* 40 */
  { "punpckhwd" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x69 },
  { "punpckhdq" , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6a },
  { "packssdw"  , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6b },
  { "movq"       , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_REGM64, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6e }, // load
  { "movd"      , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_REGM32, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6e },
  { "psrlw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x71, 2 },
  { "psraw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x71, 4 },
  { "psllw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x71, 6 },
  { "psrld"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x72, 2 },
  { "psrad"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x72, 4 },
  /* 50 */
  { "pslld"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x72, 6 },
  { "psrlq"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x73, 2 },
  { "psllq"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x73, 6 },
  { "pcmpeqb"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x74 },
  { "pcmpeqw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x75 },
  { "pcmpeqd"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x76 },
  { "movd"      , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_REGM32_SSE, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x7e },
  { "movq"      , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x7e },
  { "movq"       , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_REGM64_SSE, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x7e }, // store
  { "pinsrw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_REG32_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc4 },
  /* 60 */
  { "pextrw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_REG32TO64_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc5 },
  /* Missing PEXTRW ORC_X86_INSN_OPCODE_PREFIX_0X66 ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xc5 */
  { "psrlw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd1 },
  { "psrld"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd2 },
  { "psrlq"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd3 },
  { "paddq"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd4 },
  { "pmullw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd5 },
  { "movq"      , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSEM_SSE, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd6 },
  /* Missing PMOVMSKB */
  { "psubusb"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd8 },
  { "psubusw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xd9 },
  { "pminub"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xda },
  { "pand"      , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xdb },
  /* 70 */
  { "paddusb"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xdc },
  { "paddusw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xdd },
  { "pmaxub"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xde },
  { "pandn"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xdf },
  { "pavgb"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe0 },
  { "psraw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe1 },
  { "psrad"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe2 },
  { "pavgw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe3 },
  { "pmulhuw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe4 },
  { "pmulhw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe5 },
  /* 80 */
  { "psubsb"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe8 },
  { "psubsw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe9 },
  { "pminsw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xea },
  { "por"       , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xeb },
  { "paddsb"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xec },
  { "paddsw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xed },
  { "pmaxsw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xee },
  { "pxor"      , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xef },
  { "psllw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf1 },
  /* 90 */
  { "pslld"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf2 },
  { "psllq"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf3 },
  { "pmuludq"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf4 },
  { "pmaddwd"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf5 },
  { "psadbw"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf6 },
  { "psubb"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf8 },
  { "psubw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xf9 },
  { "psubd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xfa },
  { "psubq"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xfb },
  { "paddb"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xfc },
  /* 100 */
  { "paddw"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xfd },
  { "paddd"     , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xfe },
  /* SSE2 for SSE only */
  { "punpcklqdq", ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6c },
  { "punpckhqdq", ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6d },
  { "movdqa"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6f },
  { "pshufd"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x70 },
  { "movdqa"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSEM_SSE     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x7f },
  { "psrldq"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x73, 3 },
  { "pslldq"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x73, 7 },
  { "movntdq"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSEM_SSE     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xe7 },
  /* 110 */
  /* Missing MASKMOVDQU */
  { "pshuflw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0XF2, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x70 },
  /* Missing MOVDQ2Q */
  { "movdqu"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x6f },
  { "movdqu"    , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSEM_SSE     , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x7f },
  { "pshufhw"   , ORC_TARGET_SSE_SSE2, ORC_SSE_INSN_TYPE_SSE_SSEM_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x70 },
  /* Missing MOVQ2DQ */
  /* SSE with SSE3 */
  /* Missing MOVDDUP */
  /* Missing MOVSLDUP */
  /* Missing MOVSHDUP */
  { "pshufb"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x00 },
  { "phaddw"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x01 },
  { "phaddd"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x02 },
  { "phaddsw"   , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x03 },
  { "pmaddubsw" , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x04 },
  { "phsubw"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x05 },
  /* 120 */
  { "phsubd"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x06 },
  { "phsubsw"   , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x07 },
  { "psignb"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x08 },
  { "psignw"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x09 },
  { "psignd"    , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x0a },
  { "pmulhrsw"  , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x0b },
  { "palignr"   , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A, 0x0f },
  { "pabsb"     , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x1c },
  { "pabsw"     , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x1d },
  { "pabsd"     , ORC_TARGET_SSE_SSE3, ORC_SSE_INSN_TYPE_SSE_SSEM     , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x1e },
  /* Missing PALIGNR */
  /* Missing HADDPS */
  /* Missing HADDPD */
  /* Missing HSUBPS */
  /* Missing HSUBPD */
  /* Missing ADDSUBPS */
  /* Missing ADDSUBPD */
  /* Missing LDDQU */
  /* SSE with SSE4.1 */
  /* Missing ROUNDPS */
  /* Missing ROUNDPD */
  /* Missing ROUNDSS */
  /* Missing ROUNDSD */
  /* Missing BLENDPS */
  /* Missing BLENDPD */
  /* Missing PBLENDW */
  /* Missing PBLENDVB */
  /* Missing BLENDVPS */
  /* 130 */
  { "pextrb"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_REGM32TO64_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A, 0x14 },
  { "blendvpd"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM           , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x15 },
  { "pextrw"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_REGM32TO64_SSE_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A, 0x15 },
  /* Missing PEXTRD */
  /* Missing PEXTRQ */
  /* Missing EXTRACTPS */
  /* Missing PTEST */
  { "pinsrb"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_REG32M_IMM8   , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A, 0x20 },
  { "pmovsxbw"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x20 },
  /* Missing INSERTPS */
  { "pmovsxbd"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x21 },
  { "pinsrd"   , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_REG32M_IMM8, ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A, 0x22 },
  /* Missing PINSRQ */
  { "pmovsxbq"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x22 },
  { "pmovsxwd"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x23 },
  { "pmovsxwq"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x24 },
  /* 140 */
  { "pmovsxdq"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x25 },
  { "pmuldq"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x28 },
  { "pcmpeqq"   , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x29 },
  /* Missing MOVNTDQA */
  { "packusdw"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x2b },
  { "pmovzxbw"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x30 },
  { "pmovzxbd"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x31 },
  { "pmovzxbq"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x32 },
  { "pmovzxwd"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x33 },
  { "pmovzxwq"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x34 },
  { "pmovzxdq"  , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x35 },
  /* 150 */
  { "pminsb"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x38 },
  { "pminsd"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x39 },
  { "pminuw"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x3A },
  { "pminud"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x3B },
  { "pmaxsb"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x3C },
  { "pmaxsd"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x3D },
  { "pmaxuw"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x3E },
  { "pmaxud"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x3F },
  /* Missing DPPS */
  { "pmulld"    , ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x40 },
  /* Missing DPPD */
  { "phminposuw", ORC_TARGET_SSE_SSE4_1, ORC_SSE_INSN_TYPE_SSE_SSEM          , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x41 },
  /* Missing MPSADBW */

  /* Missing SSE with SSE4a */
  /* SSE with SSE4.2 */
  /* 160 */
  { "pcmpgtq"   , ORC_TARGET_SSE_SSE4A, ORC_SSE_INSN_TYPE_SSE_SSEM           , ORC_X86_INSN_OPCODE_PREFIX_0X66, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38, 0x37 },
};
/* clang-format on */

static orc_bool
orc_sse_insn_validate_operand1_reg (int reg, OrcX86InsnOperandSize size,
  unsigned int sse_operands, unsigned int operands)
{
  if ((reg >= X86_XMM0 && reg < X86_XMM0 + 16)) {
    return orc_sse_insn_validate_operand1_sse (reg, sse_operands);
  } else {
    return orc_x86_insn_validate_operand1_reg (reg, size, operands);
  }
}

static orc_bool
orc_sse_insn_validate_operand2_reg (int reg, OrcX86InsnOperandSize size,
  unsigned int sse_operands, unsigned int operands)
{
  if ((reg >= X86_XMM0 && reg < X86_XMM0 + 16)) {
    return orc_sse_insn_validate_operand2_sse (reg, sse_operands);
  } else {
    return orc_x86_insn_validate_operand2_reg (reg, size, operands);
  }
}

static OrcX86InsnOperandSize
orc_sse_insn_size_from_reg (OrcX86InsnOperandSize size, int reg)
{
  if (reg >= X86_XMM0 && reg < X86_XMM0 + 16)
    return ORC_X86_INSN_OPERAND_SIZE_NONE;
  else
    return size;
}

static void
orc_sse_insn_from_opcode (OrcX86Insn *insn, int index, const OrcSSEInsnOp *op)
{
  insn->name = op->name;
  insn->opcode = op->opcode;
  insn->opcode_type = ORC_X86_INSN_OPCODE_TYPE_OTHER;
  insn->opcode_escape = op->escape;
  insn->opcode_prefix = op->prefix;
  insn->prefix = ORC_X86_INSN_PREFIX_NO_PREFIX;
  insn->extension = op->extension;
}

static OrcX86Insn *
orc_sse_emit_cpuinsn_load_mem (OrcCompiler *p, int index,
    OrcX86InsnOperandType mem_type, int imm, int src, int dest)
{
  OrcX86Insn *xinsn;
  const OrcSSEInsnOp *opcode = orc_sse_ops + index;
  orc_bool has_dest = FALSE;

  /* checks */
  if (opcode->operands & ORC_X86_INSN_OPERAND_OP1_REG) {
    if (!orc_sse_insn_validate_operand1_sse (dest, opcode->sse_operands)) {
      ORC_ERROR ("Dest register %d not validated for opcode %d", dest, index);
    }
    if (!orc_x86_insn_validate_operand2_mem (src, opcode->operands)) {
      ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
    }
    has_dest = TRUE;
  } else if ((opcode->operands & ORC_X86_INSN_OPERAND_OP1_MEM) &&
      !orc_x86_insn_validate_operand1_mem (src, opcode->operands)) {
    ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
  }

  if ((opcode->operands & ORC_X86_INSN_OPERAND_OP3_IMM) &&
      !orc_x86_insn_validate_operand3_imm (imm, opcode->operands)) {
    ORC_ERROR ("Setting an imm %" PRIi64 " in a wrong opcode %d", imm, index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_sse_insn_from_opcode (xinsn, index, opcode);
  if (has_dest) {
    orc_x86_insn_operand_set (&xinsn->operands[0],
        ORC_X86_INSN_OPERAND_TYPE_REG, ORC_X86_INSN_OPERAND_SIZE_NONE, dest);
    orc_x86_insn_operand_set (&xinsn->operands[1],
        mem_type,
        p->is_64bit ? ORC_X86_INSN_OPERAND_SIZE_64 : ORC_X86_INSN_OPERAND_SIZE_32,
        src);
    xinsn->encoding = ORC_X86_INSN_ENCODING_RM;

    if (opcode->operands & ORC_X86_INSN_OPERAND_OP3_IMM) {
      orc_x86_insn_operand_set (&xinsn->operands[2],
          ORC_X86_INSN_OPERAND_TYPE_IMM, ORC_X86_INSN_OPERAND_SIZE_8, 0);
      xinsn->encoding = ORC_X86_INSN_ENCODING_RMI;
      xinsn->imm = imm;
    }
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[0],
        mem_type,
        p->is_64bit ? ORC_X86_INSN_OPERAND_SIZE_64 : ORC_X86_INSN_OPERAND_SIZE_32,
        src);
    xinsn->encoding = ORC_X86_INSN_ENCODING_M;
  }
  return xinsn;
}

orc_bool
orc_sse_insn_validate_reg (int reg)
{
  if (reg >= X86_XMM0 && reg < X86_XMM0 + 16)
    return TRUE;
  else
    return FALSE;
}

orc_bool
orc_sse_insn_validate_operand1_sse (int reg, unsigned int sse_operands)
{
  /* Confirm if the register is a SSE register */
  if (!reg)
    return FALSE;
  else if (!(sse_operands & ORC_SSE_INSN_OPERAND_OP1_XMM))
    return FALSE;
  return orc_sse_insn_validate_reg (reg);
}

orc_bool
orc_sse_insn_validate_operand2_sse (int reg, unsigned int sse_operands)
{
  /* Confirm if the register is a SSE register */
  if (!reg)
    return FALSE;
  else if (!(sse_operands & ORC_SSE_INSN_OPERAND_OP2_XMM))
    return FALSE;
  return orc_sse_insn_validate_reg (reg);
}

void
orc_x86_emit_mov_memoffset_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      orc_sse_emit_movd_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_sse_emit_movq_load_memoffset (compiler, offset, reg1, reg2);
      break;
    case 16:
      if (is_aligned) {
        orc_sse_emit_movdqa_load_memoffset (compiler, offset, reg1, reg2);
      } else {
        orc_sse_emit_movdqu_load_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_mov_memindex_sse (OrcCompiler *compiler, int size, int offset,
    int reg1, int regindex, int shift, int reg2, int is_aligned)
{
  switch (size) {
    case 4:
      orc_sse_emit_movd_load_memindex (compiler, offset,
          reg1, regindex, shift, reg2);
      break;
    case 8:
      orc_sse_emit_movq_load_memindex (compiler, offset,
          reg1, regindex, shift, reg2);
      break;
    case 16:
      if (is_aligned) {
        orc_sse_emit_movdqa_load_memindex (compiler, offset,
            reg1, regindex, shift, reg2);
      } else {
        orc_sse_emit_movdqu_load_memindex (compiler, offset,
            reg1, regindex, shift, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_mov_sse_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2, int aligned, int uncached)
{
  switch (size) {
    case 4:
      orc_sse_emit_movd_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 8:
      orc_sse_emit_movq_store_memoffset (compiler, offset, reg1, reg2);
      break;
    case 16:
      if (aligned) {
        if (uncached) {
          orc_sse_emit_movntdq_store_memoffset (compiler, offset, reg1, reg2);
        } else {
          orc_sse_emit_movdqa_store_memoffset (compiler, offset, reg1, reg2);
        }
      } else {
        orc_sse_emit_movdqu_store_memoffset (compiler, offset, reg1, reg2);
      }
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_sse_set_mxcsr (OrcCompiler *compiler)
{
  orc_sse_emit_cpuinsn_store_memoffset (compiler, ORC_SSE_stmxcsr, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      0, compiler->exec_reg);

  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      compiler->exec_reg, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_C1]),
      compiler->exec_reg);

  orc_x86_emit_cpuinsn_imm_reg (compiler, ORC_X86_or_imm32_rm, 4,
      0x8040, compiler->gp_tmpreg);

  orc_x86_emit_mov_reg_memoffset (compiler, 4, compiler->gp_tmpreg,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      compiler->exec_reg);

  orc_sse_emit_cpuinsn_store_memoffset (compiler, ORC_SSE_ldmxcsr, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      0, compiler->exec_reg);
}

void
orc_sse_restore_mxcsr (OrcCompiler *compiler)
{
  orc_sse_emit_cpuinsn_store_memoffset (compiler, ORC_SSE_ldmxcsr, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,params[ORC_VAR_A4]),
      0, compiler->exec_reg);
}

void
orc_sse_emit_cpuinsn_sse (OrcCompiler *p, int index, int src, int dest)
{
  OrcX86Insn *xinsn;
  const OrcSSEInsnOp *opcode = orc_sse_ops + index;

  /* checks */
  if (!orc_sse_insn_validate_operand1_sse (dest, opcode->sse_operands)) {
    ORC_ERROR ("Dest register %d not validated for opcode %d", dest, index);
  }

  if (src && !orc_sse_insn_validate_operand2_sse (src, opcode->sse_operands)) {
    ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
  }

  if (!(opcode->operands & (ORC_X86_INSN_OPERAND_REGM_REG | ORC_X86_INSN_OPERAND_REG_REGM))) {
    ORC_ERROR ("Unsupported operand type for opcode %d", index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_sse_insn_from_opcode (xinsn, index, opcode);
  orc_x86_insn_operand_set (&xinsn->operands[0],
      ORC_X86_INSN_OPERAND_TYPE_REG, ORC_X86_INSN_OPERAND_SIZE_NONE, dest);
  if (opcode->operands & ORC_X86_INSN_OPERAND_OP1_MEM)
    xinsn->encoding = ORC_X86_INSN_ENCODING_M;
  else
    xinsn->encoding = ORC_X86_INSN_ENCODING_O;

  if (src) {
    orc_x86_insn_operand_set (&xinsn->operands[1],
        ORC_X86_INSN_OPERAND_TYPE_REG, ORC_X86_INSN_OPERAND_SIZE_NONE, src);
    if (opcode->operands & ORC_X86_INSN_OPERAND_OP2_MEM)
      xinsn->encoding = ORC_X86_INSN_ENCODING_RM;
    else
      xinsn->encoding = ORC_X86_INSN_ENCODING_MR;
  }

  /* Special case for cmpeqpd until cmpleps, all are REG_REGM_IMM8 but faked on
   * the definition
   */
  if (index >= ORC_SSE_cmpeqpd && index <= ORC_SSE_cmpleps) {
    xinsn->encoding = ORC_X86_INSN_ENCODING_RMI;
    orc_x86_insn_operand_set (&xinsn->operands[2],
        ORC_X86_INSN_OPERAND_TYPE_IMM, ORC_X86_INSN_OPERAND_SIZE_8, 0);
    xinsn->imm = opcode->extension;
  }
}

void
orc_sse_emit_cpuinsn_size (OrcCompiler *p, int index, int size, int src, int dest)
{
  OrcX86Insn *xinsn;
  const OrcSSEInsnOp *opcode = orc_sse_ops + index;
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);

  /* checks */
  if (!orc_sse_insn_validate_operand1_reg (dest, opsize, opcode->sse_operands,
      opcode->operands)) {
    ORC_ERROR ("Dest register %d not validated for opcode %d", dest, index);
  }

  if (src && !orc_sse_insn_validate_operand2_reg (src, opsize, opcode->sse_operands,
      opcode->operands)) {
    ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_sse_insn_from_opcode (xinsn, index, opcode);
  orc_x86_insn_operand_set (&xinsn->operands[0],
      ORC_X86_INSN_OPERAND_TYPE_REG, orc_sse_insn_size_from_reg (opsize, dest),
      dest);
  if (opcode->operands & ORC_X86_INSN_OPERAND_OP1_MEM)
    xinsn->encoding = ORC_X86_INSN_ENCODING_M;
  else
    xinsn->encoding = ORC_X86_INSN_ENCODING_O;

  if (src) {
    orc_x86_insn_operand_set (&xinsn->operands[1],
        ORC_X86_INSN_OPERAND_TYPE_REG, orc_sse_insn_size_from_reg (opsize, src),
        src);
    if (opcode->operands & ORC_X86_INSN_OPERAND_OP2_MEM)
      xinsn->encoding = ORC_X86_INSN_ENCODING_RM;
    else
      xinsn->encoding = ORC_X86_INSN_ENCODING_MR;
  }
}

/*
 * Used in
 * ORC_SSE_palignr (SSE_SSEM_IMM8)
 * ORC_SSE_pshufhw (SSE_SSEM_IMM8)
 * ORC_SSE_pshuflw (SSE_SSEM_IMM8)
 * ORC_SSE_pslldq (SSE_IMM8) 
 * ORC_SSE_psrlldq (SSE_IMM8)
 * ORC_SSE_pshufd (SSE_SSEM_IMM8)
 * ORC_SSE_pinsrw (SSE_REG32M_IMM8)
 * ORC_SSE_pextrw (REGM32TO64_SSE_IMM8)
 * ORC_SSE_psllq_imm (SSE_IMM8) 
 * ORC_SSE_psrlq_imm (SSE_IMM8)
 * ORC_SSE_pslld_imm (SSE_IMM8)
 * ORC_SSE_psrad_imm (SSE_IMM8)
 * ORC_SSE_psrld_imm (SSE_IMM8)
 * ORC_SSE_psllw_imm (SSE_IMM8)
 * ORC_SSE_psraw_imm (SSE_IMM8)
 * ORC_SSE_psrlw_imm (SSE_IMM8)
 */
void
orc_sse_emit_cpuinsn_imm (OrcCompiler *p, int index, int size, int imm, int src, int dest)
{
  OrcX86Insn *xinsn;
  const OrcSSEInsnOp *opcode = orc_sse_ops + index;
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  orc_bool has_src = FALSE;
  orc_bool has_imm1;
  orc_bool has_imm2;
  orc_bool has_imm3;

  /* checks */
  has_imm1 = orc_x86_insn_validate_operand1_imm (imm, opcode->operands);
  has_imm2 = orc_x86_insn_validate_operand2_imm (imm, opcode->operands);
  has_imm3 = orc_x86_insn_validate_operand3_imm (imm, opcode->operands);

  if (!has_imm1 && !has_imm2 && !has_imm3) {
    ORC_ERROR ("Setting an imm %" PRIi64 " in a wrong opcode %d (%s)", imm, index, opcode->name);
  }

  if (!orc_sse_insn_validate_operand1_reg (dest, opsize, opcode->sse_operands,
      opcode->operands)) {
    ORC_ERROR ("Dest register %d not validated for opcode %d", dest, index);
  } 

  if (opcode->operands & ORC_X86_INSN_OPERAND_OP2_REG) {
    if (!orc_sse_insn_validate_operand2_reg (src, opsize, opcode->sse_operands,
        opcode->operands))
      ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
    has_src = TRUE; 
  }

  if (has_src && !has_imm3) {
    ORC_ERROR ("Setting a src register %d and no imm in a wrong opcode %d", src, index);
  }
  
  xinsn = orc_x86_get_output_insn (p);
  orc_sse_insn_from_opcode (xinsn, index, opcode);
  xinsn->imm = imm;
  if (has_src) {
    orc_x86_insn_operand_set (&xinsn->operands[1],
        ORC_X86_INSN_OPERAND_TYPE_REG, orc_sse_insn_size_from_reg (opsize, src),
        src);
    orc_x86_insn_operand_set (&xinsn->operands[2],
        ORC_X86_INSN_OPERAND_TYPE_IMM, ORC_X86_INSN_OPERAND_SIZE_8, 0);
    xinsn->encoding = ORC_X86_INSN_ENCODING_RMI;
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[1],
        ORC_X86_INSN_OPERAND_TYPE_IMM, ORC_X86_INSN_OPERAND_SIZE_8, 0);
    xinsn->encoding = ORC_X86_INSN_ENCODING_MI;
  }
  orc_x86_insn_operand_set (&xinsn->operands[0],
      ORC_X86_INSN_OPERAND_TYPE_REG, ORC_X86_INSN_OPERAND_SIZE_NONE, dest);
}

/*
 * Used in
 * ORC_SSE_movhps_load (SSE_REGM)
 * ORC_SSE_movd_load (SSE_REGM32)
 * ORC_SSE_movq_sse_load (SSE_SSEM)
 * ORC_SSE_pinsrw (SSE_REG32M_IMM8)
 * ORC_SSE_movdqa_load (SSE_SSEM)
 * ORC_SSE_movdqu_load (SSE_SSEM)
 * ORC_SSE_pinsrb SSE_REG32M_IMM8)
 */
void
orc_sse_emit_cpuinsn_load_memoffset (OrcCompiler *p, int index, int imm,
    int offset, int src, int dest)
{
  OrcX86Insn *xinsn;
  xinsn = orc_sse_emit_cpuinsn_load_mem (p, index,
      ORC_X86_INSN_OPERAND_TYPE_OFF, imm, src, dest);
  if (xinsn) {
    xinsn->offset = offset;
  }
}

/*
 * Used in
 * ORC_SSE_movhps_load (SSE_REGM)
 * ORC_SSE_movd_load (SSE_REGM32)
 * ORC_SSE_movq_sse_load (SSE_SSEM)
 * ORC_SSE_pinsrw (SSE_REG32M16_IMM8)
 * ORC_SSE_movdqa_load (SSE_SSEM)
 * ORC_SSE_movdqu_load (SSE_SSEM)
 */
void
orc_sse_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int imm,
    int offset, int src, int src_index, int shift, int dest)
{
  OrcX86Insn *xinsn;

  xinsn = orc_sse_emit_cpuinsn_load_mem (p, index,
      ORC_X86_INSN_OPERAND_TYPE_IDX, imm, src, dest);
  if (xinsn) {
    xinsn->offset = offset;
    xinsn->index_reg = src_index;
    xinsn->shift = shift;
  }
}

/*
 * Used in
 * ORC_SSE_movdqa_store (SSEM_SSE)
 * ORC_SSE_movntdq_store (SSEM_SSE)
 * ORC_SSE_movdqu_store (SSEM_SSE)
 * ORC_SSE_pextrb (REGM32TO64_SSE_IMM8)
 * ORC_SSE_pextrw_mem (REGM32TO64_SSE_IMM8)
 * ORC_SSE_stmxcsr (M32)
 * ORC_SSE_ldmxcsr (M32)
 */
void
orc_sse_emit_cpuinsn_store_memoffset (OrcCompiler *p, int index,
    int imm, int offset, int src, int dest)
{
  OrcX86Insn *xinsn;
  const OrcSSEInsnOp *opcode = orc_sse_ops + index;
  orc_bool has_src = FALSE;

  /* checks */
  if (!orc_x86_insn_validate_operand1_mem (dest, opcode->operands)) {
    ORC_ERROR ("Dest register %d not validated for opcode %s", dest,
        opcode->name);
  }

  if (opcode->operands & ORC_X86_INSN_OPERAND_OP2_REG) {
    if (!orc_sse_insn_validate_operand2_sse (src, opcode->sse_operands)) {
      ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
    }
    has_src = TRUE;
  }

  if ((opcode->operands & ORC_X86_INSN_OPERAND_OP3_IMM) &&
      !orc_x86_insn_validate_operand3_imm (imm, opcode->operands)) {
    ORC_ERROR ("Setting an imm %" PRIi64 " in a wrong opcode %d (%s)", imm, index, opcode->name);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_sse_insn_from_opcode (xinsn, index, opcode);
  xinsn->imm = imm;
  orc_x86_insn_operand_set (&xinsn->operands[0],
      ORC_X86_INSN_OPERAND_TYPE_OFF,
      p->is_64bit ? ORC_X86_INSN_OPERAND_SIZE_64 : ORC_X86_INSN_OPERAND_SIZE_32,
      dest);
  xinsn->encoding = ORC_X86_INSN_ENCODING_M;

  if (has_src) {
    orc_x86_insn_operand_set (&xinsn->operands[1],
        ORC_X86_INSN_OPERAND_TYPE_REG, ORC_X86_INSN_OPERAND_SIZE_NONE, src);
    xinsn->encoding = ORC_X86_INSN_ENCODING_MR;
  
    if (opcode->operands & ORC_X86_INSN_OPERAND_OP3_IMM) {
      orc_x86_insn_operand_set (&xinsn->operands[2],
          ORC_X86_INSN_OPERAND_TYPE_IMM, ORC_X86_INSN_OPERAND_SIZE_8, 0);
      xinsn->encoding = ORC_X86_INSN_ENCODING_MRI;
    }
  }
  xinsn->offset = offset;
}
