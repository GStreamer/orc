/*
  Copyright © 2024 Samsung Electronics

  Author: Maksymilian Knust, m.knust@samsung.com
  Author: Filip Wasil, f.wasil@samsung.com

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#include <orc/orccompiler.h>
#include <orc/orcdebug.h>
#include <orc/orclimits.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>

#include <orc/orcinternal.h>

#include <orc/riscv/orcriscv-internal.h>
#include <orc/riscv/orcriscv.h>
#include <orc/riscv/orcriscvinsn.h>

typedef enum
{
  OP_LUI = 0b0110111,
  OP_JAL = 0b1101111,
  OP_JALR = 0b1100111,
  OP_BRANCH = 0b1100011,
  OP_LOAD = 0b0000011,
  OP_STORE = 0b0100011,
  OP_ARITH_I = 0b0010011,
  OP_ARITH = 0b0110011,
  OP_VLOAD = 0b0000111,
  OP_VSTORE = 0b0100111,
  OP_VECTOR = 0b1010111,
  OP_FP = 0b1010111,
  OP_SYSTEM = 0b1110011,
} RiscvOpcode;

typedef enum
{
  OPIVV = 0b000,
  OPFVV = 0b001,
  OPMVV = 0b010,
  OPIVI = 0b011,
  OPIVX = 0b100,
  OPFVF = 0b101,
  OPMVX = 0b110
} RiscvVopType;

static const char *
riscv_reg_name (OrcRiscvRegister reg)
{
  static const char *names[] = {
    [ORC_GP_REG_BASE] = "zero",
    "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0",
    "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6",
    "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8",
    "s9", "s10", "s11", "t3", "t4", "t5", "t6",

    [ORC_VEC_REG_BASE] = "v0",
    "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8",
    "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16",
    "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24",
    "v25", "v26", "v27", "v28", "v29", "v30", "v31"
  };

  ORC_ASSERT (reg < ARRAY_SIZE (names));
  ORC_ASSERT (names[reg] != NULL);

  return names[reg];
}

static inline orc_uint32
riscv_gp_reg (OrcRiscvRegister reg)
{
  ORC_ASSERT (reg >= ORC_GP_REG_BASE && reg < ORC_VEC_REG_BASE);
  return reg - ORC_GP_REG_BASE;
}

static inline orc_uint32
riscv_vec_reg (OrcRiscvRegister reg)
{
  ORC_ASSERT (reg >= ORC_VEC_REG_BASE && reg < ORC_VEC_REG_BASE + 32);
  return reg - ORC_VEC_REG_BASE;
}

/* These static functions are used hundreds of times in this file */
#define XREG(reg) riscv_gp_reg(reg)
#define VREG(reg) riscv_vec_reg(reg)
#define NAME(reg) riscv_reg_name(reg)

static inline void
riscv_emit32 (OrcCompiler *const c, const orc_uint32 insn)
{
  *c->codeptr++ = (insn >> 0);
  *c->codeptr++ = (insn >> 8);
  *c->codeptr++ = (insn >> 16);
  *c->codeptr++ = (insn >> 24);
}

static void
orc_riscv_insn_csr (OrcCompiler *const c, RiscvOpcode opcode,
    orc_uint32 funct3, OrcRiscvRegister rd, OrcRiscvRegister rs1,
    orc_uint32 csr)
{
  riscv_emit32 (c,
      opcode | XREG (rd) << 7 | funct3 << 12 | XREG (rs1) << 15 | csr << 20);
}

static void
orc_riscv_insn_r (OrcCompiler *const c,
    RiscvOpcode opcode, orc_uint32 funct3, orc_uint32 funct7,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister immrs2)
{
  riscv_emit32 (c,
      opcode | (XREG (rd) << 7) | funct3 << 12 |
      XREG (rs1) << 15 | immrs2 << 20 | funct7 << 25);
}

static void
orc_riscv_insn_i (OrcCompiler *const c, RiscvOpcode opcode,
    orc_uint32 funct3, OrcRiscvRegister rd, OrcRiscvRegister rs1,
    orc_uint32 imm)
{
  riscv_emit32 (c,
      opcode | XREG (rd) << 7 | funct3 << 12 | XREG (rs1) << 15 | imm << 20);
}

static void
orc_riscv_insn_s (OrcCompiler *const c, RiscvOpcode opcode,
    orc_uint32 funct3, OrcRiscvRegister rs1, OrcRiscvRegister rs2,
    orc_uint32 imm)
{
  riscv_emit32 (c,
      opcode | (imm & 31) << 7 | funct3 << 12 | XREG (rs1) << 15 |
      XREG (rs2) << 20 | (imm & ~31) << 20);
}

static void
orc_riscv_insn_u (OrcCompiler *const c, RiscvOpcode opcode,
    OrcRiscvRegister rd, orc_uint32 imm)
{
  riscv_emit32 (c, opcode | XREG (rd) << 7 | imm << 12);
}

static void
orc_riscv_insn_b (OrcCompiler *const c, RiscvOpcode opcode,
    orc_uint32 funct3, OrcRiscvRegister rs1, OrcRiscvRegister rs2,
    orc_uint32 imm)
{
  riscv_emit32 (c, opcode | ((((imm >> 11) & 0b1) | (imm & 0b11110)) << 7)
      | funct3 << 12 | XREG (rs1) << 15 | XREG (rs2)
      << 20 | (((imm >> 5) & 0b111111) | ((imm >> 5) & 0b1000000)) << 25);
}

static void
orc_riscv_insn_j (OrcCompiler *const c, RiscvOpcode opcode,
    OrcRiscvRegister rd, orc_uint32 imm)
{
  riscv_emit32 (c,
      opcode | XREG (rd) << 7 | ((imm & 0b1111111110) << 20) |
      ((imm >> 12) & 0b1111111) << 12);
}

static void
orc_riscv_insn_vle (OrcCompiler *const c,
    orc_uint32 nf, orc_uint32 mew, orc_uint32 mop, orc_uint32 lumop,
    OrcRiscvRegister rs1, orc_uint32 width, OrcRiscvRegister vd, orc_uint32 vm)
{
  riscv_emit32 (c,
      nf << 29 | mew << 28 | mop << 26 | vm << 25 | lumop << 20 | XREG (rs1) <<
      15 | width << 12 | VREG (vd) << 7 | OP_VLOAD);
}

static void
orc_riscv_insn_vse (OrcCompiler *const c,
    orc_uint32 nf, orc_uint32 mew, orc_uint32 mop, orc_uint32 lumop,
    orc_uint32 rs1, orc_uint32 width, orc_uint32 vs3, orc_uint32 vm)
{
  riscv_emit32 (c,
      nf << 29 | mew << 28 | mop << 26 | vm << 25 | lumop << 20 | XREG (rs1) <<
      15 | width << 12 | VREG (vs3) << 7 | OP_VSTORE);
}

static void
orc_riscv_insn_vop (OrcCompiler *const c,
    orc_uint32 func6, orc_uint32 vm, orc_uint32 vs2, orc_uint32 vs1,
    orc_uint32 type, orc_uint32 d)
{
  riscv_emit32 (c,
      func6 << 26 | vm << 25 | vs2 << 20 | vs1 << 15 | type << 12 | d << 7 |
      OP_VECTOR);
}

static void
orc_riscv_insn_vsetvli (OrcCompiler *const c, orc_uint8 vtypei, orc_uint32 rs1,
    orc_uint32 rd)
{
  riscv_emit32 (c,
      (vtypei & 0b011111111111) << 20 | rs1 << 15 | 0b111 << 12 | rd << 7 |
      OP_VECTOR);
}

void
orc_riscv_insn_emit_add (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  add %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b000, 0b0000000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_addi (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  addi %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  orc_riscv_insn_i (c, OP_ARITH_I, 0b000, rd, rs1, imm);
}

void
orc_riscv_insn_emit_sub (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  sub %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b000, 0b0100000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_sll (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  sll %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b001, 0b0000000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_slli (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  slli %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  orc_riscv_insn_r (c, OP_ARITH_I, 0b001, 0, rd, rs1, imm);
}

void
orc_riscv_insn_emit_or (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  or %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b110, 0b0000000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_and (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  and %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b111, 0b0000000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_xor (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  xor %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b100, 0b0000000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_slt (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  slt %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b010, 0b0000000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_sra (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvRegister rs2)
{
  ORC_ASM_CODE (c, "  sra %s, %s, %s\n", NAME (rd), NAME (rs1), NAME (rs2));
  orc_riscv_insn_r (c, OP_ARITH, 0b101, 0b0100000, rd, rs1, XREG (rs2));
}

void
orc_riscv_insn_emit_srli (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  srli %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  orc_riscv_insn_r (c, OP_ARITH_I, 0b101, 0, rd, rs1, imm);
}

void
orc_riscv_insn_emit_sb (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sb %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  orc_riscv_insn_s (c, OP_STORE, 0b000, rs1, rs2, imm);
}


void
orc_riscv_insn_emit_lb (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  lb %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  orc_riscv_insn_i (c, OP_LOAD, 0b000, rd, rs1, imm);
}

void
orc_riscv_insn_emit_sh (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sh %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  orc_riscv_insn_s (c, OP_STORE, 0b001, rs1, rs2, imm);
}

void
orc_riscv_insn_emit_lh (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  lh %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  orc_riscv_insn_i (c, OP_LOAD, 0b001, rd, rs1, imm);
}

void
orc_riscv_insn_emit_sw (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sw %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  orc_riscv_insn_s (c, OP_STORE, 0b010, rs1, rs2, imm);
}


void
orc_riscv_insn_emit_lw (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  lw %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  orc_riscv_insn_i (c, OP_LOAD, 0b010, rd, rs1, imm);
}

void
orc_riscv_insn_emit_sd (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int imm)
{
  ORC_ASM_CODE (c, "  sd %s, %d(%s)\n", NAME (rs2), imm, NAME (rs1));
  orc_riscv_insn_s (c, OP_STORE, 0b011, rs1, rs2, imm);
}

void
orc_riscv_insn_emit_ld (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  ld %s, %d(%s)\n", NAME (rd), imm, NAME (rs1));
  orc_riscv_insn_i (c, OP_LOAD, 0b011, rd, rs1, imm);
}

void
orc_riscv_insn_emit_lui (OrcCompiler *c, OrcRiscvRegister rd, int imm)
{
  ORC_ASM_CODE (c, "  lui %s, %d\n", NAME (rd), imm);
  orc_riscv_insn_u (c, OP_LUI, rd, imm);
}

void
orc_riscv_insn_emit_beq (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  beq %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  orc_riscv_insn_b (c, OP_BRANCH, 0b000, rs1, rs2, 0);
}

void
orc_riscv_insn_emit_beqz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_beq (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bne (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bne %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  orc_riscv_insn_b (c, OP_BRANCH, 0b001, rs1, rs2, 0);
}

void
orc_riscv_insn_emit_bnez (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bne (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_blt (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  blt %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  orc_riscv_insn_b (c, OP_BRANCH, 0b100, rs1, rs2, 0);
}

void
orc_riscv_insn_emit_bltz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_blt (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bltu (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bltu %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  orc_riscv_insn_b (c, OP_BRANCH, 0b110, rs1, rs2, 0);
}

void
orc_riscv_insn_emit_bltuz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bltu (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bge (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bge %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  orc_riscv_insn_b (c, OP_BRANCH, 0b101, rs1, rs2, 0);
}

void
orc_riscv_insn_emit_bgez (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bge (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_bgeu (OrcCompiler *c,
    OrcRiscvRegister rs1, OrcRiscvRegister rs2, int label)
{
  ORC_ASM_CODE (c, "  bgeu %s, %s, .L%s_%d\n",
      NAME (rs1), NAME (rs2), c->program->name, label);
  orc_riscv_compiler_add_fixup (c, label);
  orc_riscv_insn_s (c, OP_BRANCH, 0b111, rs1, rs2, 0);
}

void
orc_riscv_insn_emit_bgeuz (OrcCompiler *c, OrcRiscvRegister rs1, int label)
{
  orc_riscv_insn_emit_bgeu (c, rs1, ORC_RISCV_ZERO, label);
}

void
orc_riscv_insn_emit_jal (OrcCompiler *c, OrcRiscvRegister rd, int imm)
{
  ORC_ASM_CODE (c, "  jal %s, %d\n", NAME (rd), imm);
  orc_riscv_insn_j (c, OP_JAL, rd, imm);
}

void
orc_riscv_insn_emit_jalr (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, int imm)
{
  ORC_ASM_CODE (c, "  jalr %s, %s, %d\n", NAME (rd), NAME (rs1), imm);
  orc_riscv_insn_i (c, OP_JALR, 0b000, rd, rs1, imm);
}

void
orc_riscv_insn_emit_ret (OrcCompiler *c)
{
  ORC_ASM_CODE (c, "  ret\n");
  riscv_emit32 (c, 0x00008067);
}

void
orc_riscv_insn_emit_csrrw (OrcCompiler *c, OrcRiscvRegister rd,
    OrcRiscvRegister rs1, int csr)
{
  ORC_ASM_CODE (c, "  csrrw %s, %d, %s\n", NAME (rd), csr, NAME (rs1));
  orc_riscv_insn_csr (c, OP_SYSTEM, 0b001, rd, rs1, csr);
}

void
orc_riscv_insn_emit_csrrs (OrcCompiler *c, OrcRiscvRegister rd,
    OrcRiscvRegister rs1, int csr)
{
  ORC_ASM_CODE (c, "  csrrs %s, %d, %s\n", NAME (rd), csr, NAME (rs1));
  orc_riscv_insn_csr (c, OP_SYSTEM, 0b010, rd, rs1, csr);
}

void
orc_riscv_insn_emit_csrrc (OrcCompiler *c, OrcRiscvRegister rd,
    OrcRiscvRegister rs1, int csr)
{
  ORC_ASM_CODE (c, "  csrrc %s, %d, %s\n", NAME (rd), csr, NAME (rs1));
  orc_riscv_insn_csr (c, OP_SYSTEM, 0b011, rd, rs1, csr);
}

static const char *
orc_riscv_lmul_str (OrcRiscvVtype vtype)
{
  switch (vtype.vlmul) {
    case ORC_RISCV_LMUL_1:
      return "m1";
    case ORC_RISCV_LMUL_2:
      return "m2";
    case ORC_RISCV_LMUL_4:
      return "m4";
    case ORC_RISCV_LMUL_8:
      return "m8";
    case ORC_RISCV_LMUL_F8:
      return "mf8";
    case ORC_RISCV_LMUL_F4:
      return "mf4";
    case ORC_RISCV_LMUL_F2:
      return "mf2";
    default:
      ORC_ASSERT ("Invalid LMUL");
      return NULL;              /* unreachable */
  }
}

void
orc_riscv_insn_emit_vsetvli (OrcCompiler *c,
    OrcRiscvRegister rd, OrcRiscvRegister rs1, OrcRiscvVtype vtype)
{
  ORC_ASSERT (vtype.vsew <= ORC_RISCV_SEW_64);
  ORC_ASM_CODE (c, "  vsetvli %s, %s, e%d, %s, %s, %s\n",
      NAME (rd),
      NAME (rs1),
      8 << vtype.vsew,
      orc_riscv_lmul_str (vtype),
      vtype.vta ? "ta" : "tu", vtype.vma ? "ma" : "mu");
  orc_uint8 type = *(orc_uint8 *) (&vtype);
  orc_riscv_insn_vsetvli (c, type, XREG (rs1), XREG (rd));
}

void
orc_riscv_insn_emit_vle8 (OrcCompiler *c, OrcRiscvRegister vd,
    OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vle8.v %s, (%s)\n", NAME (vd), NAME (rs1));
  orc_riscv_insn_vle (c, 0, 0, 0, 0, rs1, 0, vd, 1);
}

void
orc_riscv_insn_emit_vle16 (OrcCompiler *c, OrcRiscvRegister vd,
    OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vle16.v %s, (%s)\n", NAME (vd), NAME (rs1));
  orc_riscv_insn_vle (c, 0, 0, 0, 0, rs1, 0b0101, vd, 1);
}

void
orc_riscv_insn_emit_vle32 (OrcCompiler *c, OrcRiscvRegister vd,
    OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vle32.v %s, (%s)\n", NAME (vd), NAME (rs1));
  orc_riscv_insn_vle (c, 0, 0, 0, 0, rs1, 0b0110, vd, 1);
}

void
orc_riscv_insn_emit_vle64 (OrcCompiler *c, OrcRiscvRegister vd,
    OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vle64.v %s, (%s)\n", NAME (vd), NAME (rs1));
  orc_riscv_insn_vle (c, 0, 0, 0, 0, rs1, 0b0111, vd, 1);
}

void
orc_riscv_insn_emit_vse8 (OrcCompiler *c, OrcRiscvRegister vs3,
    OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vse8.v %s, (%s)\n", NAME (vs3), NAME (rs1));
  orc_riscv_insn_vse (c, 0, 0, 0, 0, rs1, 0, vs3, 1);
}

void
orc_riscv_insn_emit_vse16 (OrcCompiler *c,
    OrcRiscvRegister vs3, OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vse16.v %s, (%s)\n", NAME (vs3), NAME (rs1));
  orc_riscv_insn_vse (c, 0, 0, 0, 0, rs1, 0b0101, vs3, 1);
}

void
orc_riscv_insn_emit_vse32 (OrcCompiler *c,
    OrcRiscvRegister vs3, OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vse32.v %s, (%s)\n", NAME (vs3), NAME (rs1));
  orc_riscv_insn_vse (c, 0, 0, 0, 0, rs1, 0b0110, vs3, 1);
}

void
orc_riscv_insn_emit_vse64 (OrcCompiler *c,
    OrcRiscvRegister vs3, OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vse64.v %s, (%s)\n", NAME (vs3), NAME (rs1));
  orc_riscv_insn_vse (c, 0, 0, 0, 0, rs1, 0b0111, vs3, 1);
}

void
orc_riscv_insn_emit_vadd_vv (OrcCompiler *c,
    OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister vs1)
{
  ORC_ASM_CODE (c, "  vadd.vv %s, %s, %s\n", NAME (vd), NAME (vs2), NAME (vs1));
  orc_riscv_insn_vop (c, 0b000000, 1, VREG (vs2), VREG (vs1), OPIVV, VREG (vd));
}

void
orc_riscv_insn_emit_vadd_vvm (OrcCompiler *c,
    OrcRiscvRegister vd, OrcRiscvRegister vs1, OrcRiscvRegister vs2)
{
  ORC_ASM_CODE (c, "  vadd.vv %s, %s, %s, v0.t\n",
      NAME (vd), NAME (vs2), NAME (vs1));
  orc_riscv_insn_vop (c, 0b000000, 0, VREG (vs2), VREG (vs1), OPIVV, VREG (vd));
}

void
orc_riscv_insn_emit_vadd_vx (OrcCompiler *c,
    OrcRiscvRegister vd, OrcRiscvRegister vs2, OrcRiscvRegister rs1)
{
  ORC_ASM_CODE (c, "  vadd.vx %s, %s, %s\n", NAME (vd), NAME (vs2), NAME (rs1));
  orc_riscv_insn_vop (c, 0b000000, 1, VREG (vs2), XREG (rs1), OPIVX, VREG (vd));
}

void
orc_riscv_insn_emit_vadd_vi (OrcCompiler *c,
    OrcRiscvRegister vd, OrcRiscvRegister vs2, int imm)
{
  ORC_ASM_CODE (c, "  vadd.vi %s, %s, %d\n", NAME (vd), NAME (vs2), imm);
  /* TODO: emit machine code */
}

void
orc_riscv_insn_emit_shift_add (OrcCompiler *c, OrcRiscvRegister rd,
    orc_uint32 imm)
{
  const int top10 = (imm >> 22) & 0x3FF;
  const int mid11 = (imm >> 11) & 0x7FF;
  const int bot11 = imm & 0x7FF;

  int shifted = 0;

  if (top10 != 0) {
    orc_riscv_insn_emit_slli (c, rd, rd, 10);
    orc_riscv_insn_emit_addi (c, rd, rd, top10);
    shifted = 10;
  }

  if (mid11 != 0) {
    orc_riscv_insn_emit_slli (c, rd, rd, 21 - shifted);
    orc_riscv_insn_emit_addi (c, rd, rd, mid11);
    shifted = 21;
  }

  orc_riscv_insn_emit_slli (c, rd, rd, 32 - shifted);
  if (bot11 != 0) {
    orc_riscv_insn_emit_addi (c, rd, rd, bot11);
  }
}

void
orc_riscv_insn_emit_load_word (OrcCompiler *c, OrcRiscvRegister rd,
    orc_uint32 imm)
{
  const int lower = (imm & 0xFFF) | -(imm & 0x800);
  const int upper = ((imm >> 12) + (lower < 0)) & 0xFFFFF;

  if (upper != 0)
    orc_riscv_insn_emit_lui (c, rd, upper);

  if (lower != 0 || upper == 0)
    orc_riscv_insn_emit_addi (c, rd, upper ? rd : ORC_RISCV_ZERO, lower);
}

void
orc_riscv_insn_emit_load_immediate (OrcCompiler *c,
    OrcRiscvRegister rd, orc_uint64 imm)
{
  const uint32_t bit31 = (imm >> 31) & 1;
  const uint32_t lower = imm & 0xFFFFFFFFu;
  const uint32_t upper = sizeof (long) == 4 ? 0 : (imm >> 32);

  if ((upper ^ -bit31) == 0) {
    orc_riscv_insn_emit_load_word (c, rd, lower);
  } else if (lower == 0) {
    orc_riscv_insn_emit_load_word (c, rd, upper);
    orc_riscv_insn_emit_slli (c, rd, rd, 32);
  } else if (rd != c->gp_tmpreg) {
    orc_riscv_insn_emit_load_word (c, rd, (upper ^ -bit31));
    orc_riscv_insn_emit_slli (c, rd, rd, 32);
    orc_riscv_insn_emit_load_word (c, c->gp_tmpreg, lower);
    orc_riscv_insn_emit_xor (c, rd, rd, c->gp_tmpreg);
  } else {
    orc_riscv_insn_emit_load_word (c, rd, upper);
    orc_riscv_insn_emit_shift_add (c, rd, lower);
  }
}
