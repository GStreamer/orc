#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <orc/orc.h>
#include <orc/orcutils-private.h>
#include <orc/orcinternal.h>
#include <orc/orcx86.h>
#include <orc/orcx86-private.h>
#include <orc/orcx86insn.h>
#ifdef ENABLE_TARGET_MMX
#include <orc/orcmmx.h>
#endif
#ifdef ENABLE_TARGET_SSE
#include <orc/orcsse.h>
#endif
#ifdef ENABLE_TARGET_AVX
#include <orc/orcavx.h>
#endif

#define X86_VEX_vvvv_UNUSED (0xF << 3)
#define X86_VEX_vvvv_MASK (0xF << 3)
#define X86_VEX_2_BYTES 0xc5
#define X86_VEX_3_BYTES 0xc4
#define X86_MODRM(mod, rm, reg) ((((mod)&3)<<6)|(((rm)&7)<<0)|(((reg)&7)<<3))
#define X86_SIB(ss, ind, reg) ((((ss)&3)<<6)|(((ind)&7)<<3)|((reg)&7))

typedef struct _OrcX86InsnOpcode {
  char name[16]; /* The mnemonic in AT&T format */
  OrcX86InsnOpcodeType type;
  unsigned int operands;
  OrcX86InsnOpcodePrefix prefix;
  OrcX86InsnOpcodeEscapeSequence escape;
  orc_uint32 opcode;
  int extension;
} OrcX86InsnOpcode;

/* We don't list all the x86 instructions, but the ones used in Orc. In case a
 * new X86 instruction needs to be added, add it in order of the actual opcode
 * without any escape byte sequence.
 */
/* clang-format off */
static OrcX86InsnOpcode orc_x86_opcodes[] = {
  { "add"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x01 },
  { "add"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x03 },
  { "or"       , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x09 },
  { "or"       , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x0b },
  { "adc"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x11 },
  { "adc"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x13 },
  { "sbb"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x19 },
  { "sbb"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x1b },
  { "and"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x21 },
  { "and"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x23 },
  /* 10 */
  { "and"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_IMM32_A       , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x25, 4 },
  { "sub"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x29 },
  { "sub"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x2b },
  { "xor"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x31 },
  { "xor"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x33 },
  { "cmp"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x3b },
  { "cmp"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x39 },
  { "push"     , ORC_X86_INSN_OPCODE_TYPE_STACK , ORC_X86_INSN_TYPE_REG16TO64     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x50 },
  { "pop"      , ORC_X86_INSN_OPCODE_TYPE_STACK , ORC_X86_INSN_TYPE_REG16TO64     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x58 },
  { "jo"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x70 },
  /* 20 */
  { "jno"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x71 },
  { "jc"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x72 },
  { "jnc"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x73 },
  { "jz"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x74 },
  { "jnz"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x75 },
  { "jbe"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x76 },
  { "ja"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x77 },
  { "js"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x78 },
  { "jns"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x79 },
  { "jp"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x7a },
  /* 30 */
  { "jnp"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x7b },
  { "jl"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x7c },
  { "jge"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x7d },
  { "jle"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x7e },
  { "jg"       , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x7f },
  { "add"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 0 },
  { "or"       , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 1 },
  { "adc"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 2 },
  { "sbb"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 3 },
  { "and"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 4 },
  /* 40 */
  { "sub"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 5 },
  { "xor"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 6 },
  { "cmpd"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32TO64_IMM32    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x81, 7 },
  { "add"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 0 },
  { "or"       , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 1 },
  { "adc"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 2 },
  { "sbb"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 3 },
  { "and"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 4 },
  { "sub"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 5 },
  { "xor"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 6 },
  /* 50 */
  { "cmpb"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x83, 7 },
  { "test"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x85 },
  { "movb"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM8_REG8    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x88 },
  { "movl"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32_REG32  , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x89 },
  { "mov"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM64_REG64  , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x89 },
  { "movw"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16_REG16  , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x89 },
  { "movw"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16_REGM16, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x8b },
  { "movl"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG32_REGM32, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x8b },
  { "mov"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG64_REGM64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x8b },
  { "leal"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG32_REGM32, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x8d },
  /* 60 */
  { "leaq"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG64_MEM     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x8d },
  { "nop"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0x90 },
  { "rep movsb", ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xa4 },
  { "rep movsl", ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xa5 },
  { "rep movsw", ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xa5 },
  { "imul"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64, ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xaf },
  { "movzx"    , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG16TO64_REGM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0xb6 },
  { "mov"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG32_IMM32   , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xb8 },
  { "sar"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64_IMM8     , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xc1, 7 },
  { "ret"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xc3 },
  { "retq"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xc3 },
  /* 70 */
  { "sar"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM16TO64    , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xd1, 7 },
  { "jmp"      , ORC_X86_INSN_OPCODE_TYPE_BRANCH, ORC_X86_INSN_OPERAND_OP1_OFF        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xeb },
  { "testl"    , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32_IMM32  , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xf7, 0 },
  { "imull"    , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xf7, 5 },
  { "incl"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xff, 0 },
  { "decl"     , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REGM32        , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xff, 1 },
  { "rdtsc"    , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x31 },
  { "endbr64"  , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x1efa },
  { "endbr32"  , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_OPERAND_NONE       , ORC_X86_INSN_OPCODE_PREFIX_0XF3, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F, 0x1efb },
  /* 80 */
  { "mov"      , ORC_X86_INSN_OPCODE_TYPE_OTHER , ORC_X86_INSN_TYPE_REG64_IMM64   , ORC_X86_INSN_OPCODE_PREFIX_NONE, ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE, 0xb8 },
};
/* clang-format on */

/* FIXME this is common to all targets. Move to the compiler */
static void
x86_add_fixup (OrcCompiler *compiler, unsigned char *ptr, int label, int type)
{
  compiler->fixups[compiler->n_fixups].ptr = ptr;
  compiler->fixups[compiler->n_fixups].label = label;
  compiler->fixups[compiler->n_fixups].type = type;
  compiler->n_fixups++;
}

/* FIXME this is common to all targets. Move to the compiler */
static void
x86_add_label (OrcCompiler *compiler, unsigned char *ptr, int label)
{
  compiler->labels[label] = ptr;
}

/* FIXME this is common to all targets. Move to the compiler */
static void
x86_add_label2 (OrcCompiler *compiler, int index, int label)
{
  compiler->labels_int[label] = index;
}

static void
orc_x86_insn_output_opcode_branch_asm (OrcCompiler *p, OrcX86Insn *xinsn)
{
  ORC_ASM_CODE (p,"  %s %d%c\n", xinsn->name, xinsn->label,
          (p->labels_int[xinsn->label] <
           xinsn - ((OrcX86Insn *)p->output_insns)) ? 'b' : 'f');
}

/* FIXME handle the error at the caller */
static const char *
orc_x86_get_simd_regname (OrcX86Insn *xinsn, OrcCompiler *p, int reg, int idx)
{
#if ENABLE_TARGET_MMX
  if (reg >= X86_MM0 && reg < X86_MM0 + 16) {
    return orc_x86_get_regname_mmx (reg);
  }
#endif
#if ENABLE_TARGET_SSE
  if (reg >= X86_XMM0 && reg < X86_XMM0 + 16) {
    return orc_x86_get_regname_sse (reg);
  }
#endif
#if ENABLE_TARGET_AVX
  if (reg >= X86_YMM0 && reg < X86_YMM0 + 16) {
    return orc_x86_get_regname_avx (reg);
  }
#endif
  ORC_COMPILER_ERROR (p, "Unknown reg %d for opcode %s at idx %d", reg, xinsn->name, idx);
  return "ERROR";
}

static void
orc_x86_emit_modrm_memoffset (OrcCompiler *compiler, int offset, int src, int dest)
{
  if (offset == 0 && src != compiler->exec_reg && src != X86_EBP && src != X86_R13) {
    if (src == X86_ESP || src == X86_R12) {
      *compiler->codeptr++ = X86_MODRM(0, 4, dest);
      *compiler->codeptr++ = X86_SIB(0, 4, src);
    } else {
      *compiler->codeptr++ = X86_MODRM(0, src, dest);
    }
  } else if (offset >= -128 && offset < 128) {
    *compiler->codeptr++ = X86_MODRM(1, src, dest);
    if (src == X86_ESP || src == X86_R12) {
      *compiler->codeptr++ = X86_SIB(0, 4, src);
    }
    *compiler->codeptr++ = (offset & 0xff);
  } else {
    *compiler->codeptr++ = X86_MODRM(2, src, dest);
    if (src == X86_ESP || src == X86_R12) {
      *compiler->codeptr++ = X86_SIB(0, 4, src);
    }
    *compiler->codeptr++ = (offset & 0xff);
    *compiler->codeptr++ = ((offset>>8) & 0xff);
    *compiler->codeptr++ = ((offset>>16) & 0xff);
    *compiler->codeptr++ = ((offset>>24) & 0xff);
  }
}

/* FIXME rename this */
static void
orc_x86_emit_modrm_memindex2 (OrcCompiler *compiler, int offset,
    int src, int src_index, int shift, int dest)
{
  if (offset == 0) {
    *compiler->codeptr++ = X86_MODRM(0, 4, dest);
    *compiler->codeptr++ = X86_SIB(shift, src_index, src);
  } else if (offset >= -128 && offset < 128) {
    *compiler->codeptr++ = X86_MODRM(1, 4, dest);
    *compiler->codeptr++ = X86_SIB(shift, src_index, src);
    *compiler->codeptr++ = (offset & 0xff);
  } else {
    *compiler->codeptr++ = X86_MODRM(2, 4, dest);
    *compiler->codeptr++ = X86_SIB(shift, src_index, src);
    *compiler->codeptr++ = (offset & 0xff);
    *compiler->codeptr++ = ((offset>>8) & 0xff);
    *compiler->codeptr++ = ((offset>>16) & 0xff);
    *compiler->codeptr++ = ((offset>>24) & 0xff);
  }
}

static void
orc_x86_emit_modrm_reg (OrcCompiler *compiler, int rm, int reg)
{
  *compiler->codeptr++ = X86_MODRM(3, rm, reg);
}

static unsigned char
orc_x86_insn_get_rex (OrcCompiler *compiler, orc_bool needs_rexw, int reg, int sib, int mr)
{
  unsigned char rex = 0x40;

  if (needs_rexw) rex |= 0x08; // REX.W determines if 64-bit operand
  if (reg & 8) rex |= 0x4; // ModR/M[reg] expands to 64-bit mode operands (R)
  if (sib & 8) rex |= 0x2; // SIB index field extension (X)
  if (mr & 8) rex |= 0x1; // ModR/M[r/m] or extension expands to 64-bit mode operands (B)

  return rex;
}

static orc_bool
orc_x86_insn_need_rex_w (const OrcX86Insn *xinsn)
{
  int i;

  /* If any of the registers is of 64-bit size use REX.W */
  for (i = 0; i < 4; i++) {
    const OrcX86InsnOperand *op = &xinsn->operands[i];

    if (op->type == ORC_X86_INSN_OPERAND_TYPE_REG &&
        op->size == ORC_X86_INSN_OPERAND_SIZE_64)
      return TRUE;
  }
  return FALSE;
}

static orc_bool
orc_x86_insn_need_rex_rxb (const OrcX86Insn *xinsn)
{
  int i;

  /* If any of the registers is a extended register REX.RXB */
  for (i = 0; i < 4; i++) {
    const OrcX86InsnOperand *op = &xinsn->operands[i];

    /* These type of operands do not trigger a REX prefix */
    if (op->type == ORC_X86_INSN_OPERAND_TYPE_IMM ||
        op->type == ORC_X86_INSN_OPERAND_TYPE_NONE)
      continue;

    if (op->reg & 8) {
      return TRUE;
    }
  }
  return FALSE;
}

static orc_bool
orc_x86_insn_need_rex (OrcCompiler *c, const OrcX86Insn *xinsn)
{
  /* No REX for non-64 bits */
  if (!c->is_64bit)
    return FALSE;

  if (!orc_x86_insn_need_rex_w (xinsn) && !orc_x86_insn_need_rex_rxb (xinsn))
    return FALSE;

  /* Disable REX for M encoding or push/pop */
  if (xinsn->encoding == ORC_X86_INSN_ENCODING_M ||
      xinsn->opcode_type == ORC_X86_INSN_OPCODE_TYPE_STACK) {
    return FALSE;
  }
  return TRUE;
}

static void
orc_x86_insn_output_opcode_other_stack_asm (OrcCompiler *p, OrcX86Insn *xinsn)
{
  char operands[4][40] = {0};
  char operands_str[163] = {0}; /* (40 * 4 [operands]) + 3 [comma] */
  int i;
  orc_bool first = TRUE;

  for (i = 3; i >= 0; i--) {
    const OrcX86InsnOperand *op = &xinsn->operands[i];
    char *op_str = operands[i];

    switch (op->type) {
      case ORC_X86_INSN_OPERAND_TYPE_NONE:
        break;

      case ORC_X86_INSN_OPERAND_TYPE_REG:
        switch (op->size) {
          case ORC_X86_INSN_OPERAND_SIZE_8:
            sprintf (op_str, "%%%s", orc_x86_get_regname_8 (op->reg));
            break;
          case ORC_X86_INSN_OPERAND_SIZE_16:
            sprintf (op_str, "%%%s", orc_x86_get_regname_16 (op->reg));
            break;
          case ORC_X86_INSN_OPERAND_SIZE_32:
            sprintf (op_str, "%%%s", orc_x86_get_regname (op->reg));
            break;
          case ORC_X86_INSN_OPERAND_SIZE_64:
            sprintf (op_str, "%%%s", orc_x86_get_regname_64 (op->reg));
            break;

          default:
            sprintf (op_str, "%%%s", orc_x86_get_simd_regname (xinsn, p, op->reg, i));
            break;
        }
        break;

      case ORC_X86_INSN_OPERAND_TYPE_IMM:
        sprintf(op_str, "$%" PRIi64, xinsn->imm);
        break;

      case ORC_X86_INSN_OPERAND_TYPE_IDX:
        sprintf(op_str, "%d(%%%s,%%%s,%d)", xinsn->offset,
            orc_x86_get_regname_ptr (p, op->reg),
            orc_x86_get_regname_ptr (p, xinsn->index_reg),
            1 << xinsn->shift);
        break;

      case ORC_X86_INSN_OPERAND_TYPE_OFF:
        sprintf(op_str, "%d(%%%s)", xinsn->offset,
            orc_x86_get_regname_ptr (p, op->reg));
        break;
    }
    /* join the operands with "," */
    if (!*op_str)
      continue;

    if (first)
      first = FALSE;
    else
      strcat (operands_str, ", ");
    strcat (operands_str, op_str);
  }
  ORC_ASM_CODE (p,"  %s %s\n", xinsn->name, operands_str);
}

static void
orc_x86_insn_output_opcode_asm (OrcCompiler *p, OrcX86Insn *xinsn)
{
  switch (xinsn->opcode_type) {
    case ORC_X86_INSN_OPCODE_TYPE_BRANCH:
      orc_x86_insn_output_opcode_branch_asm (p, xinsn);
      break;

    default:
      orc_x86_insn_output_opcode_other_stack_asm (p, xinsn);
      break;
  }
}

/* Output assembler code in AT&T style (opcode src, dest)*/
static void
orc_x86_insn_output_asm (OrcCompiler *p, OrcX86Insn *xinsn)
{
  switch (xinsn->type) {
    case ORC_X86_INSN_TYPE_OP:
      orc_x86_insn_output_opcode_asm (p, xinsn);
      break;

    case ORC_X86_INSN_TYPE_ALIGN:
      if (xinsn->size > 0) ORC_ASM_CODE(p, ".p2align %d\n", xinsn->size);
      break;

    case ORC_X86_INSN_TYPE_LABEL:
      ORC_ASM_CODE (p, "%d:\n", xinsn->label);
      break;

    case ORC_X86_INSN_TYPE_COMMENT:
      ORC_ASM_CODE (p, "%s\n", xinsn->comment);
      break;
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
orc_x86_insn_output_rex (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  orc_bool needs_rexw;

  if (!orc_x86_insn_need_rex (p, xinsn))
    return;

  needs_rexw = orc_x86_insn_need_rex_w (xinsn);

  switch (xinsn->encoding) {
    case ORC_X86_INSN_ENCODING_RM:
    case ORC_X86_INSN_ENCODING_RMI:
      *p->codeptr++ = orc_x86_insn_get_rex (p, needs_rexw, xinsn->operands[0].reg, 0, xinsn->operands[1].reg);
      break;

    case ORC_X86_INSN_ENCODING_MI:
    case ORC_X86_INSN_ENCODING_MR:
    case ORC_X86_INSN_ENCODING_MRI:
      *p->codeptr++ = orc_x86_insn_get_rex (p, needs_rexw, xinsn->operands[1].reg, 0, xinsn->operands[0].reg);
      break;

    case ORC_X86_INSN_ENCODING_O:
    case ORC_X86_INSN_ENCODING_OI:
      *p->codeptr++ = orc_x86_insn_get_rex (p, needs_rexw, 0, 0, xinsn->operands[0].reg);
      break;

    default:
      break;
  }
}

static void
orc_x86_insn_output_immediate (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  int i;

  if (xinsn->encoding == ORC_X86_INSN_ENCODING_RVMR) {
    *p->codeptr++ = (xinsn->operands[3].reg & 0xF) << 4;
    return;
  }

  for (i = 0; i < 4; i++) {
    /* Check of the operand is imm */
    if (xinsn->operands[i].type != ORC_X86_INSN_OPERAND_TYPE_IMM)
      continue;
    /* Check the size of the operand */
    switch (xinsn->operands[i].size) {
      case ORC_X86_INSN_OPERAND_SIZE_8:
        *p->codeptr++ = xinsn->imm & 0xff;
        break;
      case ORC_X86_INSN_OPERAND_SIZE_16:
        *p->codeptr++ = xinsn->imm & 0xff;
        *p->codeptr++ = (xinsn->imm >> 8) & 0xff;
        break;
      case ORC_X86_INSN_OPERAND_SIZE_32:
        *p->codeptr++ = xinsn->imm & 0xff;
        *p->codeptr++ = (xinsn->imm >> 8) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 16) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 24) & 0xff;
        break;
      case ORC_X86_INSN_OPERAND_SIZE_64:
        *p->codeptr++ = xinsn->imm & 0xff;
        *p->codeptr++ = (xinsn->imm >> 8) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 16) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 24) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 32) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 40) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 48) & 0xff;
        *p->codeptr++ = (xinsn->imm >> 56) & 0xff;
        break;
      default:
        ORC_ERROR ("Unsupported size %d", xinsn->operands[i].size);
        break;
    }
    /* Only one IMM operand */
    break;
  }
}

static void
orc_x86_insn_output_modrm (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  switch (xinsn->encoding) {
    case ORC_X86_INSN_ENCODING_NONE:
      break;

    /* For M and MI the extension code is set at ModRM[reg] */
    case ORC_X86_INSN_ENCODING_M:
    case ORC_X86_INSN_ENCODING_MI:
      if (xinsn->operands[0].type == ORC_X86_INSN_OPERAND_TYPE_REG)
        orc_x86_emit_modrm_reg (p, xinsn->operands[0].reg, xinsn->extension);
      else if (xinsn->operands[0].type == ORC_X86_INSN_OPERAND_TYPE_OFF)
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->operands[0].reg,
            xinsn->extension);
      else
        ORC_ERROR ("M Encoding from wrong operand type %d", xinsn->operands[0].type);
      break;

    case ORC_X86_INSN_ENCODING_MR:
    case ORC_X86_INSN_ENCODING_MRI:
      if (xinsn->operands[0].type == ORC_X86_INSN_OPERAND_TYPE_REG)
        orc_x86_emit_modrm_reg (p, xinsn->operands[0].reg,
            xinsn->operands[1].reg);
      else if (xinsn->operands[0].type == ORC_X86_INSN_OPERAND_TYPE_OFF)
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->operands[0].reg,
            xinsn->operands[1].reg);
      else if (xinsn->operands[0].type == ORC_X86_INSN_OPERAND_TYPE_IDX)
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->operands[0].reg,
            xinsn->index_reg, xinsn->shift, xinsn->operands[1].reg);
      break;

    case ORC_X86_INSN_ENCODING_VMI:
      orc_x86_emit_modrm_reg (p, xinsn->operands[1].reg, xinsn->extension);
      break;

    case ORC_X86_INSN_ENCODING_RVM:
    case ORC_X86_INSN_ENCODING_RVMI:
    case ORC_X86_INSN_ENCODING_RVMR:
      if (xinsn->operands[2].type == ORC_X86_INSN_OPERAND_TYPE_REG)
        orc_x86_emit_modrm_reg (p, xinsn->operands[2].reg, xinsn->operands[0].reg);
      else if (xinsn->operands[2].type == ORC_X86_INSN_OPERAND_TYPE_OFF)
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->operands[2].reg,
            xinsn->operands[0].reg);
      else if (xinsn->operands[2].type == ORC_X86_INSN_OPERAND_TYPE_IDX)
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->operands[2].reg,
            xinsn->index_reg, xinsn->shift, xinsn->operands[0].reg);
      break;

    case ORC_X86_INSN_ENCODING_RM:
    case ORC_X86_INSN_ENCODING_RMI:
      if (xinsn->operands[1].type == ORC_X86_INSN_OPERAND_TYPE_REG)
        orc_x86_emit_modrm_reg (p, xinsn->operands[1].reg,
            xinsn->operands[0].reg);
      if (xinsn->operands[1].type == ORC_X86_INSN_OPERAND_TYPE_OFF)
        orc_x86_emit_modrm_memoffset (p, xinsn->offset, xinsn->operands[1].reg, 
          xinsn->operands[0].reg);
      else if (xinsn->operands[1].type == ORC_X86_INSN_OPERAND_TYPE_IDX)
        orc_x86_emit_modrm_memindex2 (p, xinsn->offset, xinsn->operands[1].reg,
            xinsn->index_reg, xinsn->shift, xinsn->operands[0].reg);
      break;

    /* Immediate only, will be handled later */
    case ORC_X86_INSN_ENCODING_I:
    /* This case is handled in the opcode itself */
    case ORC_X86_INSN_ENCODING_O:
    /* This case is handled in the opcode itself and then the immediate */
    case ORC_X86_INSN_ENCODING_OI:
    /* No operands, do nothing */
    case ORC_X86_INSN_ENCODING_ZO:
      break;
  }
}

static void
orc_x86_insn_output_op (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  /* First the escape sequence */
  if (xinsn->prefix == ORC_X86_INSN_PREFIX_NO_PREFIX) {
    switch (xinsn->opcode_escape) {
      case ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE:
        break;
      case ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F:
        *p->codeptr++ = 0x0f;
        break;
      case ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38:
        *p->codeptr++ = 0x0f;
        *p->codeptr++ = 0x38;
        break;
      case ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A:
        *p->codeptr++ = 0x0f;
        *p->codeptr++ = 0x3a;
        break;
    }
  }

  /* For branch opcodes we need to rewrite it to be short or near
   * based on orc_x86_recalc_offsets outcome
   */
  if (xinsn->opcode_type == ORC_X86_INSN_OPCODE_TYPE_BRANCH) {
    if (xinsn->size == 4) {
      if (!strcmp (xinsn->name, "jmp")) {
        *p->codeptr++ = 0xe9; /* Near JMP */
      } else {
        *p->codeptr++ = 0x0f; /* Near Jcc */
        *p->codeptr++ = xinsn->opcode + 0x10;
      }
    } else {
      *p->codeptr++ = xinsn->opcode;
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
  } else {
    /* The actual opcode */
    if (xinsn->encoding == ORC_X86_INSN_ENCODING_O || xinsn->encoding == ORC_X86_INSN_ENCODING_OI) {
      *p->codeptr++ = xinsn->opcode + (xinsn->operands[0].reg & 0x7);
    } else {
      int two_bytes;

      two_bytes = (xinsn->opcode >> 8) & 0xff;
      if (two_bytes)
        *p->codeptr++ = two_bytes;
      *p->codeptr++ = (xinsn->opcode >> 0) & 0xff;
    }
  }
}

static void
orc_x86_insn_output_opcode (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  switch (xinsn->type) {
    case ORC_X86_INSN_TYPE_OP:
      orc_x86_insn_output_op (p, xinsn);
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
      x86_add_label (p, p->codeptr, xinsn->label);
      break;

    case ORC_X86_INSN_TYPE_COMMENT:
      break;
  }
}

static void
orc_x86_insn_output_prefix (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  switch (xinsn->opcode_prefix) {
    case ORC_X86_INSN_OPCODE_PREFIX_0X66:
      *p->codeptr++ = 0x66;
      break;
    case ORC_X86_INSN_OPCODE_PREFIX_0XF2: 
      *p->codeptr++ = 0xf2;
      break;
    case ORC_X86_INSN_OPCODE_PREFIX_0XF3: 
      *p->codeptr++ = 0xf3;
      break;
    default:
      break;
  }
}

/* The four v̅ bits are the inversion of an additional source register index. */
static unsigned char
orc_x86_insn_get_vex_vvvv (const OrcX86Insn *xinsn)
{
  unsigned char vvvv = X86_VEX_vvvv_UNUSED;

  // If an instruction does not use VEX.vvvv then it should be set to
  // 1111b otherwise instruction will #UD.

  // Get the 1-complement of the register (we always use the lower four bits)
  // And mask it appropriately
  switch (xinsn->encoding) {
    case ORC_X86_INSN_ENCODING_VMI:
      vvvv = (~xinsn->operands[0].reg & 0xF) << 3 & X86_VEX_vvvv_MASK;
      break;
    case ORC_X86_INSN_ENCODING_RVM:
    case ORC_X86_INSN_ENCODING_RVMI:
    case ORC_X86_INSN_ENCODING_RVMR: 
      vvvv = (~xinsn->operands[1].reg & 0xF) << 3 & X86_VEX_vvvv_MASK;
      break;
    default:
      break;
  }
  return vvvv;
}

static unsigned char
orc_x86_insn_get_vex_rex (OrcCompiler *compiler, int reg, int sib, int rm)
{
  // In protected and compatibility modes the bit must be set to ‘1’
  // otherwise the instruction is LES or LDS.
  unsigned char rex = 0x80 | 0x40 | 0x20;

  if (compiler->is_64bit) {
    if (reg & 8) rex &= ~0x80; // ModR/M[reg] expands to 64-bit mode operands (R)
    if (sib & 8) rex &= ~0x40; // SIB index field extension (X)
    if (rm & 8) rex &= ~0x20; // ModR/M[r/m] or extension expandds to 64-bit mode operands (B)
  }

  return rex;
}

/*
 * The 2 byte version
 * 1  1  0  0  0 1  0  1
 * R̅ v̅3 v̅2 v̅1 v̅0 L p1 p0
 *
 * p1..p0 are used to code opcode prefix
 * v̅3..v̅0 are used to code the inverse of the extra source operand 
 */
static void
orc_x86_insn_output_vex2 (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  // In protected and compatibility modes the REX.R bit must be set to ‘1’
  // otherwise the instruction is LES or LDS.
  unsigned char byte2 = 0x80;

  // Section 2.3.5.6, 2.3.6
  // Instruction Operand Encoding and VEX.vvvv, ModR/M
  byte2 |= orc_x86_insn_get_vex_vvvv (xinsn);

  // Set vector length
  if (xinsn->prefix == ORC_X86_INSN_PREFIX_VEX256)
    byte2 |= 0x4;

  // Opcode prefix, we use the same representation as the expected value
  byte2 |= xinsn->opcode_prefix;

  *p->codeptr++ = X86_VEX_2_BYTES;
  *p->codeptr++ = byte2;
}

/*
 * The 3 byte version
 * 1  1  0  0  0  1  0  0
 * R̅  X̅  B̅ m4 m3 m2 m1 m0
 * W v̅3 v̅2 v̅1 v̅0  L p1 p0
 *
 * m4..m0 are used to code opcode escape byte sequences
 */
static void
orc_x86_insn_output_vex3 (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  unsigned char byte2 = 0;
  unsigned char byte3 = 0;

  if (p->is_64bit) {
    switch (xinsn->encoding) {
      case ORC_X86_INSN_ENCODING_RM:
      case ORC_X86_INSN_ENCODING_RMI:
        byte2 |= orc_x86_insn_get_vex_rex (p, xinsn->operands[0].reg, 0, xinsn->operands[1].reg);
        break;
  
      case ORC_X86_INSN_ENCODING_MI:
      case ORC_X86_INSN_ENCODING_MR:
      case ORC_X86_INSN_ENCODING_MRI:
        byte2 |= orc_x86_insn_get_vex_rex (p, xinsn->operands[1].reg, 0, xinsn->operands[0].reg);
        break;
  
      case ORC_X86_INSN_ENCODING_O:
        byte2 |= orc_x86_insn_get_vex_rex (p, 0, 0, xinsn->operands[0].reg);
        break;
  
      case ORC_X86_INSN_ENCODING_VMI:
        byte2 |= orc_x86_insn_get_vex_rex (p, 0, 0, xinsn->operands[1].reg);
        break;
  
      case ORC_X86_INSN_ENCODING_RVM:
      case ORC_X86_INSN_ENCODING_RVMI:
      case ORC_X86_INSN_ENCODING_RVMR:
        byte2 |= orc_x86_insn_get_vex_rex (p, xinsn->operands[0].reg, 0, xinsn->operands[2].reg);
        break;
  
      default:
        break;
    }
  } else {
    // REX.R,X must be force set in 32-bit
    byte2 |= 1 << 7;
    byte2 |= 1 << 6;
  }
  // Escape sequence, we use the same representation as the expected value
  byte2 |= xinsn->opcode_escape;

  // Set REX.W if forced
  if (xinsn->opcode_flags & ORC_X86_INSN_OPCODE_FLAG_VEX_W1 ||
    orc_x86_insn_need_rex_w (xinsn)) {
    byte3 |= 1 << 7;
  }
  // TODO Clear REX.W if forced

  // Section 2.3.5.6, 2.3.6
  // Instruction Operand Encoding and VEX.vvvv, ModR/M
  byte3 |= orc_x86_insn_get_vex_vvvv (xinsn);

  // Set vector length
  if (xinsn->prefix == ORC_X86_INSN_PREFIX_VEX256)
    byte3 |= 0x4;

  // Opcode prefix, we use the same representation as the expected value
  byte3 |= xinsn->opcode_prefix;

  *p->codeptr++ = X86_VEX_3_BYTES;
  *p->codeptr++ = byte2;
  *p->codeptr++ = byte3;
}

/* VEX prefix can be 2 bytes (0xC5) or 3 bytes (0xC4)
 * The difference between vex2 and vex3 is that:
 * - X̅, B̅ and W bits are not present in the VEX2 prefix
 * - m bits are not present in the VEX2 prefix
 * - The 0x0F escape is implied
 */
static void
orc_x86_insn_output_vex (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  orc_bool use_vex3 = FALSE;

  if (p->is_64bit) {
    int i;
    /* Check if we need to use X̅, B̅ registers */
    for (i = 0; i < 4; i++) {
      const OrcX86InsnOperand *op = &xinsn->operands[i];

      if ((op->type == ORC_X86_INSN_OPERAND_TYPE_REG ||
          op->type == ORC_X86_INSN_OPERAND_TYPE_OFF ||
          op->type == ORC_X86_INSN_OPERAND_TYPE_IDX) && (op->reg & 8)) {
        use_vex3 = TRUE;
        goto done;
      }
    }

    /* Check if we need REX.W */
    use_vex3 = orc_x86_insn_need_rex_w (xinsn);
    if (use_vex3)
      goto done;
  }

  /* Check if we need to use W based on the VEX flags */
  if (xinsn->opcode_flags & ORC_X86_INSN_OPCODE_FLAG_VEX_W1) {
    use_vex3 = TRUE;
    goto done;
  }

  /* Check if we need to use m4..m0 */
  if (xinsn->opcode_escape &&
      xinsn->opcode_escape != ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F) {
    use_vex3 = TRUE;
    goto done;
  }

done:
  if (use_vex3)
    orc_x86_insn_output_vex3 (p, xinsn);
  else
    orc_x86_insn_output_vex2 (p, xinsn);
}

static void
orc_x86_insn_output_machine_code (OrcCompiler *p, const OrcX86Insn *xinsn)
{
  switch (xinsn->prefix) {
    case ORC_X86_INSN_PREFIX_NO_PREFIX:
      orc_x86_insn_output_prefix (p, xinsn);
      orc_x86_insn_output_rex (p, xinsn);
      orc_x86_insn_output_opcode (p, xinsn);
      orc_x86_insn_output_modrm (p, xinsn);
      orc_x86_insn_output_immediate (p, xinsn);
      break;

    case ORC_X86_INSN_PREFIX_VEX128:
    case ORC_X86_INSN_PREFIX_VEX256:
      orc_x86_insn_output_vex (p, xinsn);
      orc_x86_insn_output_opcode (p, xinsn);
      orc_x86_insn_output_modrm (p, xinsn);
      orc_x86_insn_output_immediate (p, xinsn);
      break;

    default:
      ORC_COMPILER_ERROR (p, "Unimplemented prefix %i", xinsn->prefix);
      return;
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
    orc_x86_insn_output_machine_code (p, xinsn);

    if (xinsn->type == ORC_X86_INSN_TYPE_ALIGN) {
      if (xinsn->size > 0) {
        minptr += ((p->code - minptr)&((1<<xinsn->size) - 1));
      }
    } else {
      minptr += p->codeptr - ptr;
      if (xinsn->type == ORC_X86_INSN_TYPE_OP && xinsn->opcode_type == ORC_X86_INSN_OPCODE_TYPE_BRANCH) {
        if (xinsn->size == 4) minptr -= 4;
      }
    }

  }

  p->codeptr = p->code;
  p->n_fixups = 0;
}

/* Operand size and Address size. Vol.1, Ch.3, 3.6.1 */
static void
orc_x86_insn_set_size_modes (OrcCompiler *c, OrcX86Insn *xinsn)
{
  int i;

  /* By defaut the operand size in 64-bits and 32-bits is 32-bits, if we need
   * 16-bits use the operand size prefix.
   */
  for (i = 0; i < 4; i++) {
    const OrcX86InsnOperand *op = &xinsn->operands[i];

    if (op->type == ORC_X86_INSN_OPERAND_TYPE_REG && op->size == ORC_X86_INSN_OPERAND_SIZE_16) {
      xinsn->opcode_prefix = ORC_X86_INSN_OPCODE_PREFIX_0X66;
      ORC_DEBUG ("Setting operand size to 16-bits for instruction '%s'", xinsn->name);
      break;
    }
  }
}

static void
orc_x86_insn_from_opcode (OrcX86Insn *insn, const OrcX86InsnOpcode *opcode, int size)
{
  insn->name = opcode->name;
  insn->opcode = opcode->opcode;
  insn->opcode_prefix = opcode->prefix;
  insn->opcode_type = opcode->type;
  insn->opcode_escape = opcode->escape;
  insn->prefix = ORC_X86_INSN_PREFIX_NO_PREFIX;
  insn->extension = opcode->extension;
  insn->size = size;
}

static orc_bool
orc_x86_insn_validate_operand1_size (OrcX86InsnOperandSize size, int operands)
{
  if (size == ORC_X86_INSN_OPERAND_SIZE_8 &&
      !(operands & ORC_X86_INSN_OPERAND_OP1_8)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_16 &&
      !(operands & ORC_X86_INSN_OPERAND_OP1_16)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_32 &&
      !(operands & ORC_X86_INSN_OPERAND_OP1_32)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_64 &&
      !(operands & ORC_X86_INSN_OPERAND_OP1_64)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  }

  return TRUE;
}

static orc_bool
orc_x86_insn_validate_operand2_size (OrcX86InsnOperandSize size, int operands)
{
  if (size == ORC_X86_INSN_OPERAND_SIZE_8 &&
      !(operands & ORC_X86_INSN_OPERAND_OP2_8)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_16 &&
      !(operands & ORC_X86_INSN_OPERAND_OP2_16)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_32 &&
      !(operands & ORC_X86_INSN_OPERAND_OP2_32)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_64 &&
      !(operands & ORC_X86_INSN_OPERAND_OP2_64)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  }

  return TRUE;
}

#if 0
/* FIXME for later, once all validations are properly handled */
static orc_bool
orc_x86_insn_validate_operand3_size (OrcX86InsnOperandSize size, int operands)
{
  if (size == ORC_X86_INSN_OPERAND_SIZE_8 &&
      !(operands & ORC_X86_INSN_OPERAND_OP3_8)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_16 &&
      !(operands & ORC_X86_INSN_OPERAND_OP3_16)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_32 &&
      !(operands & ORC_X86_INSN_OPERAND_OP3_32)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  } else if (size == ORC_X86_INSN_OPERAND_SIZE_64 &&
      !(operands & ORC_X86_INSN_OPERAND_OP3_64)) {
    ORC_ERROR ("Wrong size of %d", size);
    return FALSE;
  }

  return TRUE;
}
#endif

/* FIXME don't access the compiler directly */
OrcX86Insn *
orc_x86_get_output_insn (OrcCompiler *p)
{
  OrcX86Insn *xinsn;
  if (p->n_output_insns >= p->n_output_insns_alloc) {
    p->n_output_insns_alloc += 10;
    p->output_insns = orc_realloc (p->output_insns,
        sizeof(OrcX86Insn) * p->n_output_insns_alloc);
  }

  xinsn = ((OrcX86Insn *)p->output_insns) + p->n_output_insns;
  memset (xinsn, 0, sizeof(OrcX86Insn));
  p->n_output_insns++;
  return xinsn;
}

orc_bool
orc_x86_insn_encoding_from_operands (OrcX86InsnEncoding *encoding, int operands,
    OrcX86InsnPrefix prefix)
{
  OrcX86InsnEncoding e;

  /* For zero opeands */
  if (!operands) {
    e = ORC_X86_INSN_ENCODING_ZO;
    goto done;
  }

  /* Check first operand */
  if (operands & ORC_X86_INSN_OPERAND_OP1_MEM) {
    e = ORC_X86_INSN_ENCODING_M;
  } else if (operands & ORC_X86_INSN_OPERAND_OP1_IMM) {
    e = ORC_X86_INSN_ENCODING_I;
  } else if (operands & ORC_X86_INSN_OPERAND_OP1_REG) {
    e = ORC_X86_INSN_ENCODING_O;
  } else {
    ORC_ERROR ("Unsupported first operand type");
    goto error;
  }
  /* Only one operand */
  if (!(operands & ORC_X86_INSN_OPERAND_OP2))
    goto done;

  /* Check second operand */
  if (operands & ORC_X86_INSN_OPERAND_OP2_MEM) {
    e = ORC_X86_INSN_ENCODING_RM;
  } else if (operands & ORC_X86_INSN_OPERAND_OP2_REG) {
    if (e == ORC_X86_INSN_ENCODING_O || e == ORC_X86_INSN_ENCODING_M) {
      e = ORC_X86_INSN_ENCODING_MR;
    } else {
      ORC_ERROR ("Unsupported previous encoding %d for OP2 MEM", e);
      goto error;
    }
  } else if (operands & ORC_X86_INSN_OPERAND_OP2_IMM) {
    if (e == ORC_X86_INSN_ENCODING_M) {
      e = ORC_X86_INSN_ENCODING_MI;
    } else if (e == ORC_X86_INSN_ENCODING_O) {
      e = ORC_X86_INSN_ENCODING_OI;
    } else {
      ORC_ERROR ("Unsupported previous encoding %d for OP2 IMM", e);
      goto error;
    }
  }

  /* Only two operands */
  if (!(operands & ORC_X86_INSN_OPERAND_OP3))
    goto done;

  /* Check third operand */
  if (operands & ORC_X86_INSN_OPERAND_OP3_IMM) {
    if (e == ORC_X86_INSN_ENCODING_MR) {
      e = ORC_X86_INSN_ENCODING_MRI;
      /* For VEX, MRI is encoded as VRI as long as M is also a R (i.e not M) */
      if ((prefix == ORC_X86_INSN_PREFIX_VEX128 || 
          prefix == ORC_X86_INSN_PREFIX_VEX256) &&
          !(operands & ORC_X86_INSN_OPERAND_OP1_MEM)) {
        e = ORC_X86_INSN_ENCODING_VMI;
      }
    } else if (e == ORC_X86_INSN_ENCODING_RM) {
      e = ORC_X86_INSN_ENCODING_RMI;
    } else {
      ORC_ERROR ("Unsupported previous encoding %d for OP3 IMM", e);
      goto error;
    }
  } else if (operands & ORC_X86_INSN_OPERAND_OP3_REG) {
    /* For VEX, three reg opcode is encoded as RVM */
    if ((prefix == ORC_X86_INSN_PREFIX_VEX128 || 
        prefix == ORC_X86_INSN_PREFIX_VEX256) &&
        (e == ORC_X86_INSN_ENCODING_MR ||
        e == ORC_X86_INSN_ENCODING_RM)) {
      e = ORC_X86_INSN_ENCODING_RVM;
    } else {
      ORC_ERROR ("Unsupported previous encoding %d for OP3 REG", e);
      goto error;
    }
  }

  if (!(operands & ORC_X86_INSN_OPERAND_OP4))
    goto done;

  /* Check fourth operand */
  if (operands & ORC_X86_INSN_OPERAND_OP4_IMM) {
    if (e == ORC_X86_INSN_ENCODING_RVM) {
      e = ORC_X86_INSN_ENCODING_RVMI;
    } else {
      ORC_ERROR ("Unsupported previous encoding %d for OP4 IMM", e);
      goto error;
    }
  } else if (operands & ORC_X86_INSN_OPERAND_OP4_REG) {
    if (e == ORC_X86_INSN_ENCODING_RVM) {
      e = ORC_X86_INSN_ENCODING_RVMR;
    } else {
      ORC_ERROR ("Unsupported previous encoding %d for OP4 REG", e);
      goto error;
    }
  }

done:
  *encoding = e;
  return TRUE;

error:
  ORC_ERROR ("Operand %08x can not be encoded", operands);
  return FALSE;
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
      if ((xinsn->type != ORC_X86_INSN_TYPE_OP) ||
          (xinsn->opcode_type != ORC_X86_INSN_OPCODE_TYPE_BRANCH)) {
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

OrcX86InsnOperandSize
orc_x86_insn_operand1_size_from_operands (int operands)
{
  if (operands & ORC_X86_INSN_OPERAND_OP1_8)
    return ORC_X86_INSN_OPERAND_SIZE_8;
  else if (operands & ORC_X86_INSN_OPERAND_OP1_16)
    return ORC_X86_INSN_OPERAND_SIZE_16;
  else if (operands & ORC_X86_INSN_OPERAND_OP1_32)
    return ORC_X86_INSN_OPERAND_SIZE_32;
  else if (operands & ORC_X86_INSN_OPERAND_OP1_64)
    return ORC_X86_INSN_OPERAND_SIZE_64;
  else
    return ORC_X86_INSN_OPERAND_SIZE_NONE;
}

OrcX86InsnOperandSize
orc_x86_insn_operand2_size_from_operands (int operands)
{
  if (operands & ORC_X86_INSN_OPERAND_OP2_8)
    return ORC_X86_INSN_OPERAND_SIZE_8;
  else if (operands & ORC_X86_INSN_OPERAND_OP2_16)
    return ORC_X86_INSN_OPERAND_SIZE_16;
  else if (operands & ORC_X86_INSN_OPERAND_OP2_32)
    return ORC_X86_INSN_OPERAND_SIZE_32;
  else if (operands & ORC_X86_INSN_OPERAND_OP2_64)
    return ORC_X86_INSN_OPERAND_SIZE_64;
  else
    return ORC_X86_INSN_OPERAND_SIZE_NONE;
}

OrcX86InsnOperandSize
orc_x86_insn_operand3_size_from_operands (int operands)
{
  if (operands & ORC_X86_INSN_OPERAND_OP3_8)
    return ORC_X86_INSN_OPERAND_SIZE_8;
  else if (operands & ORC_X86_INSN_OPERAND_OP3_16)
    return ORC_X86_INSN_OPERAND_SIZE_16;
  else if (operands & ORC_X86_INSN_OPERAND_OP3_32)
    return ORC_X86_INSN_OPERAND_SIZE_32;
  else if (operands & ORC_X86_INSN_OPERAND_OP3_64)
    return ORC_X86_INSN_OPERAND_SIZE_64;
  else
    return ORC_X86_INSN_OPERAND_SIZE_NONE;
}

/* FIXME remove once the emit functions receive the enum instead */
OrcX86InsnOperandSize
orc_x86_insn_size_to_operand_size (int size)
{
  switch (size) {
    case 0:
      return ORC_X86_INSN_OPERAND_SIZE_NONE;
    case 1:
      return ORC_X86_INSN_OPERAND_SIZE_8;
    case 2:
      return ORC_X86_INSN_OPERAND_SIZE_16;
    case 4:
      return ORC_X86_INSN_OPERAND_SIZE_32;
    case 8:
      return ORC_X86_INSN_OPERAND_SIZE_64;
    default:
      ORC_ERROR ("Unsupported size %d", size);
      return 0;
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
    orc_x86_insn_output_machine_code (p, xinsn);
  }
}

orc_bool
orc_x86_insn_validate_no_operands (int operands)
{
  if (!(operands == ORC_X86_INSN_OPERAND_NONE)) {
    ORC_ERROR ("Calling a no operand opcode with operands");
    return FALSE;
  }
  return TRUE;
}

static orc_bool
orc_x86_in_range(orc_int64 imm, orc_int64 int_min, orc_int64 int_max, orc_uint64 uint_max)
{
  if ((imm >= int_min && imm <= int_max) ||
      ((orc_uint64)imm >= 0 && (orc_uint64)imm <= uint_max))
    return TRUE;
  return FALSE;
}

static orc_bool
orc_x86_validate_imm_value (orc_int64 imm, OrcX86InsnOperandSize size)
{
  switch (size) {
    case ORC_X86_INSN_OPERAND_SIZE_8:
      if (!orc_x86_in_range (imm, INT8_MIN, INT8_MAX, UINT8_MAX)) {
        ORC_ERROR ("Immediate value %" PRIi64 " exceeds range for imm8", imm);
        return FALSE;
      }
      break;
    case ORC_X86_INSN_OPERAND_SIZE_16:
      if (!orc_x86_in_range (imm, INT16_MIN, INT16_MAX, UINT16_MAX)) {
        ORC_ERROR ("Immediate value %" PRIi64 " exceeds range for imm16", imm);
        return FALSE;
      }
      break;
    case ORC_X86_INSN_OPERAND_SIZE_32:
      if (!orc_x86_in_range (imm, INT32_MIN, INT32_MAX, UINT32_MAX)) {
        ORC_ERROR ("Immediate value %" PRIi64 " exceeds range for imm32", imm);
        return FALSE;
      }
      break;
    default:
      break;
  }
  return TRUE;
}

orc_bool
orc_x86_insn_validate_operand1_reg (int reg, OrcX86InsnOperandSize size, int operands)
{
  if (reg && (operands & ORC_X86_INSN_OPERAND_OP1_REG))
    return orc_x86_insn_validate_operand1_size (size, operands);
  else
    return FALSE;
}

orc_bool
orc_x86_insn_validate_operand1_mem (int reg, int operands)
{
  if (reg && (operands & ORC_X86_INSN_OPERAND_OP1_MEM))
    return TRUE;
  else
    return FALSE;
}

orc_bool
orc_x86_insn_validate_operand1_imm (orc_int64 imm, int operands)
{
  if (operands & ORC_X86_INSN_OPERAND_OP1_IMM)
    return orc_x86_validate_imm_value (imm, orc_x86_insn_operand1_size_from_operands (operands));
  else
    return FALSE;
}

orc_bool
orc_x86_insn_validate_operand2_reg (int reg, OrcX86InsnOperandSize size, int operands)
{
  if (reg && (operands & ORC_X86_INSN_OPERAND_OP2_REG))
    return orc_x86_insn_validate_operand2_size (size, operands);
  else
    return FALSE;
}

orc_bool
orc_x86_insn_validate_operand2_mem (int reg, int operands)
{
  if (reg && (operands & ORC_X86_INSN_OPERAND_OP2_MEM))
    return TRUE;
  else
    return FALSE;
}

orc_bool
orc_x86_insn_validate_operand2_imm (orc_int64 imm, int operands)
{
  if (operands & ORC_X86_INSN_OPERAND_OP2_IMM)
    return orc_x86_validate_imm_value (imm, orc_x86_insn_operand2_size_from_operands (operands));
  else
    return FALSE;
}

orc_bool
orc_x86_insn_validate_operand3_imm (orc_int64 imm, int operands)
{
  if (operands & ORC_X86_INSN_OPERAND_OP3_IMM)
    return orc_x86_validate_imm_value (imm, orc_x86_insn_operand3_size_from_operands (operands));
  else
    return FALSE;
}

void
orc_x86_emit_cpuinsn_size (OrcCompiler *p, int index, int size, int src, int dest)
{
  OrcX86Insn *xinsn;
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;

  /* checks */
  if (!orc_x86_insn_validate_operand1_reg (dest, opsize, opcode->operands)) {
    ORC_ERROR ("Setting a dst register %s in a wrong opcode %d",
        orc_x86_get_regname_size (dest, size), index);
  }

  if (src && !orc_x86_insn_validate_operand2_reg (src, opsize, opcode->operands)) {
    ORC_ERROR ("Src register %d not validated for opcode %d", dest, index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_x86_insn_from_opcode (xinsn, opcode, size);
  orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_REG,
      opsize, dest);
  if (opcode->operands & ORC_X86_INSN_OPERAND_OP1_MEM)
    xinsn->encoding = ORC_X86_INSN_ENCODING_M;
  else
    xinsn->encoding = ORC_X86_INSN_ENCODING_O;

  if (src) {
    orc_x86_insn_operand_set (&xinsn->operands[1], ORC_X86_INSN_OPERAND_TYPE_REG,
        opsize, src);
    if (opcode->operands & ORC_X86_INSN_OPERAND_OP2_MEM)
      xinsn->encoding = ORC_X86_INSN_ENCODING_RM;
    else
      xinsn->encoding = ORC_X86_INSN_ENCODING_MR;
  }

  orc_x86_insn_set_size_modes (p, xinsn);
}

void
orc_x86_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int src_index, int shift, int dest)
{
  OrcX86Insn *xinsn;
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;
  orc_bool has_imm3;

  /* checks */
  has_imm3 = orc_x86_insn_validate_operand3_imm (imm, opcode->operands);

  if (!orc_x86_insn_validate_operand1_reg (dest, opsize, opcode->operands)) {
    ORC_ERROR ("Dest register %d not validated for opcode %d", dest, index);
  }

  if (!orc_x86_insn_validate_operand2_mem (src, opcode->operands)) {
    ORC_ERROR ("Src register %d not validated for opcode %d", src, index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_x86_insn_from_opcode (xinsn, opcode, size);
  xinsn->encoding = ORC_X86_INSN_ENCODING_RM;
  orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_REG,
      opsize, dest);
  orc_x86_insn_operand_set (&xinsn->operands[1], ORC_X86_INSN_OPERAND_TYPE_IDX,
      opsize, src);
  if (has_imm3) {
    orc_x86_insn_operand_set (&xinsn->operands[2], ORC_X86_INSN_OPERAND_TYPE_IMM,
        orc_x86_insn_operand3_size_from_operands (opcode->operands), 0);
    xinsn->encoding = ORC_X86_INSN_ENCODING_RMI;
    xinsn->imm = imm;
  }
  xinsn->offset = offset;
  xinsn->index_reg = src_index;
  xinsn->shift = shift;
  orc_x86_insn_set_size_modes (p, xinsn);
}

void
orc_x86_emit_cpuinsn_imm_reg (OrcCompiler *p, int index, int size, orc_int64 imm,
    int dest)
{
  OrcX86Insn *xinsn;
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;
  orc_bool has_imm1;
  orc_bool has_imm2;

  /* checks */
  has_imm1 = orc_x86_insn_validate_operand1_imm (imm, opcode->operands);
  has_imm2 = orc_x86_insn_validate_operand2_imm (imm, opcode->operands);
  if (!has_imm1 && !has_imm2) {
    ORC_ERROR ("Setting an imm %" PRIi64 " in a wrong opcode %d (%s)", imm, index, opcode->name);
  }

  if (!has_imm1 && !orc_x86_insn_validate_operand1_reg (dest, opsize, opcode->operands)) {
    ORC_ERROR ("Setting a dst register %s in a wrong opcode %d",
        orc_x86_get_regname_size (dest, size), index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_x86_insn_from_opcode (xinsn, opcode, size);
  if (has_imm1) {
    orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_IMM,
        orc_x86_insn_operand1_size_from_operands (opcode->operands), 0);
    xinsn->encoding = ORC_X86_INSN_ENCODING_I;
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[1], ORC_X86_INSN_OPERAND_TYPE_IMM,
        orc_x86_insn_operand2_size_from_operands (opcode->operands), 0);
    orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_REG,
        opsize, dest);
    xinsn->encoding = ORC_X86_INSN_ENCODING_OI;
    if (opcode->operands & ORC_X86_INSN_OPERAND_OP1_MEM) {
      xinsn->encoding = ORC_X86_INSN_ENCODING_MI;
    }
  }

  xinsn->imm = imm;
  orc_x86_insn_set_size_modes (p, xinsn);
}

void
orc_x86_emit_cpuinsn_imm_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;

  /* checks */
  if (!orc_x86_insn_validate_operand2_imm (imm, opcode->operands))
    ORC_ERROR ("Setting an imm %" PRIi64 " in a wrong opcode %d (%s)", imm, index, opcode->name);
  else if (!orc_x86_insn_validate_operand1_mem (dest, opcode->operands))
    ORC_ERROR ("Setting a memoffset reg %d in a wrong opcode %d", dest, index);

  orc_x86_insn_from_opcode (xinsn, opcode, size);
  orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_OFF,
      opsize, dest);
  orc_x86_insn_operand_set (&xinsn->operands[1], ORC_X86_INSN_OPERAND_TYPE_IMM,
      orc_x86_insn_operand2_size_from_operands (opcode->operands), 0);

  xinsn->imm = imm;

  xinsn->offset = offset;
  xinsn->encoding = ORC_X86_INSN_ENCODING_MI;
  orc_x86_insn_set_size_modes (p, xinsn);
}

/* FIXME make this a define */
void
orc_x86_emit_cpuinsn_reg_memoffset (OrcCompiler *p, int index, int src,
    int offset, int dest)
{
  orc_x86_emit_cpuinsn_reg_memoffset_s (p, index, 4, src, offset, dest);
}

/* FIXME make this a define */
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
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;

  /* checks */
  if (!dest || !orc_x86_insn_validate_operand1_mem (dest, opcode->operands)) {
    ORC_ERROR ("Setting a memoffset reg %d in a wrong opcode %d", dest, index);
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_OFF,
        opsize, dest);
  }

  if (src && !orc_x86_insn_validate_operand2_reg (src, opsize, opcode->operands)) {
    ORC_ERROR ("Register %d not validated for opcode %d", src, index);
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[1], ORC_X86_INSN_OPERAND_TYPE_REG,
        opsize, src);
  }

  orc_x86_insn_from_opcode (xinsn, opcode, size);
  xinsn->encoding = ORC_X86_INSN_ENCODING_MR;
  xinsn->offset = offset;
  orc_x86_insn_set_size_modes (p, xinsn);
}

void
orc_x86_emit_cpuinsn_memoffset_reg (OrcCompiler *p, int index, int size,
    int offset, int src, int dest)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;

  if (!dest || !orc_x86_insn_validate_operand1_reg (dest, opsize, opcode->operands)) {
    ORC_ERROR ("Dest register %d not validated for opcode %d", dest, index);
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_REG,
        opsize, dest);
  }

  if (src && !orc_x86_insn_validate_operand2_mem (src, opcode->operands)) {
    ORC_ERROR ("Setting a memoffset reg %d in a wrong opcode %d", src, index);
  } else {
    orc_x86_insn_operand_set (&xinsn->operands[1], ORC_X86_INSN_OPERAND_TYPE_OFF,
        opsize, src);
  }

  orc_x86_insn_from_opcode (xinsn, opcode, size);
  xinsn->offset = offset;
  xinsn->encoding = ORC_X86_INSN_ENCODING_RM;
  orc_x86_insn_set_size_modes (p, xinsn);
}

void
orc_x86_emit_cpuinsn_branch (OrcCompiler *p, int index, int label)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;

  orc_x86_insn_from_opcode (xinsn, opcode, 1);
  xinsn->label = label;
}

void
orc_x86_emit_cpuinsn_none (OrcCompiler *p, int index)
{
  OrcX86Insn *xinsn;
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;

  if (!orc_x86_insn_validate_no_operands (opcode->operands)) {
    ORC_ERROR ("Calling the opcode %d with parameters", index);
    return;
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_x86_insn_from_opcode (xinsn, opcode, 4);
}

void
orc_x86_emit_cpuinsn_memoffset (OrcCompiler *p, int index, int size,
    int offset, int srcdest)
{
  OrcX86Insn *xinsn;
  const OrcX86InsnOpcode *opcode = orc_x86_opcodes + index;
  const OrcX86InsnOperandSize opsize = orc_x86_insn_size_to_operand_size (size);

  if (!srcdest || !orc_x86_insn_validate_operand1_mem (srcdest, opcode->operands)) {
    ORC_ERROR ("Setting a memoffset reg %d in a wrong opcode %d", srcdest, index);
  }

  xinsn = orc_x86_get_output_insn (p);
  orc_x86_insn_from_opcode (xinsn, opcode, size);
  orc_x86_insn_operand_set (&xinsn->operands[0], ORC_X86_INSN_OPERAND_TYPE_REG,
      opsize, srcdest);
  xinsn->offset = offset;
  xinsn->encoding = ORC_X86_INSN_ENCODING_M;
}

void
orc_x86_emit_align (OrcCompiler *p, int align_shift)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);

  xinsn->type = ORC_X86_INSN_TYPE_ALIGN;
  xinsn->size = align_shift;
}

void
orc_x86_emit_label (OrcCompiler *p, int label)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);

  xinsn->type = ORC_X86_INSN_TYPE_LABEL;
  xinsn->label = label;
  x86_add_label2 (p, p->n_output_insns - 1, label);
}

void
orc_x86_emit_cpuinsn_comment (OrcCompiler *p, const char * format, ...)
{
  OrcX86Insn *xinsn = orc_x86_get_output_insn (p);
  va_list varargs;

  xinsn->type = ORC_X86_INSN_TYPE_COMMENT;
  va_start (varargs, format);
  vsnprintf (xinsn->comment, 40 - 1, format, varargs);
  xinsn->comment[39] = '\0';
  va_end (varargs);
}

void
orc_x86_insn_operand_set (OrcX86InsnOperand * op, OrcX86InsnOperandType type,
    OrcX86InsnOperandSize size, int reg)
{
  op->type = type;
  op->size = size;
  op->reg = reg;
}

void
orc_x86_emit_push (OrcCompiler *compiler, int size, int reg)
{
  orc_x86_emit_cpuinsn_size (compiler, ORC_X86_push, size, 0, reg);
}

void
orc_x86_emit_pop (OrcCompiler *compiler, int size, int reg)
{
  orc_x86_emit_cpuinsn_size (compiler, ORC_X86_pop, size, 0, reg);
}

void
orc_x86_emit_mov_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2)
{
  switch (size) {
    case 1:
      orc_x86_emit_cpuinsn_size (compiler, ORC_X86_movb_r_rm, 1, reg1, reg2);
      return;
    case 2:
      orc_x86_emit_cpuinsn_size (compiler, ORC_X86_movw_r_rm, 2, reg1, reg2);
      break;
    case 4:
      orc_x86_emit_cpuinsn_size (compiler, ORC_X86_movl_r_rm, 4, reg1, reg2);
      break;
    case 8:
      orc_x86_emit_cpuinsn_size (compiler, ORC_X86_mov_r_rm, 8, reg1, reg2);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_mov_memoffset_reg (OrcCompiler *compiler, int size, int offset,
    int reg1, int reg2)
{

  switch (size) {
    case 1:
      orc_x86_emit_cpuinsn_memoffset_reg (compiler, ORC_X86_movzx_rm_r, 4, offset, reg1, reg2);
      return;
    case 2:
      orc_x86_emit_cpuinsn_memoffset_reg (compiler, ORC_X86_movw_rm_r, size, offset, reg1, reg2);
      break;
    case 4:
      orc_x86_emit_cpuinsn_memoffset_reg (compiler, ORC_X86_movl_rm_r, size, offset, reg1, reg2);
      break;
    case 8:
      orc_x86_emit_cpuinsn_memoffset_reg (compiler, ORC_X86_mov_rm_r, size, offset, reg1, reg2);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_mov_reg_memoffset (OrcCompiler *compiler, int size, int reg1, int offset,
    int reg2)
{
  switch (size) {
    case 1:
      orc_x86_emit_cpuinsn_reg_memoffset_s (compiler, ORC_X86_movb_r_rm, 1,
          reg1, offset, reg2);
      break;
    case 2:
      orc_x86_emit_cpuinsn_reg_memoffset_s (compiler, ORC_X86_movw_r_rm, 2,
          reg1, offset, reg2);
      break;
    case 4:
      orc_x86_emit_cpuinsn_reg_memoffset_s (compiler, ORC_X86_movl_r_rm, 4,
          reg1, offset, reg2);
      break;
    case 8:
      orc_x86_emit_cpuinsn_reg_memoffset_8 (compiler, ORC_X86_mov_r_rm,
          reg1, offset, reg2);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
      break;
  }
}

void
orc_x86_emit_add_imm_reg (OrcCompiler *compiler, int size, int value, int reg, orc_bool record)
{
  if (!record) {
    if (size == 4 && !compiler->is_64bit) {
      orc_x86_emit_cpuinsn_memoffset_reg (compiler, ORC_X86_leal, size,
          value, reg, reg);
      return;
    }
    if (size == 8 && compiler->is_64bit) {
      orc_x86_emit_cpuinsn_memoffset_reg (compiler, ORC_X86_leaq, size,
          value, reg, reg);
      return;
    }
  }

  if (value >= -128 && value < 128) {
    orc_x86_emit_cpuinsn_imm_reg (compiler, ORC_X86_add_imm8_rm, size,
        value, reg);
  } else {
    orc_x86_emit_cpuinsn_imm_reg (compiler, ORC_X86_add_imm32_rm, size,
        value, reg);
  }
}

void
orc_x86_emit_add_reg_reg_shift (OrcCompiler *compiler, int size, int reg1,
    int reg2, int shift)
{
  if (size == 4) {
    orc_x86_emit_cpuinsn_load_memindex (compiler, ORC_X86_leal, size, 0,
        0, reg2, reg1, shift, reg2);
  } else {
    orc_x86_emit_cpuinsn_load_memindex (compiler, ORC_X86_leaq, size, 0,
        0, reg2, reg1, shift, reg2);
  }
}

void
orc_x86_emit_cmp_imm_reg (OrcCompiler *compiler, int size, int value, int reg)
{
  if (value >= -128 && value < 128) {
    orc_x86_emit_cpuinsn_imm_reg (compiler, ORC_X86_cmp_imm8_rm, size, value, reg);
  } else {
    orc_x86_emit_cpuinsn_imm_reg (compiler, ORC_X86_cmp_imm32_rm, size, value, reg);
  }
}

void
orc_x86_emit_cmp_imm_memoffset (OrcCompiler *compiler, int size, int value,
    int offset, int reg)
{
  if (value >= -128 && value < 128) {
    orc_x86_emit_cpuinsn_imm_memoffset (compiler, ORC_X86_cmp_imm8_rm, size,
        value, offset, reg);
  } else {
    orc_x86_emit_cpuinsn_imm_memoffset (compiler, ORC_X86_cmp_imm32_rm, size,
        value, offset, reg);
  }
}

void
orc_x86_emit_dec_memoffset (OrcCompiler *compiler, int size,
    int offset, int reg)
{
  if (size == 4) {
    orc_x86_emit_cpuinsn_imm_memoffset (compiler, ORC_X86_add_imm8_rm, size,
        -1, offset, reg);
  } else {
    orc_x86_emit_cpuinsn_memoffset (compiler, ORC_X86_dec, size,
        offset, reg);
  }
}

void orc_x86_emit_rep_movs (OrcCompiler *compiler, int size)
{
  switch (size) {
    case 1:
      orc_x86_emit_cpuinsn_none (compiler, ORC_X86_rep_movsb);
      break;
    case 2:
      orc_x86_emit_cpuinsn_none (compiler, ORC_X86_rep_movsw);
      break;
    case 4:
      orc_x86_emit_cpuinsn_none (compiler, ORC_X86_rep_movsl);
      break;
  }
}
