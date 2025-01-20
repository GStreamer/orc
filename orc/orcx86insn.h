#ifndef ORC_ORC_X86_INSN_H_
#define ORC_ORC_X86_INSN_H_

#include <orc/orc.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

/* In x86 the format of an instruction is something like this,
 * 32 bits:
 * (in bytes)
 *     0,1     1,2,3     0,1    0,1      0,1,2,4      0,1,2,4
 * +--------+--------+--------+-----+--------------+-----------+
 * | prefix | opcode | modR/M | SIB | displacement | immediate |
 * +--------+--------+--------+-----+--------------+-----------+
 *
 * 64 bits:
 * REX encoding (in bytes):
 *     0,1    0,1    1,2,3     0,1    0,1      0,1,2,4      0,1,2,4
 * +--------+-----+--------+--------+-----+--------------+-----------+
 * | prefix | rex | opcode | modR/M | SIB | displacement | immediate |
 * +--------+-----+--------+--------+-----+--------------+-----------+
 *
 * VEX encoding (in bytes):
 *   2,3      1        1     0,1      0,1,2,4        0,1
 * +-----+--------+--------+-----+--------------+-----------+
 * | vex | opcode | modR/M | SIB | displacement | immediate |
 * +-----+--------+--------+-----+--------------+-----------+
 *
 * The ModR/M has the following syntax (in bits)
 *    2     3     3
 * +-----+-----+-----+
 * | Mod | Reg | R/M |
 * +-----+-----+-----+
 *
 * The SIB has the following syntax (in bits)
 *     2       3      3
 * +-------+-------+------+
 * | scale | index | base |
 * +-------+-------+------+
 *
 * Prefixes
 * NP Packed single
 * 0x66 Packed double
 * 0xf2 Scalar single
 * 0xf3 Scalar double
 *
 * Escape byte sequences
 * 0x0f
 * 0x0f 0x38
 * 0x0f 0x3a
 */

/* Operand types
 * Immediate
 * imm8: Immediate value (8 bits)
 * imm16: Immediate value (16 bits)
 * imm32: Immediate value (32 bits)
 * 
 * Memory
 * m64: Memory 8-byte aligned
 * m128: Memory 16-byte aligned
 * m256: Memory 32-byte aligned
 * m512: Memory 64-byte aligned
 *
 * Registers
 * r8: General register (8 bits)
 * r16: General register (16 bits)
 * r32: General register (32 bits)
 * r64: General register (64 bits)
 * mm: MMX register (64 bits)
 * xmm: SSE register (128 bits)
 * ymm: AVX register (256 bits)
 * zmm: AVX2 register (512 bits)
 *
 * Offset
 * Segment + Absolute address
 */

/*
 * Flags for potential instruction and operant types
 * Note that the first operand in Intel documentation is always the destination
 * We use the same form, like ORC_X86_INSN_OPERAND_OP1_OP2_OP3
 */

/* clang-format off */
typedef enum _OrcX86InsnOperandFlag {
  ORC_X86_INSN_OPERAND_NONE    = (0 << 0), /* No operand */
  /* First operand */
  ORC_X86_INSN_OPERAND_OP1_8   = (1 << 0), /* Operand1 of size 8 */
  ORC_X86_INSN_OPERAND_OP1_16  = (1 << 1), /* Operand1 of size 16 */
  ORC_X86_INSN_OPERAND_OP1_32  = (1 << 2), /* Operand1 of size 32 */
  ORC_X86_INSN_OPERAND_OP1_64  = (1 << 3), /* Operand1 of size 64 */
  ORC_X86_INSN_OPERAND_OP1_REG = (1 << 4), /* Operand1 is a register */
  ORC_X86_INSN_OPERAND_OP1_MEM = (1 << 5), /* Operand1 is a memory */
  ORC_X86_INSN_OPERAND_OP1_IMM = (1 << 6), /* Operand1 is an immediate */
  ORC_X86_INSN_OPERAND_OP1_OFF = (1 << 7), /* Operand1 is an offset, JMP case */
  /* Sizes for second operand */
  ORC_X86_INSN_OPERAND_OP2_8   = (1 << 8), /* Operand2 of size 8 */
  ORC_X86_INSN_OPERAND_OP2_16  = (1 << 9), /* Operand2 of size 16 */
  ORC_X86_INSN_OPERAND_OP2_32  = (1 << 10), /* Operand2 of size 32 */
  ORC_X86_INSN_OPERAND_OP2_64  = (1 << 11), /* Operand2 of size 64 */
  ORC_X86_INSN_OPERAND_OP2_REG = (1 << 12), /* Operand2 is a register */
  ORC_X86_INSN_OPERAND_OP2_MEM = (1 << 13), /* Operand2 is a memory */
  ORC_X86_INSN_OPERAND_OP2_IMM = (1 << 14), /* Operand2 is an immediate */
  /* Sizes for third operand */
  ORC_X86_INSN_OPERAND_OP3_8   = (1 << 15), /* Operand3 of size 8 */
  ORC_X86_INSN_OPERAND_OP3_16  = (1 << 16), /* Operand3 of size 16 */
  ORC_X86_INSN_OPERAND_OP3_32  = (1 << 17), /* Operand3 of size 32 */
  ORC_X86_INSN_OPERAND_OP3_64  = (1 << 18), /* Operand3 of size 64 */
  ORC_X86_INSN_OPERAND_OP3_REG = (1 << 19), /* Operand3 is a register */
  ORC_X86_INSN_OPERAND_OP3_MEM = (1 << 20), /* Operand3 is a memory */
  ORC_X86_INSN_OPERAND_OP3_IMM = (1 << 21), /* Operand3 is an immediate */
  /* Sizes for fourth operand */
  ORC_X86_INSN_OPERAND_OP4_8   = (1 << 22), /* Operand4 of size 8 */
  ORC_X86_INSN_OPERAND_OP4_REG = (1 << 23), /* Operand4 is a register */
  ORC_X86_INSN_OPERAND_OP4_MEM = (1 << 24), /* Operand4 is a memory */
  ORC_X86_INSN_OPERAND_OP4_IMM = (1 << 25), /* Operand4 is an immediate */
} OrcX86InsnOperandFlag;
/* clang-format on */

/* If non SIMD */
#define ORC_X86_INSN_OPERAND_OP1_NON_SIMD (\
  ORC_X86_INSN_OPERAND_OP1_8 |             \
  ORC_X86_INSN_OPERAND_OP1_16 |            \
  ORC_X86_INSN_OPERAND_OP1_32 |            \
  ORC_X86_INSN_OPERAND_OP1_64              \
)

#define ORC_X86_INSN_OPERAND_OP2_NON_SIMD (\
  ORC_X86_INSN_OPERAND_OP2_8 |             \
  ORC_X86_INSN_OPERAND_OP2_16 |            \
  ORC_X86_INSN_OPERAND_OP2_32 |            \
  ORC_X86_INSN_OPERAND_OP2_64              \
)

#define ORC_X86_INSN_OPERAND_OP3_NON_SIMD (\
  ORC_X86_INSN_OPERAND_OP3_8 |             \
  ORC_X86_INSN_OPERAND_OP3_16 |            \
  ORC_X86_INSN_OPERAND_OP3_32 |            \
  ORC_X86_INSN_OPERAND_OP3_64              \
)

#define ORC_X86_INSN_OPERAND_OP4_NON_SIMD (\
  ORC_X86_INSN_OPERAND_OP4_8               \
)

/* By operands */
#define ORC_X86_INSN_OPERAND_OP1 (\
  ORC_X86_INSN_OPERAND_OP1_REG |  \
  ORC_X86_INSN_OPERAND_OP1_MEM |  \
  ORC_X86_INSN_OPERAND_OP1_OFF |  \
  ORC_X86_INSN_OPERAND_OP1_IMM    \
)

#define ORC_X86_INSN_OPERAND_OP2 (\
  ORC_X86_INSN_OPERAND_OP2_REG |  \
  ORC_X86_INSN_OPERAND_OP2_MEM |  \
  ORC_X86_INSN_OPERAND_OP2_IMM    \
)

#define ORC_X86_INSN_OPERAND_OP3 (\
  ORC_X86_INSN_OPERAND_OP3_REG |  \
  ORC_X86_INSN_OPERAND_OP3_MEM |  \
  ORC_X86_INSN_OPERAND_OP3_IMM    \
)

#define ORC_X86_INSN_OPERAND_OP4 (\
  ORC_X86_INSN_OPERAND_OP3_REG |  \
  ORC_X86_INSN_OPERAND_OP3_MEM |  \
  ORC_X86_INSN_OPERAND_OP3_IMM    \
)

/* By type */
#define ORC_X86_INSN_OPERAND_REG (ORC_X86_INSN_OPERAND_OP1_REG)

#define ORC_X86_INSN_OPERAND_REGM (\
  ORC_X86_INSN_OPERAND_OP1_REG |   \
  ORC_X86_INSN_OPERAND_OP1_MEM     \
)

#define ORC_X86_INSN_OPERAND_REGM_REG (\
  ORC_X86_INSN_OPERAND_REGM |          \
  ORC_X86_INSN_OPERAND_OP2_REG         \
)

#define ORC_X86_INSN_OPERAND_REG_REGM (\
  ORC_X86_INSN_OPERAND_OP1_REG |       \
  ORC_X86_INSN_OPERAND_OP2_REG |       \
  ORC_X86_INSN_OPERAND_OP2_MEM         \
)

#define ORC_X86_INSN_OPERAND_REGM_IMM (\
  ORC_X86_INSN_OPERAND_REGM |          \
  ORC_X86_INSN_OPERAND_OP2_IMM         \
)

#define ORC_X86_INSN_OPERAND_REG_IMM (\
  ORC_X86_INSN_OPERAND_OP1_REG |      \
  ORC_X86_INSN_OPERAND_OP2_IMM        \
)

/* For example IMUL r32, r/m32, imm32 */
#define ORC_X86_INSN_OPERAND_REG_REGM_IMM (\
  ORC_X86_INSN_OPERAND_OP1_REG |           \
  ORC_X86_INSN_OPERAND_OP2_REG |           \
  ORC_X86_INSN_OPERAND_OP2_MEM |           \
  ORC_X86_INSN_OPERAND_OP3_IMM             \
)

/* For example PEXTRW reg/m16, xmm, imm8 */
#define ORC_X86_INSN_OPERAND_REGM_REG_IMM (\
  ORC_X86_INSN_OPERAND_REGM |              \
  ORC_X86_INSN_OPERAND_OP2_REG |           \
  ORC_X86_INSN_OPERAND_OP3_IMM             \
)

/* Three operands */
/* For example VMINPS ymm1, ymm2, ymm3/m256 */
#define ORC_X86_INSN_OPERAND_REG_REG_REGM  (\
  ORC_X86_INSN_OPERAND_OP1_REG |            \
  ORC_X86_INSN_OPERAND_OP2_REG |            \
  ORC_X86_INSN_OPERAND_OP3_REG |            \
  ORC_X86_INSN_OPERAND_OP3_MEM              \
)

#define ORC_X86_INSN_OPERAND_REG_REG_IMM   (\
  ORC_X86_INSN_OPERAND_OP1_REG |            \
  ORC_X86_INSN_OPERAND_OP2_REG |            \
  ORC_X86_INSN_OPERAND_OP3_IMM              \
)

/* Four operands */
/* For example VPINSRW xmm1, xmm2, r32/m16, imm8 */
#define ORC_X86_INSN_OPERAND_REG_REG_REGM_IMM (\
  ORC_X86_INSN_OPERAND_OP1_REG |            \
  ORC_X86_INSN_OPERAND_OP2_REG |            \
  ORC_X86_INSN_OPERAND_OP3_REG |            \
  ORC_X86_INSN_OPERAND_OP3_MEM |            \
  ORC_X86_INSN_OPERAND_OP4_IMM              \
)

#define ORC_X86_INSN_OPERAND_REG_REG_REGM_REG (\
  ORC_X86_INSN_OPERAND_OP1_REG |               \
  ORC_X86_INSN_OPERAND_OP2_REG |               \
  ORC_X86_INSN_OPERAND_OP3_REG |               \
  ORC_X86_INSN_OPERAND_OP3_MEM |               \
  ORC_X86_INSN_OPERAND_OP4_REG                 \
)

/* By size */
#define ORC_X86_INSN_OPERAND_OP1_16TO64 (\
  ORC_X86_INSN_OPERAND_OP1_16 |          \
  ORC_X86_INSN_OPERAND_OP1_32 |          \
  ORC_X86_INSN_OPERAND_OP1_64            \
)

#define ORC_X86_INSN_OPERAND_OP2_16TO64 (\
  ORC_X86_INSN_OPERAND_OP2_16 |          \
  ORC_X86_INSN_OPERAND_OP2_32 |          \
  ORC_X86_INSN_OPERAND_OP2_64            \
)

/* FIXME fix this */
#define ORC_X86_INSN_TYPE_IMM32_A (ORC_X86_INSN_OPERAND_REG_IMM | ORC_X86_INSN_OPERAND_OP2_32 | ORC_X86_INSN_OPERAND_OP1_32) /* For example AND RAX, imm32 */

/* clang-format off */
typedef enum _OrcX86InsnOperands {
  ORC_X86_INSN_TYPE_REGM32               = (ORC_X86_INSN_OPERAND_REGM | ORC_X86_INSN_OPERAND_OP1_32),
  ORC_X86_INSN_TYPE_REG16TO64            = (ORC_X86_INSN_OPERAND_OP1_REG | ORC_X86_INSN_OPERAND_OP1_16TO64),
  ORC_X86_INSN_TYPE_REGM16TO64           = (ORC_X86_INSN_OPERAND_REGM | ORC_X86_INSN_OPERAND_OP1_16TO64),
  ORC_X86_INSN_TYPE_REGM16TO64_REG16TO64 = (ORC_X86_INSN_OPERAND_REGM_REG | ORC_X86_INSN_OPERAND_OP1_16TO64 | ORC_X86_INSN_OPERAND_OP2_16TO64),
  ORC_X86_INSN_TYPE_REG16TO64_REGM16TO64 = (ORC_X86_INSN_OPERAND_REG_REGM | ORC_X86_INSN_OPERAND_OP1_16TO64 | ORC_X86_INSN_OPERAND_OP2_16TO64), /* For example IMUL r/m64 */
  ORC_X86_INSN_TYPE_REG64_REGM64         = (ORC_X86_INSN_OPERAND_REG_REGM | ORC_X86_INSN_OPERAND_OP1_64 | ORC_X86_INSN_OPERAND_OP2_64), /* For example MOV r64, r/m64 */
  ORC_X86_INSN_TYPE_REG32_REGM32         = (ORC_X86_INSN_OPERAND_REG_REGM | ORC_X86_INSN_OPERAND_OP1_32 | ORC_X86_INSN_OPERAND_OP2_32), /* For example MOV r32, r/m32 */
  ORC_X86_INSN_TYPE_REG16_REGM16         = (ORC_X86_INSN_OPERAND_REG_REGM | ORC_X86_INSN_OPERAND_OP1_16 | ORC_X86_INSN_OPERAND_OP2_16), /* For example MOV r16, r/m16 */
  ORC_X86_INSN_TYPE_REG16TO64_REGM8      = (ORC_X86_INSN_OPERAND_REG_REGM | ORC_X86_INSN_OPERAND_OP1_16TO64 | ORC_X86_INSN_OPERAND_OP2_8), /* For example MOVZX r32, r/m8 */
  ORC_X86_INSN_TYPE_REG64_MEM            = (ORC_X86_INSN_OPERAND_OP1_REG | ORC_X86_INSN_OPERAND_OP1_64 | ORC_X86_INSN_OPERAND_OP2_MEM), /* For example LEAQ r64, m */
  ORC_X86_INSN_TYPE_MEM32                = (ORC_X86_INSN_OPERAND_OP1_MEM | ORC_X86_INSN_OPERAND_OP1_32), /* For example LDMXCSR m32 */
  ORC_X86_INSN_TYPE_REG32_IMM32          = (ORC_X86_INSN_OPERAND_REG_IMM | ORC_X86_INSN_OPERAND_OP1_32 | ORC_X86_INSN_OPERAND_OP2_32), /* For example MOV r32, imm32 */
  ORC_X86_INSN_TYPE_REGM64_REG64         = (ORC_X86_INSN_OPERAND_REGM_REG | ORC_X86_INSN_OPERAND_OP1_64 | ORC_X86_INSN_OPERAND_OP2_64), /* For example MOV r/m64, r64 */
  ORC_X86_INSN_TYPE_REGM32_REG32         = (ORC_X86_INSN_OPERAND_REGM_REG | ORC_X86_INSN_OPERAND_OP1_32 | ORC_X86_INSN_OPERAND_OP2_32), /* For example MOV r/m32, r32 */
  ORC_X86_INSN_TYPE_REGM16_REG16         = (ORC_X86_INSN_OPERAND_REGM_REG | ORC_X86_INSN_OPERAND_OP1_16 | ORC_X86_INSN_OPERAND_OP2_16), /* For example MOV r/m16, r16 */
  ORC_X86_INSN_TYPE_REGM8_REG8           = (ORC_X86_INSN_OPERAND_REGM_REG | ORC_X86_INSN_OPERAND_OP1_8 | ORC_X86_INSN_OPERAND_OP2_8), /* For example MOV r/m8, r8 */
  ORC_X86_INSN_TYPE_REGM16TO64_IMM8      = (ORC_X86_INSN_OPERAND_REGM_IMM | ORC_X86_INSN_OPERAND_OP1_16TO64 | ORC_X86_INSN_OPERAND_OP2_8), /* For example SHR r/m32, imm8 */
  ORC_X86_INSN_TYPE_REGM32TO64_IMM32     = (ORC_X86_INSN_OPERAND_REGM_IMM | ORC_X86_INSN_OPERAND_OP1_32 | ORC_X86_INSN_OPERAND_OP1_64 | ORC_X86_INSN_OPERAND_OP2_32),  /* For example ADD r/m32, imm32 */
  ORC_X86_INSN_TYPE_REGM32_IMM32         = (ORC_X86_INSN_OPERAND_REGM_IMM | ORC_X86_INSN_OPERAND_OP1_32 | ORC_X86_INSN_OPERAND_OP2_32),  /* For example TEST r/m32, imm32 */
  ORC_X86_INSN_TYPE_REG64_IMM64          = (ORC_X86_INSN_OPERAND_REG_IMM | ORC_X86_INSN_OPERAND_OP1_64 | ORC_X86_INSN_OPERAND_OP2_64), /* For example MOV r64, imm64 */
} OrcX86InsnOperands;
/* clang-format on */

typedef enum _OrcX86InsnType {
  ORC_X86_INSN_TYPE_OP,
  ORC_X86_INSN_TYPE_ALIGN,
  ORC_X86_INSN_TYPE_LABEL,
  ORC_X86_INSN_TYPE_COMMENT,
} OrcX86InsnType;

typedef enum _OrcX86InsnOpcodeType {
  ORC_X86_INSN_OPCODE_TYPE_OTHER,
  ORC_X86_INSN_OPCODE_TYPE_BRANCH,
  ORC_X86_INSN_OPCODE_TYPE_STACK
} OrcX86InsnOpcodeType;

typedef enum _OrcX86InsnPrefix {
  ORC_X86_INSN_PREFIX_NO_PREFIX,
  ORC_X86_INSN_PREFIX_VEX128,
  ORC_X86_INSN_PREFIX_VEX256,
} OrcX86InsnPrefix;

typedef enum _OrcX86InsnOpcodeEscapeSequence {
  ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_NONE,
  ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F,
  ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F38,
  ORC_X86_INSN_OPCODE_ESCAPE_SEQUENCE_0X0F3A,
} OrcX86InsnOpcodeEscapeSequence;

typedef enum _OrcX86InsnOpcodePrefix {
  ORC_X86_INSN_OPCODE_PREFIX_NONE,
  /* Packed Double for SIMD or Operand-size override */
  ORC_X86_INSN_OPCODE_PREFIX_0X66,
  ORC_X86_INSN_OPCODE_PREFIX_0XF3, /* Scalar Double */
  ORC_X86_INSN_OPCODE_PREFIX_0XF2, /* Scalar Single */
} OrcX86InsnOpcodePrefix;

typedef enum _OrcX86InsnOpcodeFlag {
  ORC_X86_INSN_OPCODE_FLAG_NONE     = (0 << 0),
  ORC_X86_INSN_OPCODE_FLAG_VEX_W1   = (1 << 0),
} OrcX86InsnOpcodeFlag;

/* FIXME here the suffixes are inverted. src_dst */
typedef enum _OrcX86OpcodeIdx {
  ORC_X86_add_r_rm,
  ORC_X86_add_rm_r,
  ORC_X86_or_r_rm,
  ORC_X86_or_rm_r,
  ORC_X86_adc_r_rm,
  ORC_X86_adc_rm_r,
  ORC_X86_sbb_r_rm,
  ORC_X86_sbb_rm_r,
  ORC_X86_and_r_rm,
  ORC_X86_and_rm_r,
  /* 10 */
  ORC_X86_and_imm32_a,
  ORC_X86_sub_r_rm,
  ORC_X86_sub_rm_r,
  ORC_X86_xor_r_rm,
  ORC_X86_xor_rm_r,
  ORC_X86_cmp_rm_r,
  ORC_X86_cmp_r_rm,
  ORC_X86_push,
  ORC_X86_pop,
  ORC_X86_jo,
  /* 20 */
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
  /* 30 */
  ORC_X86_jnp,
  ORC_X86_jl,
  ORC_X86_jge,
  ORC_X86_jle,
  ORC_X86_jg,
  ORC_X86_add_imm32_rm,
  ORC_X86_or_imm32_rm,
  ORC_X86_adc_imm32_rm,
  ORC_X86_sbb_imm32_rm,
  ORC_X86_and_imm32_rm,
  /* 40 */
  ORC_X86_sub_imm32_rm,
  ORC_X86_xor_imm32_rm,
  ORC_X86_cmp_imm32_rm,
  ORC_X86_add_imm8_rm,
  ORC_X86_or_imm8_rm,
  ORC_X86_adc_imm8_rm,
  ORC_X86_sbb_imm8_rm,
  ORC_X86_and_imm8_rm,
  ORC_X86_sub_imm8_rm,
  ORC_X86_xor_imm8_rm,
  /* 50 */
  ORC_X86_cmp_imm8_rm,
  ORC_X86_test,
  ORC_X86_movb_r_rm,
  ORC_X86_movl_r_rm,
  ORC_X86_mov_r_rm,
  ORC_X86_movw_r_rm,
  ORC_X86_movw_rm_r,
  ORC_X86_movl_rm_r,
  ORC_X86_mov_rm_r,
  ORC_X86_leal,
  /* 60 */
  ORC_X86_leaq,
  ORC_X86_nop,
  ORC_X86_rep_movsb,
  ORC_X86_rep_movsl,
  ORC_X86_rep_movsw,
  ORC_X86_imul_rm_r,
  ORC_X86_movzx_rm_r,
  ORC_X86_mov_imm32_r,
  ORC_X86_sar_imm,
  ORC_X86_ret,
  ORC_X86_retq,
  /* 70 */
  ORC_X86_sar,
  ORC_X86_jmp,
  ORC_X86_test_imm,
  ORC_X86_imul_rm,
  ORC_X86_inc,
  ORC_X86_dec,
  ORC_X86_rdtsc,
  ORC_X86_endbr64,
  ORC_X86_endbr32,
  /* 80 */
  ORC_X86_mov_imm64_r,
} OrcX86OpcodeIdx;

typedef enum _OrcX86InsnOperandType {
  ORC_X86_INSN_OPERAND_TYPE_NONE,
  ORC_X86_INSN_OPERAND_TYPE_REG,
  ORC_X86_INSN_OPERAND_TYPE_OFF,
  ORC_X86_INSN_OPERAND_TYPE_IDX,
  ORC_X86_INSN_OPERAND_TYPE_IMM,
} OrcX86InsnOperandType;

typedef enum _OrcX86InsnOperandSize {
  ORC_X86_INSN_OPERAND_SIZE_NONE,
  ORC_X86_INSN_OPERAND_SIZE_8,
  ORC_X86_INSN_OPERAND_SIZE_16,
  ORC_X86_INSN_OPERAND_SIZE_32,
  ORC_X86_INSN_OPERAND_SIZE_64,
} OrcX86InsnOperandSize;

typedef enum _OrcX86InsnEncoding {
  ORC_X86_INSN_ENCODING_NONE, /* For offset only */
  ORC_X86_INSN_ENCODING_ZO,   /* For zero opeands */
  ORC_X86_INSN_ENCODING_O,    /* For register embedded in the opcode */
  ORC_X86_INSN_ENCODING_I,    /* For immediate only, like PUSH imm8 */
  ORC_X86_INSN_ENCODING_M,    /* For memory only or memory/register */
  ORC_X86_INSN_ENCODING_OI,   /* For register embedded in the opcode, immediate */
  ORC_X86_INSN_ENCODING_MI,   /* For memory/register and immediate, like MOV r/m32, imm32 */
  ORC_X86_INSN_ENCODING_MR,   /* For memory/register and register like ADD r/m16, r16 (Also SIMD B) */
  ORC_X86_INSN_ENCODING_MRI,  /* For memory/register, register and immdiate (Also SIMD B) */
  ORC_X86_INSN_ENCODING_VMI,  /* VEX. For register, register and immediate, like VPSRLW ymm1, ymm2, imm8 */
  ORC_X86_INSN_ENCODING_RM,   /* For register and memory/register, like ADD r16, r/m16 (Also SIMD A) */
  ORC_X86_INSN_ENCODING_RMI,  /* For register, memory and immediate, like PSHUFD xmm1, xmm2/m128, imm8 (Also SIMD A) */
  ORC_X86_INSN_ENCODING_RVM,  /* VEX. For three operands, like VPSRLW xmm1, xmm2, xmm3/m128 */
  ORC_X86_INSN_ENCODING_RVMI, /* VEX. For four operands, like VPBLENDD xmm1, xmm2, xmm3/m128, imm8 */
  ORC_X86_INSN_ENCODING_RVMR, /* VEX. For four opeands using imm for a register like VBLENDVPD xmm1, xmm2, xmm3/m128, xmm4 */
} OrcX86InsnEncoding;

typedef struct _OrcX86InsnOperand {
  OrcX86InsnOperandType type;
  OrcX86InsnOperandSize size;
  int reg;
} OrcX86InsnOperand;

typedef struct _OrcX86Insn {
  const char *name;
  // Immediate mode operand
  orc_int64 imm;
  // Size of jmp or align
  int size;
  // Label for the function (if this is a preamble)
  int label;
  // Memory offset
  int offset;
  // Index register (if applicable)
  int index_reg;
  // Shift left (if applicable)
  int shift;
  // Offset from instruction pointer (for loads)
  int code_offset;
  OrcX86InsnPrefix prefix;
  OrcX86InsnType type;
  OrcX86InsnOpcodeEscapeSequence opcode_escape;
  OrcX86InsnOpcodePrefix opcode_prefix;
  OrcX86InsnOpcodeType opcode_type;
  OrcX86InsnEncoding encoding;
  orc_uint32 opcode_flags;
  orc_uint32 opcode;
  int extension;
  OrcX86InsnOperand operands[4];
  /* FIXME we use a fixed length to avoid having to free */
  char comment[40];
} OrcX86Insn;

ORC_API OrcX86Insn * orc_x86_get_output_insn (OrcCompiler *p);
ORC_API void orc_x86_output_insns (OrcCompiler *p);
ORC_API void orc_x86_calculate_offsets (OrcCompiler *p);
ORC_API void orc_x86_emit_cpuinsn_label (OrcCompiler *p, int label);
ORC_API void orc_x86_emit_cpuinsn_align (OrcCompiler *p, int align_shift);
ORC_API void orc_x86_emit_cpuinsn_comment (OrcCompiler *p, const char * format, ...);
ORC_API void orc_x86_emit_push (OrcCompiler *compiler, int size, int reg);
ORC_API void orc_x86_emit_pop (OrcCompiler *compiler, int size, int reg);

#define orc_x86_emit_mov_imm_reg(p,size,value,reg) \
  orc_x86_emit_cpuinsn_imm_reg(p, ORC_X86_mov_imm32_r, size, value, reg)
#define orc_x86_emit_mov_imm_reg64(p,size,value,reg) \
  orc_x86_emit_cpuinsn_imm_reg(p, ORC_X86_mov_imm64_r, size, value, reg)
#define orc_x86_emit_test_reg_reg(p,size,src,dest) \
  orc_x86_emit_cpuinsn_size (p, ORC_X86_test, size, src, dest)
#define orc_x86_emit_sar_imm_reg(p,size,value,reg) do { \
    if (value == 1) { \
      orc_x86_emit_cpuinsn_size (p, ORC_X86_sar, size, 0, reg); \
    } else if (value > 1) { \
      orc_x86_emit_cpuinsn_imm_reg (p, ORC_X86_sar_imm, size, value, reg); \
    } \
  } while (0)
#define orc_x86_emit_and_imm_memoffset(p,size,value,offset,reg) \
  orc_x86_emit_cpuinsn_imm_memoffset (p, (value >= -128 && value < 128) ? \
      ORC_X86_and_imm8_rm : ORC_X86_and_imm32_rm, size, value, offset, reg)
#define orc_x86_emit_and_imm_reg(p,size,value,reg) do { \
  if ((value) >= -128 && (value) < 128) { \
    orc_x86_emit_cpuinsn_imm_reg (p, ORC_X86_and_imm8_rm, size, value, reg); \
  } else { \
    if ((reg) == X86_EAX) { \
      orc_x86_emit_cpuinsn_imm_reg (p, ORC_X86_and_imm32_a, size, value, reg); \
    } else { \
      orc_x86_emit_cpuinsn_imm_reg (p, ORC_X86_and_imm32_rm, size, value, reg); \
    } \
  } \
} while (0)
#define orc_x86_emit_add_imm_memoffset(p,size,value,offset,reg) \
  orc_x86_emit_cpuinsn_imm_memoffset (p, (value >= -128 && value < 128) ? \
      ORC_X86_add_imm8_rm : ORC_X86_add_imm32_rm, size, value, offset, reg)
#define orc_x86_emit_add_reg_memoffset(p,size,src,offset,dest) \
  orc_x86_emit_cpuinsn_reg_memoffset_s(p, ORC_X86_add_r_rm, size, src, offset, dest)
#define orc_x86_emit_add_reg_reg(p,size,src,dest) \
  orc_x86_emit_cpuinsn_size(p, ORC_X86_add_r_rm, size, src, dest)
#define orc_x86_emit_add_memoffset_reg(p,size,offset,src,dest) \
  orc_x86_emit_cpuinsn_memoffset_reg(p, ORC_X86_add_rm_r, size, offset, src, dest)
#define orc_x86_emit_sub_reg_reg(p,size,src,dest) \
  orc_x86_emit_cpuinsn_size(p, ORC_X86_sub_r_rm, size, src, dest)
#define orc_x86_emit_sub_memoffset_reg(p,size,offset,src,dest) \
  orc_x86_emit_cpuinsn_memoffset_reg(p, ORC_X86_sub_rm_r, size, offset, src, dest)
#define orc_x86_emit_imul_memoffset_reg(p,size,offset,src,dest) \
  orc_x86_emit_cpuinsn_memoffset_reg(p, ORC_X86_imul_rm_r, size, offset, src, dest)

#define orc_x86_emit_cmp_reg_memoffset(p,size,src,offset,dest) \
  orc_x86_emit_cpuinsn_reg_memoffset_s(p, ORC_X86_cmp_r_rm, size, src, offset, dest)

#define orc_x86_emit_jmp(p,label) \
  orc_x86_emit_cpuinsn_branch (p, ORC_X86_jmp, label)
#define orc_x86_emit_jg(p,label) \
  orc_x86_emit_cpuinsn_branch (p, ORC_X86_jg, label)
#define orc_x86_emit_jle(p,label) \
  orc_x86_emit_cpuinsn_branch (p, ORC_X86_jle, label)
#define orc_x86_emit_je(p,label) \
  orc_x86_emit_cpuinsn_branch (p, ORC_X86_jz, label)
#define orc_x86_emit_jne(p,label) \
  orc_x86_emit_cpuinsn_branch (p, ORC_X86_jnz, label)

#define orc_x86_emit_rdtsc(p) \
  orc_x86_emit_cpuinsn_none (p, ORC_X86_rdtsc)
#define orc_x86_emit_ret(p) \
  orc_x86_emit_cpuinsn_none (p, ((p)->is_64bit) ? ORC_X86_retq : ORC_X86_ret)

#define orc_x86_emit_test_imm_memoffset(p,size,value,offset,dest) \
  orc_x86_emit_cpuinsn_imm_memoffset (p, ORC_X86_test_imm, size, value, \
      offset, dest)

ORC_API void orc_x86_emit_mov_reg_reg (OrcCompiler *compiler, int size, int reg1, int reg2);
ORC_API void orc_x86_emit_mov_memoffset_reg (OrcCompiler *compiler, int size, int offset, int reg1, int reg2);
ORC_API void orc_x86_emit_mov_reg_memoffset (OrcCompiler *compiler, int size, int reg1, int offset, int reg2);
ORC_API void orc_x86_emit_dec_memoffset (OrcCompiler *compiler, int size, int offset, int reg);
ORC_API void orc_x86_emit_add_imm_reg (OrcCompiler *compiler, int size, int value, int reg, orc_bool record);
ORC_API void orc_x86_emit_add_reg_reg_shift (OrcCompiler *compiler, int size, int reg1, int reg2, int shift);
ORC_API void orc_x86_emit_cmp_imm_memoffset (OrcCompiler *compiler, int size, int value, int offset, int reg);
ORC_API void orc_x86_emit_cmp_imm_reg (OrcCompiler *compiler, int size, int value, int reg);
ORC_API void orc_x86_emit_rep_movs (OrcCompiler *compiler, int size);

ORC_API void orc_x86_emit_cpuinsn_size (OrcCompiler *p, int opcode, int size,
    int src, int dest);
ORC_API void orc_x86_emit_cpuinsn_load_memindex (OrcCompiler *p, int index, int size,
    int imm, int offset, int src, int src_index, int shift, int dest);
ORC_API void orc_x86_emit_cpuinsn_load_register (OrcCompiler *p, int index, int imm,
    int src, int dest);
ORC_API void orc_x86_emit_cpuinsn_imm_reg (OrcCompiler *p, int index, int size, orc_int64 imm,
    int dest);
ORC_API void orc_x86_emit_cpuinsn_imm_memoffset (OrcCompiler *p, int index, int size,
    int imm, int offset, int dest);
ORC_API void orc_x86_emit_cpuinsn_reg_memoffset (OrcCompiler *p, int index, int src,
    int offset, int dest);
ORC_API void orc_x86_emit_cpuinsn_reg_memoffset_8 (OrcCompiler *p, int index, int src,
    int offset, int dest);
ORC_API void orc_x86_emit_cpuinsn_reg_memoffset_s (OrcCompiler *p, int index, int size,
    int src, int offset, int dest);
ORC_API void orc_x86_emit_cpuinsn_memoffset_reg (OrcCompiler *p, int index, int size,
    int offset, int src, int dest);
ORC_API void orc_x86_emit_cpuinsn_memoffset (OrcCompiler *p, int index, int size,
    int offset, int srcdest);
ORC_API void orc_x86_emit_cpuinsn_branch (OrcCompiler *p, int index, int label);
ORC_API void orc_x86_emit_cpuinsn_none (OrcCompiler *p, int index);

#endif

ORC_END_DECLS

#endif
